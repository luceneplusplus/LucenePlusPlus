/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ArabicNormalizer.h"
#include "MiscUtils.h"

namespace Lucene {

const wchar_t ArabicNormalizer::ALEF = (wchar_t)0x0627;
const wchar_t ArabicNormalizer::ALEF_MADDA = (wchar_t)0x0622;
const wchar_t ArabicNormalizer::ALEF_HAMZA_ABOVE = (wchar_t)0x0623;
const wchar_t ArabicNormalizer::ALEF_HAMZA_BELOW = (wchar_t)0x0625;

const wchar_t ArabicNormalizer::YEH = (wchar_t)0x064a;
const wchar_t ArabicNormalizer::DOTLESS_YEH = (wchar_t)0x0649;

const wchar_t ArabicNormalizer::TEH_MARBUTA = (wchar_t)0x0629;
const wchar_t ArabicNormalizer::HEH = (wchar_t)0x0647;

const wchar_t ArabicNormalizer::TATWEEL = (wchar_t)0x0640;

const wchar_t ArabicNormalizer::FATHATAN = (wchar_t)0x064b;
const wchar_t ArabicNormalizer::DAMMATAN = (wchar_t)0x064c;
const wchar_t ArabicNormalizer::KASRATAN = (wchar_t)0x064d;
const wchar_t ArabicNormalizer::FATHA = (wchar_t)0x064e;
const wchar_t ArabicNormalizer::DAMMA = (wchar_t)0x064f;
const wchar_t ArabicNormalizer::KASRA = (wchar_t)0x0650;
const wchar_t ArabicNormalizer::SHADDA = (wchar_t)0x0651;
const wchar_t ArabicNormalizer::SUKUN = (wchar_t)0x0652;

ArabicNormalizer::~ArabicNormalizer() {
}

int32_t ArabicNormalizer::normalize(wchar_t* s, int32_t len) {
    for (int32_t i = 0; i < len; ++i) {
        switch (s[i]) {
        case ALEF_MADDA:
        case ALEF_HAMZA_ABOVE:
        case ALEF_HAMZA_BELOW:
            s[i] = ALEF;
            break;
        case DOTLESS_YEH:
            s[i] = YEH;
            break;
        case TEH_MARBUTA:
            s[i] = HEH;
            break;
        case TATWEEL:
        case KASRATAN:
        case DAMMATAN:
        case FATHATAN:
        case FATHA:
        case DAMMA:
        case KASRA:
        case SHADDA:
        case SUKUN:
            len = deleteChar(s, i--, len);
            break;
        default:
            break;
        }
    }
    return len;
}

int32_t ArabicNormalizer::deleteChar(wchar_t* s, int32_t pos, int32_t len) {
    if (pos < len) {
        MiscUtils::arrayCopy(s, pos + 1, s, pos, len - pos - 1);
    }
    return len - 1;
}

}
