/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "FrenchLightStemmer.h"
#include "MiscUtils.h"
#include "StemmerUtil.h"

namespace Lucene
{
    FrenchLightStemmer::~FrenchLightStemmer()
    {
    }

    int32_t FrenchLightStemmer::stem(wchar_t* s, int32_t len)
    {
        if (len > 5 && s[len - 1] == L'x')
        {
            if (s[len - 3] == L'a' && s[len - 2] == L'u' && s[len - 4] != L'e')
                s[len - 2] = L'l';
            --len;
        }

        if (len > 3 && s[len - 1] == L'x')
            --len;

        if (len > 3 && s[len - 1] == L's')
            --len;

        if (len > 9 && StemmerUtil::endsWith(s, len, L"issement"))
        {
            len -= 6;
            s[len - 1] = L'r';
            return norm(s, len);
        }

        if (len > 8 && StemmerUtil::endsWith(s, len, L"issant"))
        {
            len -= 4;
            s[len - 1] = L'r';
            return norm(s, len);
        }

        if (len > 6 && StemmerUtil::endsWith(s, len, L"ement"))
        {
            len -= 4;
            if (len > 3 && StemmerUtil::endsWith(s, len, L"ive"))
            {
                --len;
                s[len - 1] = L'f';
            }
            return norm(s, len);
        }

        if (len > 11 && StemmerUtil::endsWith(s, len, L"ficatrice"))
        {
            len -= 5;
            s[len - 2] = L'e';
            s[len - 1] = L'r';
            return norm(s, len);
        }

        if (len > 10 && StemmerUtil::endsWith(s, len, L"ficateur"))
        {
        len -= 4;
        s[len - 2] = L'e';
        s[len - 1] = L'r';
        return norm(s, len);
        }

        if (len > 9 && StemmerUtil::endsWith(s, len, L"catrice"))
        {
            len -= 3;
            s[len - 4] = L'q';
            s[len - 3] = L'u';
            s[len - 2] = L'e';
            // s[len - 1] = L'r' <-- unnecessary, already 'r'.
            return norm(s, len);
        }

        if (len > 8 && StemmerUtil::endsWith(s, len, L"cateur"))
        {
            len -= 2;
            s[len - 4] = L'q';
            s[len - 3] = L'u';
            s[len - 2] = L'e';
            s[len - 1] = L'r';
            return norm(s, len);
        }

        if (len > 8 && StemmerUtil::endsWith(s, len, L"atrice"))
        {
            len -= 4;
            s[len - 2] = L'e';
            s[len - 1] = L'r';
            return norm(s, len);
        }

        if (len > 7 && StemmerUtil::endsWith(s, len, L"ateur"))
        {
            len -= 3;
            s[len - 2] = L'e';
            s[len - 1] = L'r';
            return norm(s, len);
        }

        if (len > 6 && StemmerUtil::endsWith(s, len, L"trice"))
        {
            --len;
            s[len - 3] = L'e';
            s[len - 2] = L'u';
            s[len - 1] = L'r';
        }

        if (len > 5 && StemmerUtil::endsWith(s, len, L"i\x00e8me"))
            return norm(s, len - 4);

        if (len > 7 && StemmerUtil::endsWith(s, len, L"teuse"))
        {
            len -= 2;
            s[len - 1] = L'r';
            return norm(s, len);
        }

        if (len > 6 && StemmerUtil::endsWith(s, len, L"teur"))
        {
            --len;
            s[len - 1] = L'r';
            return norm(s, len);
        }

        if (len > 5 && StemmerUtil::endsWith(s, len, L"euse"))
            return norm(s, len - 2);

        if (len > 8 && StemmerUtil::endsWith(s, len, L"\x00e8re"))
        {
            --len;
            s[len - 2] = L'e';
            return norm(s, len);
        }

        if (len > 7 && StemmerUtil::endsWith(s, len, L"ive"))
        {
            --len;
            s[len - 1] = L'f';
            return norm(s, len);
        }

        if (len > 4 && (StemmerUtil::endsWith(s, len, L"folle") ||
                        StemmerUtil::endsWith(s, len, L"molle")))
        {
            len -= 2;
            s[len - 1] = L'u';
            return norm(s, len);
        }

        if (len > 9 && StemmerUtil::endsWith(s, len, L"nnelle"))
            return norm(s, len - 5);

        if (len > 9 && StemmerUtil::endsWith(s, len, L"nnel"))
            return norm(s, len - 3);

        if (len > 4 && StemmerUtil::endsWith(s, len, L"\x00e8te"))
        {
            --len;
            s[len - 2] = L'e';
        }

        if (len > 8 && StemmerUtil::endsWith(s, len, L"ique"))
            len -= 4;

        if (len > 8 && StemmerUtil::endsWith(s, len, L"esse"))
            return norm(s, len - 3);

        if (len > 7 && StemmerUtil::endsWith(s, len, L"inage"))
            return norm(s, len - 3);

        if (len > 9 && StemmerUtil::endsWith(s, len, L"isation"))
        {
            len -= 7;
            if (len > 5 && StemmerUtil::endsWith(s, len, L"ual"))
               s[len - 2] = L'e';
            return norm(s, len);
        }

        if (len > 9 && StemmerUtil::endsWith(s, len, L"isateur"))
            return norm(s, len - 7);

        if (len > 8 && StemmerUtil::endsWith(s, len, L"ation"))
            return norm(s, len - 5);

        if (len > 8 && StemmerUtil::endsWith(s, len, L"ition"))
            return norm(s, len - 5);

        return norm(s, len);
    }

    int32_t FrenchLightStemmer::norm(wchar_t* s, int32_t len)
    {
        if (len > 4)
        {
            for (int32_t i = 0; i < len; ++i)
            {
                switch(s[i])
                {
                    case L'\x00e0':
                    case L'\x00e1':
                    case L'\x00e2':
                        s[i] = L'a';
                        break;
                    case L'\x00f4':
                        s[i] = L'o';
                        break;
                    case L'\x00e8':
                    case L'\x00e9':
                    case L'\x00ea':
                        s[i] = L'e';
                        break;
                    case L'\x00f9':
                    case L'\x00fb':
                        s[i] = L'u';
                        break;
                    case L'\x00ee':
                        s[i] = L'i';
                        break;
                    case L'\x00e7':
                        s[i] = L'c';
                        break;
                }
            }

            wchar_t ch = s[0];
            for (int32_t i = 1; i < len; ++i)
            {
                if (s[i] == ch)
                    len = StemmerUtil::_delete(s, i--, len);
                else
                    ch = s[i];
            }
        }

        if (len > 4 && StemmerUtil::endsWith(s, len, L"ie"))
            len -= 2;

        if (len > 4)
        {
            if (s[len - 1] == L'r')
                --len;
            if (s[len - 1] == L'e')
                --len;
            if (s[len - 1] == L'e')
                --len;
            if (s[len - 1] == s[len - 2])
                --len;
        }
        return len;
    }
}

