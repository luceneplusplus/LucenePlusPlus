/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMSHASH_H
#define TERMSHASH_H

#include "InvertedDocConsumer.h"

namespace Lucene
{
    /// This class implements {@link InvertedDocConsumer}, which is passed each token produced by the analyzer on 
    /// each field.  It stores these tokens in a hash table, and allocates separate byte streams per token.  Consumers 
    /// of this class, eg {@link FreqProxTermsWriter} and {@link TermVectorsTermsWriter}, write their own byte streams
    /// under each term.
    class TermsHash : public InvertedDocConsumer
    {
    public:
        TermsHash(DocumentsWriterPtr docWriter, bool trackAllocations, TermsHashConsumerPtr consumer, TermsHashPtr nextTermsHash);
        virtual ~TermsHash();
        
        LUCENE_CLASS(TermsHash);
            
    public:
        TermsHashConsumerPtr consumer;
        TermsHashPtr nextTermsHash;
        DocumentsWriterWeakPtr _docWriter;
        bool trackAllocations;
        
    public:
        /// Add a new thread
        virtual InvertedDocConsumerPerThreadPtr addThread(DocInverterPerThreadPtr docInverterPerThread);
        virtual TermsHashPerThreadPtr addThread(DocInverterPerThreadPtr docInverterPerThread, TermsHashPerThreadPtr primaryPerThread);
        
        virtual void setFieldInfos(FieldInfosPtr fieldInfos);
        
        /// Abort (called after hitting AbortException)
        virtual void abort();
        
        /// Flush a new segment
        virtual void flush(MapInvertedDocConsumerPerThreadCollectionInvertedDocConsumerPerField threadsAndFields, SegmentWriteStatePtr state);
                
        /// Attempt to free RAM, returning true if any RAM was freed
        virtual bool freeRAM();
    };
}

#endif
