/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef RUSSIANLETTERTOKENIZER_H
#define RUSSIANLETTERTOKENIZER_H

#include "CharTokenizer.h"

namespace Lucene {

/// A RussianLetterTokenizer is a {@link Tokenizer} that extends {@link LetterTokenizer} by also
/// allowing the basic Latin digits 0-9.
class LPPCONTRIBAPI RussianLetterTokenizer : public CharTokenizer {
public:
    /// Construct a new RussianLetterTokenizer.
    RussianLetterTokenizer(const ReaderPtr& input);

    /// Construct a new RussianLetterTokenizer using a given {@link AttributeSource}.
    RussianLetterTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input);

    /// Construct a new RussianLetterTokenizer using a given {@link AttributeFactory}.
    RussianLetterTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input);

    virtual ~RussianLetterTokenizer();

    LUCENE_CLASS(RussianLetterTokenizer);

public:
    /// Collects only characters which satisfy UnicodeUtil::isAlpha(c).
    virtual bool isTokenChar(wchar_t c);
};

}

#endif
