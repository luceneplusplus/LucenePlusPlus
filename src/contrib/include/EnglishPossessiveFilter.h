/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ENGLISHPOSSESSIVEFILTER_H
#define ENGLISHPOSSESSIVEFILTER_H

#include "LuceneContrib.h"
#include "TokenFilter.h"

namespace Lucene
{
    /// TokenFilter that removes possessives (trailing 's) from words.
    class LPPCONTRIBAPI EnglishPossessiveFilter : public TokenFilter
    {
    public:
        EnglishPossessiveFilter(TokenStreamPtr input);
        virtual ~EnglishPossessiveFilter();

        LUCENE_CLASS(EnglishPossessiveFilter);

    protected:
        CharTermAttributePtr termAtt;

    public:
        virtual bool incrementToken();
    };
}

#endif

