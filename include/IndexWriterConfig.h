/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _INDEXWRITERCONFIG_H
#define _INDEXWRITERCONFIG_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Holds all the configuration of {@link IndexWriterConfig}.  You should instantiate this class, call the setters to set
    /// your configuration, then pass it to {@link IndexWriterConfig}.  Note that {@link IndexWriterConfig} makes a private clone; 
    /// if you need to subsequently change settings use {@link IndexWriterConfig#getConfig}.
    ///
    /// All setter methods return {@link IndexWriterConfigConfig} to allow chaining settings conveniently, for example:
    ///
    /// <pre>
    /// IndexWriterConfigConfigPtr conf = newLucene<IndexWriterConfigConfig>(analyzer);
    /// conf->setter1()->setter2();
    /// </pre>
    class LPPAPI IndexWriterConfig : public LuceneObject
    {
    public:
        /// Specifies the open mode for {@link IndexWriter}:
        /// <ul>
        /// {@link #CREATE} - creates a new index or overwrites an existing one.
        /// {@link #CREATE_OR_APPEND} - creates a new index if one does not exist,
        /// otherwise it opens the index and documents will be appended.
        /// {@link #APPEND} - opens an existing index.
        /// </ul>   
        enum OpenMode { CREATE, APPEND, CREATE_OR_APPEND };
        
        /// Default value is 128. Change using {@link #setTermIndexInterval(int32_t)}.
        static const int32_t DEFAULT_TERM_INDEX_INTERVAL;
        
        /// Denotes a flush trigger is disabled.
        static const int32_t DISABLE_AUTO_FLUSH;
        
        /// Disabled by default (because IndexWriter flushes by RAM usage by default).
        static const int32_t DEFAULT_MAX_BUFFERED_DELETE_TERMS;
        
        /// Disabled by default (because IndexWriter flushes by RAM usage by default).
        static const int32_t DEFAULT_MAX_BUFFERED_DOCS;
        
        /// Default value is 16 MB (which means flush when buffered docs consume 16 MB RAM).  
        static const double DEFAULT_RAM_BUFFER_SIZE_MB;
        
        /// Default value for the write lock timeout (1000 ms).
        /// @see #setDefaultWriteLockTimeout
        static int64_t WRITE_LOCK_TIMEOUT;
        
        /// The maximum number of simultaneous threads that may be indexing documents at once in IndexWriter; if more
        /// than this many threads arrive they will wait for others to finish.
        static const int32_t DEFAULT_MAX_THREAD_STATES;
        
        /// Default setting for {@link #setReaderPooling}.
        static const bool DEFAULT_READER_POOLING;
        
        /// Default value is 1. Change using {@link #setReaderTermsIndexDivisor(int32_t)}.
        static const int32_t DEFAULT_READER_TERMS_INDEX_DIVISOR;
    
    protected:
        AnalyzerPtr analyzer;
        IndexDeletionPolicyPtr delPolicy;
        IndexCommitPtr commit;
        OpenMode openMode;
        SimilarityPtr similarity;
        int32_t termIndexInterval;
        MergeSchedulerPtr mergeScheduler;
        int64_t writeLockTimeout;
        int32_t maxBufferedDeleteTerms;
        double ramBufferSizeMB;
        int32_t maxBufferedDocs;
        IndexingChainPtr indexingChain;
        IndexReaderWarmerPtr mergedSegmentWarmer;
        MergePolicyPtr mergePolicy;
        int32_t maxThreadStates;
        bool readerPooling;
        int32_t readerTermsIndexDivisor;

        LuceneVersion::Version matchVersion;

    public:
        /// Creates a new config that with defaults that match the specified {@link Version} as well as the default 
        /// {@link Analyzer}. {@link Version} is a placeholder for future changes. In the future, if different 
        /// settings will apply to different versions, they will be documented here.
        IndexWriterConfig(LuceneVersion::Version matchVersion, AnalyzerPtr analyzer);
        
        virtual ~IndexWriterConfig();
        
        LUCENE_CLASS(IndexWriterConfig);
            
    public:
        /// Sets the default (for any instance) maximum time to wait for a write lock (in milliseconds).
        static void setDefaultWriteLockTimeout(int64_t writeLockTimeout);
        
        /// Returns the default write lock timeout for newly instantiated IndexWriterConfigs.
        /// @see #setDefaultWriteLockTimeout(long)
        static int64_t getDefaultWriteLockTimeout();
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        /// Returns the default analyzer to use for indexing documents.
        AnalyzerPtr getAnalyzer();
        
        /// Specifies {@link OpenMode} of the index.
        ///
        /// Only takes effect when IndexWriter is first created. 
        IndexWriterConfigPtr setOpenMode(IndexWriterConfig::OpenMode openMode);
        
        /// Returns the {@link OpenMode} set by {@link #setOpenMode(OpenMode)}.
        IndexWriterConfig::OpenMode getOpenMode();
        
        /// Allows an optional {@link IndexDeletionPolicy} implementation to be specified. You can use this to 
        /// control when prior commits are deleted from the index. The default policy is {@link 
        /// KeepOnlyLastCommitDeletionPolicy} which removes all prior commits as soon as a new commit is done 
        /// (this matches behavior before 2.2). Creating your own policy can allow you to explicitly keep 
        /// previous "point in time" commits alive in the index for some time, to allow readers to refresh to 
        /// the new commit without having the old commit deleted out from under them. This is necessary on 
        /// file-systems like NFS that do not support "delete on last close" semantics, which Lucene's 
        /// "point in time" search normally relies on.
        ///
        /// NOTE: the deletion policy cannot be null. If null is passed, the deletion policy will be set to 
        /// the default.
        ///
        /// Only takes effect when IndexWriter is first created. 
        IndexWriterConfigPtr setIndexDeletionPolicy(IndexDeletionPolicyPtr delPolicy);
        
        /// Returns the {@link IndexDeletionPolicy} specified in {@link 
        /// #setIndexDeletionPolicy(IndexDeletionPolicy)} or the default {@link KeepOnlyLastCommitDeletionPolicy}
        IndexDeletionPolicyPtr getIndexDeletionPolicy();
        
        /// Allows to open a certain commit point. The default is null which opens the latest commit point.
        ///
        /// Only takes effect when IndexWriter is first created.
        IndexWriterConfigPtr setIndexCommit(IndexCommitPtr commit);
        
        /// Returns the {@link IndexCommit} as specified in {@link #setIndexCommit(IndexCommit)} or the default, 
        /// null which specifies to open the latest index commit point.
        IndexCommitPtr getIndexCommit();
        
        /// Set the {@link Similarity} implementation used by this IndexWriter.
        /// 
        /// NOTE: the similarity cannot be null. If null is passed, the similarity will be set to the default.
        /// @see Similarity#setDefault(Similarity)
        IndexWriterConfigPtr setSimilarity(SimilarityPtr similarity);
        
        /// Returns the {@link Similarity} implementation used by this IndexWriter. This defaults to the current 
        /// value of {@link Similarity#getDefault()}.
        SimilarityPtr getSimilarity();
        
        /// Set the interval between indexed terms. Large values cause less memory to be used by IndexReader, 
        /// but slow random-access to terms. Small values cause more memory to be used by an IndexReader, and 
        /// speed random-access to terms.
        ///
        /// This parameter determines the amount of computation required per query term, regardless of the number 
        /// of documents that contain that term. In particular, it is the maximum number of other terms that must 
        /// be scanned before a term is located and its frequency and position information may be processed. In a 
        /// large index with user-entered query terms, query processing time is likely to be dominated not by term 
        /// lookup but rather by the processing of frequency and positional data. In a small index or when many
        /// uncommon query terms are generated (eg., by wildcard queries) term lookup may become a dominant cost.
        ///
        /// In particular, numUniqueTerms/interval terms are read into memory by an IndexReader, and, on average, 
        /// interval/2 terms must be scanned for each random term access.
        ///
        /// @see #DEFAULT_TERM_INDEX_INTERVAL
        ///
        /// Takes effect immediately, but only applies to newly flushed/merged segments.
        IndexWriterConfigPtr setTermIndexInterval(int32_t interval);
        
        /// Returns the interval between indexed terms.
        ///
        /// @see #setTermIndexInterval(int)
        int32_t getTermIndexInterval();
        
        /// Sets the merge scheduler used by this writer. The default is {@link ConcurrentMergeScheduler}.
        ///
        /// NOTE: the merge scheduler cannot be null. If null is passed, the merge scheduler will be set to the 
        /// default.
        ///
        /// Only takes effect when IndexWriter is first created.
        IndexWriterConfigPtr setMergeScheduler(MergeSchedulerPtr mergeScheduler);
        
        /// Returns the {@link MergeScheduler} that was set by {@link #setMergeScheduler(MergeScheduler)}
        MergeSchedulerPtr getMergeScheduler();
        
        /// Sets the maximum time to wait for a write lock (in milliseconds) for this instance. You can change 
        /// the default value for all instances by calling {@link #setDefaultWriteLockTimeout(int64_t)}.
        ///
        /// Only takes effect when IndexWriter is first created.
        IndexWriterConfigPtr setWriteLockTimeout(int64_t writeLockTimeout);
        
        /// Returns allowed timeout when acquiring the write lock.
        /// @see #setWriteLockTimeout(int64_t)
        int64_t getWriteLockTimeout();
        
        /// Determines the minimal number of delete terms required before the buffered in-memory delete terms 
        /// are applied and flushed. If there are documents buffered in memory at the time, they are merged and 
        /// a new segment is created.
        ///
        /// Disabled by default (writer flushes by RAM usage).
        ///
        /// Takes effect immediately, but only the next time a document is added, updated or deleted.
        IndexWriterConfigPtr setMaxBufferedDeleteTerms(int32_t maxBufferedDeleteTerms);
        
        /// Returns the number of buffered deleted terms that will trigger a flush if enabled.
        ///
        /// @see #setMaxBufferedDeleteTerms(int)
        int32_t getMaxBufferedDeleteTerms();
        
        /// Determines the amount of RAM that may be used for buffering added documents and deletions before they 
        /// are flushed to the Directory. Generally for faster indexing performance it's best to flush by RAM 
        /// usage instead of document count and use as large a RAM buffer as you can.
        ///
        /// When this is set, the writer will flush whenever buffered documents and deletions use this much RAM. 
        /// Pass in {@link #DISABLE_AUTO_FLUSH} to prevent triggering a flush due to RAM usage. Note that if 
        /// flushing by document count is also enabled, then the flush will be triggered by whichever comes first.
        ///
        /// NOTE: the account of RAM usage for pending deletions is only approximate. Specifically, if you delete 
        /// by Query, Lucene currently has no way to measure the RAM usage of individual Queries so the accounting 
        /// will under-estimate and you should compensate by either calling commit() periodically yourself, or by 
        /// using {@link #setMaxBufferedDeleteTerms(int)} to flush by count instead of RAM usage (each buffered 
        /// delete Query counts as one).
        ///
        /// NOTE: because IndexWriter uses ints when managing its internal storage, the absolute maximum value for 
        /// this setting is somewhat less than 2048 MB. The precise limit depends on various factors, such as
        /// how large your documents are, how many fields have norms, etc., so it's best to set this value 
        /// comfortably under 2048.
        ///
        /// The default value is {@link #DEFAULT_RAM_BUFFER_SIZE_MB}.
        ///
        /// Takes effect immediately, but only the next time a document is added, updated or deleted.
        IndexWriterConfigPtr setRAMBufferSizeMB(double ramBufferSizeMB);
        
        /// Returns the value set by {@link #setRAMBufferSizeMB(double)} if enabled.
        double getRAMBufferSizeMB();
        
        /// Determines the minimal number of documents required before the buffered in-memory documents are flushed 
        /// as a new Segment. Large values generally give faster indexing.
        ///
        /// When this is set, the writer will flush every maxBufferedDocs added documents. Pass in {@link 
        /// #DISABLE_AUTO_FLUSH} to prevent triggering a flush due to number of buffered documents. Note that if 
        /// flushing by RAM usage is also enabled, then the flush will be triggered by whichever comes first.
        ///
        /// Disabled by default (writer flushes by RAM usage).
        ///
        /// Takes effect immediately, but only the next time a document is added, updated or deleted.
        ///
        /// @see #setRAMBufferSizeMB(double)
        IndexWriterConfigPtr setMaxBufferedDocs(int32_t maxBufferedDocs);
        
        /// Returns the number of buffered added documents that will trigger a flush if enabled.
        ///
        /// @see #setMaxBufferedDocs(int32_t)
        int32_t getMaxBufferedDocs();
        
        /// Set the merged segment warmer. See {@link IndexReaderWarmer}.
        ///
        /// Takes effect on the next merge.
        IndexWriterConfigPtr setMergedSegmentWarmer(IndexReaderWarmerPtr mergeSegmentWarmer);
        
        /// Returns the current merged segment warmer. See {@link IndexReaderWarmer}.
        IndexReaderWarmerPtr getMergedSegmentWarmer();
        
        /// {@link MergePolicy} is invoked whenever there are changes to the segments in the index. Its role is 
        /// to select which merges to do, if any, and return a {@link MergeSpecification} describing the merges.
        /// It also selects merges to do for optimize(). The default is {@link LogByteSizeMergePolicy}
        ///
        /// Only takes effect when IndexWriter is first created.
        IndexWriterConfigPtr setMergePolicy(MergePolicyPtr mergePolicy);
        
        /// Returns the current MergePolicy in use by this writer.
        ///
        /// @see #setMergePolicy(MergePolicy)
        MergePolicyPtr getMergePolicy();
        
        /// Sets the max number of simultaneous threads that may be indexing documents at once in IndexWriter. 
        /// Values < 1 are invalid and if passed maxThreadStates will be set to {@link #DEFAULT_MAX_THREAD_STATES}.
        ///
        /// Only takes effect when IndexWriter is first created.
        IndexWriterConfigPtr setMaxThreadStates(int32_t maxThreadStates);
        
        /// Returns the max number of simultaneous threads that may be indexing documents at once in IndexWriter.
        int32_t getMaxThreadStates();
        
        /// By default, IndexWriter does not pool the SegmentReaders it must open for deletions and merging, 
        /// unless a near-real-time reader has been obtained by calling {@link IndexWriter#getReader}.
        /// This method lets you enable pooling without getting a near-real-time reader.  NOTE: if you set this 
        /// to false, IndexWriter will still pool readers once {@link IndexWriter#getReader} is called.
        ///
        /// Only takes effect when IndexWriter is first created.
        IndexWriterConfigPtr setReaderPooling(bool readerPooling);
        
        /// Returns true if IndexWriter should pool readers even if {@link IndexWriter#getReader} has not 
        /// been called.
        bool getReaderPooling();
        
        /// Sets the {@link DocConsumer} chain to be used to process documents.
        ///
        /// Only takes effect when IndexWriter is first created.
        IndexWriterConfigPtr setIndexingChain(IndexingChainPtr indexingChain);
        
        /// Returns the indexing chain set on {@link #setIndexingChain(IndexingChain)}.
        IndexingChainPtr getIndexingChain();
        
        /// Sets the term index divisor passed to any readers that IndexWriter opens, for example when apply 
        /// deletes or creating a near-real-time reader in {@link IndexWriter#getReader}. If you pass -1, the 
        /// terms index won't be loaded by the readers. This is only useful in advanced situations when you 
        /// will only .next() through all terms; attempts to seek will hit an exception.
        ///
        /// Takes effect immediately, but only applies to readers opened after this call
        IndexWriterConfigPtr setReaderTermsIndexDivisor(int32_t divisor);
        
        /// @see #setReaderTermsIndexDivisor(int32_t)
        int32_t getReaderTermsIndexDivisor();
        
        virtual String toString();
    };
}

#endif
