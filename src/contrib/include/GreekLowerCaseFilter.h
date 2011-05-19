/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GREEKLOWERCASEFILTER_H
#define GREEKLOWERCASEFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// Normalizes token text to lower case, removes some Greek diacritics, and
    /// standardizes final sigma to sigma.
    /// You must specify the required {@link Version} compatibility when creating
    /// GreekLowerCaseFilter:
    /// <ul>
    ///    <li> As of 3.1, supplementary characters are properly lowercased.
    /// </ul>
    class LPPCONTRIBAPI GreekLowerCaseFilter : public TokenFilter
    {
    public:
        /// @deprecated Use {@link #GreekLowerCaseFilter(Version, TokenStream)} instead.
        GreekLowerCaseFilter(TokenStreamPtr input);

        /// Create a GreekLowerCaseFilter that normalizes Greek token text.
        /// @param matchVersion Lucene compatibility version
        /// @param in TokenStream to filter
        GreekLowerCaseFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input);

        virtual ~GreekLowerCaseFilter();

        LUCENE_CLASS(GreekLowerCaseFilter);

    protected:
        CharTermAttributePtr termAtt;

    public:
        virtual bool incrementToken();

    protected:
        wchar_t lowerCase(wchar_t codepoint);
    };
}

#endif

