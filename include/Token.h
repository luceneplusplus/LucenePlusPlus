/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TOKEN_H
#define TOKEN_H

#include "TermAttribute.h"
#include "AttributeSource.h"

namespace Lucene
{
    /// A Token is an occurrence of a term from the text of a field.  It consists of a term's 
    /// text, the start and end offset of the term in the text of the field and a type string.
    ///
    /// The start and end offsets permit applications to re-associate a token with its source 
    /// text, eg., to display highlighted query terms in a document browser, or to show 
    /// matching text fragments in a <abbr title="KeyWord In Context">KWIC</abbr> display, etc.
    ///
    /// The type is a string, assigned by a lexical analyzer (a.k.a. tokenizer), naming the 
    /// lexical or syntactic class that the token belongs to.  For example an end of sentence 
    /// marker token might be implemented with type "eos". The default token type is "word".
    ///
    /// A Token can optionally have metadata (a.k.a. Payload) in the form of a variable length 
    /// byte array. Use {@link TermPositions#getPayloadLength()} and {@link TermPositions
    /// #getPayload(byte[], int)} to retrieve the payloads from the index.
    ///
    /// Tokenizers and TokenFilters should try to re-use a Token instance when possible for 
    /// best performance, by implementing the {@link TokenStream#incrementToken()} API.  Failing 
    /// that, to create a new Token you should first use one of the constructors that starts 
    /// with null text.  To load the the token from a char[] use {@link #copyBuffer(char[], int, int)}.
    /// To load from a String use {@link #setEmpty} followed by {@link #append(CharSequence)} or 
    /// {@link #append(CharSequence, int, int)}. Alternatively you can get the Token's 
    /// termBuffer by calling either {@link #buffer()}, if you know that your text is shorter 
    /// than the capacity of the termBuffer or {@link #resizeBuffer(int)}, if there is any 
    /// possibility that you may need to grow the buffer. Fill in the characters of your term 
    /// into this buffer, with {@link String#getChars(int, int, char[], int)} if loading from 
    /// a string, or with {@link System#arraycopy(Object, int, Object, int, int)}, and finally
    /// call {@link #setLength(int)} to set the length of the term text.
    ///
    /// Typical Token reuse patterns:
    /// 
    /// Copying text from a string (type is reset to {@link #DEFAULT_TYPE} if not specified):
    /// <pre>
    /// return reusableToken->reinit(string, startOffset, endOffset[, type]);
    /// </pre>
    ///
    /// Copying some text from a string (type is reset to {@link #DEFAULT_TYPE} if not specified):
    /// <pre>
    /// return reusableToken->reinit(string, 0, string.length(), startOffset, endOffset[, type]);
    /// </pre>
    ///
    /// Copying text from char[] buffer (type is reset to {@link #DEFAULT_TYPE} if not specified):
    /// <pre>
    /// return reusableToken->reinit(buffer, 0, buffer.length, startOffset, endOffset[, type]);
    /// </pre>
    ///
    /// Copying some text from a char[] buffer (type is reset to {@link #DEFAULT_TYPE} if not specified):
    /// <pre>
    /// return reusableToken->reinit(buffer, start, end - start, startOffset, endOffset[, type]);
    /// </pre>
    ///
    /// Copying from one one Token to another (type is reset to {@link #DEFAULT_TYPE} if not specified):
    /// <pre>
    /// return reusableToken.reinit(source.buffer(), 0, source.length(), source.startOffset(), 
    ///                             source.endOffset()[, source.type()]);
    /// </pre>
    ///
    /// A few things to note:
    /// clear() initializes all of the fields to default values. This was changed in contrast 
    /// to Lucene 2.4, but should affect no one. Because TokenStreams can be chained, one cannot 
    /// assume that the Token's current type is correct.  The startOffset and endOffset represent 
    /// the start and offset in the source text, so be careful in adjusting them.  When caching a 
    /// reusable token, clone it. When injecting a cached token into a stream that can be reset, 
    /// clone it again.
    ///
    /// Please note: With Lucene 3.1, the {@linkplain #toString toString()} method had to be 
    /// changed to match the {@link CharSequence} interface introduced by the interface {@link 
    /// CharTermAttribute}. This method now only prints the term text, no additional information 
    /// anymore.
    ///
    /// @see Payload
    class LPPAPI Token : public TermAttribute
    {
    public:
        /// Constructs a Token will null text.
        Token();
        
        /// Constructs a Token with null text and start and end offsets.
        /// @param start start offset in the source text
        /// @param end end offset in the source text
        Token(int32_t start, int32_t end);
        
