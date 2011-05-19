/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "RussianStemFilter.h"
#include "RussianStemmer.h"
#include "CharTermAttribute.h"
#include "KeywordAttribute.h"

namespace Lucene
{
    RussianStemFilter::RussianStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<RussianStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
    }

    RussianStemFilter::~RussianStemFilter()
    {
    }

    bool RussianStemFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            if (!keywordAttr->isKeyword())
            {
                String term(termAtt->toString());
                String s(stemmer->stem(term));
                if (!s.empty() && s != term)
                    termAtt->setEmpty()->append(s);
            }
            return true;
        }
        else
            return false;
    }

    void RussianStemFilter::setStemmer(RussianStemmerPtr stemmer)
    {
        if (stemmer)
            this->stemmer = stemmer;
    }
}

