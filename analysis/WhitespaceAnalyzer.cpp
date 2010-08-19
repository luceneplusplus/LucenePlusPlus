/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WhitespaceAnalyzer.h"
#include "WhitespaceTokenizer.h"

namespace Lucene
{
    WhitespaceAnalyzer::~WhitespaceAnalyzer()
    {
    }
    
    TokenStreamPtr WhitespaceAnalyzer::tokenStream(const String& fieldName, ReaderPtr reader)
    {
        return newLucene<WhitespaceTokenizer>(reader);
    }
    
    TokenStreamPtr WhitespaceAnalyzer::reusableTokenStream(const String& fieldName, ReaderPtr reader)
    {
        TokenizerPtr tokenizer(boost::dynamic_pointer_cast<Tokenizer>(getPreviousTokenStream()));
        if (!tokenizer)
        {
            tokenizer = newLucene<WhitespaceTokenizer>(reader);
            setPreviousTokenStream(tokenizer);
        }
        else
            tokenizer->reset(reader);
        return tokenizer;
    }
}
