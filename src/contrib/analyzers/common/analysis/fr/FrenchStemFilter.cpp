/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "FrenchStemFilter.h"
#include "FrenchStemmer.h"
#include "TermAttribute.h"

namespace Lucene {

FrenchStemFilter::FrenchStemFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    stemmer = newLucene<FrenchStemmer>();
    termAtt = addAttribute<TermAttribute>();
}

FrenchStemFilter::FrenchStemFilter(const TokenStreamPtr& input, HashSet<String> exclusiontable) : TokenFilter(input) {
    stemmer = newLucene<FrenchStemmer>();
    termAtt = addAttribute<TermAttribute>();
    this->exclusions = exclusiontable;
}

FrenchStemFilter::~FrenchStemFilter() {
}

bool FrenchStemFilter::incrementToken() {
    if (input->incrementToken()) {
        String term(termAtt->term());
        // Check the exclusion table.
        if (!exclusions || !exclusions.contains(term)) {
            String s(stemmer->stem(term));
            // If not stemmed, don't waste the time adjusting the token.
            if (!s.empty() && s != term) {
                termAtt->setTermBuffer(s);
            }
        }
        return true;
    } else {
        return false;
    }
}

void FrenchStemFilter::setStemmer(const FrenchStemmerPtr& stemmer) {
    if (stemmer) {
        this->stemmer = stemmer;
    }
}

void FrenchStemFilter::setExclusionSet(HashSet<String> exclusiontable) {
    this->exclusions = exclusiontable;
}

}
