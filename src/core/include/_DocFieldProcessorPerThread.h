/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _DOCFIELDPROCESSORPERTHREAD_H
#define _DOCFIELDPROCESSORPERTHREAD_H

#include "_DocumentsWriter.h"

namespace Lucene
{
    class DocFieldProcessorPerThreadPerDoc : public DocWriter
    {
    public:
        DocFieldProcessorPerThreadPerDoc(DocFieldProcessorPerThreadPtr docProcessor);
        virtual ~DocFieldProcessorPerThreadPerDoc();
        
        LUCENE_CLASS(DocFieldProcessorPerThreadPerDoc);
                
    public:
        DocWriterPtr one;
        DocWriterPtr two;
    
    protected:
        DocFieldProcessorPerThreadWeakPtr _docProcessor;
        
    public:
        virtual int64_t sizeInBytes();
        virtual void finish();
        virtual void abort();
    };
}

#endif
