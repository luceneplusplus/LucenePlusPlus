/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "TurkishLowerCaseFilter.h"
#include "CharTermAttribute.h"
#include "CharFolder.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene
{
    const wchar_t TurkishLowerCaseFilter::LATIN_CAPITAL_LETTER_I = L'\x0049';
    const wchar_t TurkishLowerCaseFilter::LATIN_SMALL_LETTER_I = L'\x0069';
    const wchar_t TurkishLowerCaseFilter::LATIN_SMALL_LETTER_DOTLESS_I = L'\x0131';
    const wchar_t TurkishLowerCaseFilter::COMBINING_DOT_ABOVE = L'\x0307';

    TurkishLowerCaseFilter::TurkishLowerCaseFilter(TokenStreamPtr input) : TokenFilter(input)
    {
        termAtt = addAttribute<CharTermAttribute>();
    }

    TurkishLowerCaseFilter::~TurkishLowerCaseFilter()
    {
    }

    bool TurkishLowerCaseFilter::incrementToken()
    {
        bool iOrAfter = false;

        if (input->incrementToken())
        {
            wchar_t* buffer = termAtt->buffer().get();
            int32_t length = termAtt->length();
            for (int32_t i = 0; i < length;)
            {
                wchar_t ch = buffer[i];

                iOrAfter = (ch == LATIN_CAPITAL_LETTER_I || (iOrAfter && UnicodeUtil::isNonSpacing(ch)));

                if (iOrAfter) // all the special I turkish handling happens here.
                {
                    switch(ch)
                    {
                        case COMBINING_DOT_ABOVE:
                            // remove COMBINING_DOT_ABOVE to mimic composed lowercase
                            length = _delete(buffer, i, length);
                            continue;
                        case LATIN_CAPITAL_LETTER_I:
                            // i itself, it depends if it is followed by COMBINING_DOT_ABOVE
                            // if it is, we will make it small i and later remove the dot
                            if (isBeforeDot(buffer, i + 1, length))
                                buffer[i] = LATIN_SMALL_LETTER_I;
                            else
                            {
                                buffer[i] = LATIN_SMALL_LETTER_DOTLESS_I;
                                // below is an optimization. no COMBINING_DOT_ABOVE follows,
                                // so don't waste time calculating Character.getType(), etc
                                iOrAfter = false;
                            }
                            ++i;
                            continue;
                    }
                }
                buffer[i++] = CharFolder::toLower(ch);
            }
            termAtt->setLength(length);
            return true;
        }
        else
            return false;
    }

    bool TurkishLowerCaseFilter::isBeforeDot(const wchar_t* s, int32_t pos, int32_t len)
    {
        for (int32_t i = pos; i < len;)
        {
            wchar_t ch = s[i];
            if (UnicodeUtil::isNonSpacing(ch))
                return false;
            if (ch == COMBINING_DOT_ABOVE)
                return true;
            ++i;
        }
        return false;
    }

    int32_t TurkishLowerCaseFilter::_delete(wchar_t* s, int32_t pos, int32_t len)
    {
        if (pos < len)
            MiscUtils::arrayCopy(s, pos + 1, s, pos, len - pos - 1);
        return len - 1;
    }
}

