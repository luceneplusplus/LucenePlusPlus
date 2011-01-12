/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PAYLOADSPANUTIL_H
#define PAYLOADSPANUTIL_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Experimental class to get set of payloads for most standard Lucene queries.  Operates like Highlighter - 
    /// IndexReader should only contain doc of interest, best to use MemoryIndex.
    class LPPAPI PayloadSpanUtil : public LuceneObject
    {
    public:
        /// @param reader That contains doc with payloads to extract
        PayloadSpanUtil(IndexReaderPtr reader);
        
        virtual ~PayloadSpanUtil();
        
        LUCENE_CLASS(PayloadSpanUtil);
    
    protected:
        IndexReaderPtr reader;
    
    public:
        /// Query should be rewritten for wild/fuzzy support.
        /// @return payloads Collection
        Collection<ByteArray> getPayloadsForQuery(QueryPtr query);
    
    protected:
        void queryToSpanQuery(QueryPtr query, Collection<ByteArray> payloads);
        void getPayloads(Collection<ByteArray> payloads, SpanQueryPtr query);
    };
}

#endif
