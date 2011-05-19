/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ArabicLetterTokenizer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene
{
    ArabicLetterTokenizer::ArabicLetterTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input) : LetterTokenizer(matchVersion, input)
    {
    }
    
    ArabicLetterTokenizer::ArabicLetterTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input) : LetterTokenizer(matchVersion, source, input)
    {
    }
    
    ArabicLetterTokenizer::ArabicLetterTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input) : LetterTokenizer(matchVersion, factory, input)
    {
    }
    
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
