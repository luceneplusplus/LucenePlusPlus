/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CUSTOMSCOREQUERY_H
#define CUSTOMSCOREQUERY_H

#include "Query.h"

namespace Lucene
{
    /// Query that sets document score as a programmatic function of several (sub) scores:
    /// <ol>
    /// <li>the score of its subQuery (any query)
    /// <li>(optional) the score of its ValueSourceQuery (or queries).  For most simple/convenient use cases 
    /// this query is likely to be a {@link FieldScoreQuery}
    /// </ol>
    /// Subclasses can modify the computation by overriding {@link #getCustomScoreProvider}.
    class LPPAPI CustomScoreQuery : public Query
    {
    public:
        /// Create a CustomScoreQuery over input subQuery.
        /// @param subQuery the sub query whose scored is being customized. Must not be null. 
        CustomScoreQuery(QueryPtr subQuery);
        
        /// Create a CustomScoreQuery over input subQuery and a {@link ValueSourceQuery}.
        /// @param subQuery the sub query whose score is being customized. Must not be null.
        /// @param valSrcQuery a value source query whose scores are used in the custom score computation. For 
        /// most simple/convenient use case this would be a {@link FieldScoreQuery}.  This parameter is 
        /// optional - it can be null.
        CustomScoreQuery(QueryPtr subQuery, ValueSourceQueryPtr valSrcQuery);
        
        /// Create a CustomScoreQuery over input subQuery and a {@link ValueSourceQuery}.
        /// @param subQuery the sub query whose score is being customized. Must not be null.
        /// @param valSrcQueries value source queries whose scores are used in the custom score computation. 
        /// For most simple/convenient use case these would be {@link FieldScoreQueries}.  This parameter is 
        /// optional - it can be null or even an empty array.
        CustomScoreQuery(QueryPtr subQuery, Collection<ValueSourceQueryPtr> valSrcQueries);
        
        virtual ~CustomScoreQuery();
    
        LUCENE_CLASS(CustomScoreQuery);
    
    protected:
        QueryPtr subQuery;
        Collection<ValueSourceQueryPtr> valSrcQueries; // never null (empty array if there are no valSrcQueries).
        bool strict; // if true, valueSource part of query does not take part in weights normalization.
    
    public:
        using Query::toString;
        
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        virtual void extractTerms(SetTerm terms);
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual String toString(const String& field);
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        
        virtual WeightPtr createWeight(SearcherPtr searcher);
        
        /// Checks if this is strict custom scoring.  In strict custom scoring, the ValueSource part does not 
        /// participate in weight normalization.  This may be useful when one wants full control over how scores 
        /// are modified, and does not care about normalizing by the ValueSource part.  One particular case where 
        /// this is useful if for testing this query.   
        ///
        /// Note: only has effect when the ValueSource part is not null.
        virtual bool isStrict();
        
        /// Set the strict mode of this query. 
        /// @param strict The strict mode to set.
        /// @see #isStrict()
        virtual void setStrict(bool strict);
        
        /// A short name of this query, used in {@link #toString(String)}.
        virtual String name();
    
    protected:
        void ConstructQuery(QueryPtr subQuery, Collection<ValueSourceQueryPtr> valSrcQueries);
        
        /// Returns a {@link CustomScoreProvider} that calculates the custom scores for the given {@link 
        /// IndexReader}.  The default implementation returns a default implementation as specified in 
        /// the docs of {@link CustomScoreProvider}.
        virtual CustomScoreProviderPtr getCustomScoreProvider(IndexReaderPtr reader);
        
        friend class CustomWeight;
        friend class CustomScorer;
    };
}

#endif
