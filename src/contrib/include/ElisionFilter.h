/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ELISIONFILTER_H
#define ELISIONFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// Removes elisions from a {@link TokenStream}. For example, "l'avion" (the plane) will be
/// tokenized as "avion" (plane).
///
/// Note that {@link StandardTokenizer} sees " ' " as a space, and cuts it out.
/// @see <a href="http://fr.wikipedia.org/wiki/%C3%89lision">Elision in Wikipedia</a>
class LPPCONTRIBAPI ElisionFilter : public TokenFilter {
public:
    /// Constructs an elision filter with standard stop words.
    ElisionFilter(const TokenStreamPtr& input);

    /// Constructs an elision filter with a Set of stop words
    ElisionFilter(const TokenStreamPtr& input, HashSet<String> articles);

    virtual ~ElisionFilter();

    LUCENE_CLASS(ElisionFilter);

protected:
    static const wchar_t apostrophes[];

    CharArraySetPtr articles;
    TermAttributePtr termAtt;

public:
    void setArticles(HashSet<String> articles);

    virtual bool incrementToken();
};

}

#endif
