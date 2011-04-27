/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "LowerCaseTokenizer.h"
#include "CharFolder.h"

namespace Lucene
{
    LowerCaseTokenizer::LowerCaseTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input) : LetterTokenizer(matchVersion, input)
    {
    }
    
    LowerCaseTokenizer::LowerCaseTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input) : LetterTokenizer(matchVersion, source, input)
    {
    }
    
    LowerCaseTokenizer::LowerCaseTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input) : LetterTokenizer(matchVersion, factory, input)
    {
    }
    
    LowerCaseTokenizer::LowerCaseTokenizer(ReaderPtr input) : LetterTokenizer(LuceneVersion::LUCENE_30, input)
    {
    }
    
    LowerCaseTokenizer::LowerCaseTokenizer(AttributeSourcePtr source, ReaderPtr input) : LetterTokenizer(LuceneVersion::LUCENE_30, source, input)
    {
    }
    
    LowerCaseTokenizer::LowerCaseTokenizer(AttributeFactoryPtr factory, ReaderPtr input) : LetterTokenizer(LuceneVersion::LUCENE_30, factory, input)
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
