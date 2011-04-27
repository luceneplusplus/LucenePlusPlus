/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SpanMultiTermQueryWrapper.h"
#include "MultiTermQuery.h"
#include "TopTermsRewrite.h"
#include "SpanTermQuery.h"
#include "MiscUtils.h"

namespace Lucene
{
    SpanMultiTermQueryWrapper::SpanMultiTermQueryWrapper(MultiTermQueryPtr query)
    {
        this->query = query;
        
        RewriteMethodPtr method(query->getRewriteMethod());
        if (MiscUtils::typeOf<TopTermsRewrite>(method))
        {
            int32_t pqsize = boost::static_pointer_cast<TopTermsRewrite>(method)->getSize();
            setRewriteMethod(newLucene<TopTermsSpanBooleanQueryRewrite>(pqsize));
        }
        else
            setRewriteMethod(SCORING_SPAN_QUERY_REWRITE());
    }
    
    SpanMultiTermQueryWrapper::~SpanMultiTermQueryWrapper()
    {
    }
    
    SpanRewriteMethodPtr SpanMultiTermQueryWrapper::getRewriteMethod()
    {
        RewriteMethodPtr m(query->getRewriteMethod());
        if (!MiscUtils::typeOf<SpanRewriteMethod>(m))
            boost::throw_exception(UnsupportedOperationException(L"You can only use SpanMultiTermQueryWrapper with a suitable SpanRewriteMethod."));
        return boost::static_pointer_cast<SpanRewriteMethod>(m);
    }
    
    void SpanMultiTermQueryWrapper::setRewriteMethod(SpanRewriteMethodPtr rewriteMethod)
    {
        query->setRewriteMethod(rewriteMethod);
    }
    
    SpansPtr SpanMultiTermQueryWrapper::getSpans(IndexReaderPtr reader)
    {
        boost::throw_exception(UnsupportedOperationException(L"Query should have been rewritten"));
    }
    
    String SpanMultiTermQueryWrapper::getField()
    {
        return query->getField();
    }
    
    String SpanMultiTermQueryWrapper::toString(const String& field)
    {
        StringStream buffer;
        buffer << L"SpanMultiTermQueryWrapper(" << query->toString(field) << L")";
        return buffer.str();
    }
    
    QueryPtr SpanMultiTermQueryWrapper::rewrite(IndexReaderPtr reader)
    {
        QueryPtr q(query->rewrite(reader));
        if (!MiscUtils::typeOf<SpanQuery>(q))
            boost::throw_exception(UnsupportedOperationException(L"You can only use SpanMultiTermQueryWrapper with a suitable SpanRewriteMethod."));
        return q;
    }
    
    int32_t SpanMultiTermQueryWrapper::hashCode()
    {
        return 31 * query->hashCode();
    }
    
    bool SpanMultiTermQueryWrapper::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        
        SpanMultiTermQueryWrapperPtr otherSpanMultiTermQueryWrapper(boost::dynamic_pointer_cast<SpanMultiTermQueryWrapper>(other));
        if (!otherSpanMultiTermQueryWrapper)
            return false;
            
        return (query->equals(otherSpanMultiTermQueryWrapper->query));
    }
    
    SpanRewriteMethodPtr SpanMultiTermQueryWrapper::SCORING_SPAN_QUERY_REWRITE()
    {
        static SpanRewriteMethodPtr _SCORING_SPAN_QUERY_REWRITE;
        if (!_SCORING_SPAN_QUERY_REWRITE)
        {
            _SCORING_SPAN_QUERY_REWRITE = newLucene<SpanRewriteMethod>();
            CycleCheck::addStatic(_SCORING_SPAN_QUERY_REWRITE);
        }
        return _SCORING_SPAN_QUERY_REWRITE;
    }
    
    SpanRewriteMethod::SpanRewriteMethod()
    {
        delegate = newLucene<ScoringRewriteSpanOrQuery>();
    }
    
    SpanRewriteMethod::~SpanRewriteMethod()
    {
    }
    
    QueryPtr SpanRewriteMethod::rewrite(IndexReaderPtr reader, MultiTermQueryPtr query)
    {
        return delegate->rewrite(reader, query);
    }    
    
    ScoringRewriteSpanOrQuery::~ScoringRewriteSpanOrQuery()
    {
    }
    
    QueryPtr ScoringRewriteSpanOrQuery::getTopLevelQuery()
    {
        return newLucene<SpanOrQuery>();
    }
    
    void ScoringRewriteSpanOrQuery::addClause(QueryPtr topLevel, TermPtr term, double boost)
    {
        SpanTermQueryPtr q(newLucene<SpanTermQuery>(term));
        q->setBoost(boost);
        boost::static_pointer_cast<SpanOrQuery>(topLevel)->addClause(q);
    }
    
    TopTermsSpanBooleanQueryRewrite::TopTermsSpanBooleanQueryRewrite(int32_t size)
    {
        delegate = newLucene<TopTermsRewriteSpanOrQuery>(size);
    }
    
    TopTermsSpanBooleanQueryRewrite::~TopTermsSpanBooleanQueryRewrite()
    {
    }
    
    int32_t TopTermsSpanBooleanQueryRewrite::getSize()
    {
        return delegate->getSize();
    }
    
    QueryPtr TopTermsSpanBooleanQueryRewrite::rewrite(IndexReaderPtr reader, MultiTermQueryPtr query)
    {
        return delegate->rewrite(reader, query);
    }
    
    int32_t TopTermsSpanBooleanQueryRewrite::hashCode()
    {
        return 31 * delegate->hashCode();
    }
    
    bool TopTermsSpanBooleanQueryRewrite::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        
        TopTermsSpanBooleanQueryRewritePtr otherQueryRewrite(boost::dynamic_pointer_cast<TopTermsSpanBooleanQueryRewrite>(other));
        if (!otherQueryRewrite)
            return false;

        return delegate->equals(otherQueryRewrite->delegate);
    }
    
    TopTermsRewriteSpanOrQuery::TopTermsRewriteSpanOrQuery(int32_t size) : TopTermsRewrite(size)
    {
    }
    
    TopTermsRewriteSpanOrQuery::~TopTermsRewriteSpanOrQuery()
    {
    }
    
    int32_t TopTermsRewriteSpanOrQuery::getMaxSize()
    {
        return INT_MAX;
    }
    
    QueryPtr TopTermsRewriteSpanOrQuery::getTopLevelQuery()
    {
        return newLucene<SpanOrQuery>();
    }
    
    void TopTermsRewriteSpanOrQuery::addClause(QueryPtr topLevel, TermPtr term, double boost)
    {
        SpanTermQueryPtr q(newLucene<SpanTermQuery>(term));
        q->setBoost(boost);
        boost::static_pointer_cast<SpanOrQuery>(topLevel)->addClause(q);
    }
}
