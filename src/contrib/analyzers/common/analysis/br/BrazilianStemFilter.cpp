/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "BrazilianStemFilter.h"
#include "BrazilianStemmer.h"
#include "TermAttribute.h"

namespace Lucene {

BrazilianStemFilter::BrazilianStemFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    stemmer = newLucene<BrazilianStemmer>();
    termAtt = addAttribute<TermAttribute>();
}

BrazilianStemFilter::BrazilianStemFilter(const TokenStreamPtr& input, HashSet<String> exclusiontable) : TokenFilter(input) {
    stemmer = newLucene<BrazilianStemmer>();
    termAtt = addAttribute<TermAttribute>();
    exclusions = exclusiontable;
}

BrazilianStemFilter::~BrazilianStemFilter() {
}

bool BrazilianStemFilter::incrementToken() {
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

}
