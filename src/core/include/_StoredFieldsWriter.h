/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _STOREDFIELDSWRITER_H
#define _STOREDFIELDSWRITER_H

#include "_DocumentsWriter.h"

namespace Lucene
{
    class StoredFieldsWriterPerDoc : public DocWriter
    {
    public:
        StoredFieldsWriterPerDoc(StoredFieldsWriterPtr fieldsWriter);
        virtual ~StoredFieldsWriterPerDoc();
        
        LUCENE_CLASS(StoredFieldsWriterPerDoc);
            
    protected:
        StoredFieldsWriterWeakPtr _fieldsWriter;
    
    public:
        PerDocBufferPtr buffer;
        RAMOutputStreamPtr fdt;
        int32_t numStoredFields;
    
    public:
        void reset();
        virtual void abort();
        virtual int64_t sizeInBytes();
        virtual void finish();
    };
}

#endif
