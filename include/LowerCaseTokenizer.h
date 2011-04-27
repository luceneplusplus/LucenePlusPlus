/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LOWERCASETOKENIZER_H
#define LOWERCASETOKENIZER_H

#include "LetterTokenizer.h"

namespace Lucene
{
    /// LowerCaseTokenizer performs the function of LetterTokenizer and LowerCaseFilter 
    /// together.  It divides text at non-letters and converts them to lower case.  While 
    /// it is functionally equivalent to the combination of LetterTokenizer and 
    /// LowerCaseFilter, there is a performance advantage to doing the two tasks at once, 
    /// hence  this (redundant) implementation.
    ///
    /// Note: this does a decent job for most European languages, but does a terrible job 
    /// for some Asian languages,  where words are not separated by spaces.
    ///
    /// You must specify the required {@link Version} compatibility when creating {@link 
    /// LowerCaseTokenizer}:
    /// <ul>
    ///    <li>As of 3.1, {@link CharTokenizer} uses an int based API to normalize and 
    ///    detect token characters. See {@link CharTokenizer#isTokenChar(int)} and {@link 
    ///    CharTokenizer#normalize(int)} for details.
    /// </ul>
    class LPPAPI LowerCaseTokenizer : public LetterTokenizer
    {
    public:
        /// Construct a new LowerCaseTokenizer.
        /// @param matchVersion Lucene version to match
        /// @param input the input to split up into tokens
        LowerCaseTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input);
        
        /// Construct a new LowerCaseTokenizer using a given {@link AttributeSource}.
        /// @param matchVersion Lucene version to match
        /// @param source the attribute source to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        LowerCaseTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input);
        
        /// Construct a new LowerCaseTokenizer using a given {@link AttributeFactory}.
        /// @param matchVersion Lucene version to match
        /// @param factory the attribute factory to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        LowerCaseTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input);
        
        /// Construct a new LowerCaseTokenizer.
        /// @deprecated use {@link #LowerCaseTokenizer(Reader)} instead.
        LowerCaseTokenizer(ReaderPtr input);
        
        /// Construct a new LowerCaseTokenizer using a given {@link AttributeSource}.
        /// @deprecated use {@link #LowerCaseTokenizer(AttributeSource, Reader)} instead.
        LowerCaseTokenizer(AttributeSourcePtr source, ReaderPtr input);
        
        /// Construct a new LowerCaseTokenizer using a given {@link AttributeFactory}.
        /// @deprecated use {@link #LowerCaseTokenizer(AttributeSource.AttributeFactory, Reader)}
        /// instead.
        LowerCaseTokenizer(AttributeFactoryPtr factory, ReaderPtr input);
        
        virtual ~LowerCaseTokenizer();
        
        LUCENE_CLASS(LowerCaseTokenizer);
    
    public:
        /// Converts char to lower case CharFolder::toLower(wchar_t).
        virtual wchar_t normalize(wchar_t c);
    };
}

#endif
