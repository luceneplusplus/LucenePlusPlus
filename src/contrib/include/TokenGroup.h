/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TOKENGROUP_H
#define TOKENGROUP_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// One, or several overlapping tokens, along with the score(s) and the scope of the original text
class LPPCONTRIBAPI TokenGroup : public LuceneObject {
public:
    TokenGroup(const TokenStreamPtr& tokenStream);
    virtual ~TokenGroup();

    LUCENE_CLASS(TokenGroup);

protected:
    static const int32_t MAX_NUM_TOKENS_PER_GROUP;

    OffsetAttributePtr offsetAtt;
    TermAttributePtr termAtt;

public:
    Collection<TokenPtr> tokens;
    Collection<double> scores;

    int32_t numTokens;
    int32_t startOffset;
    int32_t endOffset;
    double tot;
    int32_t matchStartOffset;
    int32_t matchEndOffset;

public:
    void addToken(double score);
    bool isDistinct();
    void clear();

    /// @param index a value between 0 and numTokens -1
    /// @return the "n"th token
    TokenPtr getToken(int32_t index);

    /// @param index a value between 0 and numTokens -1
    /// @return the "n"th score
    double getScore(int32_t index);

    /// @return the end position in the original text
    int32_t getEndOffset();

    /// @return the number of tokens in this group
    int32_t getNumTokens();

    /// @return the start position in the original text
    int32_t getStartOffset();

    /// @return all tokens' scores summed up
    double getTotalScore();
};

}

#endif
