/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "SimpleSpanFragmenter.h"
#include "WeightedSpanTerm.h"
#include "TermAttribute.h"
#include "PositionIncrementAttribute.h"
#include "OffsetAttribute.h"
#include "QueryScorer.h"
#include "TokenStream.h"
#include "MiscUtils.h"

namespace Lucene {

const int32_t SimpleSpanFragmenter::DEFAULT_FRAGMENT_SIZE = 100;

SimpleSpanFragmenter::SimpleSpanFragmenter(const QueryScorerPtr& queryScorer) {
    this->currentNumFrags = 0;
    this->position = -1;
    this->waitForPos = -1;
    this->textSize = 0;

    this->queryScorer = queryScorer;
    this->fragmentSize = DEFAULT_FRAGMENT_SIZE;
}

SimpleSpanFragmenter::SimpleSpanFragmenter(const QueryScorerPtr& queryScorer, int32_t fragmentSize) {
    this->currentNumFrags = 0;
    this->position = -1;
    this->waitForPos = -1;
    this->textSize = 0;

    this->queryScorer = queryScorer;
    this->fragmentSize = fragmentSize;
}

SimpleSpanFragmenter::~SimpleSpanFragmenter() {
}

bool SimpleSpanFragmenter::isNewFragment() {
    position += posIncAtt->getPositionIncrement();

    if (waitForPos == position) {
        waitForPos = -1;
    } else if (waitForPos != -1) {
        return false;
    }

    WeightedSpanTermPtr wSpanTerm(queryScorer->getWeightedSpanTerm(termAtt->term()));

    if (wSpanTerm) {
        Collection<PositionSpanPtr> positionSpans(wSpanTerm->getPositionSpans());

        for (int32_t i = 0; i < positionSpans.size(); ++i) {
            if (positionSpans[i]->start == position) {
                waitForPos = positionSpans[i]->end + 1;
                break;
            }
        }
    }

    bool isNewFrag = (offsetAtt->endOffset() >= (fragmentSize * currentNumFrags) && (textSize - offsetAtt->endOffset()) >= MiscUtils::unsignedShift(fragmentSize, 1));

    if (isNewFrag) {
        ++currentNumFrags;
    }

    return isNewFrag;
}

void SimpleSpanFragmenter::start(const String& originalText, const TokenStreamPtr& tokenStream) {
    position = -1;
    currentNumFrags = 1;
    textSize = originalText.length();
    termAtt = tokenStream->addAttribute<TermAttribute>();
    posIncAtt = tokenStream->addAttribute<PositionIncrementAttribute>();
    offsetAtt = tokenStream->addAttribute<OffsetAttribute>();
}

}
