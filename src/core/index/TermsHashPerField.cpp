/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TermsHashPerField.h"
#include "TermsHashPerThread.h"
#include "TermsHashConsumerPerThread.h"
#include "TermsHashConsumerPerField.h"
#include "TermsHash.h"
#include "CharTermAttribute.h"
#include "AttributeSource.h"
#include "DocInverterPerField.h"
#include "DocumentsWriter.h"
#include "_DocumentsWriter.h"
#include "IntBlockPool.h"
#include "CharBlockPool.h"
#include "ByteSliceReader.h"
#include "FieldInvertState.h"
#include "UTF8Stream.h"
#include "MiscUtils.h"
#include "ParallelPostingsArray.h"
#include "ByteBlockPool.h"

namespace Lucene
{
    TermsHashPerField::TermsHashPerField(DocInverterPerFieldPtr docInverterPerField, TermsHashPerThreadPtr perThread, TermsHashPerThreadPtr nextPerThread, FieldInfoPtr fieldInfo)
    {
        this->_docInverterPerField = docInverterPerField;
        this->_perThread = perThread;
        this->nextPerThread = nextPerThread;
        this->fieldInfo = fieldInfo;
    }
    
    TermsHashPerField::~TermsHashPerField()
    {
    }
    
    void TermsHashPerField::initialize()
    {
        this->postingsCompacted = false;
        this->numPostings = 0;
        this->postingsHashSize = 4;
        this->postingsHashHalfSize = this->postingsHashSize / 2;
        this->postingsHashMask = this->postingsHashSize - 1;
        this->doCall = false;
        this->doNextCall = false;
        this->intUptoStart = 0;
        
        TermsHashPerThreadPtr perThread(_perThread);
        intPool = perThread->intPool;
        charPool = perThread->charPool;
        bytePool = perThread->bytePool;
        docState = perThread->docState;

        postingsHash = IntArray::newInstance(postingsHashSize);
        MiscUtils::arrayFill(postingsHash.get(), 0, postingsHash.size(), -1);
        bytesUsed(postingsHashSize * sizeof(int32_t));
        
        DocInverterPerFieldPtr docInverterPerField(_docInverterPerField);
        fieldState = docInverterPerField->fieldState;
        this->consumer = perThread->consumer->addField(shared_from_this(), fieldInfo);
        initPostingsArray();
        
        streamCount = consumer->getStreamCount();
        numPostingInt = 2 * streamCount;
        if (nextPerThread)
            nextPerField = boost::static_pointer_cast<TermsHashPerField>(nextPerThread->addField(docInverterPerField, fieldInfo));
    }
    
    void TermsHashPerField::initPostingsArray()
    {
        postingsArray = consumer->createPostingsArray(2);
        bytesUsed(postingsArray->size * postingsArray->bytesPerPosting());
    }
    
    void TermsHashPerField::bytesUsed(int64_t size)
    {
        TermsHashPtr termsHash(TermsHashPerThreadPtr(_perThread)->_termsHash);
        if (termsHash->trackAllocations)
            DocumentsWriterPtr(termsHash->_docWriter)->bytesUsed(size);
    }
    
    void TermsHashPerField::shrinkHash(int32_t targetSize)
    {
        BOOST_ASSERT(postingsCompacted || numPostings == 0);
        
        int32_t newSize = 4;
        if (newSize != postingsHash.size())
        {
            int64_t previousSize = postingsHash.size();
            postingsHash.resize(newSize);
            bytesUsed((newSize - previousSize) * sizeof(int32_t));
            MiscUtils::arrayFill(postingsHash.get(), 0, postingsHash.size(), -1);
            postingsHashSize = newSize;
            postingsHashHalfSize = newSize / 2;
            postingsHashMask = newSize - 1;
        }
        
        // Fully free the postings array on each flush
        if (postingsArray)
        {
            bytesUsed(-postingsArray->bytesPerPosting() * postingsArray->size);
            postingsArray.reset();
        }
    }
    
    void TermsHashPerField::reset()
    {
        if (!postingsCompacted)
            compactPostings();
        BOOST_ASSERT(numPostings <= postingsHash.size());
        if (numPostings > 0)
        {
            MiscUtils::arrayFill(postingsHash.get(), 0, postingsHash.size(), -1);
            numPostings = 0;
        }
        postingsCompacted = false;
        if (nextPerField)
            nextPerField->reset();
    }
    
