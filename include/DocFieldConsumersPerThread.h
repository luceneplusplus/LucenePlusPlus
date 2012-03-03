/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DOCFIELDCONSUMERSPERTHREAD_H
#define DOCFIELDCONSUMERSPERTHREAD_H

#include "DocFieldConsumerPerThread.h"

namespace Lucene
{
    class DocFieldConsumersPerThread : public DocFieldConsumerPerThread
    {
    public:
        DocFieldConsumersPerThread(DocFieldProcessorPerThreadPtr docFieldProcessorPerThread, DocFieldConsumersPtr parent,
                                   DocFieldConsumerPerThreadPtr one, DocFieldConsumerPerThreadPtr two);
        virtual ~DocFieldConsumersPerThread();

        LUCENE_CLASS(DocFieldConsumersPerThread);

    public:
        DocFieldConsumerPerThreadPtr one;
        DocFieldConsumerPerThreadPtr two;
        DocFieldConsumersPtr parent;
        DocStatePtr docState;

    protected:
        virtual void mark_members(gc* gc) const
        {
            gc->mark(one);
            gc->mark(two);
            gc->mark(parent);
            gc->mark(docState);
            DocFieldConsumerPerThread::mark_members(gc);
        }

    public:
        virtual void startDocument();
        virtual void abort();
        virtual DocWriterPtr finishDocument();
        virtual DocFieldConsumerPerFieldPtr addField(FieldInfoPtr fi);
    };
}

#endif
