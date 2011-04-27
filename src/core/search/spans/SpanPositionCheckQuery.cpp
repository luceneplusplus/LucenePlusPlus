/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SpanPositionCheckQuery.h"
#include "_SpanPositionCheckQuery.h"

namespace Lucene
{
    SpanPositionCheckQuery::SpanPositionCheckQuery(SpanQueryPtr match)
    {
        this->match = match;
    }
    
    SpanPositionCheckQuery::~SpanPositionCheckQuery()
    {
    }
    
    SpanQueryPtr SpanPositionCheckQuery::getMatch()
    {
        return match;
    }
    
    String SpanPositionCheckQuery::getField()
    {
        return match->getField();
    }
    
    void SpanPositionCheckQuery::extractTerms(SetTerm terms)
    {
        match->extractTerms(terms);
    }
    
    SpansPtr SpanPositionCheckQuery::getSpans(IndexReaderPtr reader)
    {
        return newLucene<PositionCheckSpan>(shared_from_this(), reader);
    }
    
    QueryPtr SpanPositionCheckQuery::rewrite(IndexReaderPtr reader)
    {
        SpanPositionCheckQueryPtr clone;
        SpanQueryPtr rewritten(boost::static_pointer_cast<SpanQuery>(match->rewrite(reader)));
        if (rewritten != match)
        {
            clone = boost::static_pointer_cast<SpanPositionCheckQuery>(this->clone());
            clone->match = rewritten;
        }
        if (clone)
            return clone; // some clauses rewrote
        else
            return shared_from_this(); // no clauses rewrote
    }
    
    PositionCheckSpan::PositionCheckSpan(SpanPositionCheckQueryPtr spanQuery, IndexReaderPtr reader)
    {
        _spanQuery = spanQuery;
        spans = spanQuery->match->getSpans(reader);
    }
    
    PositionCheckSpan::~PositionCheckSpan()
    {
    }
    
    bool PositionCheckSpan::next()
    {
        if (!spans->next())
            return false;
        return doNext();
    }
    
    bool PositionCheckSpan::skipTo(int32_t target)
    {
        if (!spans->skipTo(target))
            return false;
        return doNext();
    }
    
    bool PositionCheckSpan::doNext()
    {
        SpanPositionCheckQueryPtr spanQuery(_spanQuery);
        while (true)
        {
            switch (spanQuery->acceptPosition(shared_from_this()))
            {
                case SpanPositionCheckQuery::YES:
                    return true;
                case SpanPositionCheckQuery::NO:
                    if (!spans->next()) 
                        return false;
                    break;
                case SpanPositionCheckQuery::NO_AND_ADVANCE: 
                    if (!spans->skipTo(spans->doc() + 1)) 
                        return false;
                    break;
            }
        }
        return false;
    }
    
    int32_t PositionCheckSpan::doc()
    {
        return spans->doc();
    }
    
    int32_t PositionCheckSpan::start()
    {
        return spans->start();
    }
    
    int32_t PositionCheckSpan::end()
    {
        return spans->end();
    }
    
    Collection<ByteArray> PositionCheckSpan::getPayload()
    {
        Collection<ByteArray> result;
        if (spans->isPayloadAvailable())
        {
            Collection<ByteArray> payload(spans->getPayload());
            result = Collection<ByteArray>::newInstance(payload.begin(), payload.end());
        }
        return result;
    }
    
    bool PositionCheckSpan::isPayloadAvailable()
    {
        return spans->isPayloadAvailable();
    }
    
    String PositionCheckSpan::toString()
    {
        return L"spans(" + SpanPositionCheckQueryPtr(_spanQuery)->toString() + L")";
    }
}
