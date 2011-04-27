/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ConstantScoreQuery.h"
#include "_ConstantScoreQuery.h"
#include "Filter.h"
#include "ComplexExplanation.h"
#include "DocIdSet.h"
#include "MiscUtils.h"
#include "StringUtils.h"

namespace Lucene
{
    ConstantScoreQuery::ConstantScoreQuery(QueryPtr query)
    {
        if (!query)
            boost::throw_exception(NullPointerException(L"Query may not be null"));
        this->query = query;
    }
    
    ConstantScoreQuery::ConstantScoreQuery(FilterPtr filter)
    {
        if (!filter)
            boost::throw_exception(NullPointerException(L"Filter may not be null"));
        this->filter = filter;
    }
    
    ConstantScoreQuery::~ConstantScoreQuery()
    {
    }
    
    FilterPtr ConstantScoreQuery::getFilter()
    {
        return filter;
    }
    
    QueryPtr ConstantScoreQuery::getQuery()
    {
        return query;
    }
    
    QueryPtr ConstantScoreQuery::rewrite(IndexReaderPtr reader)
    {
        if (query)
        {
            QueryPtr rewritten(query->rewrite(reader));
            if (rewritten != query)
            {
                rewritten = newLucene<ConstantScoreQuery>(rewritten);
                rewritten->setBoost(this->getBoost());
                return rewritten;
            }
        }
        return shared_from_this();
    }
    
    void ConstantScoreQuery::extractTerms(SetTerm terms)
    {
        // OK to not add any terms when used for MultiSearcher, but may not be OK for highlighting
        // If a query was wrapped, we delegate to query.
        if (query)
            query->extractTerms(terms);
    }
    
    WeightPtr ConstantScoreQuery::createWeight(SearcherPtr searcher)
    {
        return newLucene<ConstantWeight>(shared_from_this(), searcher);
    }
    
    String ConstantScoreQuery::toString(const String& field)
    {
        StringStream buffer;
        buffer << L"ConstantScore(" << (query ? query->toString(field) : filter->toString());
        buffer << L")" << boostString();
        return buffer.str();
    }
    
    bool ConstantScoreQuery::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        
        if (!Query::equals(other))
            return false;
        ConstantScoreQueryPtr otherConstantScoreQuery(boost::dynamic_pointer_cast<ConstantScoreQuery>(other));
        if (!otherConstantScoreQuery)
            return false;