    void TermsHashPerField::abort()
    {
        SyncLock syncLock(this);
        reset();
        if (nextPerField)
            nextPerField->abort();
    }
    
    void TermsHashPerField::growParallelPostingsArray()
    {
        int32_t oldSize = postingsArray->size;
        this->postingsArray = this->postingsArray->grow();
        bytesUsed(postingsArray->bytesPerPosting() * (postingsArray->size - oldSize));
    }
    
    void TermsHashPerField::initReader(ByteSliceReaderPtr reader, int32_t termID, int32_t stream)
    {
        BOOST_ASSERT(stream < streamCount);
        int32_t intStart = postingsArray->intStarts[termID];
        IntArray ints = (intPool->buffers[intStart >> DocumentsWriter::INT_BLOCK_SHIFT]);
        int32_t upto = (intStart & DocumentsWriter::INT_BLOCK_MASK);
        reader->init(bytePool, postingsArray->byteStarts[termID] + stream * ByteBlockPool::FIRST_LEVEL_SIZE(), ints[upto + stream]);
    }
    
    void TermsHashPerField::compactPostings()
    {
        int32_t upto = 0;
        for (int32_t i = 0; i < postingsHashSize; ++i)
        {
            if (postingsHash[i] != -1)
            {
                if (upto < i)
                {
                    postingsHash[upto] = postingsHash[i];
                    postingsHash[i] = -1;
                }
                ++upto;
            }
        }

        BOOST_ASSERT(upto == numPostings);
        postingsCompacted = true;
    }
    
    struct comparePostings
    {
        comparePostings(ParallelPostingsArrayPtr postingsArray, Collection<CharArray> buffers)
        {
            this->postingsArray = postingsArray;
            this->buffers = buffers;
        }
        
        inline bool operator()(const int32_t& first, const int32_t& second) const
        {
            if (first == second)
                return false;
                
            int32_t textStart1 = postingsArray->textStarts[first];
            int32_t textStart2 = postingsArray->textStarts[second];
            wchar_t* text1 = buffers[textStart1 >> DocumentsWriter::CHAR_BLOCK_SHIFT].get();
            int32_t pos1 = (textStart1 & DocumentsWriter::CHAR_BLOCK_MASK);
            wchar_t* text2 = buffers[textStart2 >> DocumentsWriter::CHAR_BLOCK_SHIFT].get();
            int32_t pos2 = (textStart2 & DocumentsWriter::CHAR_BLOCK_MASK);
        
            BOOST_ASSERT(text1 != text2 || pos1 != pos2);
            
            while (true)
            {
                wchar_t c1 = text1[pos1++];
                wchar_t c2 = text2[pos2++];
                if (c1 != c2)
                {
                    if (c2 == UTF8Base::UNICODE_TERMINATOR)
                        return false;
                    else if (c1 == UTF8Base::UNICODE_TERMINATOR)
                        return true;
                    else
                        return (c1 < c2);
                }
                else
                {
                    // This method should never compare equal postings unless first == second
                    BOOST_ASSERT(c1 != UTF8Base::UNICODE_TERMINATOR);
                }
            }
        }
        
        ParallelPostingsArrayPtr postingsArray;
        Collection<CharArray> buffers;
    };
    
    IntArray TermsHashPerField::sortPostings()
    {
        compactPostings();
        std::sort(postingsHash.get(), postingsHash.get() + numPostings, comparePostings(postingsArray, charPool->buffers));
        return postingsHash;
    }
    
    bool TermsHashPerField::postingEquals(int32_t termID, const wchar_t* tokenText, int32_t tokenTextLen)
    {
        int32_t textStart = postingsArray->textStarts[termID];

        wchar_t* text = TermsHashPerThreadPtr(_perThread)->charPool->buffers[textStart >> DocumentsWriter::CHAR_BLOCK_SHIFT].get();
        BOOST_ASSERT(text);
        int32_t pos = (textStart & DocumentsWriter::CHAR_BLOCK_MASK);
        int32_t tokenPos = 0;
        for (; tokenPos < tokenTextLen; ++pos, ++tokenPos)
        {
            if (tokenText[tokenPos] != text[pos])
                return false;
        }
        return (text[pos] == UTF8Base::UNICODE_TERMINATOR);
    }
    
