/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ARABICLETTERTOKENIZER_H
#define ARABICLETTERTOKENIZER_H

#include "LuceneContrib.h"
#include "LetterTokenizer.h"

namespace Lucene
{
    /// Tokenizer that breaks text into runs of letters and diacritics.
    ///
    /// The problem with the standard Letter tokenizer is that it fails on diacritics.
    /// Handling similar to this is necessary for Indic Scripts, Hebrew, Thaana, etc.
    ///
    /// You must specify the required {@link Version} compatibility when creating
    /// {@link ArabicLetterTokenizer}:
    /// <ul>
    ///    <li>As of 3.1, {@link CharTokenizer} uses an int based API to normalize and
    ///    detect token characters. See {@link #isTokenChar(int)} and {@link 
    ///    #normalize(int)} for details.
    /// </ul>
    /// @deprecated (3.1) Use {@link StandardTokenizer} instead.
    class LPPCONTRIBAPI ArabicLetterTokenizer : public LetterTokenizer
    {
    public:
        /// Construct a new ArabicLetterTokenizer.
        /// @param matchVersion Lucene version to match
        /// @param input the input to split up into tokens
        ArabicLetterTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input);
        
        /// Construct a new ArabicLetterTokenizer using a given {@link AttributeSource}.
        /// @param matchVersion Lucene version to match
        /// @param source the attribute source to use for this Tokenizer
        /// @param input the input to split up into tokens
        ArabicLetterTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input);
        
        /// Construct a new ArabicLetterTokenizer using a given {@link AttributeFactory}.
        /// @param matchVersion Lucene version to match
        /// @param factory the attribute factory to use for this Tokenizer
        /// @param input the input to split up into tokens
        ArabicLetterTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input);
        
        /// Construct a new ArabicLetterTokenizer.
        /// @deprecated use {@link #ArabicLetterTokenizer(Version, Reader)} instead.
        ArabicLetterTokenizer(ReaderPtr input);
        
        /// Construct a new ArabicLetterTokenizer using a given {@link AttributeSource}.
        /// @deprecated use {@link #ArabicLetterTokenizer(Version, AttributeSource, Reader)}
        ArabicLetterTokenizer(AttributeSourcePtr source, ReaderPtr input);
        
        /// Construct a new ArabicLetterTokenizer using a given {@link AttributeFactory}.
        /// @deprecated use {@link #ArabicLetterTokenizer(Version, AttributeSource.AttributeFactory, Reader)}
        /// instead
        ArabicLetterTokenizer(AttributeFactoryPtr factory, ReaderPtr input);
        
        virtual ~ArabicLetterTokenizer();
        
        LUCENE_CLASS(ArabicLetterTokenizer);
    
    public:
        /// Allows for Letter category or NonspacingMark category
        virtual bool isTokenChar(wchar_t c);
    };
}

#endif
