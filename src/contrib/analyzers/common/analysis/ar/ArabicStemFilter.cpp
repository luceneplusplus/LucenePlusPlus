/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ArabicStemFilter.h"
#include "ArabicStemmer.h"
#include "TermAttribute.h"

namespace Lucene {

ArabicStemFilter::ArabicStemFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    stemmer = newLucene<ArabicStemmer>();
    termAtt = addAttribute<TermAttribute>();
}

ArabicStemFilter::~ArabicStemFilter() {
}

bool ArabicStemFilter::incrementToken() {
    if (input->incrementToken()) {
        int32_t newlen = stemmer->stem(termAtt->termBuffer().get(), termAtt->termLength());
        termAtt->setTermLength(newlen);
        return true;
    } else {
        return false;
    }
}

}
