/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SEARCHER_H
#define SEARCHER_H

#include "Searchable.h"

namespace Lucene
{
    /// An abstract base class for search implementations. Implements the main search methods.
    ///
    /// Note that you can only access hits from a Searcher as long as it is not yet closed, otherwise an IO 
    /// exception will be thrown.
    class LPPAPI Searcher : public Searchable, public LuceneObject
    {
    public:
        Searcher();
        virtual ~Searcher();
        
        LUCENE_CLASS(Searcher);
    
    protected:
        /// The Similarity implementation used by this searcher.
        SimilarityPtr similarity;
    
    public:
        /// Search implementation with arbitrary sorting.  Finds the top n hits for query, applying filter if 
        /// non-null, and sorting the hits by the criteria in sort.
        ///
        /// NOTE: this does not compute scores by default; use {@link IndexSearcher#setDefaultFieldSortScoring} 
        /// to enable scoring.
        virtual TopFieldDocsPtr search(QueryPtr query, FilterPtr filter, int32_t n, SortPtr sort);
        
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
        
        /// Lower-level search API.
        ///
        /// {@link Collector#collect(int32_t)} is called for every matching document.  Collector-based access to 
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
        
        /// Finds the top n hits for query, applying filter if non-null.
        virtual TopDocsPtr search(QueryPtr query, FilterPtr filter, int32_t n);
        
        /// Finds the top n hits for query.
        virtual TopDocsPtr search(QueryPtr query, int32_t n);
        
        /// Returns an Explanation that describes how doc scored against query.
        ///
        /// This is intended to be used in developing Similarity implementations, and for good performance, 
        /// should not be displayed with every hit.  Computing an explanation is as expensive as executing the 
        /// query over the entire index.
        virtual ExplanationPtr explain(QueryPtr query, int32_t doc);
        
        /// Set the Similarity implementation used by this Searcher.
        virtual void setSimilarity(SimilarityPtr similarity);
        
        /// Return the Similarity implementation used by this Searcher.
        ///
        /// This defaults to the current value of {@link Similarity#getDefault()}.
        virtual SimilarityPtr getSimilarity();
        
        virtual Collection<int32_t> docFreqs(Collection<TermPtr> terms);
                             
        virtual void search(WeightPtr weight, FilterPtr filter, CollectorPtr results) = 0;
        virtual void close() = 0;
        virtual int32_t docFreq(TermPtr term) = 0;
        virtual int32_t maxDoc() = 0;
        virtual TopDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n) = 0;
        virtual DocumentPtr doc(int32_t n) = 0;
        virtual DocumentPtr doc(int32_t n, FieldSelectorPtr fieldSelector) = 0;
        virtual QueryPtr rewrite(QueryPtr query) = 0;
        virtual ExplanationPtr explain(WeightPtr weight, int32_t doc) = 0;
        virtual TopFieldDocsPtr search(WeightPtr weight, FilterPtr filter, int32_t n, SortPtr sort) = 0;
    
    protected:
        /// Creates a weight for query.
        /// @return New weight
        virtual WeightPtr createWeight(QueryPtr query);
    };
}

#endif
