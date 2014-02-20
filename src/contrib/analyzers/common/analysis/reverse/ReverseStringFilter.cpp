/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ReverseStringFilter.h"
#include "TermAttribute.h"

namespace Lucene {

const wchar_t ReverseStringFilter::NOMARKER = (wchar_t)0xffff;

/// Example marker character: U+0001 (START OF HEADING)
const wchar_t ReverseStringFilter::START_OF_HEADING_MARKER = (wchar_t)0x0001;

/// Example marker character: U+001F (INFORMATION SEPARATOR ONE)
const wchar_t ReverseStringFilter::INFORMATION_SEPARATOR_MARKER = (wchar_t)0x001f;

/// Example marker character: U+EC00 (PRIVATE USE AREA: EC00)
const wchar_t ReverseStringFilter::PUA_EC00_MARKER = (wchar_t)0xec00;

/// Example marker character: U+200F (RIGHT-TO-LEFT MARK)
const wchar_t ReverseStringFilter::RTL_DIRECTION_MARKER = (wchar_t)0x200f;

ReverseStringFilter::ReverseStringFilter(const TokenStreamPtr& input) : TokenFilter(input) {
    this->marker = NOMARKER;
    termAtt = addAttribute<TermAttribute>();
}

ReverseStringFilter::ReverseStringFilter(const TokenStreamPtr& input, wchar_t marker) : TokenFilter(input) {
    this->marker = marker;
    termAtt = addAttribute<TermAttribute>();
}

ReverseStringFilter::~ReverseStringFilter() {
}

bool ReverseStringFilter::incrementToken() {
    if (input->incrementToken()) {
        int32_t len = termAtt->termLength();
        if (marker != NOMARKER) {
            ++len;
            termAtt->resizeTermBuffer(len);
            termAtt->termBuffer()[len - 1] = marker;
        }
        CharArray term(termAtt->termBuffer());
        std::reverse(term.get(), term.get() + len);
        termAtt->setTermLength(len);
        return true;
    } else {
        return false;
    }
}

}
