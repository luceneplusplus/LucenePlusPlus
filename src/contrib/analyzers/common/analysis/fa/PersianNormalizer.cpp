/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "PersianNormalizer.h"
#include "MiscUtils.h"

namespace Lucene {

const wchar_t PersianNormalizer::YEH = (wchar_t)0x064a;
const wchar_t PersianNormalizer::FARSI_YEH = (wchar_t)0x06cc;
const wchar_t PersianNormalizer::YEH_BARREE = (wchar_t)0x06d2;
const wchar_t PersianNormalizer::KEHEH = (wchar_t)0x06a9;
const wchar_t PersianNormalizer::KAF = (wchar_t)0x0643;
const wchar_t PersianNormalizer::HAMZA_ABOVE = (wchar_t)0x0654;
const wchar_t PersianNormalizer::HEH_YEH = (wchar_t)0x06c0;
const wchar_t PersianNormalizer::HEH_GOAL = (wchar_t)0x06c1;
const wchar_t PersianNormalizer::HEH = (wchar_t)0x0647;

PersianNormalizer::~PersianNormalizer() {
}

int32_t PersianNormalizer::normalize(wchar_t* s, int32_t len) {
    for (int32_t i = 0; i < len; ++i) {
        switch (s[i]) {
        case FARSI_YEH:
        case YEH_BARREE:
            s[i] = YEH;
            break;
        case KEHEH:
            s[i] = KAF;
            break;
        case HEH_YEH:
        case HEH_GOAL:
            s[i] = HEH;
            break;
        case HAMZA_ABOVE: // necessary for HEH + HAMZA
            len = deleteChar(s, i--, len);
            break;
        default:
            break;
        }
    }
    return len;
}

int32_t PersianNormalizer::deleteChar(wchar_t* s, int32_t pos, int32_t len) {
    if (pos < len) {
        MiscUtils::arrayCopy(s, pos + 1, s, pos, len - pos - 1);
    }
    return len - 1;
}

}
