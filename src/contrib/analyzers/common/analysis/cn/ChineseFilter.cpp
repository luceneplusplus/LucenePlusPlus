/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ChineseFilter.h"
#include "TermAttribute.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene {

const wchar_t* ChineseFilter::STOP_WORDS[] = {
    L"and", L"are", L"as", L"at", L"be", L"but", L"by",
    L"for", L"if", L"in", L"into", L"is", L"it",
    L"no", L"not", L"of", L"on", L"or", L"such",
    L"that", L"the", L"their", L"then", L"there", L"these",
    L"they", L"this", L"to", L"was", L"will", L"with"
};

ChineseFilter::ChineseFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    stopTable = HashSet<String>::newInstance(STOP_WORDS, STOP_WORDS + SIZEOF_ARRAY(STOP_WORDS));
    termAtt = addAttribute<TermAttribute>();
}

ChineseFilter::~ChineseFilter() {
}

bool ChineseFilter::incrementToken() {
    while (input->incrementToken()) {
        String text(termAtt->term());

        if (!stopTable.contains(text)) {
            if (UnicodeUtil::isLower(text[0]) || UnicodeUtil::isUpper(text[0])) {
                // English word/token should larger than 1 character.
                if (text.length() > 1) {
                    return true;
                }
            } else if (UnicodeUtil::isOther(text[0])) {
                // One Chinese character as one Chinese word.
                // Chinese word extraction to be added later here.
                return true;
            }
        }
    }
    return false;
}

}
