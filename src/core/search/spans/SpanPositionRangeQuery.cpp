/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SpanPositionRangeQuery.h"
#include "SpanQuery.h"
#include "Spans.h"
#include "MiscUtils.h"

namespace Lucene
{
    SpanPositionRangeQuery::SpanPositionRangeQuery(SpanQueryPtr match, int32_t start, int32_t end) : SpanPositionCheckQuery(match)
    {
        this->start = start;
        this->end = end;
    }
    
    SpanPositionRangeQuery::~SpanPositionRangeQuery()
    {
    }
    
    SpanPositionCheckQuery::AcceptStatus SpanPositionRangeQuery::acceptPosition(SpansPtr spans)
    {
        BOOST_ASSERT(spans->start() != spans->end());
        if (spans->start() >= end)
            return NO_AND_ADVANCE;
        else if (spans->start() >= start && spans->end() <= end)
            return YES;
        else
            return NO;
    }
    
    int32_t SpanPositionRangeQuery::getStart()
    {
        return start;
    }
    
    int32_t SpanPositionRangeQuery::getEnd()
    {
        return end;
    }
    
    String SpanPositionRangeQuery::toString(const String& field)
    {
        StringStream buffer;
        buffer << L"spanPosRange(";
        buffer << match->toString(field);
        buffer << L", " << start << L", " << end << L")";
        buffer << boostString();
        return buffer.str();
    }
    
    LuceneObjectPtr SpanPositionRangeQuery::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = SpanPositionCheckQuery::clone(other ? other : newLucene<SpanPositionRangeQuery>(boost::static_pointer_cast<SpanQuery>(match->clone()), start, end));
        SpanPositionRangeQueryPtr spanPositionRangeQuery(boost::static_pointer_cast<SpanPositionRangeQuery>(clone));
        spanPositionRangeQuery->match = match;
        spanPositionRangeQuery->start = start;
        spanPositionRangeQuery->end = end;
        spanPositionRangeQuery->setBoost(getBoost());
        return spanPositionRangeQuery;
    }
    
    bool SpanPositionRangeQuery::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        
        SpanPositionRangeQueryPtr otherQuery(boost::dynamic_pointer_cast<SpanPositionRangeQuery>(other));
        if (!otherQuery)
            return false;
        
        return (end == otherQuery->end && start == otherQuery->start && match->equals(otherQuery->match) && getBoost() == otherQuery->getBoost());
    }
    
    int32_t SpanPositionRangeQuery::hashCode()
    {
        int32_t result = match->hashCode();
        result ^= (result << 8) | MiscUtils::unsignedShift(result, 25); // reversible
        result ^= MiscUtils::doubleToRawIntBits(getBoost()) ^ end ^ start;
        return result;
    }
}
