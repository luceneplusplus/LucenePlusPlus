/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "KeywordMarkerFilter.h"
#include "CharArraySet.h"
#include "KeywordAttribute.h"
#include "CharTermAttribute.h"

namespace Lucene
{
    KeywordMarkerFilter::KeywordMarkerFilter(TokenStreamPtr input, CharArraySetPtr keywordSet) : TokenFilter(input)
    {
        this->keywordSet = keywordSet;
        this->keywordAttr = addAttribute<KeywordAttribute>();
        this->termAtt = addAttribute<CharTermAttribute>();
    }
    
    KeywordMarkerFilter::KeywordMarkerFilter(TokenStreamPtr input, HashSet<String> keywordSet) : TokenFilter(input)
    {
        this->keywordSet = newLucene<CharArraySet>(LuceneVersion::LUCENE_31, keywordSet, false);
        this->keywordAttr = addAttribute<KeywordAttribute>();
        this->termAtt = addAttribute<CharTermAttribute>();
    }
    
    KeywordMarkerFilter::~KeywordMarkerFilter()
    {
    }
    
    bool KeywordMarkerFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            if (keywordSet->contains(termAtt->buffer().get(), 0, termAtt->length()))
                keywordAttr->setKeyword(true);
            return true;
        }
        else
            return false;
    }
}
