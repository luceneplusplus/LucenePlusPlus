/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STOREDFIELDSWRITERPERTHREAD_H
#define STOREDFIELDSWRITERPERTHREAD_H

#include "LuceneObject.h"

namespace Lucene
{
    class StoredFieldsWriterPerThread : public LuceneObject
    {
    public:
        StoredFieldsWriterPerThread(DocStatePtr docState, StoredFieldsWriterPtr storedFieldsWriter);
        virtual ~StoredFieldsWriterPerThread();

        LUCENE_CLASS(StoredFieldsWriterPerThread);

    public:
        FieldsWriterPtr localFieldsWriter;
        StoredFieldsWriterPtr storedFieldsWriter;
        DocStatePtr docState;

        StoredFieldsWriterPerDocPtr doc;

    public:
        void startDocument();
        void addField(FieldablePtr field, FieldInfoPtr fieldInfo);
        DocWriterPtr finishDocument();
        void abort();
    };
}

#endif
