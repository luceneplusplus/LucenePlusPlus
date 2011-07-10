/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TermVectorsTermsWriterPerField.h"
#include "_TermVectorsTermsWriterPerField.h"
#include "TermVectorsTermsWriterPerThread.h"
#include "TermVectorsTermsWriter.h"
#include "_TermVectorsTermsWriter.h"
#include "TermsHashPerField.h"
#include "TermsHashPerThread.h"
#include "TermVectorsReader.h"
#include "Fieldable.h"
#include "FieldInfo.h"
#include "FieldInvertState.h"
#include "RAMOutputStream.h"
#include "ByteSliceReader.h"
#include "CharBlockPool.h"
#include "OffsetAttribute.h"
#include "AttributeSource.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"
#include "StringUtils.h"
#include "DocumentsWriter.h"

namespace Lucene
{
    TermVectorsTermsWriterPerField::TermVectorsTermsWriterPerField(TermsHashPerFieldPtr termsHashPerField, TermVectorsTermsWriterPerThreadPtr perThread, FieldInfoPtr fieldInfo)
    {
        this->doVectors = false;
        this->doVectorPositions = false;
        this->doVectorOffsets = false;
        this->maxNumPostings = 0;

        this->_termsHashPerField = termsHashPerField;
        this->_perThread = perThread;
        this->_termsWriter = perThread->_termsWriter;
        this->fieldInfo = fieldInfo;
        _docState = termsHashPerField->docState;
        _fieldState = termsHashPerField->fieldState;
    }

    TermVectorsTermsWriterPerField::~TermVectorsTermsWriterPerField()
    {
    }

    int32_t TermVectorsTermsWriterPerField::getStreamCount()
    {
        return 2;
    }

    bool TermVectorsTermsWriterPerField::start(Collection<FieldablePtr> fields, int32_t count)
    {
        doVectors = false;
        doVectorPositions = false;
        doVectorOffsets = false;

        for (int32_t i = 0; i < count; ++i)
        {
            FieldablePtr field(fields[i]);
            if (field->isIndexed() && field->isTermVectorStored())
            {
                doVectors = true;
                if (field->isStorePositionWithTermVector())
                    doVectorPositions = true;
                if (field->isStoreOffsetWithTermVector())
                    doVectorOffsets = true;
            }
        }

        if (doVectors)
        {
            TermVectorsTermsWriterPerThreadPtr perThread(_perThread);
            DocStatePtr docState(_docState);
            if (!perThread->doc)
            {
                perThread->doc = TermVectorsTermsWriterPtr(_termsWriter)->getPerDoc();
                perThread->doc->docID = docState->docID;
                BOOST_ASSERT(perThread->doc->numVectorFields == 0);
                BOOST_ASSERT(perThread->doc->perDocTvf->length() == 0);
                BOOST_ASSERT(perThread->doc->perDocTvf->getFilePointer() == 0);
            }

            BOOST_ASSERT(perThread->doc->docID == docState->docID);

            TermsHashPerFieldPtr termsHashPerField(_termsHashPerField);

            if (termsHashPerField->numPostings != 0)
            {
                // Only necessary if previous doc hit a non-aborting exception while writing vectors
                // in this field
                termsHashPerField->reset();
                TermsHashPerThreadPtr(perThread->_termsHashPerThread)->reset(false);
            }
        }

        return doVectors;
    }

    void TermVectorsTermsWriterPerField::abort()
    {
    }

