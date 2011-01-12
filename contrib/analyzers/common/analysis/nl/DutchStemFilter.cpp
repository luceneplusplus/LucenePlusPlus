/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "DutchStemFilter.h"
#include "DutchStemmer.h"
#include "TermAttribute.h"

namespace Lucene
{
    DutchStemFilter::DutchStemFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        stemmer = newLucene<DutchStemmer>();
        termAtt = addAttribute<TermAttribute>();
    }
    
    DutchStemFilter::DutchStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable) : TokenFilter(input)
    {
        stemmer = newLucene<DutchStemmer>();
        termAtt = addAttribute<TermAttribute>();
        this->exclusions = exclusiontable;
    }
    
    DutchStemFilter::DutchStemFilter(TokenStreamPtr input, HashSet<String> exclusiontable, MapStringString stemdictionary) : TokenFilter(input)
    {
        stemmer = newLucene<DutchStemmer>();
        termAtt = addAttribute<TermAttribute>();
        this->exclusions = exclusiontable;
        this->stemmer->setStemDictionary(stemdictionary);
    }
    
    DutchStemFilter::~DutchStemFilter()
    {
    }
    
    bool DutchStemFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            String term(termAtt->term());
            
            // Check the exclusion table.
            if (!exclusions || !exclusions.contains(term))
            {
                String s(stemmer->stem(term));
                // If not stemmed, don't waste the time adjusting the token.
                if (!s.empty() && s != term)
                    termAtt->setTermBuffer(s);
            }
            return true;
        }
        else
            return false;
    }
    
    void DutchStemFilter::setStemmer(DutchStemmerPtr stemmer)
    {
        if (stemmer)
            this->stemmer = stemmer;
    }
    
    void DutchStemFilter::setExclusionSet(HashSet<String> exclusiontable)
    {
        this->exclusions = exclusiontable;
    }
    
    void DutchStemFilter::setStemDictionary(MapStringString dict)
    {
        if (stemmer)
            this->stemmer->setStemDictionary(dict);
    }
}
