/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMVECTORSTERMSWRITER_H
#define TERMVECTORSTERMSWRITER_H

#include "TermsHashConsumer.h"

namespace Lucene
{
    class TermVectorsTermsWriter : public TermsHashConsumer
    {
    public:
        TermVectorsTermsWriter(DocumentsWriterPtr docWriter);
        virtual ~TermVectorsTermsWriter();
        
        LUCENE_CLASS(TermVectorsTermsWriter);
            
    public:
        DocumentsWriterWeakPtr _docWriter;
        Collection<TermVectorsTermsWriterPerDocPtr> docFreeList;
        int32_t freeCount;
        IndexOutputPtr tvx;
        IndexOutputPtr tvd;
        IndexOutputPtr tvf;
        int32_t lastDocID;
        int32_t allocCount;
        bool hasVectors;
    
    public:
        virtual TermsHashConsumerPerThreadPtr addThread(TermsHashPerThreadPtr perThread);
        virtual void flush(MapTermsHashConsumerPerThreadCollectionTermsHashConsumerPerField threadsAndFields, SegmentWriteStatePtr state);
        
        TermVectorsTermsWriterPerDocPtr getPerDoc();
        
        /// Fills in no-term-vectors for all docs we haven't seen since the last doc that had term vectors.
        void fill(int32_t docID);
        
        void initTermVectorsWriter();
        void finishDocument(TermVectorsTermsWriterPerDocPtr perDoc);
        void free(TermVectorsTermsWriterPerDocPtr doc);
        
        virtual void abort();
    };
}

#endif
