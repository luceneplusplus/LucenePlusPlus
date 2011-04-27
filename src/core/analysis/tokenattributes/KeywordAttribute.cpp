/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "KeywordAttribute.h"
#include "StringUtils.h"

namespace Lucene
{
    KeywordAttribute::KeywordAttribute()
    {
        keyword = false;
    }
    
    KeywordAttribute::~KeywordAttribute()
    {
    }
    
    String KeywordAttribute::toString()
    {
        return L"keyword=" + StringUtils::toString(keyword);
    }
    
    void KeywordAttribute::clear()
    {
        keyword = false;
    }
    
    void KeywordAttribute::copyTo(AttributePtr target)
    {
        KeywordAttributePtr targetKeywordAttribute(boost::static_pointer_cast<KeywordAttribute>(target));
        targetKeywordAttribute->setKeyword(keyword);
    }
    
    int32_t KeywordAttribute::hashCode()
    {
        return keyword ? 31 : 37;
    }
    
    bool KeywordAttribute::equals(LuceneObjectPtr other)
    {
        if (Attribute::equals(other))
            return true;
        
        KeywordAttributePtr otherKeywordAttribute(boost::dynamic_pointer_cast<KeywordAttribute>(other));
        if (!otherKeywordAttribute)
            return false;
        
        return (otherKeywordAttribute->keyword == keyword);
    }
    
    LuceneObjectPtr KeywordAttribute::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = other ? other : newLucene<KeywordAttribute>();
        KeywordAttributePtr cloneAttribute(boost::static_pointer_cast<KeywordAttribute>(Attribute::clone(clone)));
        cloneAttribute->keyword = keyword;
        return cloneAttribute;
    }
    
    bool KeywordAttribute::isKeyword()
    {
        return keyword;
    }
    
    void KeywordAttribute::setKeyword(bool isKeyword)
    {
        keyword = isKeyword;
    }
}
