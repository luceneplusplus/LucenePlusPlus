/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "MultiTermQuery.h"
#include "_MultiTermQuery.h"
#include "TopTermsRewrite.h"
#include "ScoringRewrite.h"
#include "ConstantScoreQuery.h"
#include "ConstantScoreAutoRewrite.h"
#include "MultiTermQueryWrapperFilter.h"
#include "QueryWrapperFilter.h"
#include "BooleanQuery.h"
#include "Term.h"
#include "TermQuery.h"
#include "TermDocs.h"
#include "FilteredTermEnum.h"
#include "IndexReader.h"
#include "MiscUtils.h"

namespace Lucene
{
    MultiTermQuery::MultiTermQuery()
    {
        numberOfTerms = 0;
        rewriteMethod = CONSTANT_SCORE_AUTO_REWRITE_DEFAULT();
    }
    
    MultiTermQuery::~MultiTermQuery()
    {
    }
    
    RewriteMethodPtr MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE()
    {
        static RewriteMethodPtr _CONSTANT_SCORE_FILTER_REWRITE;
        if (!_CONSTANT_SCORE_FILTER_REWRITE)
        {
            _CONSTANT_SCORE_FILTER_REWRITE = newLucene<ConstantScoreFilterRewrite>();
            CycleCheck::addStatic(_CONSTANT_SCORE_FILTER_REWRITE);
        }
        return _CONSTANT_SCORE_FILTER_REWRITE;
    }
    
    RewriteMethodPtr MultiTermQuery::SCORING_BOOLEAN_QUERY_REWRITE()
    {
        static RewriteMethodPtr _SCORING_BOOLEAN_QUERY_REWRITE;
        if (!_SCORING_BOOLEAN_QUERY_REWRITE)
        {
            _SCORING_BOOLEAN_QUERY_REWRITE = ScoringRewrite::SCORING_BOOLEAN_QUERY_REWRITE();
            CycleCheck::addStatic(_SCORING_BOOLEAN_QUERY_REWRITE);
        }
        return _SCORING_BOOLEAN_QUERY_REWRITE;
    }
    
    RewriteMethodPtr MultiTermQuery::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE()
    {
        static RewriteMethodPtr _CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE;
        if (!_CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE)
        {
            _CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE = ScoringRewrite::CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE();
            CycleCheck::addStatic(_CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE);
        }
        return _CONSTANT_SCORE_BOOLEAN_QUERY_REWRITE;
    }
    
    RewriteMethodPtr MultiTermQuery::CONSTANT_SCORE_AUTO_REWRITE_DEFAULT()
    {
        static RewriteMethodPtr _CONSTANT_SCORE_AUTO_REWRITE_DEFAULT;
        if (!_CONSTANT_SCORE_AUTO_REWRITE_DEFAULT)
        {
            _CONSTANT_SCORE_AUTO_REWRITE_DEFAULT = newLucene<ConstantScoreAutoRewriteDefault>();
            CycleCheck::addStatic(_CONSTANT_SCORE_AUTO_REWRITE_DEFAULT);
        }
        return _CONSTANT_SCORE_AUTO_REWRITE_DEFAULT;
    }
    
    int32_t MultiTermQuery::getTotalNumberOfTerms()
    {
        return numberOfTerms;
    }
    
    void MultiTermQuery::clearTotalNumberOfTerms()
    {
        numberOfTerms = 0;
    }
    
    void MultiTermQuery::incTotalNumberOfTerms(int32_t inc)
    {
        numberOfTerms += inc;
    }
    
    QueryPtr MultiTermQuery::rewrite(IndexReaderPtr reader)
    {
        return rewriteMethod->rewrite(reader, shared_from_this());
    }
    
    RewriteMethodPtr MultiTermQuery::getRewriteMethod()
    {
        return rewriteMethod;
    }
    
    void MultiTermQuery::setRewriteMethod(RewriteMethodPtr method)
    {
        rewriteMethod = method;
    }
    
    LuceneObjectPtr MultiTermQuery::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = Query::clone(other);
        MultiTermQueryPtr cloneQuery(boost::static_pointer_cast<MultiTermQuery>(clone));
        cloneQuery->rewriteMethod = rewriteMethod;
        cloneQuery->numberOfTerms = numberOfTerms;
        return cloneQuery;
    }
    
    int32_t MultiTermQuery::hashCode()
    {
        int32_t prime = 31;
        int32_t result = 1;
        result = prime * result + MiscUtils::doubleToIntBits(getBoost());
        result = prime * result;
        result += rewriteMethod->hashCode();
        return result;
    }
    
    bool MultiTermQuery::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        if (!other)
            return false;
        if (!MiscUtils::equalTypes(shared_from_this(), other))
            return false;
        MultiTermQueryPtr otherMultiTermQuery(boost::dynamic_pointer_cast<MultiTermQuery>(other));
        if (!otherMultiTermQuery)
            return false;
        if (MiscUtils::doubleToIntBits(getBoost()) != MiscUtils::doubleToIntBits(otherMultiTermQuery->getBoost()))
            return false;
        if (!rewriteMethod->equals(otherMultiTermQuery->rewriteMethod))
            return false;
        return true;
    }
    
    RewriteMethod::~RewriteMethod()
    {
    }
}
