/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "FreqProxTermsWriterPerField.h"
#include "_FreqProxTermsWriterPerField.h"
#include "FreqProxTermsWriterPerField.h"
#include "_FreqProxTermsWriterPerField.h"
#include "FreqProxTermsWriter.h"
#include "FieldInfo.h"
#include "Fieldable.h"
#include "TermsHashPerField.h"
#include "FieldInvertState.h"
#include "AttributeSource.h"
#include "Payload.h"
#include "PayloadAttribute.h"
#include "DocumentsWriter.h"
#include "_DocumentsWriter.h"
#include "MiscUtils.h"

namespace Lucene
{
    FreqProxTermsWriterPerField::FreqProxTermsWriterPerField(TermsHashPerFieldPtr termsHashPerField, FreqProxTermsWriterPerThreadPtr perThread, FieldInfoPtr fieldInfo)
    {
        this->hasPayloads = false;
        this->_termsHashPerField = termsHashPerField;
        this->_perThread = perThread;
        this->fieldInfo = fieldInfo;
        docState = termsHashPerField->docState;
        fieldState = termsHashPerField->fieldState;
        omitTermFreqAndPositions = fieldInfo->omitTermFreqAndPositions;
    }
    
    FreqProxTermsWriterPerField::~FreqProxTermsWriterPerField()
    {
    }
    
    int32_t FreqProxTermsWriterPerField::getStreamCount()
    {
        return fieldInfo->omitTermFreqAndPositions ? 1 : 2;
    }
    
    void FreqProxTermsWriterPerField::finish()
    {
    }
    
    void FreqProxTermsWriterPerField::skippingLongTerm()
    {
    }
    
    int32_t FreqProxTermsWriterPerField::compareTo(LuceneObjectPtr other)
    {
        return fieldInfo->name.compare(boost::static_pointer_cast<FreqProxTermsWriterPerField>(other)->fieldInfo->name);
    }
    
    void FreqProxTermsWriterPerField::reset()
    {
        // Record, up front, whether our in-RAM format will be with or without term freqs
        omitTermFreqAndPositions = fieldInfo->omitTermFreqAndPositions;
        payloadAttribute.reset();
    }
    
    bool FreqProxTermsWriterPerField::start(Collection<FieldablePtr> fields, int32_t count)
    {
        for (int32_t i = 0; i < count; ++i)
        {
            if (fields[i]->isIndexed())
                return true;
        }
        return false;
    }
    
    void FreqProxTermsWriterPerField::start(FieldablePtr field)
    {
        if (fieldState->attributeSource->hasAttribute<PayloadAttribute>())
            payloadAttribute = fieldState->attributeSource->getAttribute<PayloadAttribute>();
        else
            payloadAttribute.reset();
    }
    
    void FreqProxTermsWriterPerField::writeProx(int32_t termID, int32_t proxCode)
    {
        PayloadPtr payload;
        if (payloadAttribute)
            payload = payloadAttribute->getPayload();
        
        TermsHashPerFieldPtr termsHashPerField(_termsHashPerField);
        
        if (payload && payload->length() > 0)
        {
            termsHashPerField->writeVInt(1, (proxCode << 1) | 1);
            termsHashPerField->writeVInt(1, payload->length());
            termsHashPerField->writeBytes(1, payload->getData().get(), payload->getOffset(), payload->length());
            hasPayloads = true;
        }
        else
            termsHashPerField->writeVInt(1, proxCode << 1);
        FreqProxPostingsArrayPtr postings(boost::static_pointer_cast<FreqProxPostingsArray>(termsHashPerField->postingsArray));
        postings->lastPositions[termID] = fieldState->position;
    }
    
    void FreqProxTermsWriterPerField::newTerm(int32_t termID)
    {
        // First time we're seeing this term since the last flush
        BOOST_ASSERT(docState->testPoint(L"FreqProxTermsWriterPerField.newTerm start"));

        TermsHashPerFieldPtr termsHashPerField(_termsHashPerField);
        FreqProxPostingsArrayPtr postings(boost::static_pointer_cast<FreqProxPostingsArray>(termsHashPerField->postingsArray));
        postings->lastDocIDs[termID] = docState->docID;
        if (omitTermFreqAndPositions)
            postings->lastDocCodes[termID] = docState->docID;
        else
        {
            postings->lastDocCodes[termID] = docState->docID << 1;
            postings->docFreqs[termID] = 1;
            writeProx(termID, fieldState->position);
        }
        fieldState->maxTermFrequency = std::max(1, fieldState->maxTermFrequency);
    }
    
