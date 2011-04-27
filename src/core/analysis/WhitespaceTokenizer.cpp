/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "WhitespaceTokenizer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene
{
    WhitespaceTokenizer::WhitespaceTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input) : CharTokenizer(matchVersion, input)
    {
    }
    
    WhitespaceTokenizer::WhitespaceTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input) : CharTokenizer(matchVersion, source, input)
    {
    }
    
    WhitespaceTokenizer::WhitespaceTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input) : CharTokenizer(matchVersion, factory, input)
    {
    }
    
    WhitespaceTokenizer::WhitespaceTokenizer(ReaderPtr input) : CharTokenizer(input)
    {
    }
    
    WhitespaceTokenizer::WhitespaceTokenizer(AttributeSourcePtr source, ReaderPtr input) : CharTokenizer(source, input)
    {
    }
    
    WhitespaceTokenizer::WhitespaceTokenizer(AttributeFactoryPtr factory, ReaderPtr input) : CharTokenizer(factory, input)
    {
    }
    
    WhitespaceTokenizer::~WhitespaceTokenizer()
    {
    }
    
    bool WhitespaceTokenizer::isTokenChar(wchar_t c)
    {
        return !UnicodeUtil::isSpace(c);
    }
}
