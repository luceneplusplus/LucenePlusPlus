/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "FrenchMinimalStemmer.h"

namespace Lucene
{
    FrenchMinimalStemmer::~FrenchMinimalStemmer()
    {
    }

    int32_t FrenchMinimalStemmer::stem(wchar_t* s, int32_t len)
    {
        if (len < 6)
            return len;

        if (s[len - 1] == L'x')
        {
            if (s[len - 3] == L'a' && s[len - 2] == L'u')
            s[len - 2] = L'l';
            return len - 1;
        }

        if (s[len - 1] == L's')
            --len;
        if (s[len - 1] == L'r')
            --len;
        if (s[len - 1] == L'e')
            --len;
        if (s[len - 1] == L'\x00e9')
            --len;
        if (s[len - 1] == s[len - 2])
            --len;
        return len;
    }
}

