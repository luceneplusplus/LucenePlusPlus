/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ARABICLETTERTOKENIZER_H
#define ARABICLETTERTOKENIZER_H

#include "LuceneContrib.h"
#include "LetterTokenizer.h"

namespace Lucene {

/// Tokenizer that breaks text into runs of letters and diacritics.
///
/// The problem with the standard Letter tokenizer is that it fails on diacritics.
/// Handling similar to this is necessary for Indic Scripts, Hebrew, Thaana, etc.
///
class LPPCONTRIBAPI ArabicLetterTokenizer : public LetterTokenizer {
public:
    /// Construct a new ArabicLetterTokenizer.
    ArabicLetterTokenizer(const ReaderPtr& input);

    /// Construct a new ArabicLetterTokenizer using a given {@link AttributeSource}.
    ArabicLetterTokenizer(const AttributeSourcePtr& source, const ReaderPtr& input);

    /// Construct a new ArabicLetterTokenizer using a given {@link AttributeFactory}.
    ArabicLetterTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& input);

    virtual ~ArabicLetterTokenizer();

    LUCENE_CLASS(ArabicLetterTokenizer);

public:
    /// Allows for Letter category or NonspacingMark category
    virtual bool isTokenChar(wchar_t c);
};

}

#endif
