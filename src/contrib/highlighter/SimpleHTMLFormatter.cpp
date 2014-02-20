/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "SimpleHTMLFormatter.h"
#include "TokenGroup.h"

namespace Lucene {

const String SimpleHTMLFormatter::DEFAULT_PRE_TAG = L"<B>";
const String SimpleHTMLFormatter::DEFAULT_POST_TAG = L"</B>";

SimpleHTMLFormatter::SimpleHTMLFormatter() {
    this->preTag = DEFAULT_PRE_TAG;
    this->postTag = DEFAULT_POST_TAG;
}

SimpleHTMLFormatter::SimpleHTMLFormatter(const String& preTag, const String& postTag) {
    this->preTag = preTag;
    this->postTag = postTag;
}

SimpleHTMLFormatter::~SimpleHTMLFormatter() {
}

String SimpleHTMLFormatter::highlightTerm(const String& originalText, const TokenGroupPtr& tokenGroup) {
    if (tokenGroup->getTotalScore() == 0) {
        return originalText;
    }
    StringStream buffer;
    buffer << preTag << originalText << postTag;
    return buffer.str();
}

}
