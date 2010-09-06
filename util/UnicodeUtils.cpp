/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UnicodeUtils.h"
#include "utf8.h"
#include "guniprop.h"

namespace Lucene
{
    #ifdef LPP_UNICODE_CHAR_SIZE_2
    const int32_t UnicodeUtil::UNICODE_SUR_HIGH_START = 0xd800;
    const int32_t UnicodeUtil::UNICODE_SUR_HIGH_END = 0xdbff;
    const int32_t UnicodeUtil::UNICODE_SUR_LOW_START = 0xdc00;
    const int32_t UnicodeUtil::UNICODE_SUR_LOW_END = 0xdfff;
    const wchar_t UnicodeUtil::UNICODE_REPLACEMENT_CHAR = (wchar_t)0xfffd;
    const wchar_t UnicodeUtil::UNICODE_TERMINATOR = (wchar_t)0xffff;
    #else
    const wchar_t UnicodeUtil::UNICODE_REPLACEMENT_CHAR = (wchar_t)0x0001fffd;
    const wchar_t UnicodeUtil::UNICODE_TERMINATOR = (wchar_t)0x0001ffff;
    #endif

    UnicodeUtil::~UnicodeUtil()
    {
    }
    
    template <typename u16bit_iterator, typename octet_iterator>
    octet_iterator utf16to8(u16bit_iterator start, u16bit_iterator end, octet_iterator result)
    {       
        while (start != end && *start != 0xffff)
        {
            uint32_t cp = utf8::internal::mask16(*start++);
            if (utf8::internal::is_lead_surrogate(cp))
            {
                if (start != end)
                {
                    uint32_t trail_surrogate = utf8::internal::mask16(*start++);
                    if (utf8::internal::is_trail_surrogate(trail_surrogate))
                        cp = (cp << 10) + trail_surrogate + utf8::internal::SURROGATE_OFFSET;
                    else
                        throw utf8::invalid_utf16(static_cast<uint16_t>(trail_surrogate));
                }
                else
                    throw utf8::invalid_utf16(static_cast<uint16_t>(cp));
            }
            else if (utf8::internal::is_trail_surrogate(cp))
                throw utf8::invalid_utf16(static_cast<uint16_t>(cp));
            result = utf8::append(cp, result);
        }
        return result;         
    }
    
    template <typename octet_iterator, typename u32bit_iterator>
    octet_iterator utf32to8(u32bit_iterator start, u32bit_iterator end, octet_iterator result)
    {
        while (start != end && *start != 0x0001ffff)
            result = utf8::append(*(start++), result);
        return result;
    }
    
    template <typename u16bit_iterator, typename u32bit_iterator>
    u32bit_iterator utf16to32(u16bit_iterator start, u16bit_iterator end, u32bit_iterator result)
    {
        while (start != end && *start != 0xffff)
        {
            uint32_t cp = *(start++);
            if (utf8::internal::is_lead_surrogate(cp))
            {
                if (start != end)
                {
                    uint32_t trail_surrogate = *(start++);
                    if (utf8::internal::is_trail_surrogate(trail_surrogate))
                        *(result++) = static_cast<uint32_t>(((cp - utf8::internal::LEAD_SURROGATE_MIN) << 10) + (trail_surrogate - utf8::internal::TRAIL_SURROGATE_MIN) + 0x0010000);
                    else
                        throw utf8::invalid_utf16(static_cast<uint16_t>(trail_surrogate));
                }
            }
            else if (utf8::internal::is_trail_surrogate(cp))
                throw utf8::invalid_utf16(static_cast<uint16_t>(cp));
            else
                *(result++) = static_cast<uint32_t>(cp);
        }
        return result;
    }
    
    int32_t UnicodeUtil::toUnicode(const uint8_t* utf8, int32_t length, wchar_t* unicode)
    {
        try
        {
            #ifdef LPP_UNICODE_CHAR_SIZE_2
            return (utf8::utf8to16(utf8, utf8 + length, (uint16_t*)unicode) - (uint16_t*)unicode);
            #else
            return (utf8::utf8to32(utf8, utf8 + length, (uint32_t*)unicode) - (uint32_t*)unicode);
            #endif
        }
        catch (...)
        {
            return 0;
        }
    }
    
    int32_t UnicodeUtil::utf16ToUnicode(const uint8_t* utf16, int32_t length, wchar_t* unicode)
    {
        try
        {
            #ifdef LPP_UNICODE_CHAR_SIZE_2
            MiscUtils::arrayCopy((const wchar_t*)utf16, 0, unicode, 0, length);
            return length;
            #else
            return ((utf16to32((uint16_t*)utf16, (uint16_t*)utf16 + length, (uint32_t*)unicode)) - (uint32_t*)unicode);
            #endif
        }
        catch (...)
        {
            return 0;
        }
    }
    
    int32_t UnicodeUtil::toUTF8(const uint8_t* unicode, int32_t length, uint8_t* utf8)
    {
        try
        {
            #ifdef LPP_UNICODE_CHAR_SIZE_2
            return (utf16to8((uint16_t*)unicode, (uint16_t*)unicode + length, utf8) - utf8);
            #else
            return (utf32to8((uint32_t*)unicode, (uint32_t*)unicode + length, utf8) - utf8);
            #endif
        }
        catch (...)
        {
            return 0;
        }
    }
    
    bool UnicodeUtil::isAlnum(wchar_t c)
    {
        return g_unichar_isalnum(c);
    }
    
    bool UnicodeUtil::isAlpha(wchar_t c)
    {
        return g_unichar_isalpha(c);
    }
    
    bool UnicodeUtil::isDigit(wchar_t c)
    {
        return g_unichar_isdigit(c);
    }
    
    bool UnicodeUtil::isSpace(wchar_t c)
    {
        return g_unichar_isspace(c);
    }
    
    bool UnicodeUtil::isNonSpacing(wchar_t c)
    {
        return (g_unichar_type(c) == G_UNICODE_NON_SPACING_MARK);
    }
    
    wchar_t UnicodeUtil::toUpper(wchar_t c)
    {
        return (wchar_t)g_unichar_toupper(c);
    }
    
    wchar_t UnicodeUtil::toLower(wchar_t c)
    {
        return (wchar_t)g_unichar_tolower(c);
    }
    
    UTF8Result::~UTF8Result()
    {
    }
    
    UnicodeResult::~UnicodeResult()
    {
    }
}
