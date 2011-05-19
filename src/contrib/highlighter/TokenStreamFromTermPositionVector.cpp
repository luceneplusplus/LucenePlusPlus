/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "TokenStreamFromTermPositionVector.h"
#include "CharTermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "OffsetAttribute.h"
#include "Token.h"
#include "TermPositionVector.h"
#include "TermVectorOffsetInfo.h"

namespace Lucene
{
    struct lessTokenPositionIncrement
    {
        inline bool operator()(const TokenPtr& first, const TokenPtr& second) const
        {
            return (first->getPositionIncrement() < second->getPositionIncrement());
        }
    };

    TokenStreamFromTermPositionVector::TokenStreamFromTermPositionVector(TermPositionVectorPtr termPositionVector)
    {
        positionedTokens = Collection<TokenPtr>::newInstance();
        termAttribute = addAttribute<CharTermAttribute>();
        positionIncrementAttribute = addAttribute<PositionIncrementAttribute>();
        offsetAttribute = addAttribute<OffsetAttribute>();
        Collection<String> terms(termPositionVector->getTerms());
        for (int32_t i = 0; i < terms.size(); ++i)
        {
            Collection<TermVectorOffsetInfoPtr> offsets(termPositionVector->getOffsets(i));
            Collection<int32_t> termPositions(termPositionVector->getTermPositions(i));
            for (int32_t j = 0; j < termPositions.size(); ++j)
            {
                TokenPtr token;
                if (offsets)
                    token = newLucene<Token>(terms[i], offsets[j]->getStartOffset(), offsets[j]->getEndOffset());
                else
                {
                    token = newLucene<Token>();
                    token->setEmpty()->append(terms[i]);
                }
                // Yes - this is the position, not the increment! This is for
                // sorting. This value will be corrected before use.
                token->setPositionIncrement(termPositions[j]);
                this->positionedTokens.add(token);
            }
        }
        std::sort(this->positionedTokens.begin(), this->positionedTokens.end(), lessTokenPositionIncrement());

        int32_t lastPosition = -1;
        for (Collection<TokenPtr>::iterator token = this->positionedTokens.begin(); token != this->positionedTokens.end(); ++token)
        {
            int32_t thisPosition = (*token)->getPositionIncrement();
            (*token)->setPositionIncrement(thisPosition - lastPosition);
            lastPosition = thisPosition;
        }
        this->tokensAtCurrentPosition = 0;
    }

    TokenStreamFromTermPositionVector::~TokenStreamFromTermPositionVector()
    {
    }

    bool TokenStreamFromTermPositionVector::incrementToken()
    {
        if (this->tokensAtCurrentPosition < this->positionedTokens.size())
        {
            TokenPtr next(this->positionedTokens[this->tokensAtCurrentPosition++]);
            clearAttributes();
            termAttribute->setEmpty()->append(next);
            positionIncrementAttribute->setPositionIncrement(next->getPositionIncrement());
            offsetAttribute->setOffset(next->startOffset(), next->endOffset());
            return true;
        }
        return false;
    }

    void TokenStreamFromTermPositionVector::reset()
    {
        this->tokensAtCurrentPosition = 0;
    }
}