        return (!this->filter ? !otherConstantScoreQuery->filter : this->filter->equals(otherConstantScoreQuery->filter)) &&
               (!this->query ? !otherConstantScoreQuery->query : this->query->equals(otherConstantScoreQuery->query));
    }
    
    int32_t ConstantScoreQuery::hashCode()
    {
        // Simple add is OK since no existing filter hashcode has a float component.
        return 31 * Query::hashCode() + (query ? query->hashCode() : filter->hashCode());
    }
    
    LuceneObjectPtr ConstantScoreQuery::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = other ? other : newLucene<ConstantScoreQuery>(filter);
        ConstantScoreQueryPtr cloneQuery(boost::static_pointer_cast<ConstantScoreQuery>(Query::clone(clone)));
        cloneQuery->filter = filter;
        cloneQuery->query = query;
        return cloneQuery;
    }
    
    ConstantWeight::ConstantWeight(ConstantScoreQueryPtr constantScorer, SearcherPtr searcher)
    {
        this->constantScorer = constantScorer;
        this->similarity = constantScorer->getSimilarity(searcher);
        this->queryNorm = 0;
        this->queryWeight = 0;
        this->innerWeight = !constantScorer->query ? WeightPtr() : constantScorer->query->createWeight(searcher);
    }
    
    ConstantWeight::~ConstantWeight()
    {
    }
    
    QueryPtr ConstantWeight::getQuery()
    {
        return constantScorer;
    }
    
    double ConstantWeight::getValue()
    {
        return queryWeight;
    }
    
    double ConstantWeight::sumOfSquaredWeights()
    {
        // we calculate sumOfSquaredWeights of the inner weight, but ignore it (just to initialize everything)
        if (innerWeight)
            innerWeight->sumOfSquaredWeights();
        queryWeight = constantScorer->getBoost();
        return queryWeight * queryWeight;
    }
    
    void ConstantWeight::normalize(double norm)
    {
        this->queryNorm = norm;
        queryWeight *= this->queryNorm;
        // we normalize the inner weight, but ignore it (just to initialize everything)
        if (innerWeight)
            innerWeight->normalize(norm);
    }
    
    ScorerPtr ConstantWeight::scorer(IndexReaderPtr reader, bool scoreDocsInOrder, bool topScorer)
    {
        DocIdSetIteratorPtr disi;
        if (constantScorer->filter)
        {
            BOOST_ASSERT(!constantScorer->query);
            DocIdSetPtr dis(constantScorer->filter->getDocIdSet(reader));
            if (!dis)
                return ScorerPtr();
            disi = dis->iterator();
        }
        else
        {
            BOOST_ASSERT(constantScorer->query && innerWeight);
            disi = innerWeight->scorer(reader, scoreDocsInOrder, topScorer);
        }
        if (!disi)
            return ScorerPtr();
        return newLucene<ConstantScorer>(similarity, disi, shared_from_this());
    }
    
    bool ConstantWeight::scoresDocsOutOfOrder()
    {
        return innerWeight ? innerWeight->scoresDocsOutOfOrder() : false;
    }
    
    ExplanationPtr ConstantWeight::explain(IndexReaderPtr reader, int32_t doc)
    {
        ScorerPtr cs(scorer(reader, true, false));
        bool exists = (cs && cs->advance(doc) == doc);

        ComplexExplanationPtr result(newLucene<ComplexExplanation>());
        if (exists)
        {
            result->setDescription(constantScorer->toString() + L", product of:");
            result->setValue(queryWeight);
            result->setMatch(true);
            result->addDetail(newLucene<Explanation>(constantScorer->getBoost(), L"boost"));
            result->addDetail(newLucene<Explanation>(queryNorm, L"queryNorm"));
        }
        else
        {
            result->setDescription(constantScorer->toString() + L" doesn't match id " + StringUtils::toString(doc));
            result->setValue(0);
            result->setMatch(false);
        }
        return result;
    }
    
    ConstantScorer::ConstantScorer(SimilarityPtr similarity, DocIdSetIteratorPtr docIdSetIterator, WeightPtr w) : Scorer(similarity, w)
    {
        this->theScore = w->getValue();
        this->docIdSetIterator = docIdSetIterator;
    }
    
    ConstantScorer::~ConstantScorer()
    {
    }
    
    int32_t ConstantScorer::nextDoc()
    {
        return docIdSetIterator->nextDoc();
    }
    
    int32_t ConstantScorer::docID()
    {
        return docIdSetIterator->docID();
    }
    
    double ConstantScorer::score()
    {
        return theScore;
    }
    
    int32_t ConstantScorer::advance(int32_t target)
    {
        return docIdSetIterator->advance(target);
    }
    
    CollectorPtr ConstantScorer::wrapCollector(CollectorPtr collector)
    {
        return newLucene<ConstantCollector>(shared_from_this(), collector);
    }
    
    void ConstantScorer::score(CollectorPtr collector)
    {
        if (MiscUtils::typeOf<Scorer>(docIdSetIterator))
            boost::static_pointer_cast<Scorer>(docIdSetIterator)->score(wrapCollector(collector));
        else
            Scorer::score(collector);
    }
    
    bool ConstantScorer::score(CollectorPtr collector, int32_t max, int32_t firstDocID)
    {
        if (MiscUtils::typeOf<Scorer>(docIdSetIterator))
            return boost::static_pointer_cast<Scorer>(docIdSetIterator)->score(wrapCollector(collector), max, firstDocID);
        else
            return Scorer::score(collector, max, firstDocID);
    }
    
    ConstantCollector::ConstantCollector(ConstantScorerPtr scorer, CollectorPtr collector)
    {
        this->_scorer = scorer;
        this->collector = collector;
    }
    
    ConstantCollector::~ConstantCollector()
    {
    }
    
    void ConstantCollector::setScorer(ScorerPtr scorer)
    {
        ConstantScorerPtr constantScorer(_scorer);
        // we must wrap again here, but using the scorer passed in as parameter
        collector->setScorer(newLucene<ConstantScorer>(scorer->getSimilarity(), scorer, constantScorer->weight));
    }
    
    void ConstantCollector::collect(int32_t doc)
    {
        collector->collect(doc);
    }
    
    void ConstantCollector::setNextReader(IndexReaderPtr reader, int32_t docBase)
    {
        collector->setNextReader(reader, docBase);
    }
    
    bool ConstantCollector::acceptsDocsOutOfOrder()
    {
        return collector->acceptsDocsOutOfOrder();
    }
}
