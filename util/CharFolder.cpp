/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CharFolder.h"

namespace Lucene
{
    bool CharFolder::lowerCache = CharFolder::fillLower();
    bool CharFolder::upperCache = CharFolder::fillUpper();
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
    
    bool CharFolder::fillLower()
    {
        for (int32_t index = CHAR_MIN; index < CHAR_MAX; ++index)
            lowerChars[index - CHAR_MIN] = std::tolower<wchar_t>((wchar_t)index, std::locale());
        return true;
    }
    
    bool CharFolder::fillUpper()
    {
        for (int32_t index = CHAR_MIN; index < CHAR_MAX; ++index)
            upperChars[index - CHAR_MIN] = std::toupper<wchar_t>((wchar_t)index, std::locale());
        return true;
    }
}
