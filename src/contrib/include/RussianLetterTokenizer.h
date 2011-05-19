/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef RUSSIANLETTERTOKENIZER_H
#define RUSSIANLETTERTOKENIZER_H

#include "CharTokenizer.h"

namespace Lucene
{
    /// A RussianLetterTokenizer is a {@link Tokenizer} that extends {@link
    /// LetterTokenizer} by also allowing the basic Latin digits 0-9.
    ///
    /// You must specify the required {@link Version} compatibility when creating
    /// {@link RussianLetterTokenizer}:
    /// <ul>
    ///    <li>As of 3.1, {@link CharTokenizer} uses an int based API to normalize
    ///        and detect token characters. See {@link CharTokenizer#isTokenChar(int)}
    ///        and {@link CharTokenizer#normalize(int)} for details.</li>
    /// </ul>
    /// @deprecated Use {@link StandardTokenizer} instead, which has the same
    /// functionality.
    class LPPCONTRIBAPI RussianLetterTokenizer : public CharTokenizer
    {
    public:
        /// Construct a new RussianLetterTokenizer.
        /// @param matchVersion Lucene version to match
        /// @param input the input to split up into tokens
        RussianLetterTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input);

        /// Construct a new RussianLetterTokenizer.
        /// @deprecated use {@link #RussianLetterTokenizer(Version, Reader)} instead.
        RussianLetterTokenizer(ReaderPtr input);

        /// Construct a new RussianLetterTokenizer using a given {@link AttributeSource}.
        /// @param matchVersion Lucene version to match
        /// @param source the attribute source to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        RussianLetterTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input);

        /// Construct a new RussianLetterTokenizer using a given {@link AttributeSource}.
        /// @deprecated use {@link #RussianLetterTokenizer(Version, AttributeSource, Reader)}
        /// instead.
        RussianLetterTokenizer(AttributeSourcePtr source, ReaderPtr input);

        /// Construct a new RussianLetterTokenizer using a given {@link AttributeFactory}.
        /// @param matchVersion Lucene version to match
        /// @param factory the attribute factory to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        RussianLetterTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input);

        /// Construct a new RussianLetterTokenizer using a given {@link AttributeFactory}.
        /// @deprecated use {@link #RussianLetterTokenizer(Version, AttributeSource.AttributeFactory, Reader)}
        /// instead.
        RussianLetterTokenizer(AttributeFactoryPtr factory, ReaderPtr input);

        virtual ~RussianLetterTokenizer();

        LUCENE_CLASS(RussianLetterTokenizer);

    public:
        /// Collects only characters which satisfy UnicodeUtil::isAlpha(c).
        virtual bool isTokenChar(wchar_t c);
    };
}

#endif

