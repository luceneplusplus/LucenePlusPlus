/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GermanStemFilter.h"
#include "GermanStemmer.h"
#include "TermAttribute.h"

namespace Lucene {

GermanStemFilter::GermanStemFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    stemmer = newLucene<GermanStemmer>();
    termAtt = addAttribute<TermAttribute>();
}

GermanStemFilter::GermanStemFilter(const TokenStreamPtr& input, HashSet<String> exclusionSet) : TokenFilter(input) {
    stemmer = newLucene<GermanStemmer>();
    termAtt = addAttribute<TermAttribute>();
    this->exclusionSet = exclusionSet;
}

GermanStemFilter::~GermanStemFilter() {
}

bool GermanStemFilter::incrementToken() {
    if (input->incrementToken()) {
        String term(termAtt->term());
        // Check the exclusion table.
        if (!exclusionSet || !exclusionSet.contains(term)) {
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

void GermanStemFilter::setStemmer(const GermanStemmerPtr& stemmer) {
    if (stemmer) {
        this->stemmer = stemmer;
    }
}

void GermanStemFilter::setExclusionSet(HashSet<String> exclusionSet) {
    this->exclusionSet = exclusionSet;
}

}
