/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DOCFIELDPROCESSOR_H
#define DOCFIELDPROCESSOR_H

#include "DocConsumer.h"

namespace Lucene
{
    /// This is a DocConsumer that gathers all fields under the same name, and calls per-field consumers to process
    /// field by field.  This class doesn't doesn't do any "real" work of its own: it just forwards the fields to a
    /// DocFieldConsumer.
    class DocFieldProcessor : public DocConsumer
    {
    public:
        DocFieldProcessor(DocumentsWriterPtr docWriter, DocFieldConsumerPtr consumer);
        virtual ~DocFieldProcessor();

        LUCENE_CLASS(DocFieldProcessor);

    public:
        DocumentsWriterPtr docWriter;
        FieldInfosPtr fieldInfos;
        DocFieldConsumerPtr consumer;
        StoredFieldsWriterPtr fieldsWriter;

    protected:
        virtual void mark_members(gc* gc) const
        {
            gc->mark(docWriter);
            gc->mark(fieldInfos);
            gc->mark(consumer);
            gc->mark(fieldsWriter);
            DocConsumer::mark_members(gc);
        }

    public:
        virtual void closeDocStore(SegmentWriteStatePtr state);
        virtual void flush(Collection<DocConsumerPerThreadPtr> threads, SegmentWriteStatePtr state);
        virtual void abort();
        virtual bool freeRAM();
        virtual DocConsumerPerThreadPtr addThread(DocumentsWriterThreadStatePtr perThread);
    };
}

#endif
