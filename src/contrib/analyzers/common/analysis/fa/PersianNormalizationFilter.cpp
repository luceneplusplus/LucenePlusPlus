/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "PersianNormalizationFilter.h"
#include "PersianNormalizer.h"
#include "TermAttribute.h"

namespace Lucene {

PersianNormalizationFilter::PersianNormalizationFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    normalizer = newLucene<PersianNormalizer>();
    termAtt = addAttribute<TermAttribute>();
}

PersianNormalizationFilter::~PersianNormalizationFilter() {
}

bool PersianNormalizationFilter::incrementToken() {
    if (input->incrementToken()) {
        int32_t newlen = normalizer->normalize(termAtt->termBuffer().get(), termAtt->termLength());
        termAtt->setTermLength(newlen);
        return true;
    } else {
        return false;
    }
}

}
