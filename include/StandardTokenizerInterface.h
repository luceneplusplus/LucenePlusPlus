/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STANDARDTOKENIZERINTERFACE_H
#define STANDARDTOKENIZERINTERFACE_H

#include "LuceneObject.h"

namespace Lucene
{
    class StandardTokenizerInterface : public LuceneObject
    {
    public:
        virtual ~StandardTokenizerInterface();
        LUCENE_CLASS(StandardTokenizerInterface);
        
    public:
        /// This character denotes the end of file
        static const int32_t YYEOF;
        
    public:
        /// Fills CharTermAttribute with the current token text.
        virtual void getText(CharTermAttributePtr t) = 0;
        
        /// Returns the current position.
        virtual int32_t yychar() = 0;
        
        /// Resets the scanner to read from a new input stream.  Does not close the 
        /// old reader.
        ///
        /// All internal variables are reset, the old input stream cannot be reused 
        /// (internal buffer is discarded and lost). Lexical state is set to ZZ_INITIAL.
        ///
        /// Internal scan buffer is resized down to its initial length, if it has grown.
        ///
        /// @param reader the new input stream.
        virtual void yyreset(ReaderPtr reader) = 0;
        
        /// Returns the length of the matched text region.
        virtual int32_t yylength() = 0;
        
        /// Resumes scanning until the next regular expression is matched, the end of 
        /// input is encountered or an I/O-Error occurs.
        /// @return the next token, {@link #YYEOF} on end of stream
        virtual int32_t getNextToken() = 0;
    };
}

#endif
