/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ConstantScoreAutoRewrite.h"
#include "_ConstantScoreAutoRewrite.h"
#include "BooleanQuery.h"
#include "BooleanClause.h"
#include "TermQuery.h"
#include "MultiTermQuery.h"
#include "Term.h"
#include "ConstantScoreQuery.h"
#include "IndexReader.h"
#include "MiscUtils.h"

namespace Lucene
{
    // Defaults derived from rough tests with a 20.0 million doc Wikipedia index.  
    /// With more than 350 terms in the query, the filter method is fastest
    const int32_t ConstantScoreAutoRewrite::DEFAULT_TERM_COUNT_CUTOFF = 350;
    
    // If the query will hit more than 1 in 1000 of the docs in the index (0.1%), 
    /// the filter method is fastest
    const double ConstantScoreAutoRewrite::DEFAULT_DOC_COUNT_PERCENT = 0.1;
    
    ConstantScoreAutoRewrite::ConstantScoreAutoRewrite()
    {
        termCountCutoff = DEFAULT_TERM_COUNT_CUTOFF;
        docCountPercent = DEFAULT_DOC_COUNT_PERCENT;
    }
    
    ConstantScoreAutoRewrite::~ConstantScoreAutoRewrite()
    {
    }
    
    void ConstantScoreAutoRewrite::setTermCountCutoff(int32_t count)
    {
        termCountCutoff = count;
    }
    
    int32_t ConstantScoreAutoRewrite::getTermCountCutoff()
    {
        return termCountCutoff;
    }
    
    void ConstantScoreAutoRewrite::setDocCountPercent(double percent)
    {
        docCountPercent = percent;
    }
    
    double ConstantScoreAutoRewrite::getDocCountPercent()
    {
        return docCountPercent;
    }
    
    QueryPtr ConstantScoreAutoRewrite::getTopLevelQuery()
    {
        return newLucene<BooleanQuery>(true);
    }
    
    void ConstantScoreAutoRewrite::addClause(QueryPtr topLevel, TermPtr term, double boost)
    {
        boost::static_pointer_cast<BooleanQuery>(topLevel)->add(newLucene<TermQuery>(term), BooleanClause::SHOULD);
    }
    
    QueryPtr ConstantScoreAutoRewrite::rewrite(IndexReaderPtr reader, MultiTermQueryPtr query)
    {
        // Get the enum and start visiting terms.  If we exhaust the enum before hitting either of the
        // cutoffs, we use ConstantBooleanQueryRewrite; else, ConstantFilterRewrite:
        int32_t docCountCutoff = (int32_t)((docCountPercent / 100.0) * (double)reader->maxDoc());
        int32_t termCountLimit = std::min(BooleanQuery::getMaxClauseCount(), termCountCutoff);

        CutOffTermCollectorPtr col = newLucene<CutOffTermCollector>(reader, docCountCutoff, termCountLimit);
        collectTerms(reader, query, col);

        if (col->hasCutOff)
            return MultiTermQuery::CONSTANT_SCORE_FILTER_REWRITE()->rewrite(reader, query);
        else
        {
            QueryPtr result;
            if (col->pendingTerms.empty())
                result = getTopLevelQuery();
            else
            {
                BooleanQueryPtr bq(boost::static_pointer_cast<BooleanQuery>(getTopLevelQuery()));
                for (Collection<TermPtr>::iterator term = col->pendingTerms.begin(); term != col->pendingTerms.end(); ++term)
                    addClause(bq, *term, 1.0);
                // Strip scores
                result = newLucene<ConstantScoreQuery>(bq);
                result->setBoost(query->getBoost());
            }
            query->incTotalNumberOfTerms(col->pendingTerms.size());
            return result;
        }
    }
    
    int32_t ConstantScoreAutoRewrite::hashCode()
    {
        int32_t prime = 1279;
        return (int32_t)(prime * termCountCutoff + MiscUtils::doubleToLongBits(docCountPercent));
    }
    
    bool ConstantScoreAutoRewrite::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        if (!other)
            return false;
        if (!MiscUtils::equalTypes(shared_from_this(), other))
            return false;
        
        ConstantScoreAutoRewritePtr otherConstantScoreAutoRewrite(boost::dynamic_pointer_cast<ConstantScoreAutoRewrite>(other));
        if (!otherConstantScoreAutoRewrite)
            return false;
        
        if (termCountCutoff != otherConstantScoreAutoRewrite->termCountCutoff)
            return false;
        
        if (MiscUtils::doubleToLongBits(docCountPercent) != MiscUtils::doubleToLongBits(otherConstantScoreAutoRewrite->docCountPercent))
            return false;
        
        return true;
    }
    
    CutOffTermCollector::CutOffTermCollector(IndexReaderPtr reader, int32_t docCountCutoff, int32_t termCountLimit)
    {
        this->docVisitCount = 0;
        this->hasCutOff = false;
        this->pendingTerms = Collection<TermPtr>::newInstance();
        
        this->reader = reader;
        this->docCountCutoff = docCountCutoff;
        this->termCountLimit = termCountLimit;
    }
    
    CutOffTermCollector::~CutOffTermCollector()
    {
    }
    
    bool CutOffTermCollector::collect(TermPtr t, double boost)
    {
        pendingTerms.add(t);
        // Loading the TermInfo from the terms dict here should not be costly, because 1) the
        // query/filter will load the TermInfo when it runs, and 2) the terms dict has a cache
        docVisitCount += reader->docFreq(t);
        if (pendingTerms.size() >= termCountLimit || docVisitCount >= docCountCutoff)
        {
            hasCutOff = true;
            return false;
        }
        return true;
    }
    
    ConstantScoreAutoRewriteDefault::~ConstantScoreAutoRewriteDefault()
    {
    }
    
    void ConstantScoreAutoRewriteDefault::setTermCountCutoff(int32_t count)
    {
        boost::throw_exception(UnsupportedOperationException(L"Please create a private instance"));
    }
    
    void ConstantScoreAutoRewriteDefault::setDocCountPercent(double percent)
    {
        boost::throw_exception(UnsupportedOperationException(L"Please create a private instance"));
    }
}
