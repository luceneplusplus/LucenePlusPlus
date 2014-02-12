/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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
        virtual int32_t bytesPerPosting() = 0;
        virtual void createPostings(Collection<RawPostingListPtr> postings, int32_t start, int32_t count) = 0;
        virtual TermsHashConsumerPerThreadPtr addThread(TermsHashPerThreadPtr perThread) = 0;
        virtual void flush(MapTermsHashConsumerPerThreadCollectionTermsHashConsumerPerField threadsAndFields, SegmentWriteStatePtr state) = 0;
        virtual void abort() = 0;
        virtual void closeDocStore(SegmentWriteStatePtr state) = 0;

        virtual void setFieldInfos(FieldInfosPtr fieldInfos);
    };
}

#endif
