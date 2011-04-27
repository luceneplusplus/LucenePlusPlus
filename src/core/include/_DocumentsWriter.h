/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _DOCUMENTSWRITER_H
#define _DOCUMENTSWRITER_H

#include "ByteBlockPool.h"
#include "RAMFile.h"

namespace Lucene
{
    class DocState : public LuceneObject
    {
    public:
        DocState();
        virtual ~DocState();
        
        LUCENE_CLASS(DocState);
                
    public:
        DocumentsWriterWeakPtr _docWriter;
        AnalyzerPtr analyzer;
        int32_t maxFieldLength;
        InfoStreamPtr infoStream;
        SimilarityPtr similarity;
        int32_t docID;
        DocumentPtr doc;
        String maxTermPrefix;
    
    public:
        /// Only called by asserts
        virtual bool testPoint(const String& name);
        
        void clear();
    };
    
    /// RAMFile buffer for DocWriters.
    class PerDocBuffer : public RAMFile
    {
    public:
        PerDocBuffer(DocumentsWriterPtr docWriter);
        virtual ~PerDocBuffer();
        
        LUCENE_CLASS(PerDocBuffer);
    
    protected:
        DocumentsWriterWeakPtr _docWriter;
    
    public:
        /// Recycle the bytes used.
        void recycle();
    
    protected:
        /// Allocate bytes used from shared pool.
        virtual ByteArray newBuffer(int32_t size);
    };
    
    /// Consumer returns this on each doc.  This holds any state that must be flushed synchronized 
    /// "in docID order".  We gather these and flush them in order.
    class DocWriter : public LuceneObject
    {
    public:
        DocWriter();
        virtual ~DocWriter();
        
        LUCENE_CLASS(DocWriter);
    
    public:
        DocWriterPtr next;
        int32_t docID;
    
    public:
        virtual void finish() = 0;
        virtual void abort() = 0;
        virtual int64_t sizeInBytes() = 0;
        
        virtual void setNext(DocWriterPtr next);
    };
    
    /// The IndexingChain must define the {@link #getChain(DocumentsWriter)} method which returns the DocConsumer 
    /// that the DocumentsWriter calls to process the documents. 
    class IndexingChain : public LuceneObject
    {
    public:
        virtual ~IndexingChain();
        
        LUCENE_CLASS(IndexingChain);
    
    public:
        virtual DocConsumerPtr getChain(DocumentsWriterPtr documentsWriter) = 0;
    };
    
    /// This is the current indexing chain:
    /// DocConsumer / DocConsumerPerThread
    ///   --> code: DocFieldProcessor / DocFieldProcessorPerThread
    ///     --> DocFieldConsumer / DocFieldConsumerPerThread / DocFieldConsumerPerField
    ///       --> code: DocFieldConsumers / DocFieldConsumersPerThread / DocFieldConsumersPerField
    ///         --> code: DocInverter / DocInverterPerThread / DocInverterPerField
    ///          --> InvertedDocConsumer / InvertedDocConsumerPerThread / InvertedDocConsumerPerField
    ///            --> code: TermsHash / TermsHashPerThread / TermsHashPerField
    ///              --> TermsHashConsumer / TermsHashConsumerPerThread / TermsHashConsumerPerField
    ///                --> code: FreqProxTermsWriter / FreqProxTermsWriterPerThread / FreqProxTermsWriterPerField
    ///                --> code: TermVectorsTermsWriter / TermVectorsTermsWriterPerThread / TermVectorsTermsWriterPerField
    ///          --> InvertedDocEndConsumer / InvertedDocConsumerPerThread / InvertedDocConsumerPerField
    ///            --> code: NormsWriter / NormsWriterPerThread / NormsWriterPerField
    ///        --> code: StoredFieldsWriter / StoredFieldsWriterPerThread / StoredFieldsWriterPerField
    class DefaultIndexingChain : public IndexingChain
    {
    public:
        virtual ~DefaultIndexingChain();
        
        LUCENE_CLASS(DefaultIndexingChain);
            
    public:
        virtual DocConsumerPtr getChain(DocumentsWriterPtr documentsWriter);
    };
    
    class SkipDocWriter : public DocWriter
    {
    public:
        virtual ~SkipDocWriter();
        
        LUCENE_CLASS(SkipDocWriter);
            
    public:
        virtual void finish();
        virtual void abort();
        virtual int64_t sizeInBytes();
    };
    
    class WaitQueue : public LuceneObject
    {
    public:
        WaitQueue(DocumentsWriterPtr docWriter);
        virtual ~WaitQueue();
        
        LUCENE_CLASS(WaitQueue);
            
    protected:
        DocumentsWriterWeakPtr _docWriter;
    
    public:
        Collection<DocWriterPtr> waiting;
        int32_t nextWriteDocID;
        int32_t nextWriteLoc;
        int32_t numWaiting;
        int64_t waitingBytes;
    
    public:
        void reset();
        bool doResume();
        bool doPause();
        void abort();
        bool add(DocWriterPtr doc);
    
    protected:
        void writeDocument(DocWriterPtr doc);
    };
    
    class ByteBlockAllocator : public ByteBlockPoolAllocatorBase
    {
    public:
        ByteBlockAllocator(DocumentsWriterPtr docWriter, int32_t blockSize);
        virtual ~ByteBlockAllocator();
        
        LUCENE_CLASS(ByteBlockAllocator);
            
    protected:
        DocumentsWriterWeakPtr _docWriter;
    
    public:
        int32_t blockSize;
        Collection<ByteArray> freeByteBlocks;
    
    public:
        /// Allocate another byte[] from the shared pool
        virtual ByteArray getByteBlock();
        
        /// Return byte[]'s to the pool
        virtual void recycleByteBlocks(Collection<ByteArray> blocks, int32_t start, int32_t end);
        virtual void recycleByteBlocks(Collection<ByteArray> blocks);
    };
}

#endif
