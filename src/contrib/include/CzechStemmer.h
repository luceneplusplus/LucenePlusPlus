/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CZECHSTEMMER_H
#define CZECHSTEMMER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene
{
    /// Light Stemmer for Czech.
    ///
    /// Implements the algorithm described in:  
    /// Indexing and stemming approaches for the Czech language
    /// http://portal.acm.org/citation.cfm?id=1598600
    class LPPCONTRIBAPI CzechStemmer : public LuceneObject
    {
    public:
        CzechStemmer();
        virtual ~CzechStemmer();
        
        LUCENE_CLASS(CzechStemmer);
    
    public:
        /// Stem an input buffer of Czech text.
        /// @param s input buffer
        /// @param len length of input buffer
        /// @return length of input buffer after normalization
        /// NOTE: Input is expected to be in lowercase, but with diacritical marks
        int32_t stem(wchar_t* s, int32_t len);

    protected:
        int32_t removeCase(wchar_t* s, int32_t len);
        int32_t removePossessives(wchar_t* s, int32_t len);
        int32_t normalize(wchar_t* s, int32_t len);
    };
}

#endif
