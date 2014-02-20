/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "SimpleFragmenter.h"
#include "TokenGroup.h"
#include "OffsetAttribute.h"
#include "TokenStream.h"

namespace Lucene {

const int32_t SimpleFragmenter::DEFAULT_FRAGMENT_SIZE = 100;

SimpleFragmenter::SimpleFragmenter() {
    this->currentNumFrags = 0;
    this->fragmentSize = DEFAULT_FRAGMENT_SIZE;
}

SimpleFragmenter::SimpleFragmenter(int32_t fragmentSize) {
    this->currentNumFrags = 0;
    this->fragmentSize = fragmentSize;
}

SimpleFragmenter::~SimpleFragmenter() {
}

void SimpleFragmenter::start(const String& originalText, const TokenStreamPtr& tokenStream) {
    offsetAtt = tokenStream->addAttribute<OffsetAttribute>();
    currentNumFrags = 1;
}

bool SimpleFragmenter::isNewFragment() {
    bool isNewFrag = (offsetAtt->endOffset() >= (fragmentSize * currentNumFrags));
    if (isNewFrag) {
        ++currentNumFrags;
    }
    return isNewFrag;
}

int32_t SimpleFragmenter::getFragmentSize() {
    return fragmentSize;
}

void SimpleFragmenter::setFragmentSize(int32_t size) {
    fragmentSize = size;
}

}
