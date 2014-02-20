/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "SnowballFilter.h"
#include "TermAttribute.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"
#include "StringUtils.h"
#include "libstemmer_c/include/libstemmer.h"

namespace Lucene {

SnowballFilter::SnowballFilter(const TokenStreamPtr& input, const String& name) : TokenFilter(input) {
    stemmer = sb_stemmer_new(StringUtils::toUTF8(name).c_str(), "UTF_8");
    if (stemmer == NULL) {
        boost::throw_exception(IllegalArgumentException(L"language not available for stemming:" + name));
    }
    termAtt = addAttribute<TermAttribute>();
    utf8Result = newLucene<UTF8Result>();
}

SnowballFilter::~SnowballFilter() {
}

bool SnowballFilter::incrementToken() {
    if (input->incrementToken()) {
        StringUtils::toUTF8(termAtt->termBuffer().get(), termAtt->termLength(), utf8Result);
        const sb_symbol* stemmed = sb_stemmer_stem(stemmer, utf8Result->result.get(), utf8Result->length);
        if (stemmed == NULL) {
            boost::throw_exception(RuntimeException(L"exception stemming word:" + termAtt->term()));
        }
        int32_t newlen = StringUtils::toUnicode(stemmed, sb_stemmer_length(stemmer), termAtt->termBuffer());
        termAtt->setTermLength(newlen);
        return true;
    } else {
        return false;
    }
}

}
