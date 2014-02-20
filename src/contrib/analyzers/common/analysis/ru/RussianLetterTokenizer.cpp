/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "RussianLetterTokenizer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene {

RussianLetterTokenizer::RussianLetterTokenizer(const ReaderPtr& input) : CharTokenizer(input) {
}

RussianLetterTokenizer::RussianLetterTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input) : CharTokenizer(source, input) {
}

RussianLetterTokenizer::RussianLetterTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input) : CharTokenizer(factory, input) {
}

RussianLetterTokenizer::~RussianLetterTokenizer() {
}

bool RussianLetterTokenizer::isTokenChar(wchar_t c) {
    return (UnicodeUtil::isAlpha(c) || UnicodeUtil::isDigit(c));
}

}
