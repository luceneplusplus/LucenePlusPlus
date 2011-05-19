/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GermanMinimalStemmer.h"
#include "MiscUtils.h"

namespace Lucene
{
    GermanMinimalStemmer::~GermanMinimalStemmer()
    {
    }

    int32_t GermanMinimalStemmer::stem(wchar_t* s, int32_t len)
    {
        if (len < 5)
            return len;

        for (int32_t i = 0; i < len; ++i)
        {
            switch (s[i])
            {
                case 0x00e4:
                    s[i] = L'a';
                    break;
                case 0x00f6:
                    s[i] = L'o';
                    break;
                case 0x00fc:
                    s[i] = L'u';
                    break;
            }
        }

        if (len > 6 && s[len - 3] == L'n' && s[len - 2] == L'e' && s[len - 1] == L'n')
            return len - 3;

        if (len > 5)
        {
            switch (s[len - 1])
            {
                case L'n':
                    if (s[len - 2] == L'e')
                        return len - 2;
                    else
                        break;
                case L'e':
                    if (s[len - 2] == L's')
                        return len - 2;
                    else
                        break;
                case L's':
                    if (s[len - 2] == L'e')
                        return len - 2;
                    else
                        break;
                case L'r':
                    if (s[len - 2] == L'e')
                        return len - 2;
                    else
                        break;
            }
        }

        switch (s[len - 1])
        {
            case L'n':
            case L'e':
            case L's':
            case L'r':
                return len - 1;
        }

        return len;
    }
}

