/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef WHITESPACETOKENIZER_H
#define WHITESPACETOKENIZER_H

#include "CharTokenizer.h"

namespace Lucene
{
    /// A WhitespaceTokenizer is a tokenizer that divides text at whitespace. Adjacent sequences 
    /// of non-Whitespace characters form tokens.
    ///
    /// You must specify the required {@link Version} compatibility when creating {@link 
    /// WhitespaceTokenizer}:
    /// <ul>
    ///    <li>As of 3.1, {@link CharTokenizer} uses an int based API to normalize and detect 
    ///    token characters. See {@link CharTokenizer#isTokenChar(int)} and {@link 
    ///    CharTokenizer#normalize(wchar_t)} for details.
    /// </ul>
    class LPPAPI WhitespaceTokenizer : public CharTokenizer
    {
    public:
        /// Construct a new WhitespaceTokenizer. 
        /// @param matchVersion Lucene version to match
        /// @param input the input to split up into tokens
        WhitespaceTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input);
        
        /// Construct a new WhitespaceTokenizer using a given {@link AttributeSource}.
        /// @param matchVersion Lucene version to match
        /// @param source the attribute source to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        WhitespaceTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input);
        
        /// Construct a new WhitespaceTokenizer using a given {@link AttributeSource.AttributeFactory}.
        /// @param matchVersion Lucene version to match
        /// @param factory the attribute factory to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        WhitespaceTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input);
        
        /// Construct a new WhitespaceTokenizer.
        /// @deprecated use {@link #WhitespaceTokenizer(Version, Reader)} instead.
        WhitespaceTokenizer(ReaderPtr input);
        
        /// Construct a new WhitespaceTokenizer using a given {@link AttributeSource}.
        /// @deprecated use {@link #WhitespaceTokenizer(Version, AttributeSource, Reader)} instead.
        WhitespaceTokenizer(AttributeSourcePtr source, ReaderPtr input);
        
        /// Construct a new WhitespaceTokenizer using a given {@link AttributeSource.AttributeFactory}.
        /// @deprecated use {@link #WhitespaceTokenizer(Version, AttributeSource.AttributeFactory, Reader)}
        /// instead.
        WhitespaceTokenizer(AttributeFactoryPtr factory, ReaderPtr input);
        
        virtual ~WhitespaceTokenizer();
        
        LUCENE_CLASS(WhitespaceTokenizer);
    
    public:
        /// Collects only characters which do not satisfy {@link Character#isWhitespace(int)}.
        virtual bool isTokenChar(wchar_t c);
    };
}

#endif
