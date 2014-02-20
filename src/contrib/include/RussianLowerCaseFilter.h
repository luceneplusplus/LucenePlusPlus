/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef RUSSIANLOWERCASEFILTER_H
#define RUSSIANLOWERCASEFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// Normalizes token text to lower case.
class LPPCONTRIBAPI RussianLowerCaseFilter : public TokenFilter {
public:
    RussianLowerCaseFilter(const TokenStreamPtr& input);

    virtual ~RussianLowerCaseFilter();

    LUCENE_CLASS(RussianLowerCaseFilter);

protected:
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken();
};

}

#endif
