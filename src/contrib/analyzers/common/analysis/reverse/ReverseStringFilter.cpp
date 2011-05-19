/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ReverseStringFilter.h"
#include "CharTermAttribute.h"

namespace Lucene
{
    const wchar_t ReverseStringFilter::NOMARKER = (wchar_t)0xffff;

    /// Example marker character: U+0001 (START OF HEADING)
    const wchar_t ReverseStringFilter::START_OF_HEADING_MARKER = (wchar_t)0x0001;

    /// Example marker character: U+001F (INFORMATION SEPARATOR ONE)
    const wchar_t ReverseStringFilter::INFORMATION_SEPARATOR_MARKER = (wchar_t)0x001f;

    /// Example marker character: U+EC00 (PRIVATE USE AREA: EC00)
    const wchar_t ReverseStringFilter::PUA_EC00_MARKER = (wchar_t)0xec00;

    /// Example marker character: U+200F (RIGHT-TO-LEFT MARK)
    const wchar_t ReverseStringFilter::RTL_DIRECTION_MARKER = (wchar_t)0x200f;

    ReverseStringFilter::ReverseStringFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        this->marker = NOMARKER;
        this->matchVersion = LuceneVersion::LUCENE_30;
        termAtt = addAttribute<CharTermAttribute>();
    }

    ReverseStringFilter::ReverseStringFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input) : TokenFilter(input)
    {
        this->marker = NOMARKER;
        this->matchVersion = matchVersion;
        termAtt = addAttribute<CharTermAttribute>();
    }

    ReverseStringFilter::ReverseStringFilter(TokenStreamPtr input, wchar_t marker) : TokenFilter(input)
    {
        this->marker = marker;
        this->matchVersion = LuceneVersion::LUCENE_30;
        termAtt = addAttribute<CharTermAttribute>();
    }

    ReverseStringFilter::ReverseStringFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input, wchar_t marker) : TokenFilter(input)
    {
        this->marker = marker;
        this->matchVersion = matchVersion;
        termAtt = addAttribute<CharTermAttribute>();
    }

    ReverseStringFilter::~ReverseStringFilter()
    {
    }

    bool ReverseStringFilter::incrementToken()
    {
        if (input->incrementToken())
        {
            int32_t len = termAtt->length();
            if (marker != NOMARKER)
            {
                ++len;
                termAtt->resizeBuffer(len);
                termAtt->buffer()[len - 1] = marker;
            }
            CharArray term(termAtt->buffer());
            std::reverse(term.get(), term.get() + len);
            termAtt->setLength(len);
            return true;
        }
        else
            return false;
    }
}

