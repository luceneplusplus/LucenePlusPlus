/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "CzechStemmer.h"
#include "StemmerUtil.h"
#include "MiscUtils.h"

namespace Lucene
{
    CzechStemmer::~CzechStemmer()
    {
    }
    
    int32_t CzechStemmer::stem(wchar_t* s, int32_t len)
    {
        len = removeCase(s, len);
        len = removePossessives(s, len);
        len = normalize(s, len);
        return len;
    }
    
    int32_t CzechStemmer::removeCase(wchar_t* s, int32_t len)
    {
        if (len > 7 && StemmerUtil::endsWith(s, len, L"atech"))
            return len - 5;

        if (len > 6 && 
            (StemmerUtil::endsWith(s, len, L"\x0011btem") ||
             StemmerUtil::endsWith(s, len, L"etem") ||
             StemmerUtil::endsWith(s, len, L"at\x0016fm")))
            return len - 4;

        if (len > 5 && 
            (StemmerUtil::endsWith(s, len, L"ech") ||
             StemmerUtil::endsWith(s, len, L"ich") ||
             StemmerUtil::endsWith(s, len, L"\x00edch") ||
             StemmerUtil::endsWith(s, len, L"\x00e9ho") ||
             StemmerUtil::endsWith(s, len, L"\x011bmi") ||
             StemmerUtil::endsWith(s, len, L"emi") ||
             StemmerUtil::endsWith(s, len, L"\x00e9mu") ||
             StemmerUtil::endsWith(s, len, L"\x011bte") ||
             StemmerUtil::endsWith(s, len, L"ete") ||
             StemmerUtil::endsWith(s, len, L"\x011bti") ||
             StemmerUtil::endsWith(s, len, L"eti") ||
             StemmerUtil::endsWith(s, len, L"\x00edho") ||
             StemmerUtil::endsWith(s, len, L"iho") ||
             StemmerUtil::endsWith(s, len, L"\x00edmi") ||
             StemmerUtil::endsWith(s, len, L"\x00edmu") ||
             StemmerUtil::endsWith(s, len, L"imu") ||
             StemmerUtil::endsWith(s, len, L"\x00e1ch") ||
             StemmerUtil::endsWith(s, len, L"ata") ||
             StemmerUtil::endsWith(s, len, L"aty") ||
             StemmerUtil::endsWith(s, len, L"\x00fdch") ||
             StemmerUtil::endsWith(s, len, L"ama") ||
             StemmerUtil::endsWith(s, len, L"ami") ||
             StemmerUtil::endsWith(s, len, L"ov\x00e9") ||
             StemmerUtil::endsWith(s, len, L"ovi") ||
             StemmerUtil::endsWith(s, len, L"\x00fdmi")))
            return len - 3;

        if (len > 4 && 
            (StemmerUtil::endsWith(s, len, L"em") ||
             StemmerUtil::endsWith(s, len, L"es") ||
             StemmerUtil::endsWith(s, len, L"\x00e9m") ||
             StemmerUtil::endsWith(s, len, L"\x00edm") ||
             StemmerUtil::endsWith(s, len, L"\x016fm") ||
             StemmerUtil::endsWith(s, len, L"at") ||
             StemmerUtil::endsWith(s, len, L"\x00e1m") ||
             StemmerUtil::endsWith(s, len, L"os") ||
             StemmerUtil::endsWith(s, len, L"us") ||
             StemmerUtil::endsWith(s, len, L"\x00fdm") ||
             StemmerUtil::endsWith(s, len, L"mi") ||
             StemmerUtil::endsWith(s, len, L"ou")))
            return len - 2;

        if (len > 3)
        {
            switch (s[len - 1])
            {
                case L'a':
                case L'e':
                case L'i':
                case L'o':
                case L'u':
                case L'\x016f':
                case L'y':
                case L'\x00e1':
                case L'\x00e9':
                case L'\x00ed':
                case L'\x00fd':
                case L'\x011b':
                    return len - 1;
            }
        }

        return len;
    }
    
    int32_t CzechStemmer::removePossessives(wchar_t* s, int32_t len)
    {
        if (len > 5 &&
            (StemmerUtil::endsWith(s, len, L"ov") ||
             StemmerUtil::endsWith(s, len, L"in") ||
             StemmerUtil::endsWith(s, len, L"\x016fv")))
            return len - 2;
        return len;
    }
    
    int32_t CzechStemmer::normalize(wchar_t* s, int32_t len)
    {
        if (StemmerUtil::endsWith(s, len, L"\x010dt"))
        {
            s[len - 2] = L'c';
            s[len - 1] = L'k';
            return len;
        }

        if (StemmerUtil::endsWith(s, len, L"\x0161t"))
        {
            s[len - 2] = L's';
            s[len - 1] = L'k';
            return len;
        }

        switch (s[len - 1])
        {
            case L'c':
            case L'\x010d':
                s[len - 1] = L'k';
                return len;
            case L'z':
            case L'\x017e':
                s[len - 1] = L'h';
                return len;
        }

        if (len > 1 && s[len - 2] == L'e')
        {
            s[len - 2] = s[len - 1];
            return len - 1;
        }

        if (len > 2 && s[len - 2] == L'\x016f')
        {
            s[len - 2] = L'o';
            return len;
        }

        return len;
    }
}