    void TermsHashPerField::start(FieldablePtr field)
    {
        termAtt = fieldState->attributeSource->addAttribute<CharTermAttribute>();
        consumer->start(field);
        if (nextPerField)
            nextPerField->start(field);
    }
    
    bool TermsHashPerField::start(Collection<FieldablePtr> fields, int32_t count)
    {
        doCall = consumer->start(fields, count);
        if (!postingsArray)
            initPostingsArray();
        if (nextPerField)
            doNextCall = nextPerField->start(fields, count);
        return (doCall || doNextCall);
    }
    
    void TermsHashPerField::add(int32_t textStart)
    {
        // Secondary entry point (for 2nd and subsequent TermsHash), we hash by textStart
        int32_t code = textStart;
        
        int32_t hashPos = (code & postingsHashMask);
        
        BOOST_ASSERT(!postingsCompacted);
        
        // Locate RawPostingList in hash
        int32_t termID = postingsHash[hashPos];
        
        if (termID != -1 && postingsArray->textStarts[termID] != textStart)
        {
            // Conflict: keep searching different locations in the hash table.
            int32_t inc = (((code >> 8) + code) | 1);
            do
            {
                code += inc;
                hashPos = (code & postingsHashMask);
                termID = postingsHash[hashPos];
            }
            while (termID != -1 && postingsArray->textStarts[termID] != textStart);
        }
        
        if (termID == -1)
        {
            // First time we are seeing this token since we last flushed the hash.
            TermsHashPerThreadPtr perThread(_perThread);
            
            // New posting
            termID = numPostings++;
            if (termID >= postingsArray->size)
                growParallelPostingsArray();

            BOOST_ASSERT(termID >= 0);

            postingsArray->textStarts[termID] = textStart;

            BOOST_ASSERT(postingsHash[hashPos] == -1);
            postingsHash[hashPos] = termID;
            
            if (numPostings == postingsHashHalfSize)
                rehashPostings(2 * postingsHashSize);
            
            // Init stream slices
            if (numPostingInt + intPool->intUpto > DocumentsWriter::INT_BLOCK_SIZE)
                intPool->nextBuffer();
            
            if (DocumentsWriter::BYTE_BLOCK_SIZE - bytePool->byteUpto < numPostingInt * ByteBlockPool::FIRST_LEVEL_SIZE())
                bytePool->nextBuffer();
            
            intUptos = intPool->buffer;
            intUptoStart = intPool->intUpto;
            intPool->intUpto += streamCount;
            
            postingsArray->intStarts[termID] = intUptoStart + intPool->intOffset;
            
            for (int32_t i = 0; i < streamCount; ++i)
            {
                int32_t upto = bytePool->newSlice(ByteBlockPool::FIRST_LEVEL_SIZE());
                intUptos[intUptoStart + i] = upto + bytePool->byteOffset;
            }
            postingsArray->byteStarts[termID] = intUptos[intUptoStart];
            
            consumer->newTerm(termID);
        }
        else
        {
            int32_t intStart = postingsArray->intStarts[termID];
            intUptos = intPool->buffers[intStart >> DocumentsWriter::INT_BLOCK_SHIFT];
            intUptoStart = (intStart & DocumentsWriter::INT_BLOCK_MASK);
            consumer->addTerm(termID);
        }
    }
    
