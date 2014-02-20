/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ARABICNORMALIZER_H
#define ARABICNORMALIZER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Normalizer for Arabic.
///
/// Normalization is done in-place for efficiency, operating on a termbuffer.
///
/// Normalization is defined as:
/// <ul>
/// <li> Normalization of hamza with alef seat to a bare alef.
/// <li> Normalization of teh marbuta to heh
/// <li> Normalization of dotless yeh (alef maksura) to yeh.
/// <li> Removal of Arabic diacritics (the harakat)
/// <li> Removal of tatweel (stretching character).
/// </ul>
class LPPCONTRIBAPI ArabicNormalizer : public LuceneObject {
public:
    virtual ~ArabicNormalizer();

    LUCENE_CLASS(ArabicNormalizer);

public:
    static const wchar_t ALEF;
    static const wchar_t ALEF_MADDA;
    static const wchar_t ALEF_HAMZA_ABOVE;
    static const wchar_t ALEF_HAMZA_BELOW;

    static const wchar_t YEH;
    static const wchar_t DOTLESS_YEH;

    static const wchar_t TEH_MARBUTA;
    static const wchar_t HEH;

    static const wchar_t TATWEEL;

    static const wchar_t FATHATAN;
    static const wchar_t DAMMATAN;
    static const wchar_t KASRATAN;
    static const wchar_t FATHA;
    static const wchar_t DAMMA;
    static const wchar_t KASRA;
    static const wchar_t SHADDA;
    static const wchar_t SUKUN;

public:
    /// Normalize an input buffer of Arabic text
    /// @param s input buffer
    /// @param len length of input buffer
    /// @return length of input buffer after normalization
    int32_t normalize(wchar_t* s, int32_t len);

    /// Delete a character in-place
    /// @param s Input Buffer
    /// @param pos Position of character to delete
    /// @param len length of input buffer
    /// @return length of input buffer after deletion
    int32_t deleteChar(wchar_t* s, int32_t pos, int32_t len);
};

}

#endif
