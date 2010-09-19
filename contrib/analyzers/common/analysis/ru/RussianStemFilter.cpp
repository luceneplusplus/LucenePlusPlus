/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RussianStemFilter.h"
#include "RussianStemmer.h"
#include "TermAttribute.h"

namespace Lucene
{
    RussianStemFilter::RussianStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<RussianStemmer>();
        termAtt = addAttribute<TermAttribute>();
    }
    
    RussianStemFilter::~RussianStemFilter()
    {
    }
    
    bool RussianStemFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            String term(termAtt->term());
            String s(stemmer->stem(term));
            if (!s.empty() && s != term)
                termAtt->setTermBuffer(s);
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
