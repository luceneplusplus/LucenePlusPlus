/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TermsHash.h"
#include "DocumentsWriter.h"
#include "TermsHashConsumer.h"
#include "TermsHashPerThread.h"
#include "TermsHashPerField.h"
#include "TermsHashConsumerPerThread.h"
#include "DocFieldConsumerPerThread.h"
#include "DocInverterPerThread.h"
#include "InvertedDocConsumerPerField.h"
#include "TermsHashConsumerPerField.h"
#include "IndexWriter.h"

namespace Lucene
{
    TermsHash::TermsHash(DocumentsWriterPtr docWriter, bool trackAllocations, TermsHashConsumerPtr consumer, TermsHashPtr nextTermsHash)
    {
        this->postingsFreeCount = 0;
        this->postingsAllocCount = 0;
        this->trackAllocations = false;
        this->postingsFreeList = Collection<RawPostingListPtr>::newInstance(1);
        
        this->_docWriter = docWriter;
        this->consumer = consumer;
        this->nextTermsHash = nextTermsHash;
        this->trackAllocations = trackAllocations;
        
        bytesPerPosting = consumer->bytesPerPosting() + 4 * DocumentsWriter::POINTER_NUM_BYTE;
        postingsFreeChunk = (int32_t)((double)DocumentsWriter::BYTE_BLOCK_SIZE / (double)bytesPerPosting);
    }
    
    TermsHash::~TermsHash()
    {
    }
    
    InvertedDocConsumerPerThreadPtr TermsHash::addThread(DocInverterPerThreadPtr docInverterPerThread)
    {
        return newLucene<TermsHashPerThread>(docInverterPerThread, shared_from_this(), nextTermsHash, TermsHashPerThreadPtr());
    }
    
    TermsHashPerThreadPtr TermsHash::addThread(DocInverterPerThreadPtr docInverterPerThread, TermsHashPerThreadPtr primaryPerThread)
    {
        return newLucene<TermsHashPerThread>(docInverterPerThread, shared_from_this(), nextTermsHash, primaryPerThread);
    }
    
    void TermsHash::setFieldInfos(FieldInfosPtr fieldInfos)
    {
        this->fieldInfos = fieldInfos;
        consumer->setFieldInfos(fieldInfos);
    }
    
    void TermsHash::abort()
    {
        SyncLock syncLock(this);
        consumer->abort();
        if (nextTermsHash)
            nextTermsHash->abort();
    }
    
    void TermsHash::shrinkFreePostings(MapInvertedDocConsumerPerThreadCollectionInvertedDocConsumerPerField threadsAndFields, SegmentWriteStatePtr state)
    {
        BOOST_ASSERT(postingsFreeCount == postingsAllocCount);
        
        int32_t newSize = MiscUtils::getShrinkSize(postingsFreeList.size(), postingsAllocCount);
        if (newSize != postingsFreeList.size())
            postingsFreeList.resize(newSize);
    }
    
    void TermsHash::closeDocStore(SegmentWriteStatePtr state)
    {
        SyncLock syncLock(this);
        consumer->closeDocStore(state);
        if (nextTermsHash)
            nextTermsHash->closeDocStore(state);
    }
    
    void TermsHash::flush(MapInvertedDocConsumerPerThreadCollectionInvertedDocConsumerPerField threadsAndFields, SegmentWriteStatePtr state)
    {
        SyncLock syncLock(this);
        MapTermsHashConsumerPerThreadCollectionTermsHashConsumerPerField childThreadsAndFields(MapTermsHashConsumerPerThreadCollectionTermsHashConsumerPerField::newInstance());
        MapInvertedDocConsumerPerThreadCollectionInvertedDocConsumerPerField nextThreadsAndFields;
        if (nextTermsHash)
            nextThreadsAndFields = MapInvertedDocConsumerPerThreadCollectionInvertedDocConsumerPerField::newInstance();
        
        for (MapInvertedDocConsumerPerThreadCollectionInvertedDocConsumerPerField::iterator entry = threadsAndFields.begin(); entry != threadsAndFields.end(); ++entry)
        {
            Collection<TermsHashConsumerPerFieldPtr> childFields(Collection<TermsHashConsumerPerFieldPtr>::newInstance());
            Collection<InvertedDocConsumerPerFieldPtr> nextChildFields;
            if (nextTermsHash)
                nextChildFields = Collection<InvertedDocConsumerPerFieldPtr>::newInstance();
            
            for (Collection<InvertedDocConsumerPerFieldPtr>::iterator perField = entry->second.begin(); perField != entry->second.end(); ++perField)
            {
                childFields.add(boost::static_pointer_cast<TermsHashPerField>(*perField)->consumer);
                if (nextTermsHash)
                    nextChildFields.add(boost::static_pointer_cast<TermsHashPerField>(*perField)->nextPerField);
            }
            
            childThreadsAndFields.put(boost::static_pointer_cast<TermsHashPerThread>(entry->first)->consumer, childFields);
            if (nextTermsHash)
                nextThreadsAndFields.put(boost::static_pointer_cast<TermsHashPerThread>(entry->first)->nextPerThread, nextChildFields);
        }
        
        consumer->flush(childThreadsAndFields, state);
        
        shrinkFreePostings(threadsAndFields, state);
        
        if (nextTermsHash)
            nextTermsHash->flush(nextThreadsAndFields, state);
    }
    
