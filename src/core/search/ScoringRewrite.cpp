/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ScoringRewrite.h"
#include "_ScoringRewrite.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "ConstantScoreQuery.h"

namespace Lucene
{
    ScoringRewrite::~ScoringRewrite()
    {
    }

    ScoringRewritePtr ScoringRewrite::SCORING_BOOLEAN_QUERY_REWRITE()
    {
        static ScoringRewritePtr _SCORING_BOOLEAN_QUERY_REWRITE;
        if (!_SCORING_BOOLEAN_QUERY_REWRITE)
        {
            _SCORING_BOOLEAN_QUERY_REWRITE = newLucene<ScoringRewriteBooleanQuery>();
            CycleCheck::addStatic(_SCORING_BOOLEAN_QUERY_REWRITE);
        }
        return _SCORING_BOOLEAN_QUERY_REWRITE;
    }
    
    RewriteMethodPtr ScoringRewrite::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE()
    {
        static RewriteMethodPtr _CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE;
        if (!_CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE)
        {
            _CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE = newLucene<ConstantScoreBooleanQueryRewrite>();
            CycleCheck::addStatic(_CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE);
        }
        return _CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE;
    }
    
    QueryPtr ScoringRewrite::rewrite(IndexReaderPtr reader, MultiTermQueryPtr query)
    {
        QueryPtr result(getTopLevelQuery());
        IntArray size(IntArray::newInstance(1));
        collectTerms(reader, query, newLucene<ScoringRewriteTermCollector>(shared_from_this(), query, result, size));
        query->incTotalNumberOfTerms(size[0]);
        return result;
    }

    ScoringRewriteBooleanQuery::~ScoringRewriteBooleanQuery()
    {
    }
    
    QueryPtr ScoringRewriteBooleanQuery::getTopLevelQuery()
    {
        return newLucene<BooleanQuery>(true);
    }
    
    void ScoringRewriteBooleanQuery::addClause(QueryPtr topLevel, TermPtr term, double boost)
    {
        TermQueryPtr tq(newLucene<TermQuery>(term));
        tq->setBoost(boost);
        boost::static_pointer_cast<BooleanQuery>(topLevel)->add(tq, BooleanClause::SHOULD);
    }
    
    ConstantScoreBooleanQueryRewrite::~ConstantScoreBooleanQueryRewrite()
    {
    }
    
    QueryPtr ConstantScoreBooleanQueryRewrite::rewrite(IndexReaderPtr reader, MultiTermQueryPtr query)
    {
        BooleanQueryPtr bq(boost::static_pointer_cast<BooleanQuery>(ScoringRewrite::SCORING_BOOLEAN_QUERY_REWRITE()->rewrite(reader, query)));
        if (bq->clauses().empty())
            return bq;
        // strip the scores off
        QueryPtr result(newLucene<ConstantScoreQuery>(bq));
        result->setBoost(query->getBoost());
        return result;
    }
    
    ScoringRewriteTermCollector::ScoringRewriteTermCollector(ScoringRewritePtr query, MultiTermQueryPtr multiQuery, QueryPtr result, IntArray size)
    {
        this->_query = query;
        this->multiQuery = multiQuery;
        this->result = result;
        this->size = size;
    }
    
    ScoringRewriteTermCollector::~ScoringRewriteTermCollector()
    {
    }
    
    bool ScoringRewriteTermCollector::collect(TermPtr t, double boost)
    {
        ScoringRewritePtr query(_query);
        query->addClause(result, t, multiQuery->getBoost() * boost);
        ++size[0];
        return true;
    }
}
