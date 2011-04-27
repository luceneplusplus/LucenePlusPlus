/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CONSTANTSCOREQUERY_H
#define CONSTANTSCOREQUERY_H

#include "Query.h"
#include "Weight.h"
#include "Scorer.h"

namespace Lucene
{
    /// A query that wraps another query or a filter and simply returns a constant score equal to the
    /// query boost for every document that matches the filter or query. For queries it therefore 
    /// simply strips of all scores and returns a constant one.
    ///
    /// NOTE: if the wrapped filter is an instance of {@link CachingWrapperFilter}, you'll likely want 
    /// to enforce deletions in the filter (using either {@link CachingWrapperFilter.DeletesMode#RECACHE}
    /// or {@link CachingWrapperFilter.DeletesMode#DYNAMIC}).
    class LPPAPI ConstantScoreQuery : public Query
    {
    public:
        /// Strips off scores from the passed in Query. The hits will get a constant score dependent on 
        /// the boost factor of this query.
        ConstantScoreQuery(QueryPtr query);
        
        /// Wraps a Filter as a Query. The hits will get a constant score dependent on the boost factor 
        /// of this query. If you simply want to strip off scores from a Query, no longer use {@code 
        /// new ConstantScoreQuery(new QueryWrapperFilter(query))}, instead use {@link 
        /// #ConstantScoreQuery(Query)}
        ConstantScoreQuery(FilterPtr filter);
        
        virtual ~ConstantScoreQuery();
    
        LUCENE_CLASS(ConstantScoreQuery);
    
    protected:
        FilterPtr filter;
        QueryPtr query;
    
    public:
        /// Returns the encapsulated filter, returns {@code null} if a query is wrapped.
        FilterPtr getFilter();
        
        /// Returns the encapsulated query, returns {@code null} if a filter is wrapped.
        QueryPtr getQuery();
        
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        virtual void extractTerms(SetTerm terms);
        
        virtual WeightPtr createWeight(SearcherPtr searcher);
        
        using Query::toString;
        virtual String toString(const String& field);
        
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        friend class ConstantWeight;
        friend class ConstantScorer;
        friend class ConstantCollector;
    };
}

#endif
