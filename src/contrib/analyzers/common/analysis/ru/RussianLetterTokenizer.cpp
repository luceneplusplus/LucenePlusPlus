/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "RussianLetterTokenizer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene
{
    RussianLetterTokenizer::RussianLetterTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input) : CharTokenizer(matchVersion, input)
    {
    }

    RussianLetterTokenizer::RussianLetterTokenizer(ReaderPtr input) : CharTokenizer(input)
    {
    }

    RussianLetterTokenizer::RussianLetterTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input) : CharTokenizer(matchVersion, source, input)
    {
    }

    RussianLetterTokenizer::RussianLetterTokenizer(AttributeSourcePtr source, ReaderPtr input) : CharTokenizer(source, input)
    {
    }

    RussianLetterTokenizer::RussianLetterTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input) : CharTokenizer(matchVersion, factory, input)
    {
    }

    RussianLetterTokenizer::RussianLetterTokenizer(AttributeFactoryPtr factory, ReaderPtr input) : CharTokenizer(factory, input)
    {
    }

    RussianLetterTokenizer::~RussianLetterTokenizer()
    {
    }

    bool RussianLetterTokenizer::isTokenChar(wchar_t c)
    {
        return (UnicodeUtil::isAlpha(c) || UnicodeUtil::isDigit(c));
    }
}

