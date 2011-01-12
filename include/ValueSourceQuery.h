/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef VALUESOURCEQUERY_H
#define VALUESOURCEQUERY_H

#include "Query.h"
#include "Weight.h"
#include "Scorer.h"

namespace Lucene
{
    /// A Query that sets the scores of document to the values obtained from a {@link ValueSource}.
    ///
    /// This query provides a score for each and every undeleted document in the index.
    ///
    /// The value source can be based on a (cached) value of an indexed field, but it can also be based on an 
    /// external source, eg. values read from an external database. 
    ///
    /// Score is set as: Score(doc,query) = (query.getBoost() * query.getBoost()) * valueSource(doc).
    class LPPAPI ValueSourceQuery : public Query
    {
    public:
        /// Create a value source query
        /// @param valSrc provides the values defines the function to be used for scoring
        ValueSourceQuery(ValueSourcePtr valSrc);
        
        virtual ~ValueSourceQuery();
    
        LUCENE_CLASS(ValueSourceQuery);
    
    public:
        ValueSourcePtr valSrc;
    
    public:
        using Query::toString;
        
        virtual QueryPtr rewrite(IndexReaderPtr reader);
        virtual void extractTerms(SetTerm terms);
        virtual WeightPtr createWeight(SearcherPtr searcher);
        virtual String toString(const String& field);
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
    };
    
    class LPPAPI ValueSourceWeight : public Weight
    {
    public:
        ValueSourceWeight(ValueSourceQueryPtr query, SearcherPtr searcher);
        virtual ~ValueSourceWeight();
        
        LUCENE_CLASS(ValueSourceWeight);
    
    public:
        ValueSourceQueryPtr query;
        SimilarityPtr similarity;
        double queryNorm;
        double queryWeight;
    
    public:
        virtual QueryPtr getQuery();
        virtual double getValue();
        virtual double sumOfSquaredWeights();
        virtual void normalize(double norm);
        virtual ScorerPtr scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer);
        virtual ExplanationPtr explain(IndexReaderPtr reader, int32_t doc);
    };
    
    /// A scorer that (simply) matches all documents, and scores each document with the value of the value 
    /// source in effect. As an example, if the value source is a (cached) field source, then value of that
    /// field in that document will be used. (assuming field is indexed for this doc, with a single token.) 
    class LPPAPI ValueSourceScorer : public Scorer
    {
    public:
        ValueSourceScorer(SimilarityPtr similarity, IndexReaderPtr reader, ValueSourceWeightPtr weight);
        virtual ~ValueSourceScorer();
    
        LUCENE_CLASS(ValueSourceScorer);
    
    public:
        ValueSourceWeightPtr weight;
        double qWeight;
        DocValuesPtr vals;
        TermDocsPtr termDocs;
        int32_t doc;
        
    public:
        virtual int32_t nextDoc();
        virtual int32_t docID();
        virtual int32_t advance(int32_t target);
        virtual double score();
    };
}

#endif
