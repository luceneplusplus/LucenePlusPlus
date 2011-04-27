/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STANDARDTOKENIZER_H
#define STANDARDTOKENIZER_H

#include "Tokenizer.h"

namespace Lucene
{
    /// A grammar-based tokenizer.
    ///
    /// As of Lucene version 3.1, this class implements the Word Break rules from the Unicode Text 
    /// Segmentation algorithm, as specified in 
    /// <a href="http://unicode.org/reports/tr29/">Unicode Standard Annex #29</a>.
    ///
    /// Many applications have specific tokenizer needs.  If this tokenizer does not suit your application,
    /// please consider copying this source code directory to your project and maintaining your own 
    /// grammar-based tokenizer.
    ///
    /// You must specify the required {@link Version} compatibility when creating StandardTokenizer:
    /// <ul>
    ///   <li> As of 3.1, StandardTokenizer implements Unicode text segmentation.
    ///   If you use a previous version number, you get the exact behavior of {@link ClassicTokenizer} for 
    /// backwards compatibility.
    /// </ul>
    class LPPAPI StandardTokenizer : public Tokenizer
    {
    public:
        /// Creates a new instance of the {@link StandardTokenizer}.  Attaches the input to the newly created scanner.
        /// @param input The input reader
        StandardTokenizer(LuceneVersion::Version matchVersion, ReaderPtr input);
        
        /// Creates a new StandardTokenizer with a given {@link AttributeSource}.
        StandardTokenizer(LuceneVersion::Version matchVersion, AttributeSourcePtr source, ReaderPtr input);
        
        /// Creates a new StandardTokenizer with a given {@link AttributeSource.AttributeFactory}
        StandardTokenizer(LuceneVersion::Version matchVersion, AttributeFactoryPtr factory, ReaderPtr input);
        
        virtual ~StandardTokenizer();
        
        LUCENE_CLASS(StandardTokenizer);
    
    protected:
        /// A private instance of the scanner
        StandardTokenizerInterfacePtr scanner;
        
        bool replaceInvalidAcronym;
        int32_t maxTokenLength;
        
        // this tokenizer generates three attributes: offset, positionIncrement and type
        CharTermAttributePtr termAtt;
        OffsetAttributePtr offsetAtt;
        PositionIncrementAttributePtr posIncrAtt;
        TypeAttributePtr typeAtt;
        
    public:
        static const int32_t ALPHANUM;
        
        /// @deprecated
        static const int32_t APOSTROPHE;
        
        /// @deprecated
        static const int32_t ACRONYM;
        
        /// @deprecated
        static const int32_t COMPANY;
        
        static const int32_t EMAIL;
        
        /// @deprecated
        static const int32_t HOST;
        
        static const int32_t NUM;
        
        /// @deprecated
        static const int32_t CJ;
        
        /// @deprecated this solves a bug where HOSTs that end with '.' are identified as ACRONYMs.
        static const int32_t ACRONYM_DEP;
        
        static const int32_t SOUTHEAST_ASIAN;
        static const int32_t IDEOGRAPHIC;
        static const int32_t HIRAGANA;
        static const int32_t KATAKANA;
        static const int32_t HANGUL;
  
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
        
        /// @return true if StandardTokenizer now returns these tokens as Hosts, otherwise false
        /// @deprecated Remove in 3.X and make true the only valid value
        bool isReplaceInvalidAcronym();
        
        /// @param replaceInvalidAcronym Set to true to replace mischaracterized acronyms as HOST.
        /// @deprecated Remove in 3.X and make true the only valid value
        void setReplaceInvalidAcronym(bool replaceInvalidAcronym);
    };
}

#endif
