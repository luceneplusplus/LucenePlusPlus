/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "RussianLightStemFilter.h"
#include "RussianLightStemmer.h"
#include "CharTermAttribute.h"
#include "KeywordAttribute.h"

namespace Lucene
{
    RussianLightStemFilter::RussianLightStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<RussianLightStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
    }

    RussianLightStemFilter::~RussianLightStemFilter()
    {
    }

    bool RussianLightStemFilter::incrementToken()
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

