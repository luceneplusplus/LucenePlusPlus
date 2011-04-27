/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DOCUMENTSWRITER_H
#define DOCUMENTSWRITER_H

#include "LuceneObject.h"

namespace Lucene
{
    /// This class accepts multiple added documents and directly writes a single segment file.  It does this more
    /// efficiently than creating a single segment per document (with DocumentWriter) and doing standard merges on 
    /// those segments.
    ///
    /// Each added document is passed to the {@link DocConsumer}, which in turn processes the document and interacts 
    /// with other consumers in the indexing chain.  Certain consumers, like {@link StoredFieldsWriter} and {@link
    /// TermVectorsTermsWriter}, digest a document and immediately write bytes to the "doc store" files (ie,
    /// they do not consume RAM per document, except while they are processing the document).
    /// 
    /// Other consumers, eg {@link FreqProxTermsWriter} and {@link NormsWriter}, buffer bytes in RAM and flush only
    /// when a new segment is produced.
    ///
    /// Once we have used our allowed RAM buffer, or the number of added docs is large enough (in the case we are
    /// flushing by doc count instead of RAM usage), we create a real segment and flush it to the Directory.
    ///
    /// Threads:
    /// Multiple threads are allowed into addDocument at once. There is an initial synchronized call to 
    /// getThreadState which allocates a ThreadState for this thread.  The same thread will get the same ThreadState 
    /// over time (thread affinity) so that if there are consistent patterns (for example each thread is indexing a 
    /// different content source) then we make better use of RAM.  Then processDocument is called on that ThreadState 
    /// without synchronization (most of the "heavy lifting" is in this call).  Finally the synchronized 
    /// "finishDocument" is called to flush changes to the directory.
    ///
    /// When flush is called by IndexWriter we forcefully idle all threads and flush only once they are all idle.  
    /// This means you can call flush with a given thread even while other threads are actively adding/deleting 
    /// documents.
    ///
    /// Exceptions:
    /// Because this class directly updates in-memory posting lists, and flushes stored fields and term vectors
    /// directly to files in the directory, there are certain limited times when an exception can corrupt this state.
    /// For example, a disk full while flushing stored fields leaves this file in a corrupt state.  Or, an 
    /// std::bad_alloc exception while appending to the in-memory posting lists can corrupt that posting list.  
    /// We call such exceptions "aborting exceptions".  In these cases we must call abort() to discard all docs added 
    /// since the last flush.
    ///
    /// All other exceptions ("non-aborting exceptions") can still partially update the index structures.  These
    /// updates are consistent, but, they represent only a part of the document seen up until the exception was hit.
    /// When this happens, we immediately mark the document as deleted so that the document is always atomically 
    /// ("all or none") added to the index.
    class DocumentsWriter : public LuceneObject
    {
    public:
        DocumentsWriter(IndexWriterConfigPtr config, DirectoryPtr directory, IndexWriterPtr writer, 
                        FieldInfosPtr fieldInfos, BufferedDeletesPtr bufferedDeletes);
        virtual ~DocumentsWriter();
        
        LUCENE_CLASS(DocumentsWriter);
                    
    protected:
        int32_t nextDocID; // Next docID to be added
        int32_t numDocs; // # of docs added, but not yet flushed
        
        Collection<DocumentsWriterThreadStatePtr> threadStates;
        SortedMapThreadDocumentsWriterThreadState threadBindings;
        
        bool aborting; // True if an abort is pending
        
        /// max # simultaneous threads; if there are more than this, they wait for others to finish first
        int32_t maxThreadStates;
        
        /// Deletes for our still-in-RAM (to be flushed next) segment
        SegmentDeletesPtr pendingDeletes;
        
        IndexWriterConfigPtr config;
        
        bool closed;
        FieldInfosPtr fieldInfos;
        
        BufferedDeletesPtr bufferedDeletes;
        FlushControlPtr flushControl;
        
        Collection<IntArray> freeIntBlocks;
        Collection<CharArray> freeCharBlocks;
        
    public:
        /// Initial chunks size of the shared byte[] blocks used to store postings data
        static const int32_t BYTE_BLOCK_SHIFT;
        static const int32_t BYTE_BLOCK_SIZE;
        static const int32_t BYTE_BLOCK_MASK;
        static const int32_t BYTE_BLOCK_NOT_MASK;
        
        /// Initial chunk size of the shared char[] blocks used to store term text
        static const int32_t CHAR_BLOCK_SHIFT;
        static const int32_t CHAR_BLOCK_SIZE;
        static const int32_t CHAR_BLOCK_MASK;
        
        static const int32_t MAX_TERM_LENGTH;
        
        /// Initial chunks size of the shared int[] blocks used to store postings data
        static const int32_t INT_BLOCK_SHIFT;
        static const int32_t INT_BLOCK_SIZE;
        static const int32_t INT_BLOCK_MASK;
        
