/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef UAX29URLEMAILTOKENIZER_H
#define UAX29URLEMAILTOKENIZER_H

#include "Tokenizer.h"

namespace Lucene
{
    /// This class implements Word Break rules from the Unicode Text Segmentation algorithm.
    /// URLs and email addresses are also tokenized according to the relevant RFCs.
    ///
    /// Tokens produced are of the following types:
    /// <ul>
    ///    <li><ALPHANUM>: A sequence of alphabetic and numeric characters
    ///    <li><NUM>: A number
    ///    <li><URL>: A URL
    ///    <li><EMAIL>: An email address
    ///    <li><SOUTHEAST_ASIAN>: A sequence of characters from South and Southeast Asian 
    ///    languages, including Thai, Lao, Myanmar, and Khmer
    ///    <li><IDEOGRAPHIC>: A single CJKV ideographic character
    ///    <li><HIRAGANA>: A single hiragana character
    /// </ul>
    class UAX29URLEmailTokenizer : public Tokenizer
    {
    public:
        /// Construct a token stream processing the given input.
        UAX29URLEmailTokenizer(ReaderPtr input);
        
        /// Construct a token stream processing the given input using the given AttributeFactory.
        UAX29URLEmailTokenizer(AttributeFactoryPtr factory, ReaderPtr input);
        
        /// Construct a token stream processing the given input using the given AttributeSource.
        UAX29URLEmailTokenizer(AttributeSourcePtr source, ReaderPtr input);
        
        virtual ~UAX29URLEmailTokenizer();
        
        LUCENE_CLASS(UAX29URLEmailTokenizer);
    
    protected:
        /// This character denotes the end of file
        static const int32_t YYEOF;
        
        /// Lexical states
        static const int32_t YYINITIAL;
        
        /// Initial size of the lookahead buffer
        static const int32_t ZZ_BUFFERSIZE;
        
        /// ZZ_LEXSTATE[l] is the state in the DFA for the lexical state l
        /// ZZ_LEXSTATE[l+1] is the state in the DFA for the lexical state l at the beginning 
        /// of a line l is of the form l = 2*k, k a non negative integer
        static const int32_t ZZ_LEXSTATE[];
        
        /// Translates characters to character classes
        static const wchar_t ZZ_CMAP_PACKED[];
        static const int32_t ZZ_CMAP_LENGTH;
        static const int32_t ZZ_CMAP_PACKED_LENGTH;
        
        /// Translates characters to character classes
        static const wchar_t* ZZ_CMAP();
        
        /// Translates DFA states to action switch labels.
        static const wchar_t ZZ_ACTION_PACKED_0[];
        static const int32_t ZZ_ACTION_LENGTH;
        static const int32_t ZZ_ACTION_PACKED_LENGTH;
        
        /// Translates DFA states to action switch labels.
        static const int32_t* ZZ_ACTION();
        
        /// Translates a state to a row index in the transition table
        static const String ZZ_ROWMAP_PACKED_0_CHUNK1;
        static const String ZZ_ROWMAP_PACKED_0_CHUNK2;
        static const String ZZ_ROWMAP_PACKED_0_CHUNK3;
        static const String ZZ_ROWMAP_PACKED_0;
        static const int32_t ZZ_ROWMAP_LENGTH;
        static const int32_t ZZ_ROWMAP_PACKED_LENGTH;
        
        /// Translates a state to a row index in the transition table
        static const int32_t* ZZ_ROWMAP();
        
        /// The transition table of the DFA
        static const String ZZ_TRANS_PACKED_0;
        static const String ZZ_TRANS_PACKED_0_CHUNK1;
        static const String ZZ_TRANS_PACKED_0_CHUNK2;
        static const String ZZ_TRANS_PACKED_0_CHUNK3;
        static const String ZZ_TRANS_PACKED_0_CHUNK4;
        static const String ZZ_TRANS_PACKED_0_CHUNK5;
        static const String ZZ_TRANS_PACKED_0_CHUNK6;
        static const String ZZ_TRANS_PACKED_0_CHUNK7;
        static const int32_t ZZ_TRANS_LENGTH;
        static const int32_t ZZ_TRANS_PACKED_LENGTH;
        
        /// The transition table of the DFA
        static const int32_t* ZZ_TRANS();
        
        // error codes
        static const int32_t ZZ_UNKNOWN_ERROR;
        static const int32_t ZZ_NO_MATCH;
        static const int32_t ZZ_PUSHBACK_2BIG;
        
        static const wchar_t* ZZ_ERROR_MSG[];
        
        /// ZZ_ATTRIBUTE[aState] contains the attributes of state aState
        static const wchar_t ZZ_ATTRIBUTE_PACKED_0[];
        static const int32_t ZZ_ATTRIBUTE_LENGTH;
        static const int32_t ZZ_ATTRIBUTE_PACKED_LENGTH;
        
        /// ZZ_ATTRIBUTE[aState] contains the attributes of state aState
        static const int32_t* ZZ_ATTRIBUTE();
        
        /// The input device
        ReaderPtr zzReader;
        
        /// The current state of the DFA
        int32_t zzState;
        
        /// The current lexical state
        int32_t zzLexicalState;
        
