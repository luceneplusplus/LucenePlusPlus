/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TURKISHLOWERCASEFILTER_H
#define TURKISHLOWERCASEFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// Normalizes Turkish token text to lower case.
    ///
    /// Turkish and Azeri have unique casing behavior for some characters. This
    /// filter applies Turkish lowercase rules. For more information, see
    /// <a href="http://en.wikipedia.org/wiki/Turkish_dotted_and_dotless_I"
    /// >http://en.wikipedia.org/wiki/Turkish_dotted_and_dotless_I</a>
    class LPPCONTRIBAPI TurkishLowerCaseFilter : public TokenFilter
    {
    public:
        /// Create a new TurkishLowerCaseFilter, that normalizes Turkish token
        /// text to lower case.
        /// @param in TokenStream to filter
        TurkishLowerCaseFilter(TokenStreamPtr input);

        virtual ~TurkishLowerCaseFilter();

        LUCENE_CLASS(TurkishLowerCaseFilter);

    protected:
        static const wchar_t LATIN_CAPITAL_LETTER_I;
        static const wchar_t LATIN_SMALL_LETTER_I;
        static const wchar_t LATIN_SMALL_LETTER_DOTLESS_I;
        static const wchar_t COMBINING_DOT_ABOVE;

        CharTermAttributePtr termAtt;

    public:
        virtual bool incrementToken();

    private:
        /// Lookahead for a combining dot above. Other NSMs may be in between.
        bool isBeforeDot(const wchar_t* s, int32_t pos, int32_t len);

        /// Delete a character in-place. Rarely happens, only if
        /// COMBINING_DOT_ABOVE is found after an i
        int32_t _delete(wchar_t* s, int32_t pos, int32_t len);
    };
}

#endif