    void FreqProxTermsWriterPerField::addTerm(int32_t termID)
    {
        BOOST_ASSERT(docState->testPoint(L"FreqProxTermsWriterPerField.addTerm start"));
        
        TermsHashPerFieldPtr termsHashPerField(_termsHashPerField);
        FreqProxPostingsArrayPtr postings(boost::static_pointer_cast<FreqProxPostingsArray>(termsHashPerField->postingsArray));

        BOOST_ASSERT(omitTermFreqAndPositions || postings->docFreqs[termID] > 0);

        if (omitTermFreqAndPositions)
        {
            if (docState->docID != postings->lastDocIDs[termID])
            {
                BOOST_ASSERT(docState->docID > postings->lastDocIDs[termID]);
                termsHashPerField->writeVInt(0, postings->lastDocCodes[termID]);
                postings->lastDocCodes[termID] = docState->docID - postings->lastDocIDs[termID];
                postings->lastDocIDs[termID] = docState->docID;
            }
        }
        else
        {
            if (docState->docID != postings->lastDocIDs[termID])
            {
                BOOST_ASSERT(docState->docID > postings->lastDocIDs[termID]);
                // Term not yet seen in the current doc but previously seen in other doc(s) since the last flush

                // Now that we know doc freq for previous doc, write it and lastDocCode
                if (1 == postings->docFreqs[termID])
                    termsHashPerField->writeVInt(0, postings->lastDocCodes[termID]|1);
                else
                {
                    termsHashPerField->writeVInt(0, postings->lastDocCodes[termID]);
                    termsHashPerField->writeVInt(0, postings->docFreqs[termID]);
                }
                postings->docFreqs[termID] = 1;
                fieldState->maxTermFrequency = std::max(1, fieldState->maxTermFrequency);
                postings->lastDocCodes[termID] = ((docState->docID - postings->lastDocIDs[termID]) << 1);
                postings->lastDocIDs[termID] = docState->docID;
                writeProx(termID, fieldState->position);
            }
            else
            {
                fieldState->maxTermFrequency = std::max(fieldState->maxTermFrequency, ++postings->docFreqs[termID]);
                writeProx(termID, fieldState->position - postings->lastPositions[termID]);
            }
        }
    }
    
    ParallelPostingsArrayPtr FreqProxTermsWriterPerField::createPostingsArray(int32_t size)
    {
        return newLucene<FreqProxPostingsArray>(size);
    }
    
    void FreqProxTermsWriterPerField::abort()
    {
    }
    
    FreqProxPostingsArray::FreqProxPostingsArray(int32_t size) : ParallelPostingsArray(size)
    {
        docFreqs = IntArray::newInstance(size);
        lastDocIDs = IntArray::newInstance(size);
        lastDocCodes = IntArray::newInstance(size);
        lastPositions = IntArray::newInstance(size);
    }
    
    FreqProxPostingsArray::~FreqProxPostingsArray()
    {
    }
    
    ParallelPostingsArrayPtr FreqProxPostingsArray::newInstance(int32_t size)
    {
        return newLucene<FreqProxPostingsArray>(size);
    }
    
    void FreqProxPostingsArray::copyTo(ParallelPostingsArrayPtr toArray, int32_t numToCopy)
    {
        BOOST_ASSERT(MiscUtils::typeOf<FreqProxPostingsArray>(toArray));
        FreqProxPostingsArrayPtr to(boost::static_pointer_cast<FreqProxPostingsArray>(toArray));

        ParallelPostingsArray::copyTo(toArray, numToCopy);

        MiscUtils::arrayCopy(docFreqs.get(), 0, to->docFreqs.get(), 0, numToCopy);
        MiscUtils::arrayCopy(lastDocIDs.get(), 0, to->lastDocIDs.get(), 0, numToCopy);
        MiscUtils::arrayCopy(lastDocCodes.get(), 0, to->lastDocCodes.get(), 0, numToCopy);
        MiscUtils::arrayCopy(lastPositions.get(), 0, to->lastPositions.get(), 0, numToCopy);
    }
}
