/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef INDEXSEARCHER_H
#define INDEXSEARCHER_H

#include "Searcher.h"

namespace Lucene
{
    /// Implements search over a single IndexReader.
    ///
    /// Applications usually need only call the inherited {@link #search(QueryPtr, int32_t)} or {@link 
    /// #search(QueryPtr, FilterPtr, int32_t)} methods.  For performance reasons it is recommended to open only 
    /// one IndexSearcher and use it for all of your searches.
    ///
    /// NOTE: {@link IndexSearcher} instances are completely thread safe, meaning multiple threads can call any 
    /// of its methods, concurrently.  If your application requires external synchronization, you should not
    /// synchronize on the IndexSearcher instance; use your own (non-Lucene) objects instead.
    class LPPAPI IndexSearcher : public Searcher
    {
    public:
        /// Used for multi-threaded search
        enum ExecutorService
        {
            EXECUTOR,            
            NONE
        };
        
    public:
        /// Creates a searcher searching the index in the named directory.  You should pass readOnly = true, 
        /// since it gives much better concurrent performance, unless you intend to do write operations (delete 
        /// documents or change norms) with the underlying IndexReader.
        /// @param path Directory where IndexReader will be opened
        /// @param readOnly If true, the underlying IndexReader will be opened readOnly
        IndexSearcher(DirectoryPtr path, bool readOnly = true);
        
        /// Creates a searcher searching the provided index.
        /// @param executor Indicates whether it should Run searches for segments in separate threads.
        IndexSearcher(IndexReaderPtr reader, ExecutorService executor = NONE);
        
        /// Directly specify the reader, subReaders and their docID starts.
        /// @param executor Indicates whether it should Run searches for segments in separate threads.
        IndexSearcher(IndexReaderPtr reader, Collection<IndexReaderPtr> subReaders, Collection<int32_t> docStarts, ExecutorService executor = NONE);
        
        virtual ~IndexSearcher();
        
        LUCENE_CLASS(IndexSearcher);
    
    public:
        IndexReaderPtr reader;
    
    protected:
        bool closeReader;
        
        Collection<IndexReaderPtr> subReaders;
        Collection<int32_t> docStarts;
        
        // Only used for multi-threaded search
        ExecutorService executor;
        Collection<IndexSearcherPtr> subSearchers;
        
        bool fieldSortDoTrackScores;
        bool fieldSortDoMaxScore;
    
    private:
        /// The Similarity implementation used by this searcher.
        SimilarityPtr similarity;
    
    public:
        /// Return the {@link IndexReader} this searches.
        IndexReaderPtr getIndexReader();
        
        /// Returns the atomic subReaders used by this searcher.
        Collection<IndexReaderPtr> getSubReaders();
    
        /// Returns one greater than the largest possible document number.
        virtual int32_t maxDoc();
        
        /// Returns total docFreq for this term.
        virtual int32_t docFreq(TermPtr term);
        
        virtual DocumentPtr doc(int32_t n);
        virtual DocumentPtr doc(int32_t n, FieldSelectorPtr fieldSelector);
        
        /// Set the Similarity implementation used by this Searcher.
        /// @see Similarity#setDefault(Similarity)
        virtual void setSimilarity(SimilarityPtr similarity);
        
        virtual SimilarityPtr getSimilarity();
        
        /// Note that the underlying IndexReader is not closed, if IndexSearcher was constructed with 
        /// IndexSearcher(IndexReaderPtr reader).  If the IndexReader was supplied implicitly by specifying a 
        /// directory, then the IndexReader is closed.
        virtual void close();
        
        using Searcher::search;
        using Searcher::explain;
        
        /// Finds the top n hits for query.
        virtual TopDocsPtr search(QueryPtr query, int32_t n);
        
        /// Finds the top n hits for query, applying filter if non-null.
        virtual TopDocsPtr search(QueryPtr query, FilterPtr filter, int32_t n);
        
        /// Lower-level search API.
        ///
        /// {@link Collector#collect(int32_t)} is called for every matching document. Collector-based access to 
        /// remote indexes is discouraged.
        ///
        /// Applications should only use this if they need all of the matching documents.  The high-level search 
        /// API ({@link Searcher#search(QueryPtr, FilterPtr, int32_t)}) is usually more efficient, as it skips
        /// non-high-scoring hits.
        ///
        /// @param query To match documents
        /// @param filter If non-null, used to permit documents to be collected.
        /// @param results To receive hits
        virtual void search(QueryPtr query, FilterPtr filter, CollectorPtr results);
        