        /// Constructs a Token with null text and start and end offsets plus the Token type.
        /// @param start start offset in the source text
        /// @param end end offset in the source text
        /// @param type the lexical type of this Token
        Token(int32_t start, int32_t end, const String& type);
        
        /// Constructs a Token with null text and start and end offsets plus flags.
        /// @param start start offset in the source text
        /// @param end end offset in the source text
        /// @param flags The bits to set for this token
        Token(int32_t start, int32_t end, int32_t flags);
        
        /// Constructs a Token with the given term text, start and end offsets.  The type defaults 
        /// to "word."
        /// NOTE: for better indexing speed you should instead use the char[] termBuffer methods to 
        /// set the term text.
        /// @param text term text
        /// @param start start offset in the source text
        /// @param end end offset in the source text
        Token(const String& text, int32_t start, int32_t end);
        
        /// Constructs a Token with the given term text, start and end offsets and type.
        /// NOTE: for better indexing speed you should instead use the char[] termBuffer methods 
        /// to set the term text.
        /// @param text term text
        /// @param start start offset in the source text
        /// @param end end offset in the source text
        /// @param type the lexical type of this Token
        Token(const String& text, int32_t start, int32_t end, const String& type);
        
        /// Constructs a Token with the given term text, start and end offsets and flags.
        /// NOTE: for better indexing speed you should instead use the char[] termBuffer methods 
        /// to set the term text.
        /// @param text term text
        /// @param start start offset in the source text
        /// @param end end offset in the source text
        /// @param flags The bits to set for this token
        Token(const String& text, int32_t start, int32_t end, int32_t flags);
        
        /// Constructs a Token with the given term buffer (offset and length), start and end offsets
        Token(CharArray startTermBuffer, int32_t termBufferOffset, int32_t termBufferLength, int32_t start, int32_t end);
        
        virtual ~Token();
        
        LUCENE_CLASS(Token);
    
    public:
        /// the default type
        static const String DEFAULT_TYPE;

    protected:
        int32_t _startOffset;
        int32_t _endOffset;
        String _type;
        int32_t flags;
        PayloadPtr payload;
        int32_t positionIncrement;
    
    public:
        /// Set the position increment.  This determines the position of this token relative 
        /// to the previous Token  in a {@link TokenStream}, used in phrase searching.
        ///
        /// The default value is one.
        ///
        /// Some common uses for this are:
        ///
        /// Set it to zero to put multiple terms in the same position.  This is useful if, 
        /// eg., a word has multiple stems.  Searches for phrases including either stem will 
        /// match.  In this case, all but the first stem's increment should be set to zero: 
        /// the increment of the first instance should be one.  Repeating a token  with an 
        /// increment of zero can also be used to boost the scores of matches on that token.
        ///
        /// Set it to values greater than one to inhibit exact phrase matches.  If, for 
        /// example, one does not want phrases to match across removed stop words, then one 
        /// could build a stop word filter that removes stop words and also sets the increment 
        /// to the number of stop words removed before each non-stop word.  Then exact phrase 
        /// queries will only match when the terms occur with no intervening stop words.
        ///
        /// @param positionIncrement the distance from the prior term
        /// @see TermPositions
        virtual void setPositionIncrement(int32_t positionIncrement);
        
        /// Returns the position increment of this Token.
        /// @see #setPositionIncrement
        virtual int32_t getPositionIncrement();
        
        /// Returns this Token's starting offset, the position of the first character 
        /// corresponding to this token in the source text.
        ///
        /// Note that the difference between endOffset() and startOffset() may not be equal 
        /// to {@link #length}, as the term text may have been altered by a stemmer or some 
        /// other filter.
        virtual int32_t startOffset();
        
        /// Set the starting offset.
        /// @see #startOffset()
        virtual void setStartOffset(int32_t offset);
        
        /// Returns this Token's ending offset, one greater than the position of the last 
        /// character corresponding to this token in the source text.  The length of the 
        /// token in the source text is (endOffset - startOffset).
        virtual int32_t endOffset();
        
        /// Set the ending offset.
        /// @see #endOffset()
        virtual void setEndOffset(int32_t offset);
        
        /// Set the starting and ending offset.
        /// @see #startOffset() and #endOffset()
        virtual void setOffset(int32_t startOffset, int32_t endOffset);
        
        /// Returns this Token's lexical type.  Defaults to "word".
        virtual String type();
        
        /// Set the lexical type.
        /// @see #type()
        virtual void setType(const String& type);
        
