/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STOREDFIELDSWRITER_H
#define STOREDFIELDSWRITER_H

#include "DocumentsWriter.h"

namespace Lucene
{
    /// This is a DocFieldConsumer that writes stored fields.
    class StoredFieldsWriter : public LuceneObject
    {
    public:
        StoredFieldsWriter(DocumentsWriterPtr docWriter, FieldInfosPtr fieldInfos);
        virtual ~StoredFieldsWriter();
        
        LUCENE_CLASS(StoredFieldsWriter);
            
    public:
        FieldsWriterPtr fieldsWriter;
        DocumentsWriterWeakPtr _docWriter;
        FieldInfosPtr fieldInfos;
        int32_t lastDocID;

        Collection<StoredFieldsWriterPerDocPtr> docFreeList;
        int32_t freeCount;
        int32_t allocCount;
        
    public:
        StoredFieldsWriterPerThreadPtr addThread(DocStatePtr docState);
        void flush(SegmentWriteStatePtr state);
        StoredFieldsWriterPerDocPtr getPerDoc();
        void abort();
        
        /// Fills in any hole in the docIDs
        void fill(int32_t docID);
        
        void finishDocument(StoredFieldsWriterPerDocPtr perDoc);
        void free(StoredFieldsWriterPerDocPtr perDoc);
        
    private:
        void initFieldsWriter();
    };
}

#endif
