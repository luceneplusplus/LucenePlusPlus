/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "CharTermAttribute.h"
#include "TermAttribute.h"
#include "MiscUtils.h"
#include "StringUtils.h"

namespace Lucene
{
    const int32_t CharTermAttribute::MIN_BUFFER_SIZE = 10;
    
    CharTermAttribute::CharTermAttribute()
    {
        _termLength = 0;
        _termBuffer = CharArray::newInstance(MiscUtils::oversize(MIN_BUFFER_SIZE, sizeof(wchar_t)));
    }
    
    CharTermAttribute::~CharTermAttribute()
    {
    }
    
    String CharTermAttribute::term()
    {
        // don't delegate to toString() here
        return String(_termBuffer.get(), _termLength);
    }
    
    void CharTermAttribute::copyBuffer(const wchar_t* buffer, int32_t offset, int32_t length)
    {
        growTermBuffer(length);
        MiscUtils::arrayCopy(buffer, offset, _termBuffer.get(), 0, length);
        _termLength = length;
    }
    
    void CharTermAttribute::setTermBuffer(const wchar_t* buffer, int32_t offset, int32_t length)
    {
        copyBuffer(buffer, offset, length);
    }
    
    void CharTermAttribute::setTermBuffer(const String& buffer)
    {
        int32_t length = (int32_t)buffer.size();
        growTermBuffer(length);
        MiscUtils::arrayCopy(buffer.begin(), 0, _termBuffer.get(), 0, length);
        _termLength = length;
    }
    
    void CharTermAttribute::setTermBuffer(const String& buffer, int32_t offset, int32_t length)
    {
        BOOST_ASSERT(offset <= (int32_t)buffer.size());
        BOOST_ASSERT(offset + length <= (int32_t)buffer.size());
        growTermBuffer(length);
        MiscUtils::arrayCopy(buffer.begin(), offset, _termBuffer.get(), 0, length);
        _termLength = length;
    }
    
    CharArray CharTermAttribute::buffer()
    {
        return _termBuffer;
    }
    
    CharArray CharTermAttribute::termBuffer()
    {
        return _termBuffer;
    }
    
    wchar_t* CharTermAttribute::termBufferArray()
    {
        return _termBuffer.get();
    }
    
    wchar_t* CharTermAttribute::bufferArray()
    {
        return _termBuffer.get();
    }
    
    CharArray CharTermAttribute::resizeBuffer(int32_t newSize)
    {
        if (_termBuffer.size() < newSize)
        {
            // Not big enough; resize array with slight over allocation and preserve content
            MiscUtils::grow(_termBuffer, newSize);
        }
        return _termBuffer;
    }
    
    CharArray CharTermAttribute::resizeTermBuffer(int32_t newSize)
    {
        return resizeBuffer(newSize);
    }
    
    void CharTermAttribute::growTermBuffer(int32_t newSize)
    {
        if (_termBuffer.size() < newSize)
        {
            // Not big enough; resize array with slight over allocation and preserve content
            MiscUtils::grow(_termBuffer, newSize);
        }
    }
    
    int32_t CharTermAttribute::termLength()
    {
        return _termLength;
    }
    
    CharTermAttributePtr CharTermAttribute::setLength(int32_t length)
    {
        if (length > _termBuffer.size())
        {
            boost::throw_exception(IllegalArgumentException(L"length " + StringUtils::toString(length) + 
                                                            L" exceeds the size of the termBuffer (" + 
                                                            StringUtils::toString(_termBuffer.size()) + L")"));
        }
        _termLength = length;
        return shared_from_this();
    }
    
    CharTermAttributePtr CharTermAttribute::setEmpty()
    {
        _termLength = 0;
        return shared_from_this();
    }
    
    void CharTermAttribute::setTermLength(int32_t length)
    {
        setLength(length);
    }
    
    int32_t CharTermAttribute::length()
    {
        return _termLength;
    }
    
    wchar_t CharTermAttribute::charAt(int32_t index)
    {
        if (index >= _termLength)
            boost::throw_exception(IndexOutOfBoundsException());
        return _termBuffer[index];
    }
    
    String CharTermAttribute::subSequence(int32_t start, int32_t end)
    {
        if (start > _termLength || end > _termLength)
            boost::throw_exception(IndexOutOfBoundsException());
        return String(_termBuffer.get() + start, end - start);
    }
    
    CharTermAttributePtr CharTermAttribute::append(const String& s)
    {
        return append(s, 0, s.length());
    }
    
    CharTermAttributePtr CharTermAttribute::append(const String& s, int32_t start, int32_t end)
    {
        int32_t len = end - start;
        int32_t slen = s.length();
        if (len < 0 || start > slen || end > slen)
            boost::throw_exception(IndexOutOfBoundsException());
        if (len == 0)
            return shared_from_this();
        resizeBuffer(_termLength + len);
        MiscUtils::arrayCopy(s.begin(), start, _termBuffer.get(), 0, len);
        _termLength += len;
        return shared_from_this();
    }
    
    CharTermAttributePtr CharTermAttribute::append(wchar_t c)
    {
        resizeBuffer(_termLength + 1)[_termLength++] = c;
        return shared_from_this();
    }
    
    CharTermAttributePtr CharTermAttribute::append(CharTermAttributePtr termAtt)
    {
        int32_t len = termAtt->length();
        resizeBuffer(_termLength + len);
        MiscUtils::arrayCopy(termAtt->bufferArray(), 0, _termBuffer.get(), _termLength, len);
        _termLength += len;
        return shared_from_this();
    }
    
    int32_t CharTermAttribute::hashCode()
    {
        int32_t code = _termLength;
        code = code * 31 + MiscUtils::hashCode(_termBuffer.get(), 0, _termLength);
        return code;
    }
    
    void CharTermAttribute::clear()
    {
        _termLength = 0;
    }
    
    LuceneObjectPtr CharTermAttribute::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = Attribute::clone(other ? other : newLucene<CharTermAttribute>());
        CharTermAttributePtr cloneAttribute(boost::static_pointer_cast<CharTermAttribute>(clone));
        cloneAttribute->_termLength = _termLength;
        cloneAttribute->_termBuffer = CharArray::newInstance(_termLength);
        MiscUtils::arrayCopy(_termBuffer.get(), 0, cloneAttribute->_termBuffer.get(), 0, _termLength);
        return cloneAttribute;
    }
    
    bool CharTermAttribute::equals(LuceneObjectPtr other)
    {
        if (Attribute::equals(other))
            return true;
        
        CharTermAttributePtr otherCharTermAttribute(boost::dynamic_pointer_cast<CharTermAttribute>(other));
        if (otherCharTermAttribute)
        {
            if (_termLength != otherCharTermAttribute->_termLength)
                return false;
            return (std::memcmp(_termBuffer.get(), otherCharTermAttribute->_termBuffer.get(), _termLength) == 0);
        }
        
        return false;
    }
    
    String CharTermAttribute::toString()
    {
        // CharSequence requires that only the contents are returned.
        return String(_termBuffer.get(), _termLength);
    }
    
    void CharTermAttribute::copyTo(AttributePtr target)
    {
        if (MiscUtils::typeOf<CharTermAttribute>(target))
        {
            CharTermAttributePtr t(boost::static_pointer_cast<CharTermAttribute>(target));
            t->copyBuffer(_termBuffer.get(), 0, _termLength);
        }
        else
        {
            TermAttributePtr t(boost::static_pointer_cast<TermAttribute>(target));
            t->setTermBuffer(_termBuffer.get(), 0, _termLength);
        }
    }
}
