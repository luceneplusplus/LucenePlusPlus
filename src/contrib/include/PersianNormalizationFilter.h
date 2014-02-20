/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PERSIANNORMALIZATIONFILTER_H
#define PERSIANNORMALIZATIONFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene {

/// A {@link TokenFilter} that applies {@link PersianNormalizer} to normalize the orthography.
class LPPCONTRIBAPI PersianNormalizationFilter : public TokenFilter {
public:
    PersianNormalizationFilter(const TokenStreamPtr& input);
    virtual ~PersianNormalizationFilter();

    LUCENE_CLASS(PersianNormalizationFilter);

protected:
    PersianNormalizerPtr normalizer;
    TermAttributePtr termAtt;

public:
    virtual bool incrementToken();
};

}

#endif
