/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "RussianLightStemmer.h"
#include "StemmerUtil.h"
#include "MiscUtils.h"

namespace Lucene
{
    RussianLightStemmer::~RussianLightStemmer()
    {
    }

    int32_t RussianLightStemmer::stem(wchar_t* s, int32_t len)
    {
        len = removeCase(s, len);
        return normalize(s, len);
    }

    int32_t RussianLightStemmer::normalize(wchar_t* s, int32_t len)
    {
        if (len > 3)
        {
            switch (s[len - 1])
            {
                case L'\x044c':
                case L'\x0438':
                    return len - 1;
                case L'\x043d':
                    if (s[len - 2] == L'\x043d')
                        return len - 1;
            }
        }
        return len;
    }

    int32_t RussianLightStemmer::removeCase(wchar_t* s, int32_t len)
    {
        if (len > 6 &&
            (StemmerUtil::endsWith(s, len, L"\x0438\x044f\x043c\x0438") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x044f\x043c\x0438")))
        return len - 4;

        if (len > 5 &&
            (StemmerUtil::endsWith(s, len, L"\x0438\x044f\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x0438\x044f\x0445") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x044f\x0445") ||
            StemmerUtil::endsWith(s, len, L"\x044f\x043c\x0438") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x044f\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x044c\x0432") ||
            StemmerUtil::endsWith(s, len, L"\x0430\x043c\x0438") ||
            StemmerUtil::endsWith(s, len, L"\x0435\x0433\x043e") ||
            StemmerUtil::endsWith(s, len, L"\x0435\x043c\x0443") ||
            StemmerUtil::endsWith(s, len, L"\x0435\x0440\x0438") ||
            StemmerUtil::endsWith(s, len, L"\x0438\x043c\x0438") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x0433\x043e") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x043c\x0443") ||
            StemmerUtil::endsWith(s, len, L"\x044b\x043c\x0438") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x0435\x0432")))
            return len - 3;

        if (len > 4 &&
            (StemmerUtil::endsWith(s, len, L"\x0430\x044f") ||
            StemmerUtil::endsWith(s, len, L"\x044f\x044f") ||
            StemmerUtil::endsWith(s, len, L"\x044f\x0445") ||
            StemmerUtil::endsWith(s, len, L"\x044e\x044e") ||
            StemmerUtil::endsWith(s, len, L"\x0430\x0445") ||
            StemmerUtil::endsWith(s, len, L"\x0435\x044e") ||
            StemmerUtil::endsWith(s, len, L"\x0438\x0445") ||
            StemmerUtil::endsWith(s, len, L"\x0438\x044f") ||
            StemmerUtil::endsWith(s, len, L"\x0438\x044e") ||
            StemmerUtil::endsWith(s, len, L"\x044c\x0432") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x044e") ||
            StemmerUtil::endsWith(s, len, L"\x0443\x044e") ||
            StemmerUtil::endsWith(s, len, L"\x044f\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x044b\x0445") ||
            StemmerUtil::endsWith(s, len, L"\x0435\x044f") ||
            StemmerUtil::endsWith(s, len, L"\x0430\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x0435\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x0435\x0439") ||
            StemmerUtil::endsWith(s, len, L"\x0451\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x0435\x0432") ||
            StemmerUtil::endsWith(s, len, L"\x0438\x0439") ||
            StemmerUtil::endsWith(s, len, L"\x0438\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x0435") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x0439") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x043e\x0432") ||
            StemmerUtil::endsWith(s, len, L"\x044b\x0435") ||
            StemmerUtil::endsWith(s, len, L"\x044b\x0439") ||
            StemmerUtil::endsWith(s, len, L"\x044b\x043c") ||
            StemmerUtil::endsWith(s, len, L"\x043c\x0438")))
            return len - 2;

        if (len > 3)
        {
            switch (s[len - 1])
            {
                case L'\x0430':
                case L'\x0435':
                case L'\x0438':
                case L'\x043e':
                case L'\x0443':
                case L'\x0439':
                case L'\x044b':
                case L'\x044f':
                case L'\x044c':
                    return len - 1;
            }
        }

        return len;
    }
}