    bool TermsHash::freeRAM()
    {
        SyncLock syncLock(this);
        if (!trackAllocations)
            return false;
        
        int32_t numToFree = postingsFreeCount >= postingsFreeChunk ? postingsFreeChunk : postingsFreeCount;
        bool any = (numToFree > 0);
        if (any)
        {
            MiscUtils::arrayFill(postingsFreeList.begin(), postingsFreeCount - numToFree, postingsFreeCount, RawPostingListPtr());
            postingsFreeCount -= numToFree;
            postingsAllocCount -= numToFree;
            DocumentsWriterPtr(_docWriter)->bytesAllocated(-numToFree * bytesPerPosting);
            any = true;
        }
        
        if (nextTermsHash && nextTermsHash->freeRAM())
            any = true;
        
        return any;
    }
    
    void TermsHash::recyclePostings(Collection<RawPostingListPtr> postings, int32_t numPostings)
    {
        SyncLock syncLock(this);
        BOOST_ASSERT(postings.size() >= numPostings);
        
        // Move all Postings from this ThreadState back to our free list.  We pre-allocated this array while we 
        // were creating Postings to make sure it's large enough
        BOOST_ASSERT(postingsFreeCount + numPostings <= postingsFreeList.size());
        MiscUtils::arrayCopy(postings.begin(), 0, postingsFreeList.begin(), postingsFreeCount, numPostings);
        postingsFreeCount += numPostings;
    }
    
    void TermsHash::getPostings(Collection<RawPostingListPtr> postings)
    {
        SyncLock syncLock(this);
        DocumentsWriterPtr docWriter(_docWriter);
        IndexWriterPtr writer(docWriter->_writer);
        
        BOOST_ASSERT(writer->testPoint(L"TermsHash.getPostings start"));
        
        BOOST_ASSERT(postingsFreeCount <= postingsFreeList.size());
        BOOST_ASSERT(postingsFreeCount <= postingsAllocCount);
        
        int32_t numToCopy = postingsFreeCount < postings.size() ? postingsFreeCount : postings.size();
        int32_t start = postingsFreeCount - numToCopy;
        BOOST_ASSERT(start >= 0);
        BOOST_ASSERT(start + numToCopy <= postingsFreeList.size());
        BOOST_ASSERT(numToCopy <= postings.size());
        MiscUtils::arrayCopy(postingsFreeList.begin(), start, postings.begin(), 0, numToCopy);
        
        // Directly allocate the remainder if any
        if (numToCopy != postings.size())
        {
            int32_t extra = postings.size() - numToCopy;
            int32_t newPostingsAllocCount = postingsAllocCount + extra;
            
            consumer->createPostings(postings, numToCopy, extra);
            BOOST_ASSERT(writer->testPoint(L"TermsHash.getPostings after create"));
            postingsAllocCount += extra;
            
            if (trackAllocations)
                docWriter->bytesAllocated(extra * bytesPerPosting);
            
            if (newPostingsAllocCount > postingsFreeList.size())
            {
                // Pre-allocate the postingsFreeList so it's large enough to hold all postings we've given out
                postingsFreeList = Collection<RawPostingListPtr>::newInstance(MiscUtils::getNextSize(newPostingsAllocCount));
            }
        }
        
        postingsFreeCount -= numToCopy;
        
        if (trackAllocations)
            docWriter->bytesUsed(postings.size() * bytesPerPosting);
    }
}
