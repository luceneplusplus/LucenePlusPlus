/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "RussianLowerCaseFilter.h"
#include "TermAttribute.h"
#include "CharFolder.h"

namespace Lucene {

RussianLowerCaseFilter::RussianLowerCaseFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    termAtt = addAttribute<TermAttribute>();
}

RussianLowerCaseFilter::~RussianLowerCaseFilter() {
}

bool RussianLowerCaseFilter::incrementToken() {
    if (input->incrementToken()) {
        wchar_t* buffer = termAtt->termBufferArray();
        int32_t length = termAtt->termLength();
        for (int32_t i = 0; i < length; ++i) {
            buffer[i] = CharFolder::toLower(buffer[i]);
        }
        return true;
    } else {
        return false;
    }
}

}