        static const int32_t PER_DOC_BLOCK_SIZE;

    INTERNAL:
        AtomicLongPtr _bytesUsed;
        
        IndexWriterWeakPtr _writer;
        DirectoryPtr directory;
        IndexingChainPtr indexingChain;
        String segment; // Current segment we are working on
        
        bool bufferIsFull; // True when it's time to write segment
        
        InfoStreamPtr infoStream;
        int32_t maxFieldLength;
        SimilarityPtr similarity;
        
        DocConsumerPtr consumer;
        
        WaitQueuePtr waitQueue;
        SkipDocWriterPtr skipDocWriter;
        
        ByteBlockAllocatorPtr byteBlockAllocator;
        ByteBlockAllocatorPtr perDocAllocator;
        
    public:
        virtual void initialize();
        
        /// Buffer a specific docID for deletion.  Currently only used when we hit a exception when adding 
        /// a document
        void deleteDocID(int32_t docIDUpto);
        
        bool deleteQueries(Collection<QueryPtr> queries);
        bool deleteQuery(QueryPtr query);
        bool deleteTerms(Collection<TermPtr> terms);
        bool deleteTerm(TermPtr term, bool skipWait);
        
        FieldInfosPtr getFieldInfos();
    
        /// Create and return a new DocWriterBuffer.
        PerDocBufferPtr newPerDocBuffer();
    
        static IndexingChainPtr defaultIndexingChain();
        
        /// If non-null, various details of indexing are printed here.
        void setInfoStream(InfoStreamPtr infoStream);
        
        void setMaxFieldLength(int32_t maxFieldLength);
        void setSimilarity(SimilarityPtr similarity);
        
        /// Get current segment name we are writing.
        String getSegment();
        
        /// Returns how many docs are currently buffered in RAM.
        int32_t getNumDocs();
        
        void message(const String& message);
        
        void setAborting();
        
        /// Called if we hit an exception at a bad time (when updating the index files) and must discard all
        /// currently buffered docs.  This resets our state, discarding any docs added since last flush.
        void abort();
        
        bool anyChanges();
        
        /// for testing
        SegmentDeletesPtr getPendingDeletes();
        
        bool anyDeletions();
        
        /// Flush all pending docs to a new segment
        /// Lock order: IW -> DW
        SegmentInfoPtr flush(IndexWriterPtr writer, IndexFileDeleterPtr deleter, MergePolicyPtr mergePolicy, SegmentInfosPtr segmentInfos);
        
        void close();
        
        /// Returns a free (idle) ThreadState that may be used for indexing this one document.  This call also 
        /// pauses if a flush is pending.  If delTerm is non-null then we buffer this deleted term after the 
        /// thread state has been acquired.
        DocumentsWriterThreadStatePtr getThreadState(DocumentPtr doc, TermPtr delTerm);
        
        /// Returns true if the caller (IndexWriter) should now flush.
        bool addDocument(DocumentPtr doc, AnalyzerPtr analyzer);
        
        bool updateDocument(DocumentPtr doc, AnalyzerPtr analyzer, TermPtr delTerm);
        
        void waitIdle();
        
        void waitForWaitQueue();
        
        /// Allocate another int[] from the shared pool
        IntArray getIntBlock();
        
        void bytesUsed(int64_t numBytes);
        int64_t bytesUsed();
        
        void recycleIntBlocks(Collection<IntArray> blocks, int32_t start, int32_t end);
        
        CharArray getCharBlock();
        void recycleCharBlocks(Collection<CharArray> blocks, int32_t numBlocks);
        
        String toMB(int64_t v);
        
        /// We have four pools of RAM: Postings, byte blocks (holds freq/prox posting data), char blocks (holds
        /// characters in the term) and per-doc buffers (stored fields/term vectors).  Different docs require 
        /// varying amount of storage from these four classes.
        ///
        /// For example, docs with many unique single-occurrence short terms will use up the Postings
        /// RAM and hardly any of the other two.  Whereas docs with very large terms will use alot of char blocks 
        /// RAM and relatively less of the other two.  This method just frees allocations from the pools once we 
        /// are over-budget, which balances the pools to match the current docs.
        void balanceRAM();
            
    protected:
        /// Reset after a flush
        void doAfterFlush();
        
        bool allThreadsIdle();
        
        void pushDeletes(SegmentInfoPtr newSegment, SegmentInfosPtr segmentInfos);
        
        void waitReady(DocumentsWriterThreadStatePtr state);
        
        /// Does the synchronized work to finish/flush the inverted document.
        void finishDocument(DocumentsWriterThreadStatePtr perThread, DocWriterPtr docWriter);
        
        friend class WaitQueue;
    };
}

#endif
