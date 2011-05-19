/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GermanLightStemFilter.h"
#include "GermanLightStemmer.h"
#include "CharTermAttribute.h"
#include "KeywordAttribute.h"

namespace Lucene
{
    GermanLightStemFilter::GermanLightStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<GermanLightStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
    }

    GermanLightStemFilter::~GermanLightStemFilter()
    {
    }

    bool GermanLightStemFilter::incrementToken()
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