    void TermVectorsTermsWriterPerField::finish()
    {
        BOOST_ASSERT(DocStatePtr(_docState)->testPoint(L"TermVectorsTermsWriterPerField.finish start"));

        TermsHashPerFieldPtr termsHashPerField(_termsHashPerField);
        int32_t numPostings = termsHashPerField->numPostings;

        BOOST_ASSERT(numPostings >= 0);

        if (!doVectors || numPostings == 0)
            return;

        if (numPostings > maxNumPostings)
            maxNumPostings = numPostings;

        TermVectorsTermsWriterPerThreadPtr perThread(_perThread);
        IndexOutputPtr tvf(perThread->doc->perDocTvf);

        // This is called once, after inverting all occurrences of a given field in the doc.  At this point we flush
        // our hash into the DocWriter.

        BOOST_ASSERT(fieldInfo->storeTermVector);
        BOOST_ASSERT(perThread->vectorFieldsInOrder(fieldInfo));

        perThread->doc->addField(termsHashPerField->fieldInfo->number);
        TermVectorsPostingsArrayPtr postings(boost::static_pointer_cast<TermVectorsPostingsArray>(termsHashPerField->postingsArray));

        IntArray termIDs(termsHashPerField->sortPostings());

        tvf->writeVInt(numPostings);
        uint8_t bits = 0x0;
        if (doVectorPositions)
            bits |= TermVectorsReader::STORE_POSITIONS_WITH_TERMVECTOR;
        if (doVectorOffsets)
            bits |= TermVectorsReader::STORE_OFFSET_WITH_TERMVECTOR;
        tvf->writeByte(bits);

        int32_t encoderUpto = 0;
        int32_t lastTermBytesCount = 0;

        ByteSliceReaderPtr reader(perThread->vectorSliceReader);
        Collection<CharArray> charBuffers(TermsHashPerThreadPtr(perThread->_termsHashPerThread)->charPool->buffers);

        for (int32_t j = 0; j < numPostings; ++j)
        {
            int32_t termID = termIDs[j];
            int32_t freq = postings->freqs[termID];

            CharArray text2(charBuffers[postings->textStarts[termID] >> DocumentsWriter::CHAR_BLOCK_SHIFT]);
            int32_t start2 = (postings->textStarts[termID] & DocumentsWriter::CHAR_BLOCK_MASK);

            // We swap between two encoders to save copying last Term's byte array
            UTF8ResultPtr utf8Result(perThread->utf8Results[encoderUpto]);

            StringUtils::toUTF8(text2.get() + start2, text2.size(), utf8Result);
            int32_t termBytesCount = utf8Result->length;

            // Compute common prefix between last term and this term
            int32_t prefix = 0;
            if (j > 0)
            {
                ByteArray lastTermBytes(perThread->utf8Results[1 - encoderUpto]->result);
                ByteArray termBytes(perThread->utf8Results[encoderUpto]->result);
                while (prefix < lastTermBytesCount && prefix < termBytesCount)
                {
                    if (lastTermBytes[prefix] != termBytes[prefix])
                        break;
                    ++prefix;
                }
            }
            encoderUpto = 1 - encoderUpto;
            lastTermBytesCount = termBytesCount;

            int32_t suffix = termBytesCount - prefix;
            tvf->writeVInt(prefix);
            tvf->writeVInt(suffix);
            tvf->writeBytes(utf8Result->result.get(), prefix, suffix);
            tvf->writeVInt(freq);

            if (doVectorPositions)
            {
                termsHashPerField->initReader(reader, termID, 0);
                reader->writeTo(tvf);
            }

            if (doVectorOffsets)
            {
                termsHashPerField->initReader(reader, termID, 1);
                reader->writeTo(tvf);
            }
        }

        termsHashPerField->reset();

        // NOTE: we clear per-field at the thread level, because term vectors fully write themselves on each
        // field; this saves RAM (eg if large doc has two large fields with term vectors on) because we
        // recycle/reuse all RAM after each field
        TermsHashPerThreadPtr(perThread->_termsHashPerThread)->reset(false);
    }

    void TermVectorsTermsWriterPerField::shrinkHash()
    {
        TermsHashPerFieldPtr(_termsHashPerField)->shrinkHash(maxNumPostings);
        maxNumPostings = 0;
    }

    void TermVectorsTermsWriterPerField::start(FieldablePtr field)
    {
        if (doVectorOffsets)
            offsetAttribute = FieldInvertStatePtr(_fieldState)->attributeSource->addAttribute<OffsetAttribute>();
        else
            offsetAttribute.reset();
    }