        /// Lower-level search API.
        ///
        /// {@link Collector#collect(int32_t)} is called for every matching document.
        ///
        /// Applications should only use this if they need all of the matching documents.  The high-level 
        /// search API ({@link Searcher#search(QueryPtr, int32_t)}) is usually more efficient, as it skips
        /// non-high-scoring hits.
        ///
        /// Note: The score passed to this method is a raw score.  In other words, the score will not necessarily 
        /// be a double whose value is between 0 and 1.
        virtual void search(QueryPtr query, CollectorPtr results);
        
        /// Search implementation with arbitrary sorting.  Finds the top n hits for query, applying filter if 
        /// non-null, and sorting the hits by the criteria in sort.
        ///
        /// NOTE: this does not compute scores by default; use {@link IndexSearcher#setDefaultFieldSortScoring} 
        /// to enable scoring.
        virtual TopFieldDocsPtr search(QueryPtr query, FilterPtr filter, int32_t n, SortPtr sort);
        
        /// Search implementation with arbitrary sorting and no filter.
        /// @param query The query to search for
        /// @param n Return only the top n results
        /// @param sort The {@link Sort} object
        /// @return The top docs, sorted according to the supplied {@link Sort} instance
        virtual TopFieldDocsPtr search(QueryPtr query, int32_t n, SortPtr sort);
        
        /// Low-level search implementation.  Finds the top n hits for query, applying filter if non-null.
        ///
        /// Applications should usually call {@link Searcher#search(Query,int)} or {@link 
        /// Searcher#search(Query,Filter,int)} instead.
        virtual TopDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n);
        
        /// Low-level search implementation with arbitrary sorting.  Finds the top n hits for query, applying
        /// filter if non-null, and sorting the hits by the criteria in sort.
        ///
        /// Applications should usually call {@link Searcher#search(Query,Filter,int,Sort)} instead.
        virtual TopFieldDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort);
        
        /// Lower-level search API.
        ///
        /// {@link Collector#collect(int)} is called for every document.
        /// Collector-based access to remote indexes is discouraged.
        ///
        /// Applications should only use this if they need all of the matching documents. The high-level search 
        /// API ({@link Searcher#search(Query,int)}) is usually more efficient, as it skips non-high-scoring hits.
        ///
        /// @param weight to match documents
        /// @param filter if non-null, used to permit documents to be collected.
        /// @param results to receive hits
        virtual void search(WeightPtr weight, FilterPtr filter, CollectorPtr results);
        
        /// Called to re-write queries into primitive queries.
        virtual QueryPtr rewrite(QueryPtr query);
        
        /// Returns an Explanation that describes how doc scored against query.
        ///
        /// This is intended to be used in developing Similarity implementations, and, for good performance, 
        /// should not be displayed with every hit. Computing an explanation is as expensive as executing the
        /// query over the entire index.
        virtual ExplanationPtr explain(QueryPtr query, int32_t doc);
        
        /// Low-level implementation method
        ///
        /// Returns an Explanation that describes how doc scored against weight.
        ///
        /// This is intended to be used in developing Similarity implementations, and, for good performance, 
        /// should not be displayed with every hit. Computing an explanation is as expensive as executing the
        /// query over the entire index.
        ///
        /// Applications should call {@link Searcher#explain(Query, int)}.
        virtual ExplanationPtr explain(WeightPtr weight, int32_t doc);
        
        /// By default, no scores are computed when sorting by field (using {@link #search(QueryPtr, FilterPtr,
        /// int32_t, SortPtr)}).  You can change that, per IndexSearcher instance, by calling this method.  Note 
        /// that this will incur a CPU cost.
        ///
        /// @param doTrackScores If true, then scores are returned for every matching document in {@link TopFieldDocs}.
        /// @param doMaxScore If true, then the max score for all matching docs is computed.
        virtual void setDefaultFieldSortScoring(bool doTrackScores, bool doMaxScore);
    
    protected:
        void ConstructSearcher(IndexReaderPtr reader, bool closeReader, ExecutorService executor);
        
        /// Just like {@link #search(WeightPtr, FilterPtr, int32_t, SortPtr)}, but you choose whether or not the 
        /// fields in the returned {@link FieldDoc} instances should be set by specifying fillFields.
        ///
        /// NOTE: this does not compute scores by default.  If you need scores, create a {@link TopFieldCollector}
        /// instance by calling {@link TopFieldCollector#create} and then pass that to {@link #search(WeightPtr,
        /// FilterPtr, CollectorPtr)}.
        virtual TopFieldDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort, bool fillFields);
        
        void gatherSubReaders(Collection<IndexReaderPtr> allSubReaders, IndexReaderPtr reader);
        void searchWithFilter(IndexReaderPtr reader, WeightPtr weight, FilterPtr filter, CollectorPtr collector);

        /// Creates a weight for query.
        /// @return New weight
        virtual WeightPtr createWeight(QueryPtr query);
    };
}

#endif
