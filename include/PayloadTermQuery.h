/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PAYLOADTERMQUERY_H
#define PAYLOADTERMQUERY_H

#include "SpanTermQuery.h"
#include "SpanWeight.h"
#include "SpanScorer.h"

namespace Lucene
{
    /// This class is very similar to {@link SpanTermQuery} except that it factors in the value of the payload 
    /// located at each of the positions where the {@link Term} occurs.
    ///
    /// In order to take advantage of this, you must override {@link Similarity#scorePayload(int32_t, const String&, 
    /// int32_t, int32_t, ByteArray, int32_t, int32_t)} which returns 1 by default.
    ///
    /// Payload scores are aggregated using a pluggable {@link PayloadFunction}.
    class LPPAPI PayloadTermQuery : public SpanTermQuery
    {
    public:
        PayloadTermQuery(TermPtr term, PayloadFunctionPtr function, bool includeSpanScore = true);
        virtual ~PayloadTermQuery();
        
        LUCENE_CLASS(PayloadTermQuery);
    
    protected:
        PayloadFunctionPtr function;
        bool includeSpanScore;
    
    public:
        virtual WeightPtr createWeight(SearcherPtr searcher);
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        
        friend class PayloadTermWeight;
        friend class PayloadTermSpanScorer;
    };
    
    class LPPAPI PayloadTermWeight : public SpanWeight
    {
    public:
        PayloadTermWeight(PayloadTermQueryPtr query, SearcherPtr searcher);
        virtual ~PayloadTermWeight();
        
        LUCENE_CLASS(PayloadTermWeight);
    
    public:
        virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
    };
    
    class LPPAPI PayloadTermSpanScorer : public SpanScorer
    {
    public:
        PayloadTermSpanScorer(TermSpansPtr spans, WeightPtr weight, SimilarityPtr similarity, ByteArray norms);
        virtual ~PayloadTermSpanScorer();
        
        LUCENE_CLASS(PayloadTermSpanScorer);
    
    protected:
        ByteArray payload;
        TermPositionsPtr positions;
        double payloadScore;
        int32_t payloadsSeen;
    
    public:
        virtual double score();
    
    protected:
        virtual bool setFreqCurrentDoc();
        
        void processPayload(SimilarityPtr similarity);
        
        /// Returns the SpanScorer score only.
        ///
        /// Should not be overridden without good cause
        ///
        /// @return the score for just the Span part without the payload
        /// @see #score()
        virtual double getSpanScore();
        
        /// The score for the payload
        ///
        /// @return The score, as calculated by {@link PayloadFunction#docScore(int32_t, const String&, 
        /// int32_t, double)}
        virtual double getPayloadScore();
        
        virtual ExplanationPtr explain(int32_t doc);
    };    
}

#endif
