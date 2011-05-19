/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "FrenchMinimalStemFilter.h"
#include "FrenchMinimalStemmer.h"
#include "CharTermAttribute.h"
#include "KeywordAttribute.h"

namespace Lucene
{
    FrenchMinimalStemFilter::FrenchMinimalStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<FrenchMinimalStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
    }

    FrenchMinimalStemFilter::~FrenchMinimalStemFilter()
    {
    }

    bool FrenchMinimalStemFilter::incrementToken()
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

