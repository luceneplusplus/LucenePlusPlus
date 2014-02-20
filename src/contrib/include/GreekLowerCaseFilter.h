/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GREEKLOWERCASEFILTER_H
#define GREEKLOWERCASEFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// Normalizes token text to lower case, removes some Greek diacritics, and standardizes
/// final sigma to sigma.
class LPPCONTRIBAPI GreekLowerCaseFilter : public TokenFilter {
public:
    GreekLowerCaseFilter(const TokenStreamPtr& input);
    virtual ~GreekLowerCaseFilter();

    LUCENE_CLASS(GreekLowerCaseFilter);

protected:
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken();

protected:
    wchar_t lowerCase(wchar_t codepoint);
};

}

#endif