        /// Get the bitset for any bits that have been set.  This is completely distinct 
        /// from {@link #type()}, although they do share similar purposes.  The flags can
        /// be used to encode information about the token for use by other {@link TokenFilter}s.
        ///
        /// @return The bits
        virtual int32_t getFlags();
        
        /// @see #getFlags()
        virtual void setFlags(int32_t flags);
        
        /// Returns this Token's payload.
        virtual PayloadPtr getPayload();
        
        /// Sets this Token's payload.
        virtual void setPayload(PayloadPtr payload);
        
        /// Resets the term text, payload, flags, and positionIncrement, startOffset, endOffset
        /// and token type to default.
        virtual void clear();
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        /// Makes a clone, but replaces the term buffer and start/end offset in the process.  
        /// This is more efficient than doing a full clone (and then calling copyBuffer) because 
        /// it saves a wasted copy of the old termBuffer.
        TokenPtr clone(CharArray newTermBuffer, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset);
        
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
        
        /// Shorthand for calling {@link #clear}, {@link #copyBuffer(char[], int, int)}, {@link 
        /// #setStartOffset}, {@link #setEndOffset}, {@link #setType}
        /// @return this Token instance
        TokenPtr reinit(CharArray newTermBuffer, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset, const String& newType);
        
        /// Shorthand for calling {@link #clear}, {@link #copyBuffer(char[], int, int)}, {@link 
        /// #setStartOffset}, {@link #setEndOffset}, {@link #setType} on Token::DEFAULT_TYPE
        /// @return this Token instance
        TokenPtr reinit(CharArray newTermBuffer, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset);
        
        /// Shorthand for calling {@link #clear}, {@link #append(String)}, {@link #setStartOffset},
        /// {@link #setEndOffset}, {@link #setType}
        /// @return this Token instance
        TokenPtr reinit(const String& newTerm, int32_t newStartOffset, int32_t newEndOffset, const String& newType);
        
        /// Shorthand for calling {@link #clear}, {@link #append(String)}, {@link #setStartOffset},
        /// {@link #setEndOffset}, {@link #setType}
        /// @return this Token instance
        TokenPtr reinit(const String& newTerm, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset, const String& newType);
        
        /// Shorthand for calling {@link #clear}, {@link #append(String)}, {@link #setStartOffset},
        /// {@link #setEndOffset}, {@link #setType} on Token::DEFAULT_TYPE
        /// @return this Token instance
        TokenPtr reinit(const String& newTerm, int32_t newStartOffset, int32_t newEndOffset);
        
        /// Shorthand for calling {@link #clear}, {@link #append(String, int, int)}, {@link 
        /// #setStartOffset}, {@link #setEndOffset}, {@link #setType} on Token::DEFAULT_TYPE
        /// @return this Token instance
        TokenPtr reinit(const String& newTerm, int32_t newTermOffset, int32_t newTermLength, int32_t newStartOffset, int32_t newEndOffset);
        
        /// Copy the prototype token's fields into this one. Note: Payloads are shared.
        void reinit(TokenPtr prototype);
        
        /// Copy the prototype token's fields into this one, with a different term. 
        /// Note: Payloads are shared.
        void reinit(TokenPtr prototype, const String& newTerm);
        
        /// Copy the prototype token's fields into this one, with a different term. 
        /// Note: Payloads are shared.
        void reinit(TokenPtr prototype, CharArray newTermBuffer, int32_t offset, int32_t length);
        
        virtual void copyTo(AttributePtr target);
        
        /// Convenience factory that returns Token as implementation for the basic attributes
        static AttributeFactoryPtr TOKEN_ATTRIBUTE_FACTORY();
    
    protected:
        /// Construct Token and initialize values
        void ConstructToken(int32_t start, int32_t end, const String& type, int32_t flags);
        
        /// Like clear() but doesn't clear termBuffer/text
        void clearNoTermBuffer();
    };
    
    /// Creates a TokenAttributeFactory returning {@link Token} as instance for the basic 
    /// attributes and for all other attributes calls the given delegate factory.
    class LPPAPI TokenAttributeFactory : public AttributeFactory
    {
    public:
        TokenAttributeFactory(AttributeFactoryPtr delegate);
        virtual ~TokenAttributeFactory();
        
        LUCENE_CLASS(TokenAttributeFactory);
    
    protected:
        AttributeFactoryPtr delegate;
    
    public:
        virtual AttributePtr createAttributeInstance(const String& className);
        virtual bool equals(LuceneObjectPtr other);
        virtual int32_t hashCode();
    };
}

#endif
