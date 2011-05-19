/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef STEMMERUTIL_H
#define STEMMERUTIL_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene
{
    /// Some commonly-used stemming functions.
    class LPPCONTRIBAPI StemmerUtil : public LuceneObject
    {
    public:
        virtual ~StemmerUtil();
        LUCENE_CLASS(StemmerUtil);
    
    public:
        /// Returns true if the prefix matches and can be stemmed
        /// @param s input buffer
        /// @param len length of input buffer
        /// @param prefix prefix to check
        /// @return true if the prefix matches and can be stemmed
        static bool startsWith(wchar_t* s, int32_t len, const String& prefix);
        
        /// Returns true if the suffix matches and can be stemmed
        /// @param s input buffer
        /// @param len length of input buffer
        /// @param suffix suffix to check
        /// @return true if the suffix matches and can be stemmed
        static bool endsWith(wchar_t* s, int32_t len, const String& suffix);
        
        /// Delete a character in-place
        /// @param s Input Buffer
        /// @param pos Position of character to delete
        /// @param len length of input buffer
        /// @return length of input buffer after deletion
        static int32_t _delete(wchar_t* s, int32_t pos, int32_t len);
        
        /// Delete n characters in-place
        /// @param s Input Buffer
        /// @param pos Position of character to delete
        /// @param len Length of input buffer
        /// @param chars number of characters to delete
        /// @return length of input buffer after deletion
        static int32_t _deleteN(wchar_t* s, int32_t pos, int32_t len, int32_t chars);
    };
}

#endif
