/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "BrazilianStemFilter.h"
#include "BrazilianStemmer.h"
#include "CharTermAttribute.h"
#include "KeywordAttribute.h"

namespace Lucene
{
    BrazilianStemFilter::BrazilianStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<BrazilianStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
    }

    BrazilianStemFilter::BrazilianStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable) : TokenFilter(input)
    {
        stemmer = newLucene<BrazilianStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
        exclusions = exclusiontable;
    }

    BrazilianStemFilter::~BrazilianStemFilter()
    {
    }

    bool BrazilianStemFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            String term(termAtt->toString());
            // Check the exclusion table.
            if (!keywordAttr->isKeyword() && (!exclusions || !exclusions.contains(term)))
            {
                String s(stemmer->stem(term));
                // If not stemmed, don't waste the time adjusting the token.
                if (!s.empty() && s != term)
                    termAtt->setEmpty()->append(s);
            }
            return true;
        }
        else
            return false;
    }
}

