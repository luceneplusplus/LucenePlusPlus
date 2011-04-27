/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FUZZYQUERY_H
#define FUZZYQUERY_H

#include "MultiTermQuery.h"

namespace Lucene
{
    /// Implements the fuzzy search query.  The similarity measurement is based on the Levenshtein (edit 
    /// distance) algorithm.
    ///
    /// Warning: this query is not very scalable with its default prefix length of 0 - in this case, *every* 
    /// term will be enumerated and cause an edit score calculation.
    ///
    /// This query uses {@link MultiTermQuery.TopTermsScoringBooleanQueryRewrite} as default. So terms will 
    /// be collected and scored according to their edit distance. Only the top terms are used for building 
    /// the {@link BooleanQuery}.
    /// It is not recommended to change the rewrite mode for fuzzy queries.
    class LPPAPI FuzzyQuery : public MultiTermQuery
    {
    public:
        /// Create a new FuzzyQuery that will match terms with a similarity of at least minimumSimilarity 
        /// to term.  If a prefixLength > 0 is specified, a common prefix of that length is also required.
        /// @param term The term to search for
        /// @param minimumSimilarity A value between 0 and 1 to set the required similarity between the query 
        /// term and the matching terms.  For example, for a minimumSimilarity of 0.5 a term of the same 
        /// length as the query term is considered similar to the query term if the edit distance between 
        /// both terms is less than length(term) * 0.5
        /// @param prefixLength Length of common (non-fuzzy) prefix
        /// @param maxExpansions the maximum number of terms to match. If this number is greater than {@link 
        /// BooleanQuery#getMaxClauseCount} when the query is rewritten, then the maxClauseCount will be 
        /// used instead.
        FuzzyQuery(TermPtr term, double minimumSimilarity, int32_t prefixLength, int32_t maxExpansions);
        
        FuzzyQuery(TermPtr term, double minimumSimilarity, int32_t prefixLength);
        
        FuzzyQuery(TermPtr term, double minimumSimilarity);
        FuzzyQuery(TermPtr term);
        
        virtual ~FuzzyQuery();
    
        LUCENE_CLASS(FuzzyQuery);
    
    protected:
        double minimumSimilarity;
        int32_t prefixLength;
        bool termLongEnough;
        
        TermPtr term;
    
    public:
        static double defaultMinSimilarity();
        static const int32_t defaultPrefixLength;
        static const int32_t defaultMaxExpansions;
    
    public:
        using MultiTermQuery::toString;
        
        /// Returns the minimum similarity that is required for this query to match.
        /// @return float value between 0.0 and 1.0
        double getMinSimilarity();
        
        /// Returns the non-fuzzy prefix length. This is the number of characters at the start of a term that 
        /// must be identical (not fuzzy) to the query term if the query is to match that term. 
        int32_t getPrefixLength();
        
        /// Returns the pattern term.
        TermPtr getTerm();
        
        virtual String getField();
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual String toString(const String& field);
        virtual int32_t hashCode();
        virtual bool equals(LuceneObjectPtr other);
    
    protected:
        void ConstructQuery(TermPtr term, double minimumSimilarity, int32_t prefixLength, int32_t maxExpansions);
        
        virtual FilteredTermEnumPtr getEnum(IndexReaderPtr reader);
    };
}

#endif
