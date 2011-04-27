/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "TypeAttribute.h"
#include "StringUtils.h"

namespace Lucene
{
    /// the default type
    const String TypeAttribute::DEFAULT_TYPE = L"word";
    
    TypeAttribute::TypeAttribute()
    {
        _type = DEFAULT_TYPE;
    }
    
    TypeAttribute::TypeAttribute(const String& type)
    {
        _type = type;
    }
    
    TypeAttribute::~TypeAttribute()
    {
    }
    
    String TypeAttribute::toString()
    {
        return L"type=" + _type;
    }
    
    String TypeAttribute::type()
    {
        return _type;
    }
    
    void TypeAttribute::setType(const String& type)
    {
        _type = type;
    }
    
    void TypeAttribute::clear()
    {
        _type = DEFAULT_TYPE;
    }
    
    bool TypeAttribute::equals(LuceneObjectPtr other)
    {
        if (Attribute::equals(other))
            return true;
        
        TypeAttributePtr otherTypeAttribute(boost::dynamic_pointer_cast<TypeAttribute>(other));
        if (otherTypeAttribute)
            return (otherTypeAttribute->_type == _type);
        
        return false;
    }
    
    int32_t TypeAttribute::hashCode()
    {
        return _type.empty() ? 0 : StringUtils::hashCode(_type);
    }
    
    void TypeAttribute::copyTo(AttributePtr target)
    {
        boost::static_pointer_cast<TypeAttribute>(target)->setType(_type);
    }
    
    LuceneObjectPtr TypeAttribute::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = other ? other : newLucene<TypeAttribute>();
        TypeAttributePtr cloneAttribute(boost::static_pointer_cast<TypeAttribute>(Attribute::clone(clone)));
        cloneAttribute->_type = _type;
        return cloneAttribute;
    }
}
