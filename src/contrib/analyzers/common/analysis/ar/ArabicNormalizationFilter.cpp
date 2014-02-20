/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ArabicNormalizationFilter.h"
#include "ArabicNormalizer.h"
#include "TermAttribute.h"

namespace Lucene {

ArabicNormalizationFilter::ArabicNormalizationFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    normalizer = newLucene<ArabicNormalizer>();
    termAtt = addAttribute<TermAttribute>();
}

ArabicNormalizationFilter::~ArabicNormalizationFilter() {
}

bool ArabicNormalizationFilter::incrementToken() {
    if (input->incrementToken()) {
        int32_t newlen = normalizer->normalize(termAtt->termBuffer().get(), termAtt->termLength());
        termAtt->setTermLength(newlen);
        return true;
    } else {
        return false;
    }
}

}
