/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CharFolder.h"

namespace Lucene
{
    bool CharFolder::lowerCache = false;
    bool CharFolder::upperCache = false;
    wchar_t CharFolder::lowerChars[CHAR_MAX - CHAR_MIN + 1];
    wchar_t CharFolder::upperChars[CHAR_MAX - CHAR_MIN + 1];

    CharFolder::~CharFolder()
    {
    }
    
    wchar_t CharFolder::toLower(wchar_t ch)
    {
        return (ch > CHAR_MIN && ch < CHAR_MAX) ? lowerChars[ch - CHAR_MIN] : std::tolower<wchar_t>(ch, std::locale());
    }
    
    wchar_t CharFolder::toUpper(wchar_t ch)
    {
        return (ch > CHAR_MIN && ch < CHAR_MAX) ? upperChars[ch - CHAR_MIN] : std::toupper<wchar_t>(ch, std::locale());
    }
    
    void CharFolder::fillLower()
    {
        if (!lowerCache)
        {
            for (int32_t index = CHAR_MIN; index < CHAR_MAX; ++index)
                lowerChars[index - CHAR_MIN] = std::tolower<wchar_t>((wchar_t)index, std::locale());
            lowerCache = true;
        }
    }
    
    void CharFolder::fillUpper()
    {
        if (!upperCache)
        {
            for (int32_t index = CHAR_MIN; index < CHAR_MAX; ++index)
                upperChars[index - CHAR_MIN] = std::toupper<wchar_t>((wchar_t)index, std::locale());
            upperCache = true;
        }
    }
}
