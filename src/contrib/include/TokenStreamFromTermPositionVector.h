/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TOKENSTREAMFROMTERMPOSITIONVECTOR_H
#define TOKENSTREAMFROMTERMPOSITIONVECTOR_H

#include "LuceneContrib.h"
#include "TokenStream.h"

namespace Lucene
{
    class LPPCONTRIBAPI TokenStreamFromTermPositionVector : public TokenStream
    {
    public:
        TokenStreamFromTermPositionVector(TermPositionVectorPtr termPositionVector);
        virtual ~TokenStreamFromTermPositionVector();

        LUCENE_CLASS(TokenStreamFromTermPositionVector);

    private:
        Collection<TokenPtr> positionedTokens;
        int32_t tokensAtCurrentPosition;
        CharTermAttributePtr termAttribute;
        PositionIncrementAttributePtr positionIncrementAttribute;
        OffsetAttributePtr offsetAttribute;

    public:
        virtual bool incrementToken();
        virtual void reset();
    };
}

#endif

