/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ELISIONFILTER_H
#define ELISIONFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// Removes elisions from a {@link TokenStream}. For example, "l'avion" (the plane) will be
    /// tokenized as "avion" (plane).
    ///
    /// Note that {@link StandardTokenizer} sees " ' " as a space, and cuts it out.
    /// @see <a href="http://fr.wikipedia.org/wiki/%C3%89lision">Elision in Wikipedia</a>
    class LPPCONTRIBAPI ElisionFilter : public TokenFilter
    {
    public:
        /// Constructs an elision filter with standard stop words.
        ElisionFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input);

        /// Constructs an elision filter with standard stop words.
        /// @deprecated use {@link #ElisionFilter(Version, TokenStream)} instead
        ElisionFilter(TokenStreamPtr input);

        /// Constructs an elision filter with a Set of stop words
        /// @deprecated use {@link #ElisionFilter(Version, TokenStream, Set)} instead
        ElisionFilter(TokenStreamPtr input, HashSet<String> articles);

        /// Constructs an elision filter with a Set of stop words
        /// @param matchVersion the lucene backwards compatibility version
        /// @param input the source {@link TokenStream}
        /// @param articles a set of stopword articles
        ElisionFilter(LuceneVersion::Version matchVersion, TokenStreamPtr input, HashSet<String> articles);

        virtual ~ElisionFilter();

        LUCENE_CLASS(ElisionFilter);

    protected:
        static const wchar_t apostrophes[];

        CharArraySetPtr articles;
        CharTermAttributePtr termAtt;

    public:
        /// Set the stopword articles
        /// @param matchVersion the lucene backwards compatibility version
        /// @param articles a set of articles
        /// @deprecated use {@link #ElisionFilter(Version, TokenStream, Set)} instead
        void setArticles(LuceneVersion::Version matchVersion, HashSet<String> articles);

        /// Set the stopword articles
        /// @param articles a set of articles
        /// @deprecated use {@link #setArticles(Version, Set)} instead
        void setArticles(HashSet<String> articles);

        /// Increments the {@link TokenStream} with a {@link CharTermAttribute}
        /// without elisioned start
        virtual bool incrementToken();
    };
}

#endif

