/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LowerCaseTokenizer.h"
#include "CharFolder.h"

namespace Lucene
{
    LowerCaseTokenizer::LowerCaseTokenizer(ReaderPtr input) : LetterTokenizer(input)
    {
    }
    
    LowerCaseTokenizer::LowerCaseTokenizer(AttributeSourcePtr source, ReaderPtr input) : LetterTokenizer(source, input)
    {
    }
    
    LowerCaseTokenizer::LowerCaseTokenizer(AttributeFactoryPtr factory, ReaderPtr input) : LetterTokenizer(factory, input)
    {
    }
    
    LowerCaseTokenizer::~LowerCaseTokenizer()
    {
    }
    
    wchar_t LowerCaseTokenizer::normalize(wchar_t c)
    {
        return CharFolder::toLower(c);
    }
}
