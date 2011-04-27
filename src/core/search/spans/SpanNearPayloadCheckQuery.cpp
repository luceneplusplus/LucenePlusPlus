/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SpanNearPayloadCheckQuery.h"
#include "SpanNearQuery.h"
#include "Spans.h"
#include "MiscUtils.h"

namespace Lucene
{
    SpanNearPayloadCheckQuery::SpanNearPayloadCheckQuery(SpanNearQueryPtr match, Collection<ByteArray> payloadToMatch) : SpanPositionCheckQuery(match)
    {
        this->payloadToMatch = payloadToMatch;
    }
    
    SpanNearPayloadCheckQuery::~SpanNearPayloadCheckQuery()
    {
    }
    
    SpanPositionCheckQuery::AcceptStatus SpanNearPayloadCheckQuery::acceptPosition(SpansPtr spans)
    {
        bool result = spans->isPayloadAvailable();
        if (result == true)
        {
            Collection<ByteArray> candidate = spans->getPayload();
            if (candidate.size() == payloadToMatch.size())
            {
                int32_t matches = 0;
                for (Collection<ByteArray>::iterator candBytes = candidate.begin(); candBytes != candidate.end(); ++candBytes)
                {
                    // Unfortunately, we can't rely on order, so we need to compare all
                    for (Collection<ByteArray>::iterator payBytes = payloadToMatch.begin(); payBytes != payloadToMatch.end(); ++payBytes)
                    {
                        if (candBytes->equals(*payBytes))
                        {
                            ++matches;
                            break;
                        }
                    }
                }
                if (matches == payloadToMatch.size())
                {
                    // we've verified all the bytes
                    return YES;
                }
                else
                    return NO;
            }
            else
                return NO;
        }
        return NO;
    }
    
    String SpanNearPayloadCheckQuery::toString(const String& field)
    {
        StringStream buffer;
        buffer << L"spanPayCheck(" << match->toString(field) << L", payloadRef: ";
        for (Collection<ByteArray>::iterator bytes = payloadToMatch.begin(); bytes != payloadToMatch.end(); ++bytes)
            buffer << bytes->size() << L";";
        buffer << L")" << boostString();
        return buffer.str();
    }
    
    LuceneObjectPtr SpanNearPayloadCheckQuery::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = SpanPositionCheckQuery::clone(other ? other : newLucene<SpanNearPayloadCheckQuery>(boost::static_pointer_cast<SpanNearQuery>(match->clone()), payloadToMatch));
        SpanNearPayloadCheckQueryPtr spanPositionRangeQuery(boost::static_pointer_cast<SpanNearPayloadCheckQuery>(clone));
        spanPositionRangeQuery->payloadToMatch = payloadToMatch;
        spanPositionRangeQuery->setBoost(getBoost());
        return spanPositionRangeQuery;
    }
    
    bool SpanNearPayloadCheckQuery::equals(LuceneObjectPtr other)
    {
        if (LuceneObject::equals(other))
            return true;
        
        SpanNearPayloadCheckQueryPtr otherQuery(boost::dynamic_pointer_cast<SpanNearPayloadCheckQuery>(other));
        if (!otherQuery)
            return false;
        
        return (payloadToMatch.equals(otherQuery->payloadToMatch) && match->equals(otherQuery->match) && getBoost() == otherQuery->getBoost());
    }
    
    int32_t SpanNearPayloadCheckQuery::hashCode()
    {
        int32_t result = match->hashCode();
        result ^= (result << 8) | MiscUtils::unsignedShift(result, 25); // reversible
        result ^= payloadToMatch.hashCode();
        result ^= MiscUtils::doubleToRawIntBits(getBoost());
        return result;
    }
}