    void TermVectorsTermsWriterPerField::newTerm(int32_t termID)
    {
        BOOST_ASSERT(DocStatePtr(_docState)->testPoint(L"TermVectorsTermsWriterPerField.newTerm start"));

        TermsHashPerFieldPtr termsHashPerField(_termsHashPerField);
        TermVectorsPostingsArrayPtr postings(boost::static_pointer_cast<TermVectorsPostingsArray>(termsHashPerField->postingsArray));

        postings->freqs[termID] = 1;

        FieldInvertStatePtr fieldState(_fieldState);

        if (doVectorOffsets)
        {
            int32_t startOffset = fieldState->offset + offsetAttribute->startOffset();
            int32_t endOffset = fieldState->offset + offsetAttribute->endOffset();

            termsHashPerField->writeVInt(1, startOffset);
            termsHashPerField->writeVInt(1, endOffset - startOffset);
            postings->lastOffsets[termID] = endOffset;
        }

        if (doVectorPositions)
        {
            termsHashPerField->writeVInt(0, fieldState->position);
            postings->lastPositions[termID] = fieldState->position;
        }
    }

    void TermVectorsTermsWriterPerField::addTerm(int32_t termID)
    {
        BOOST_ASSERT(DocStatePtr(_docState)->testPoint(L"TermVectorsTermsWriterPerField.newTerm start"));

        TermsHashPerFieldPtr termsHashPerField(_termsHashPerField);
        TermVectorsPostingsArrayPtr postings(boost::static_pointer_cast<TermVectorsPostingsArray>(termsHashPerField->postingsArray));

        ++postings->freqs[termID];

        FieldInvertStatePtr fieldState(_fieldState);

        if (doVectorOffsets)
        {
            int32_t startOffset = fieldState->offset + offsetAttribute->startOffset();
            int32_t endOffset = fieldState->offset + offsetAttribute->endOffset();

            termsHashPerField->writeVInt(1, startOffset - postings->lastOffsets[termID]);
            termsHashPerField->writeVInt(1, endOffset - startOffset);
            postings->lastOffsets[termID] = endOffset;
        }

        if (doVectorPositions)
        {
            termsHashPerField->writeVInt(0, fieldState->position - postings->lastPositions[termID]);
            postings->lastPositions[termID] = fieldState->position;
        }
    }

    void TermVectorsTermsWriterPerField::skippingLongTerm()
    {
    }

    ParallelPostingsArrayPtr TermVectorsTermsWriterPerField::createPostingsArray(int32_t size)
    {
        return newLucene<TermVectorsPostingsArray>(size);
    }

    TermVectorsPostingsArray::TermVectorsPostingsArray(int32_t size) : ParallelPostingsArray(size)
    {
        freqs = IntArray::newInstance(size);
        lastOffsets = IntArray::newInstance(size);
        lastPositions = IntArray::newInstance(size);
    }

    TermVectorsPostingsArray::~TermVectorsPostingsArray()
    {
    }

    int32_t TermVectorsPostingsArray::bytesPerPosting()
    {
        return ParallelPostingsArray::bytesPerPosting() + (3 * sizeof(int32_t));
    }

    ParallelPostingsArrayPtr TermVectorsPostingsArray::newInstance(int32_t size)
    {
        return newLucene<TermVectorsPostingsArray>(size);
    }

    void TermVectorsPostingsArray::copyTo(ParallelPostingsArrayPtr toArray, int32_t numToCopy)
    {
        BOOST_ASSERT(MiscUtils::typeOf<TermVectorsPostingsArray>(toArray));
        TermVectorsPostingsArrayPtr to(boost::static_pointer_cast<TermVectorsPostingsArray>(toArray));

        ParallelPostingsArray::copyTo(toArray, numToCopy);

        MiscUtils::arrayCopy(freqs.get(), 0, to->freqs.get(), 0, size);
        MiscUtils::arrayCopy(lastOffsets.get(), 0, to->lastOffsets.get(), 0, size);
        MiscUtils::arrayCopy(lastPositions.get(), 0, to->lastPositions.get(), 0, size);
    }
}

