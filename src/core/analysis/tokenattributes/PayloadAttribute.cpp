/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "PayloadAttribute.h"
#include "Payload.h"
#include "StringUtils.h"

namespace Lucene
{
    PayloadAttribute::PayloadAttribute()
    {
    }

    PayloadAttribute::PayloadAttribute(PayloadPtr payload)
    {
        this->payload = payload;
    }

    PayloadAttribute::~PayloadAttribute()
    {
    }

    String PayloadAttribute::toString()
    {
        return L"payload(length)=" + StringUtils::toString(payload->length());
    }

    PayloadPtr PayloadAttribute::getPayload()
    {
        return this->payload;
    }

    void PayloadAttribute::setPayload(PayloadPtr payload)
    {
        this->payload = payload;
    }

    void PayloadAttribute::clear()
    {
        payload.reset();
    }

    LuceneObjectPtr PayloadAttribute::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = Attribute::clone(other ? other : newLucene<PayloadAttribute>());
        PayloadAttributePtr cloneAttribute(gc_ptr_dynamic_cast<PayloadAttribute>(clone));
        if (payload)
            cloneAttribute->payload = gc_ptr_dynamic_cast<Payload>(payload->clone());
        return cloneAttribute;
    }

    bool PayloadAttribute::equals(LuceneObjectPtr other)
    {
        if (Attribute::equals(other))
            return true;

        PayloadAttributePtr otherAttribute(gc_ptr_dynamic_cast<PayloadAttribute>(other));
        if (otherAttribute)
        {
            if (!otherAttribute->payload && !payload)
                return true;
            return otherAttribute->payload->equals(payload);
        }

        return false;
    }

    int32_t PayloadAttribute::hashCode()
    {
        return payload ? payload->hashCode() : 0;
    }

    void PayloadAttribute::copyTo(AttributePtr target)
    {
        PayloadAttributePtr targetPayloadAttribute(gc_ptr_dynamic_cast<PayloadAttribute>(target));
        targetPayloadAttribute->setPayload(payload ? gc_ptr_dynamic_cast<Payload>(payload->clone()) : PayloadPtr());
    }
}
