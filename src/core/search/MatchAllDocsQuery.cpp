/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "MatchAllDocsQuery.h"
#include "_MatchAllDocsQuery.h"
#include "IndexReader.h"
#include "Similarity.h"
#include "TermDocs.h"
#include "ComplexExplanation.h"
#include "Searcher.h"
#include "MiscUtils.h"

namespace Lucene
{
    MatchAllDocsQuery::MatchAllDocsQuery(const String& normsField)
    {
        this->normsField = normsField;
    }
    
    MatchAllDocsQuery::~MatchAllDocsQuery()
    {
    }
    
    WeightPtr MatchAllDocsQuery::createWeight(SearcherPtr searcher)
    {
        return newLucene<MatchAllDocsWeight>(shared_from_this(), searcher);
    }
    
    void MatchAllDocsQuery::extractTerms(SetTerm terms)
    {
    }
    
    String MatchAllDocsQuery::toString(const String& field)
    {
        StringStream buffer;
        buffer << L"*:*" << boostString();
        return buffer.str();
    }
    
    bool MatchAllDocsQuery::equals(LuceneObjectPtr other)
    {
        return Query::equals(other);
    }
    
    int32_t MatchAllDocsQuery::hashCode()
    {
        return MiscUtils::doubleToIntBits(getBoost()) ^ 0x1aa71190;
    }
    
    LuceneObjectPtr MatchAllDocsQuery::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = other ? other : newLucene<MatchAllDocsQuery>();
        MatchAllDocsQueryPtr cloneQuery(boost::dynamic_pointer_cast<MatchAllDocsQuery>(Query::clone(clone)));
        cloneQuery->normsField = normsField;
        return cloneQuery;
    }
    
    MatchAllDocsWeight::MatchAllDocsWeight(MatchAllDocsQueryPtr query, SearcherPtr searcher)
    {
        this->query = query;
        this->similarity = searcher->getSimilarity();
        this->queryWeight = 0.0;
        this->queryNorm = 0.0;
    }
    
    MatchAllDocsWeight::~MatchAllDocsWeight()
    {
    }
    
    String MatchAllDocsWeight::toString()
    {
        StringStream buffer;
        buffer << L"weight(" << queryWeight << L", " << queryNorm << L")";
        return buffer.str();
    }
    
    QueryPtr MatchAllDocsWeight::getQuery()
    {
        return query;
    }
    
    double MatchAllDocsWeight::getValue()
    {
        return queryWeight;
    }
    
    double MatchAllDocsWeight::sumOfSquaredWeights()
    {
        queryWeight = getQuery()->getBoost();
        return queryWeight * queryWeight;
    }
    
    void MatchAllDocsWeight::normalize(double norm)
    {
        this->queryNorm = norm;
        queryWeight *= this->queryNorm;
    }
    
    ScorerPtr MatchAllDocsWeight::scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer)
    {
        return newLucene<MatchAllScorer>(query, reader, similarity, shared_from_this(), !query->normsField.empty() ? reader->norms(query->normsField) : ByteArray());
    }
    
    ExplanationPtr MatchAllDocsWeight::explain(IndexReaderPtr reader, int32_t doc)
    {
        // explain query weight
        ExplanationPtr queryExpl(newLucene<ComplexExplanation>(true, getValue(), L"MatchAllDocsQuery, product of:"));
        if (getQuery()->getBoost() != 1.0)
            queryExpl->addDetail(newLucene<Explanation>(getQuery()->getBoost(), L"boost"));
        queryExpl->addDetail(newLucene<Explanation>(queryNorm, L"queryNorm"));
        return queryExpl;
    }
    
    MatchAllScorer::MatchAllScorer(MatchAllDocsQueryPtr query, IndexReaderPtr reader, SimilarityPtr similarity, WeightPtr weight, ByteArray norms) : Scorer(similarity)
    {
        this->query = query;
        this->termDocs = reader->termDocs(TermPtr());
        this->_score = weight->getValue();
        this->norms = norms;
        this->doc = -1;
    }
    
    MatchAllScorer::~MatchAllScorer()
    {
    }
    
    int32_t MatchAllScorer::docID()
    {
        return doc;
    }
    
    int32_t MatchAllScorer::nextDoc()
    {
        doc = termDocs->next() ? termDocs->doc() : NO_MORE_DOCS;
        return doc;
    }
    
    double MatchAllScorer::score()
    {
        return norms ? _score * Similarity::decodeNorm(norms[docID()]) : _score;
    }
    
    int32_t MatchAllScorer::advance(int32_t target)
    {
        doc = termDocs->skipTo(target) ? termDocs->doc() : NO_MORE_DOCS;
        return doc;
    }
}
