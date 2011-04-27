/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TermsHash.h"
#include "DocumentsWriter.h"
#include "TermsHashConsumer.h"
#include "TermsHashPerThread.h"
#include "TermsHashPerField.h"
#include "TermsHashConsumerPerThread.h"
#include "DocInverterPerThread.h"
#include "TermsHashConsumerPerField.h"
#include "IndexWriter.h"
#include "MiscUtils.h"

namespace Lucene
{
    TermsHash::TermsHash(DocumentsWriterPtr docWriter, bool trackAllocations, TermsHashConsumerPtr consumer, TermsHashPtr nextTermsHash)
    {
        this->trackAllocations = false;
        this->_docWriter = docWriter;
        this->consumer = consumer;
        this->nextTermsHash = nextTermsHash;
        this->trackAllocations = trackAllocations;
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
        consumer->abort();
        if (nextTermsHash)
            nextTermsHash->abort();
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
        
        if (nextTermsHash)
            nextTermsHash->flush(nextThreadsAndFields, state);
    }
    
    bool TermsHash::freeRAM()
    {
        SyncLock syncLock(this);
        return false;
    }
}
