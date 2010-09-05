/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringUtils.h"
#include "UnicodeUtils.h"
#include "CharFolder.h"

namespace Lucene
{
    /// Default character radix.
    const int32_t StringUtils::CHARACTER_MAX_RADIX = 36;
    
    /// Maximum length of unicode encoding.
    const int32_t StringUtils::MAX_ENCODING_UNICODE_SIZE = 2;
    
    /// Maximum length of UTF encoding.
    const int32_t StringUtils::MAX_ENCODING_UTF8_SIZE = 6;
    
    int32_t StringUtils::toUnicode(const uint8_t* utf8, int32_t length, wchar_t* unicode)
    {
        return UnicodeUtil::toUnicode(utf8, length, unicode);
    }
    
    int32_t StringUtils::toUnicode(const uint8_t* utf8, int32_t length, CharArray unicode)
    {
        return length == 0 ? 0 : toUnicode(utf8, length, unicode.get());
    }
    
    void StringUtils::toUnicode(const uint8_t* utf8, int32_t length, UnicodeResultPtr unicodeResult)
    {
        if (length == 0)
            unicodeResult->length = 0;
        else
        {
            unicodeResult->result.resize(length * MAX_ENCODING_UNICODE_SIZE);
            unicodeResult->length = toUnicode(utf8, length, unicodeResult->result);
        }
    }
    
    String StringUtils::toUnicode(const uint8_t* utf8, int32_t length)
    {
        CharArray unicode(CharArray::newInstance(length * MAX_ENCODING_UNICODE_SIZE));
        int32_t result = toUnicode(utf8, length, unicode);
        return String(unicode.get(), result);
    }
    
    String StringUtils::toUnicode(const SingleString& s)
    {
        return toUnicode((uint8_t*)s.c_str(), s.length());
    }
    
    int32_t StringUtils::toUTF8(const uint8_t* unicode, int32_t length, uint8_t* utf8)
    {
        return UnicodeUtil::toUTF8(unicode, length, utf8);
    }
    
    int32_t StringUtils::toUTF8(const uint8_t* unicode, int32_t length, ByteArray utf8)
    {
        return length == 0 ? 0 : toUTF8(unicode, length, utf8.get());
    }
    
    void StringUtils::toUTF8(const uint8_t* unicode, int32_t length, UTF8ResultPtr utf8Result)
    {
        if (length == 0)
            utf8Result->length = 0;
        else
        {
            utf8Result->result.resize(length * MAX_ENCODING_UTF8_SIZE);
            utf8Result->length = toUTF8(unicode, length, utf8Result->result);
        }
    }
    
    SingleString StringUtils::toUTF8(const uint8_t* unicode, int32_t length)
    {
        ByteArray utf8(ByteArray::newInstance(length * MAX_ENCODING_UTF8_SIZE));
        int32_t result = toUTF8(unicode, length, utf8);
        return SingleString((char*)utf8.get(), result);
    }
    
    SingleString StringUtils::toUTF8(const String& s)
    {
        return toUTF8((uint8_t*)s.c_str(), s.length());
    }
    
    void StringUtils::toLower(String& str)
    {
        CharFolder::toLower(str.begin(), str.end());
    }
    
    String StringUtils::toLower(const String& str)
    {
        String lowerStr(str);
        toLower(lowerStr);
        return lowerStr;
    }
    
    void StringUtils::toUpper(String& str)
    {
        CharFolder::toUpper(str.begin(), str.end());
    }
    
    String StringUtils::toUpper(const String& str)
    {
        String upperStr(str);
        toUpper(upperStr);
        return upperStr;
    }
    
    int32_t StringUtils::compareCase(const String& first, const String& second)
    {
        return (toLower(first) == toLower(second));
    }
    
    Collection<String> StringUtils::split(const String& str, const String& delim)
    {
        std::vector<String> tokens;
        boost::split(tokens, str, boost::is_any_of(delim.c_str()));
        return Collection<String>::newInstance(tokens.begin(), tokens.end());
    }
    
    int32_t StringUtils::toInt(const String& value)
    {
        if (value.empty())
            boost::throw_exception(NumberFormatException());
        if (value.length() > 1 && value[0] == L'-' && !UnicodeUtil::isDigit(value[1]))
            boost::throw_exception(NumberFormatException());
        if (value[0] != L'-' && !UnicodeUtil::isDigit(value[0]))
            boost::throw_exception(NumberFormatException());
        return (int32_t)std::wcstol(value.c_str(), NULL, 10);
    }
    
    int64_t StringUtils::toLong(const String& value)
    {
        if (value.empty())
            boost::throw_exception(NumberFormatException());
        if (value.length() > 1 && value[0] == L'-' && !UnicodeUtil::isDigit(value[1]))
            boost::throw_exception(NumberFormatException());
        if (value[0] != L'-' && !UnicodeUtil::isDigit(value[0]))
            boost::throw_exception(NumberFormatException());
        #ifdef _WIN32
        return _wcstoi64(value.c_str(), 0, 10);
        #else
        return wcstoll(value.c_str(), 0, 10);
        #endif
    }
    
    int64_t StringUtils::toLong(const String& value, int32_t base)
    {
        int64_t longValue = 0;
        for (String::const_iterator ptr = value.begin(); ptr != value.end(); ++ptr)
            longValue = UnicodeUtil::isDigit(*ptr) ? (base * longValue) + (*ptr - L'0') : (base * longValue) + (*ptr - L'a' + 10);
        return longValue;
    }
    
    double StringUtils::toDouble(const String& value)
    {
        if (value.empty())
            boost::throw_exception(NumberFormatException());
        if (value.length() > 1 && (value[0] == L'-' || value[0] == L'.') && !UnicodeUtil::isDigit(value[1]))
            boost::throw_exception(NumberFormatException());
        if (value[0] != L'-' && value[0] != L'.' && !UnicodeUtil::isDigit(value[0]))
            boost::throw_exception(NumberFormatException());
        return std::wcstod(value.c_str(), NULL);
    }
    
    int32_t StringUtils::hashCode(const String& value)
    {
        int32_t hashCode = 0;
        for (String::const_iterator ptr = value.begin(); ptr != value.end(); ++ptr)
            hashCode = hashCode * 31 + *ptr;
        return hashCode;
    }
    
    String StringUtils::toString(int64_t value, int32_t base)
    {
        static const wchar_t* digits = L"0123456789abcdefghijklmnopqrstuvwxyz";
        
        int32_t bufferSize = (sizeof(int32_t) << 3) + 1;
        CharArray baseOutput(CharArray::newInstance(bufferSize));
        
        wchar_t* ptr = baseOutput.get() + bufferSize - 1;
        *ptr = L'\0';
        
        do
        {
            *--ptr = digits[value % base];
            value /= base;
        }
        while (ptr > baseOutput.get() && value > 0);
        
        return String(ptr, (baseOutput.get() + bufferSize - 1) - ptr);
    }
}

