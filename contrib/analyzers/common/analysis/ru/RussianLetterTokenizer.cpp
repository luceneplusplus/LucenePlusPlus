/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RussianLetterTokenizer.h"
#include "UnicodeUtils.h"

namespace Lucene
{
    RussianLetterTokenizer::RussianLetterTokenizer(ReaderPtr input) : CharTokenizer(input)
    {
    }
    
    RussianLetterTokenizer::RussianLetterTokenizer(AttributeSourcePtr source, ReaderPtr input) : CharTokenizer(source, input)
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
