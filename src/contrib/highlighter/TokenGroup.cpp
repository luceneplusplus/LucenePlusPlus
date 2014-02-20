/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "TokenGroup.h"
#include "OffsetAttribute.h"
#include "TermAttribute.h"
#include "TokenStream.h"
#include "Token.h"

namespace Lucene {

const int32_t TokenGroup::MAX_NUM_TOKENS_PER_GROUP = 50;

TokenGroup::TokenGroup(const TokenStreamPtr& tokenStream) {
    offsetAtt = tokenStream->addAttribute<OffsetAttribute>();
    termAtt = tokenStream->addAttribute<TermAttribute>();

    tokens = Collection<TokenPtr>::newInstance(MAX_NUM_TOKENS_PER_GROUP);
    scores = Collection<double>::newInstance(MAX_NUM_TOKENS_PER_GROUP);
    numTokens = 0;
    startOffset = 0;
    endOffset = 0;
    tot = 0.0;
    matchStartOffset = 0;
    matchEndOffset = 0;
}

TokenGroup::~TokenGroup() {
}

void TokenGroup::addToken(double score) {
    if (numTokens < MAX_NUM_TOKENS_PER_GROUP) {
        int32_t termStartOffset = offsetAtt->startOffset();
        int32_t termEndOffset = offsetAtt->endOffset();
        if (numTokens == 0) {
            matchStartOffset = termStartOffset;
            startOffset = termStartOffset;
            matchEndOffset = termEndOffset;
            endOffset = termEndOffset;
            tot += score;
        } else {
            startOffset = std::min(startOffset, termStartOffset);
            endOffset = std::max(endOffset, termEndOffset);
            if (score > 0) {
                if (tot == 0) {
                    matchStartOffset = offsetAtt->startOffset();
                    matchEndOffset = offsetAtt->endOffset();
                } else {
                    matchStartOffset = std::min(matchStartOffset, termStartOffset);
                    matchEndOffset = std::max(matchEndOffset, termEndOffset);
                }
                tot += score;
            }
        }
        TokenPtr token(newLucene<Token>(termStartOffset, termEndOffset));
        token->setTermBuffer(termAtt->term());
        tokens[numTokens] = token;
        scores[numTokens] = score;
        ++numTokens;
    }
}

bool TokenGroup::isDistinct() {
    return (offsetAtt->startOffset() >= endOffset);
}

void TokenGroup::clear() {
    numTokens = 0;
    tot = 0;
}

TokenPtr TokenGroup::getToken(int32_t index) {
    return tokens[index];
}

double TokenGroup::getScore(int32_t index) {
    return scores[index];
}

int32_t TokenGroup::getEndOffset() {
    return endOffset;
}

int32_t TokenGroup::getNumTokens() {
    return numTokens;
}

int32_t TokenGroup::getStartOffset() {
    return startOffset;
}

double TokenGroup::getTotalScore() {
    return tot;
}

}
