/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LETTERTOKENIZER_H
#define LETTERTOKENIZER_H

#include "CharTokenizer.h"

namespace Lucene
{
    /// A LetterTokenizer is a tokenizer that divides text at non-letters. That's to say, 
    /// it defines tokens as maximal strings of adjacent letters, as defined by 
    /// UnicodeUtil::isAlpha(c) predicate.
    ///
    /// Note: this does a decent job for most European languages, but does a terrible job 
    /// for some Asian languages, where words are not separated by spaces.
    ///
    /// You must specify the required {@link Version} compatibility when creating {@link 
    /// LetterTokenizer}:
    /// <ul>
    ///    <li>As of 3.1, {@link CharTokenizer} uses an int based API to normalize and 
    ///    detect token characters. See {@link CharTokenizer#isTokenChar(int)} and {@link 
    ///    CharTokenizer#normalize(int)} for details.
    /// </ul>
    class LPPAPI LetterTokenizer : public CharTokenizer
    {
    public:
        /// Construct a new LetterTokenizer.
        /// @param matchVersion Lucene version to match
        /// @param input the input to split up into tokens
        LetterTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input);
        
        /// Construct a new LetterTokenizer using a given {@link AttributeSource}.
        /// @param matchVersion Lucene version to match
        /// @param source the attribute source to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        LetterTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input);
        
        /// Construct a new LetterTokenizer using a given {@link AttributeFactory}.
        /// @param matchVersion Lucene version to match
        /// @param factory the attribute factory to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        LetterTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input);
        
        /// Construct a new LetterTokenizer.
        /// @deprecated use {@link #LetterTokenizer(Version, Reader)} instead.
        LetterTokenizer(ReaderPtr input);
        
        /// Construct a new LetterTokenizer using a given {@link AttributeSource}.
        /// @deprecated use {@link #LetterTokenizer(Version, AttributeSource, Reader)} instead.
        LetterTokenizer(AttributeSourcePtr source, ReaderPtr input);
        
        /// Construct a new LetterTokenizer using a given {@link AttributeFactory}.
        /// @deprecated use {@link #LetterTokenizer(Version, AttributeSource.AttributeFactory, Reader)}
        /// instead.
        LetterTokenizer(AttributeFactoryPtr factory, ReaderPtr input);
        
        virtual ~LetterTokenizer();
        
        LUCENE_CLASS(LetterTokenizer);
    
    public:
        /// Collects only characters which satisfy UnicodeUtil::isAlpha(c).
        virtual bool isTokenChar(wchar_t c);
    };
}

#endif
