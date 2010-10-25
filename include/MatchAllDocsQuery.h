/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MATCHALLDOCSQUERY_H
#define MATCHALLDOCSQUERY_H

#include "Query.h"
#include "Weight.h"
#include "Scorer.h"

namespace Lucene
{
    /// A query that matches all documents.
    class LPPAPI MatchAllDocsQuery : public Query
    {
    public:
        /// @param normsField Field used for normalization factor (document boost). Null if nothing.
        MatchAllDocsQuery(const String& normsField = L"");
        
        virtual ~MatchAllDocsQuery();
    
        LUCENE_CLASS(MatchAllDocsQuery);
    
    protected:
        String normsField;
    
    public:
        using Query::toString;
    
        virtual WeightPtr createWeight(SearcherPtr searcher);
        virtual void extractTerms(SetTerm terms);
        virtual String toString(const String& field);
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        friend class MatchAllDocsWeight;
    };
    
    class LPPAPI MatchAllDocsWeight : public Weight
    {
    public:
        MatchAllDocsWeight(MatchAllDocsQueryPtr query, SearcherPtr searcher);
        virtual ~MatchAllDocsWeight();
    
        LUCENE_CLASS(MatchAllDocsWeight);
    
    protected:
        MatchAllDocsQueryPtr query;
        SimilarityPtr similarity;
        double queryWeight;
        double queryNorm;
    
    public:
        virtual String toString();
        virtual QueryPtr getQuery();
        virtual double getValue();
        virtual double sumOfSquaredWeights();
        virtual void normalize(double norm);
        virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
        virtual ExplanationPtr explain(IndexReaderPtr reader, int32_t doc);
    };
    
    class LPPAPI MatchAllScorer : public Scorer
    {
    public:
        MatchAllScorer(MatchAllDocsQueryPtr query, IndexReaderPtr reader, SimilarityPtr similarity, WeightPtr weight, ByteArray norms);
        virtual ~MatchAllScorer();
    
        LUCENE_CLASS(MatchAllScorer);
    
    public:
        TermDocsPtr termDocs;
        double _score;
        ByteArray norms;
    
    protected:
        MatchAllDocsQueryPtr query;
        int32_t doc;
    
    public:
        virtual int32_t docID();
        virtual int32_t nextDoc();
        virtual double score();
        virtual int32_t advance(int32_t target);
    };
}

#endif
