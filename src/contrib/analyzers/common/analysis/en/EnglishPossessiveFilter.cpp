/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "EnglishPossessiveFilter.h"
#include "CharTermAttribute.h"

namespace Lucene
{
    EnglishPossessiveFilter::EnglishPossessiveFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        termAtt = addAttribute<CharTermAttribute>();
    }

    EnglishPossessiveFilter::~EnglishPossessiveFilter()
    {
    }

    bool EnglishPossessiveFilter::incrementToken()
    {
        if (!input->incrementToken())
            return false;

        wchar_t* buffer = termAtt->buffer().get();
        int32_t bufferLength = termAtt->length();

        if (bufferLength >= 2 && buffer[bufferLength - 2] == L'\'' &&
            (buffer[bufferLength - 1] == L's' || buffer[bufferLength - 1] == L'S'))
            termAtt->setLength(bufferLength - 2); // Strip last 2 characters off

        return true;
    }
}

