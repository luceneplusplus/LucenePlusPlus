/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CLASSICTOKENIZER_H
#define CLASSICTOKENIZER_H

#include "Tokenizer.h"

namespace Lucene
{
    /// A grammar-based tokenizer
    ///
    /// This should be a good tokenizer for most European-language documents:
    ///
    /// <ul>
    ///   <li>Splits words at punctuation characters, removing punctuation. However, 
    ///     a dot that's not followed by  whitespace is considered part of a token.
    ///   <li>Splits words at hyphens, unless there's a number in the token, in which
    ///     case the whole token is interpreted as a product number and is not split.
    ///   <li>Recognizes email addresses and internet hostnames as one token.
    /// </ul>
    ///
    /// Many applications have specific tokenizer needs.  If this tokenizer does not suit your 
    /// application, please consider copying this source code directory to your project and 
    /// maintaining your own grammar-based tokenizer.
    ///
    /// You must specify the required {@link Version} compatibility when creating ClassicAnalyzer:
    ///
    /// <ul>
    ///   <li> As of 2.4, Tokens incorrectly identified as acronyms are corrected
    /// </ul>
    ///
    /// ClassicTokenizer was named StandardTokenizer in Lucene versions prior to 3.1.
    /// As of 3.1, {@link StandardTokenizer} implements Unicode text segmentation.
    class LPPAPI ClassicTokenizer : public Tokenizer
    {
    public:
        /// Creates a new instance of the {@link ClassicTokenizer}.  Attaches the input to the 
        /// newly created scanner.
        /// @param input The input reader
        ClassicTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input);
        
        /// Creates a new ClassicTokenizer with a given {@link AttributeSource}.
        ClassicTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input);
        
        /// Creates a new ClassicTokenizer with a given {@link AttributeSource.AttributeFactory}
        ClassicTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input);
        
        virtual ~ClassicTokenizer();
        
        LUCENE_CLASS(ClassicTokenizer);
    
    protected:
        /// A private instance of the scanner
        ClassicTokenizerImplPtr scanner;
        
        bool replaceInvalidAcronym;
        int32_t maxTokenLength;
        
        // this tokenizer generates three attributes: term offset, positionIncrement and type
        CharTermAttributePtr termAtt;
        OffsetAttributePtr offsetAtt;
        PositionIncrementAttributePtr posIncrAtt;
        TypeAttributePtr typeAtt;
        
    public:
        static const int32_t ALPHANUM;
        static const int32_t APOSTROPHE;
        static const int32_t ACRONYM;
        static const int32_t COMPANY;
        static const int32_t EMAIL;
        static const int32_t HOST;
        static const int32_t NUM;
        static const int32_t CJ;
        
        /// @deprecated this solves a bug where HOSTs that end with '.' are identified as ACRONYMs.
        static const int32_t ACRONYM_DEP;
  
        /// String token types that correspond to token type int constants
        static const Collection<String> TOKEN_TYPES();
    
    protected:
        void init(ReaderPtr input, LuceneVersion::Version matchVersion);
    
    public:
        /// Set the max allowed token length.  Any token longer than this is skipped.
        void setMaxTokenLength(int32_t length);
        
        /// @see #setMaxTokenLength
        int32_t getMaxTokenLength();
        
        /// @see TokenStream#next()
        virtual bool incrementToken();
        
        virtual void end();
        
        virtual void reset(ReaderPtr input);
        
        /// @return true if ClassicTokenizer now returns these tokens as Hosts, otherwise false
        /// @deprecated Remove in 3.X and make true the only valid value
        bool isReplaceInvalidAcronym();
        
        /// @param replaceInvalidAcronym Set to true to replace mischaracterized acronyms as HOST.
        /// @deprecated Remove in 3.X and make true the only valid value
        void setReplaceInvalidAcronym(bool replaceInvalidAcronym);
    };
}

#endif
