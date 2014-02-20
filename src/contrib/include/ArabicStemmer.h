/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ARABICSTEMMER_H
#define ARABICSTEMMER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Stemmer for Arabic.
///
/// Stemming  is done in-place for efficiency, operating on a termbuffer.
///
/// Stemming is defined as:
/// <ul>
/// <li> Removal of attached definite article, conjunction, and prepositions.
/// <li> Stemming of common suffixes.
/// </ul>
class LPPCONTRIBAPI ArabicStemmer : public LuceneObject {
public:
    virtual ~ArabicStemmer();

    LUCENE_CLASS(ArabicStemmer);

public:
    static const wchar_t ALEF;
    static const wchar_t BEH;
    static const wchar_t TEH_MARBUTA;
    static const wchar_t TEH;
    static const wchar_t FEH;
    static const wchar_t KAF;
    static const wchar_t LAM;
    static const wchar_t NOON;
    static const wchar_t HEH;
    static const wchar_t WAW;
    static const wchar_t YEH;

public:
    static const Collection<String> prefixes();
    static const Collection<String> suffixes();

    /// Stem an input buffer of Arabic text.
    /// @param s input buffer
    /// @param len length of input buffer
    /// @return length of input buffer after normalization
    int32_t stem(wchar_t* s, int32_t len);

    /// Stem a prefix off an Arabic word.
    /// @param s input buffer
    /// @param len length of input buffer
    /// @return new length of input buffer after stemming.
    int32_t stemPrefix(wchar_t* s, int32_t len);

    /// Stem suffix(es) off an Arabic word.
    /// @param s input buffer
    /// @param len length of input buffer
    /// @return new length of input buffer after stemming
    int32_t stemSuffix(wchar_t* s, int32_t len);

    /// Returns true if the prefix matches and can be stemmed
    /// @param s input buffer
    /// @param len length of input buffer
    /// @param prefix prefix to check
    /// @return true if the prefix matches and can be stemmed
    bool startsWith(wchar_t* s, int32_t len, const String& prefix);

    /// Returns true if the suffix matches and can be stemmed
    /// @param s input buffer
    /// @param len length of input buffer
    /// @param suffix suffix to check
    /// @return true if the suffix matches and can be stemmed
    bool endsWith(wchar_t* s, int32_t len, const String& suffix);

protected:
    /// Delete n characters in-place
    /// @param s Input Buffer
    /// @param pos Position of character to delete
    /// @param len Length of input buffer
    /// @param chars number of characters to delete
    /// @return length of input buffer after deletion
    int32_t deleteChars(wchar_t* s, int32_t pos, int32_t len, int32_t chars);

    /// Delete a character in-place
    /// @param s Input Buffer
    /// @param pos Position of character to delete
    /// @param len length of input buffer
    /// @return length of input buffer after deletion
    int32_t deleteChar(wchar_t* s, int32_t pos, int32_t len);
};

}

#endif
