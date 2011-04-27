/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _TERMVECTORSTERMSWRITER_H
#define _TERMVECTORSTERMSWRITER_H

#include "_DocumentsWriter.h"

namespace Lucene
{
    class TermVectorsTermsWriterPerDoc : public DocWriter
    {
    public:
        TermVectorsTermsWriterPerDoc(TermVectorsTermsWriterPtr termsWriter = TermVectorsTermsWriterPtr());
        virtual ~TermVectorsTermsWriterPerDoc();
        
        LUCENE_CLASS(TermVectorsTermsWriterPerDoc);
            
    protected:
        TermVectorsTermsWriterWeakPtr _termsWriter;
    
    public:
        PerDocBufferPtr buffer;
        RAMOutputStreamPtr perDocTvf;
        int32_t numVectorFields;
        
        Collection<int32_t> fieldNumbers;
        Collection<int64_t> fieldPointers;
    
    public:
        void reset();
        virtual void abort();
        void addField(int32_t fieldNumber);
        virtual int64_t sizeInBytes();
        virtual void finish();
    };
}

#endif
