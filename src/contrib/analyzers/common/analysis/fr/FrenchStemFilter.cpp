/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "FrenchStemFilter.h"
#include "FrenchStemmer.h"
#include "CharTermAttribute.h"
#include "KeywordAttribute.h"

namespace Lucene
{
    FrenchStemFilter::FrenchStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<FrenchStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
    }

    FrenchStemFilter::FrenchStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable) : TokenFilter(input)
    {
        stemmer = newLucene<FrenchStemmer>();
        termAtt = addAttribute<CharTermAttribute>();
        keywordAttr = addAttribute<KeywordAttribute>();
        this->exclusions = exclusiontable;
    }

    FrenchStemFilter::~FrenchStemFilter()
    {
    }

    bool FrenchStemFilter::incrementToken()
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

    void FrenchStemFilter::setStemmer(FrenchStemmerPtr stemmer)
    {
        if (stemmer)
            this->stemmer = stemmer;
    }

    void FrenchStemFilter::setExclusionSet(HashSet<String> exclusiontable)
    {
        this->exclusions = exclusiontable;
    }
}