    void TermsHashPerField::add()
    {
        BOOST_ASSERT(!postingsCompacted);
        
        // Get the text of this term.
        wchar_t* tokenText = termAtt->bufferArray();
        int32_t tokenTextLen = termAtt->length();
        
        // Compute hashcode and replace any invalid UTF16 sequences
        int32_t downto = tokenTextLen;
        int32_t code = 0;
        
        while (downto > 0)
        {
            wchar_t ch = tokenText[--downto];
            
            #ifdef LPP_UNICODE_CHAR_SIZE_2
            if (ch >= UTF8Base::TRAIL_SURROGATE_MIN && ch <= UTF8Base::TRAIL_SURROGATE_MAX)
            {
                if (downto == 0)
                {
                    // Unpaired
                    ch = UTF8Base::UNICODE_REPLACEMENT_CHAR;
                    tokenText[downto] = ch;
                }
                else
                {
                    wchar_t ch2 = tokenText[downto - 1];
                    if (ch2 >= UTF8Base::LEAD_SURROGATE_MIN && ch2 <= UTF8Base::LEAD_SURROGATE_MAX)
                    {
                        // OK: high followed by low.  This is a valid surrogate pair.
                        code = ((code * 31) + ch) * 31 + ch2;
                        --downto;
                        continue;
                    }
                    else
                    {
                        // Unpaired
                        ch = UTF8Base::UNICODE_REPLACEMENT_CHAR;
                        tokenText[downto] = ch;
                    }
                }
            }
            else if (ch >= UTF8Base::LEAD_SURROGATE_MIN && (ch <= UTF8Base::LEAD_SURROGATE_MAX || ch == UTF8Base::UNICODE_TERMINATOR))
            {
                // Unpaired or UTF8Base::UNICODE_TERMINATOR
                ch = UTF8Base::UNICODE_REPLACEMENT_CHAR;
                tokenText[downto] = ch;
            }
            #else
            if (ch == UTF8Base::UNICODE_TERMINATOR)
            {
                // Unpaired or UTF8Base::UNICODE_TERMINATOR
                ch = UTF8Base::UNICODE_REPLACEMENT_CHAR;
                tokenText[downto] = ch;
            }
            #endif
            
            code = (code * 31) + ch;
        }
        
        int32_t hashPos = (code & postingsHashMask);
        
        // Locate RawPostingList in hash
        int32_t termID = postingsHash[hashPos];
        
        if (termID != -1 && !postingEquals(termID, tokenText, tokenTextLen))
        {
            // Conflict: keep searching different locations in the hash table.
            int32_t inc = (((code >> 8) + code) | 1);
            do
            {
                code += inc;
                hashPos = (code & postingsHashMask);
                termID = postingsHash[hashPos];
            }
            while (termID != -1 && !postingEquals(termID, tokenText, tokenTextLen));
        }
            
        if (termID == -1)
        {
            // First time we are seeing this token since we last flushed the hash.
            int32_t textLen1 = 1 + tokenTextLen;
            if (textLen1 + charPool->charUpto > DocumentsWriter::CHAR_BLOCK_SIZE)
            {
                if (textLen1 > DocumentsWriter::CHAR_BLOCK_SIZE)
                {
                    // Just skip this term, to remain as robust as possible during indexing.  A TokenFilter
                    // can be inserted into the analyzer chain if other behavior is wanted (pruning the term
                    // to a prefix, throwing an exception, etc).
                    
                    if (docState->maxTermPrefix.empty())
                        docState->maxTermPrefix.append(tokenText, std::min((int32_t)30, tokenTextLen));
                    
                    consumer->skippingLongTerm();
                    return;
                }
                charPool->nextBuffer();
            }
            
            TermsHashPerThreadPtr perThread(_perThread);
            
            // New posting
            termID = numPostings++;
            if (termID >= postingsArray->size)
                growParallelPostingsArray();

            BOOST_ASSERT(termID != -1);
            
            wchar_t* text = charPool->buffer.get();
            int32_t textUpto = charPool->charUpto;
            
            postingsArray->textStarts[termID] = textUpto + charPool->charOffset;
            charPool->charUpto += textLen1;
            
            MiscUtils::arrayCopy(tokenText, 0, text, textUpto, tokenTextLen);
            text[textUpto + tokenTextLen] = UTF8Base::UNICODE_TERMINATOR;
            
            BOOST_ASSERT(postingsHash[hashPos] == -1);
            postingsHash[hashPos] = termID;
            
            if (numPostings == postingsHashHalfSize)
            {
                rehashPostings(2 * postingsHashSize);
                bytesUsed(2 * numPostings * sizeof(int32_t));
            }
            
            // Init stream slices
            if (numPostingInt + intPool->intUpto > DocumentsWriter::INT_BLOCK_SIZE)
                intPool->nextBuffer();
            
            if (DocumentsWriter::BYTE_BLOCK_SIZE - bytePool->byteUpto < numPostingInt * ByteBlockPool::FIRST_LEVEL_SIZE())
                bytePool->nextBuffer();
            
            intUptos = intPool->buffer;
            intUptoStart = intPool->intUpto;
            intPool->intUpto += streamCount;
            
            postingsArray->intStarts[termID] = intUptoStart + intPool->intOffset;
            
            for (int32_t i = 0; i < streamCount; ++i)
            {
                int32_t upto = bytePool->newSlice(ByteBlockPool::FIRST_LEVEL_SIZE());
                intUptos[intUptoStart + i] = upto + bytePool->byteOffset;
            }
            postingsArray->byteStarts[termID] = intUptos[intUptoStart];
            
            consumer->newTerm(termID);
        }
        else
        {
            int32_t intStart = postingsArray->intStarts[termID];
            intUptos = intPool->buffers[intStart >> DocumentsWriter::INT_BLOCK_SHIFT];
            intUptoStart = (intStart & DocumentsWriter::INT_BLOCK_MASK);
            consumer->addTerm(termID);
        }
        
        if (doNextCall)
            nextPerField->add(postingsArray->textStarts[termID]);
    }
    
