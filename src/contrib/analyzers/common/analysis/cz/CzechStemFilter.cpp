/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "CzechStemFilter.h"
#include "CzechStemmer.h"
#include "CharTermAttribute.h"
#include "KeywordAttribute.h"

namespace Lucene
{
    CzechStemFilter::CzechStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<CzechStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
    }
    
    CzechStemFilter::~CzechStemFilter()
    {
    }
    
    bool CzechStemFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            if (!keywordAttr->isKeyword())
            {
                int32_t newlen = stemmer->stem(termAtt->buffer().get(), termAtt->length());
                termAtt->setLength(newlen);
            }
            return true;
        }
        else
            return false;
    }
}
