/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "GreekLowerCaseFilter.h"
#include "TermAttribute.h"
#include "CharFolder.h"

namespace Lucene {

GreekLowerCaseFilter::GreekLowerCaseFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    termAtt = addAttribute<TermAttribute>();
}

GreekLowerCaseFilter::~GreekLowerCaseFilter() {
}

bool GreekLowerCaseFilter::incrementToken() {
    if (input->incrementToken()) {
        wchar_t* chArray = termAtt->termBufferArray();
        int32_t chLen = termAtt->termLength();

        for (int32_t i = 0; i < chLen; ++i) {
            chArray[i] = lowerCase(chArray[i]);
        }

        return true;
    } else {
        return false;
    }
}

wchar_t GreekLowerCaseFilter::lowerCase(wchar_t codepoint) {
    switch (codepoint) {
    case L'\x03c2': // small final sigma
        return 0x03c3; // small sigma

    // Some Greek characters contain diacritics.
    // This filter removes these, converting to the lowercase base form.

    case L'\x0386': // capital alpha with tonos
    case L'\x03ac': // small alpha with tonos
        return L'\x03b1'; // small alpha

    case L'\x0388': // capital epsilon with tonos
    case L'\x03ad': // small epsilon with tonos
        return L'\x03b5'; // small epsilon

    case L'\x0389': // capital eta with tonos
    case L'\x03ae': // small eta with tonos
        return L'\x03b7'; // small eta

    case L'\x038a': // capital iota with tonos
    case L'\x03aa': // capital iota with dialytika
    case L'\x03af': // small iota with tonos
    case L'\x03ca': // small iota with dialytika
    case L'\x0390': // small iota with dialytika and tonos
        return L'\x03b9'; // small iota

    case L'\x038e': // capital upsilon with tonos
    case L'\x03ab': // capital upsilon with dialytika
    case L'\x03cd': // small upsilon with tonos
    case L'\x03cb': // small upsilon with dialytika
    case L'\x03b0': // small upsilon with dialytika and tonos
        return L'\x03c5'; // small upsilon

    case L'\x038c': // capital omicron with tonos
    case L'\x03cc': // small omicron with tonos
        return L'\x03bf'; // small omicron

    case L'\x038f': // capital omega with tonos
    case L'\x03ce': // small omega with tonos
        return L'\x03c9'; // small omega

    // The previous implementation did the conversion below.
    // Only implemented for backwards compatibility with old indexes.

    case L'\x03a2': // reserved
        return L'\x03c2'; // small final sigma

    default:
        return CharFolder::toLower(codepoint);
    }
}

}
