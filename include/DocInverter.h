/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DOCINVERTER_H
#define DOCINVERTER_H

#include "DocFieldConsumer.h"

namespace Lucene
{
    /// This is a DocFieldConsumer that inverts each field, separately, from a Document, and accepts a
    /// InvertedTermsConsumer to process those terms.
    class DocInverter : public DocFieldConsumer
    {
    public:
        DocInverter(InvertedDocConsumerPtr consumer, InvertedDocEndConsumerPtr endConsumer);
        virtual ~DocInverter();

        LUCENE_CLASS(DocInverter);

    public:
        InvertedDocConsumerPtr consumer;
        InvertedDocEndConsumerPtr endConsumer;

    protected:
        virtual void mark_members(gc* gc) const
        {
            gc->mark(consumer);
            gc->mark(endConsumer);
            DocFieldConsumer::mark_members(gc);
        }

    public:
        virtual void setFieldInfos(FieldInfosPtr fieldInfos);

        /// Called when DocumentsWriter decides to create a new segment
        virtual void flush(MapDocFieldConsumerPerThreadCollectionDocFieldConsumerPerField threadsAndFields, SegmentWriteStatePtr state);

        /// Called when DocumentsWriter decides to close the doc stores
        virtual void closeDocStore(SegmentWriteStatePtr state);

        /// Called when an aborting exception is hit
        virtual void abort();

        /// Called when DocumentsWriter is using too much RAM.
        virtual bool freeRAM();

        /// Add a new thread
        virtual DocFieldConsumerPerThreadPtr addThread(DocFieldProcessorPerThreadPtr docFieldProcessorPerThread);
    };
}

#endif
