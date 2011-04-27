/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "LetterTokenizer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene
{
    LetterTokenizer::LetterTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input) : CharTokenizer(matchVersion, input)
    {
    }
    
    LetterTokenizer::LetterTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input) : CharTokenizer(matchVersion, source, input)
    {
    }
    
    LetterTokenizer::LetterTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input) : CharTokenizer(matchVersion, factory, input)
    {
    }
    
    LetterTokenizer::LetterTokenizer(ReaderPtr input) : CharTokenizer(LuceneVersion::LUCENE_30, input)
    {
    }
    
    LetterTokenizer::LetterTokenizer(AttributeSourcePtr source, ReaderPtr input) : CharTokenizer(LuceneVersion::LUCENE_30, source, input)
    {
    }
    
    LetterTokenizer::LetterTokenizer(AttributeFactoryPtr factory, ReaderPtr input) : CharTokenizer(LuceneVersion::LUCENE_30, factory, input)
    {
    }
    
    LetterTokenizer::~LetterTokenizer()
    {
    }
    
    bool LetterTokenizer::isTokenChar(wchar_t c)
    {
        return UnicodeUtil::isAlpha(c);
    }
}
