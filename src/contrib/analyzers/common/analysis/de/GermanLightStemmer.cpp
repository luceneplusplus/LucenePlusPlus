/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GermanLightStemmer.h"
#include "MiscUtils.h"

namespace Lucene
{
    GermanLightStemmer::~GermanLightStemmer()
    {
    }

    int32_t GermanLightStemmer::stem(wchar_t* s, int32_t len)
    {
        for (int32_t i = 0; i < len; ++i)
        {
            switch (s[i])
            {
                case 0x00e4:
                case 0x00e0:
                case 0x00e1:
                case 0x00e2:
                    s[i] = L'a';
                    break;
                case 0x00f6:
                case 0x00f2:
                case 0x00f3:
                case 0x00f4:
                    s[i] = L'o';
                    break;
                case 0x00ef:
                case 0x00ec:
                case 0x00ed:
                case 0x00ee:
                    s[i] = L'i';
                    break;
                case 0x00fc:
                case 0x00f9:
                case 0x00fa:
                case 0x00fb:
                    s[i] = L'u';
                    break;
            }
        }
        len = step1(s, len);
        return step2(s, len);
    }

    bool GermanLightStemmer::stEnding(wchar_t ch)
    {
        switch (ch)
        {
            case L'b':
            case L'd':
            case L'f':
            case L'g':
            case L'h':
            case L'k':
            case L'l':
            case L'm':
            case L'n':
            case L't':
                return true;
            default:
                return false;
        }
    }

    int32_t GermanLightStemmer::step1(wchar_t* s, int32_t len)
    {
        if (len > 5 && s[len - 3] == L'e' && s[len - 2] == L'r' && s[len - 1] == L'n')
            return len - 3;

        if (len > 4 && s[len - 2] == L'e')
        {
            switch (s[len - 1])
            {
                case L'm':
                case L'n':
                case L'r':
                case L's':
                    return len - 2;
            }
        }

        if (len > 3 && s[len - 1] == L'e')
            return len - 1;

        if (len > 3 && s[len - 1] == L's' && stEnding(s[len - 2]))
            return len - 1;

        return len;
    }

    int32_t GermanLightStemmer::step2(wchar_t* s, int32_t len)
    {
        if (len > 5 && s[len - 3] == L'e' && s[len - 2] == L's' && s[len - 1] == L't')
            return len - 3;

        if (len > 4 && s[len - 2] == L'e' && (s[len - 1] == L'r' || s[len - 1] == L'n'))
            return len - 2;

        if (len > 4 && s[len - 2] == L's' && s[len - 1] == L't' && stEnding(s[len - 3]))
            return len - 2;

        return len;
    }
}