    void TermsHashPerField::writeByte(int32_t stream, int8_t b)
    {
        int32_t upto = intUptos[intUptoStart + stream];
        ByteArray bytes(bytePool->buffers[upto >> DocumentsWriter::BYTE_BLOCK_SHIFT]);
        BOOST_ASSERT(bytes);
        int32_t offset = (upto & DocumentsWriter::BYTE_BLOCK_MASK);
        if (bytes[offset] != 0)
        {
            // End of slice; allocate a new one
            offset = bytePool->allocSlice(bytes, offset);
            bytes = bytePool->buffer;
            intUptos[intUptoStart + stream] = offset + bytePool->byteOffset;
        }
        bytes[offset] = b;
        intUptos[intUptoStart + stream]++;
    }
    
    void TermsHashPerField::writeBytes(int32_t stream, const uint8_t* b, int32_t offset, int32_t length)
    {
        int32_t end = offset + length;
        for (int32_t i = offset; i < end; ++i)
            writeByte(stream, b[i]);
    }
    
    void TermsHashPerField::writeVInt(int32_t stream, int32_t i)
    {
        BOOST_ASSERT(stream < streamCount);
        while ((i & ~0x7f) != 0)
        {
            writeByte(stream, (uint8_t)((i & 0x7f) | 0x80));
            i = MiscUtils::unsignedShift(i, 7);
        }
        writeByte(stream, (uint8_t)i);
    }
    
    void TermsHashPerField::finish()
    {
        consumer->finish();
        if (nextPerField)
            nextPerField->finish();
    }
    
    void TermsHashPerField::rehashPostings(int32_t newSize)
    {
        int32_t newMask = newSize - 1;
        
        IntArray newHash(IntArray::newInstance(newSize));
        MiscUtils::arrayFill(newHash.get(), 0, newHash.size(), -1);
        TermsHashPerThreadPtr perThread(_perThread);
        
        for (int32_t i = 0; i < postingsHashSize; ++i)
        {
            int32_t termID = postingsHash[i];
            if (termID != -1)
            {
                int32_t code;
                if (perThread->primary)
                {
                    int32_t textStart = postingsArray->textStarts[termID];
                    int32_t start = (textStart & DocumentsWriter::CHAR_BLOCK_MASK);
                    CharArray text = charPool->buffers[textStart >> DocumentsWriter::CHAR_BLOCK_SHIFT];
                    int32_t pos = start;
                    while (text[pos] != UTF8Base::UNICODE_TERMINATOR)
                        ++pos;
                    code = 0;
                    while (pos > start)
                        code = (code * 31) + text[--pos];
                }
                else
                    code = postingsArray->textStarts[termID];
                
                int32_t hashPos = (code & newMask);
                BOOST_ASSERT(hashPos >= 0);
                if (newHash[hashPos] != -1)
                {
                    int32_t inc = (((code >> 8) + code) | 1);
                    do
                    {
                        code += inc;
                        hashPos = (code & newMask);
                    }
                    while (newHash[hashPos] != -1);
                }
                newHash[hashPos] = termID;
            }
        }
        
        postingsHashMask = newMask;
        postingsHash = newHash;
        postingsHashSize = newSize;
        postingsHashHalfSize = (newSize >> 1);
    }
}