        /// This buffer contains the current text to be matched and is the source of the yytext() string
        CharArray zzBuffer;
        
        /// The text position at the last accepting state
        int32_t zzMarkedPos;
        
        /// The current text position in the buffer
        int32_t zzCurrentPos;
        
        /// StartRead marks the beginning of the yytext() string in the buffer
        int32_t zzStartRead;
        
        /// EndRead marks the last character in the buffer, that has been read from input
        int32_t zzEndRead;
        
        /// Number of newlines encountered up to the start of the matched text
        int32_t yyline;
        
        /// The number of characters up to the start of the matched text
        int32_t _yychar;
        
        /// The number of characters from the last newline up to the start of the matched text
        int32_t yycolumn;
        
        /// zzAtBOL == true if the scanner is currently at the beginning of a line
        bool zzAtBOL;
        
        /// zzAtEOF == true if the scanner is at the EOF
        bool zzAtEOF;
        
        /// denotes if the user-EOF-code has already been executed
        bool zzEOFDone;
        
        CharTermAttributePtr termAtt;
        OffsetAttributePtr offsetAtt;
        PositionIncrementAttributePtr posIncrAtt;
        TypeAttributePtr typeAtt;
        
        int32_t maxTokenLength;
        int32_t posIncr;
    
    public:
        /// Alphanumeric sequences
        static const String WORD_TYPE;
        
        /// Numbers
        static const String NUMERIC_TYPE;
        
        /// URLs with scheme: HTTP(S), FTP, or FILE; no-scheme URLs match HTTP syntax
        static const String URL_TYPE;
        
        /// E-mail addresses
        static const String EMAIL_TYPE;
        
        static const String SOUTH_EAST_ASIAN_TYPE;
        static const String IDEOGRAPHIC_TYPE;
        static const String HIRAGANA_TYPE;
        static const String KATAKANA_TYPE;
        static const String HANGUL_TYPE;
        
    public:
        /// Set the max allowed token length.  Any token longer than this is skipped.
        /// @param length the new max allowed token length
        void setMaxTokenLength(int32_t length);
        
        /// Returns the max allowed token length.  Any token longer than this is skipped.
        /// @return the max allowed token length 
        int32_t getMaxTokenLength();
        
        virtual void end();
        
        using Tokenizer::reset;
        virtual void reset(ReaderPtr input);
        
        virtual bool incrementToken();
        
        /// Closes the input stream.
        void yyclose();
        
        /// Resets the scanner to read from a new input stream.  Does not close the old reader.
        ///
        /// All internal variables are reset, the old input stream cannot be reused (internal buffer is
        /// discarded and lost). Lexical state is set to ZZ_INITIAL.
        ///
        /// Internal scan buffer is resized down to its initial length, if it has grown.
        ///
        /// @param reader the new input stream.
        void yyreset(ReaderPtr reader);
        
        /// Returns the current lexical state.
        int32_t yystate();
        
        /// Enters a new lexical state
        /// @param newState the new lexical state.
        void yybegin(int32_t newState);
        
        /// Returns the text matched by the current regular expression.
        String yytext();
        
        /// Returns the character at position pos from the  matched text. 
        ///
        /// It is equivalent to yytext()[pos], but faster
        /// @param pos the position of the character to fetch.  A value from 0 to yylength() - 1.
        /// @return the character at position pos.
        wchar_t yycharat(int32_t pos);
        
        /// Returns the length of the matched text region.
        int32_t yylength();
        
        /// Pushes the specified amount of characters back into the input stream.
        ///
        /// They will be read again by then next call of the scanning method
        /// @param number  the number of characters to be read again.  This number must not be
        /// greater than yylength()
        void yypushback(int32_t number);
        
        /// Resumes scanning until the next regular expression is matched, the end of input is 
        /// encountered or an I/O-Error occurs.
        bool getNextToken();
        
    protected:
        void ConstructUAX29URLEmailTokenizer(ReaderPtr input);
        
        /// Populates this TokenStream's CharTermAttribute and OffsetAttribute from the current 
        /// match, the TypeAttribute from the passed-in tokenType, and the 
        /// PositionIncrementAttribute to one, unless the immediately previous token(s) was/were 
        /// skipped because maxTokenLength was exceeded, in which case the 
        /// PositionIncrementAttribute is set to one plus the number of skipped overly long tokens. 
        ///
        /// If maxTokenLength is exceeded, the CharTermAttribute is set back to empty and false 
        /// is returned.
        ///
        /// @param tokenType The type of the matching token
        /// @return true there is a token available (not too long); false otherwise 
        bool populateAttributes(const String& tokenType);
        
        /// Refills the input buffer.
        bool zzRefill();
        
        /// Reports an error that occurred while scanning.
        ///
        /// In a well-formed scanner (no or only correct usage of yypushback(int32_t) and a 
        /// match-all fallback rule) this method will only be called with things that 
        /// "Can't Possibly Happen".  If this method is called,  something is seriously wrong.
        ///
        /// Usual syntax/scanner level error handling should be done in error fallback rules.
        ///
        /// @param errorCode The code of the error message to display.
        void zzScanError(int32_t errorCode);
    };
}

#endif
