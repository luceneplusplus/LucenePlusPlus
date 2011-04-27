/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "LowerCaseFilter.h"
#include "CharTermAttribute.h"
#include "CharFolder.h"

namespace Lucene
{
    LowerCaseFilter::LowerCaseFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input) : TokenFilter(input)
    {
        termAtt = addAttribute<CharTermAttribute>();
    }
    
    LowerCaseFilter::LowerCaseFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        termAtt = addAttribute<CharTermAttribute>();
    }
    
    LowerCaseFilter::~LowerCaseFilter()
    {
    }
    
    bool LowerCaseFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            wchar_t* buffer = termAtt->bufferArray();
            CharFolder::toLower(buffer, buffer + termAtt->length());
            return true;
        }
        return false;
    }
}
