/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PERSIANNORMALIZER_H
#define PERSIANNORMALIZER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Normalizer for Persian.
///
/// Normalization is done in-place for efficiency, operating on a termbuffer.
///
/// Normalization is defined as:
/// <ul>
/// <li> Normalization of various heh + hamza forms and heh goal to heh.
/// <li> Normalization of farsi yeh and yeh barree to arabic yeh.
/// <li> Normalization of persian keheh to arabic kaf.
/// </ul>
class LPPCONTRIBAPI PersianNormalizer : public LuceneObject {
public:
    virtual ~PersianNormalizer();

    LUCENE_CLASS(PersianNormalizer);

public:
    static const wchar_t YEH;
    static const wchar_t FARSI_YEH;
    static const wchar_t YEH_BARREE;
    static const wchar_t KEHEH;
    static const wchar_t KAF;
    static const wchar_t HAMZA_ABOVE;
    static const wchar_t HEH_YEH;
    static const wchar_t HEH_GOAL;
    static const wchar_t HEH;

public:
    /// Normalize an input buffer of Persian text
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
