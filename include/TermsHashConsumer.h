/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMSHASHCONSUMER_H
#define TERMSHASHCONSUMER_H

#include "LuceneObject.h"

namespace Lucene
{
    class TermsHashConsumer : public LuceneObject
    {
    public:
        virtual ~TermsHashConsumer();
        
        LUCENE_CLASS(TermsHashConsumer);
    
    public:
        FieldInfosPtr fieldInfos;
    
    public:
        virtual TermsHashConsumerPerThreadPtr addThread(TermsHashPerThreadPtr perThread) = 0;
        virtual void flush(MapTermsHashConsumerPerThreadCollectionTermsHashConsumerPerField threadsAndFields, SegmentWriteStatePtr state) = 0;
        virtual void abort() = 0;
        
        virtual void setFieldInfos(FieldInfosPtr fieldInfos);
    };
}

#endif
