/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ArabicLetterTokenizer.h"
#include "UnicodeUtils.h"

namespace Lucene
{
    ArabicLetterTokenizer::ArabicLetterTokenizer(ReaderPtr input) : LetterTokenizer(input)
    {
    }
    
    ArabicLetterTokenizer::ArabicLetterTokenizer(AttributeSourcePtr source, ReaderPtr input) : LetterTokenizer(source, input)
    {
    }
    
    ArabicLetterTokenizer::ArabicLetterTokenizer(AttributeFactoryPtr factory, ReaderPtr input) : LetterTokenizer(factory, input)
    {
    }
    
    ArabicLetterTokenizer::~ArabicLetterTokenizer()
    {
    }
    
    bool ArabicLetterTokenizer::isTokenChar(wchar_t c)
    {
        return LetterTokenizer::isTokenChar(c) || UnicodeUtil::isNonSpacing(c);
    }
}
