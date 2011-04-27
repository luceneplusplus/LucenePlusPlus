/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHARTOKENIZER_H
#define CHARTOKENIZER_H

#include "Tokenizer.h"

namespace Lucene
{
    /// An abstract base class for simple, character-oriented tokenizers. 
    /// 
    /// You must specify the required {@link Version} compatibility when creating {@link 
    /// CharTokenizer}:
    /// <ul>
    ///    <li>As of 3.1, {@link CharTokenizer} uses an int based API to normalize and
    ///    detect token codepoints. See {@link #isTokenChar(int)} and {@link 
    ///    #normalize(int)} for details.
    /// </ul>
    ///
    /// As of Lucene 3.1 each {@link CharTokenizer} - constructor expects a {@link Version} 
    /// argument. Based on the given {@link Version} either the new API or a backwards 
    /// compatibility layer is used at runtime. For {@link Version} < 3.1 the backwards 
    /// compatibility layer ensures correct behavior even for indexes build with previous 
    /// versions of Lucene. If a {@link Version} >= 3.1 is used {@link CharTokenizer} 
    /// requires the new API to be implemented by the instantiated class. {@link 
    /// CharTokenizer} subclasses implementing the new API are fully backwards compatible 
    /// if instantiated with {@link Version} < 3.1.
    ///
    /// Note: If you use a subclass of {@link CharTokenizer} with {@link Version} >= 3.1 
    /// on an index build with a version < 3.1, created tokens might not be compatible 
    /// with the terms in your index.
    class LPPAPI CharTokenizer : public Tokenizer
    {
    public:
        /// Creates a new {@link CharTokenizer} instance
        /// @param matchVersion Lucene version to match
        /// @param input the input to split up into tokens
        CharTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input);
        
        /// Creates a new {@link CharTokenizer} instance
        /// @param matchVersion Lucene version to match
        /// @param source the attribute source to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens        
        CharTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input);
        
        /// Creates a new {@link CharTokenizer} instance
        /// @param matchVersion Lucene version to match
        /// @param factory the attribute factory to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        CharTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input);
        
        /// Creates a new {@link CharTokenizer} instance
        /// @param input the input to split up into tokens
        /// @deprecated use {@link #CharTokenizer(Version, Reader)} instead.
        CharTokenizer(ReaderPtr input);
        
        /// Creates a new {@link CharTokenizer} instance
        /// @param source the attribute source to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        /// @deprecated use {@link #CharTokenizer(Version, AttributeSource, Reader)} instead.
        CharTokenizer(AttributeSourcePtr source, ReaderPtr input);
        
        /// Creates a new {@link CharTokenizer} instance
        /// @param factory the attribute factory to use for this {@link Tokenizer}
        /// @param input the input to split up into tokens
        /// @deprecated use {@link #CharTokenizer(Version, AttributeFactory, Reader)} instead.
        CharTokenizer(AttributeFactoryPtr factory, ReaderPtr input);
        
        virtual ~CharTokenizer();
        
        LUCENE_CLASS(CharTokenizer);
    
    protected:
        int32_t offset;
        int32_t bufferIndex;
        int32_t dataLen;
        int32_t finalOffset;
        
        static const int32_t MAX_WORD_LEN;
        static const int32_t IO_BUFFER_SIZE;
        
        CharArray ioBuffer;
        CharTermAttributePtr termAtt;
        OffsetAttributePtr offsetAtt;
    
    public:
        virtual bool incrementToken();
        virtual void end();
        virtual void reset(ReaderPtr input);
    
    protected:
        void ConstructCharTokenizer();
        
        /// Returns true if a character should be included in a token.  This tokenizer generates as tokens adjacent 
        /// sequences of characters which satisfy this predicate.  Characters for which this is false are used to
        /// define token boundaries and are not included in tokens.
        virtual bool isTokenChar(wchar_t c) = 0;
        
        /// Called on each token character to normalize it before it is added to the token.  The default implementation 
        /// does nothing.  Subclasses may use this to, eg., lowercase tokens.
        virtual wchar_t normalize(wchar_t c);
    };
}

#endif
