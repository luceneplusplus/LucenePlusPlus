/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SpanFirstQuery.h"
#include "SpanQuery.h"
#include "Spans.h"
#include "MiscUtils.h"

namespace Lucene
{
    SpanFirstQuery::SpanFirstQuery(SpanQueryPtr match, int32_t end) : SpanPositionRangeQuery(match, 0, end)
    {
    }
    
    SpanFirstQuery::~SpanFirstQuery()
    {
    }
    
    SpanPositionCheckQuery::AcceptStatus SpanFirstQuery::acceptPosition(SpansPtr spans)
    {
        BOOST_ASSERT(spans->start() != spans->end());
        if (spans->start() >= end)
            return NO_AND_ADVANCE;
        else if (spans->end() <= end)
            return YES;
        else
            return NO;
    }
    
    String SpanFirstQuery::toString(const String& field)
    {
        StringStream buffer;
        buffer << L"spanFirst(" << match->toString(field) << L", " << end << L")" << boostString();
        return buffer.str();
    }
    
    LuceneObjectPtr SpanFirstQuery::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = SpanPositionRangeQuery::clone(other ? other : newLucene<SpanFirstQuery>(boost::static_pointer_cast<SpanQuery>(match->clone()), end));
        SpanFirstQueryPtr spanFirstQuery(boost::static_pointer_cast<SpanFirstQuery>(clone));
        spanFirstQuery->setBoost(getBoost());
        return spanFirstQuery;
    }
    
    bool SpanFirstQuery::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        
        SpanFirstQueryPtr otherQuery(boost::dynamic_pointer_cast<SpanFirstQuery>(other));
        if (!otherQuery)
            return false;
        
        return (end == otherQuery->end && match->equals(otherQuery->match) && getBoost() == otherQuery->getBoost());
    }
    
    int32_t SpanFirstQuery::hashCode()
    {
        int32_t result = match->hashCode();
        result ^= (result << 8) | MiscUtils::unsignedShift(result, 25); // reversible
        result ^= MiscUtils::doubleToRawIntBits(getBoost()) ^ end;
        return result;
    }
}
