/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "BrazilianStemmer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"
#include "StringUtils.h"

namespace Lucene {

BrazilianStemmer::~BrazilianStemmer() {
}

String BrazilianStemmer::stem(const String& term) {
    // creates CT
    createCT(term);

    if (!isIndexable(CT)) {
        return L"";
    }
    if (!isStemmable(CT)) {
        return CT;
    }

    R1 = getR1(CT);
    R2 = getR1(R1);
    RV = getRV(CT);
    TERM = term + L";" + CT;

    bool altered = step1();
    if (!altered) {
        altered = step2();
    }

    if (altered) {
        step3();
    } else {
        step4();
    }

    step5();

    return CT;
}

bool BrazilianStemmer::isStemmable(const String& term) {
    for (int32_t c = 0; c < (int32_t)term.length(); ++c) {
        // Discard terms that contain non-letter characters.
        if (!UnicodeUtil::isAlpha(term[c])) {
            return false;
        }
    }
    return true;
}

bool BrazilianStemmer::isIndexable(const String& term) {
    return (term.length() < 30) && (term.length() > 2);
}

bool BrazilianStemmer::isVowel(wchar_t value) {
    return (value == L'a' || value == L'e' || value == L'i' || value == L'o' || value == L'u');
}

String BrazilianStemmer::getR1(const String& value) {
    if (value.empty()) {
        return L"";
    }

    // find 1st vowel
    int32_t i = (int32_t)(value.length() - 1);
    int32_t j = 0;
    for (; j < i; ++j) {
        if (isVowel(value[j])) {
            break;
        }
    }

    if (j >= i) {
        return L"";
    }

    // find 1st non-vowel
    for (; j < i; ++j) {
        if (!isVowel(value[j])) {
            break;
        }
    }

    if (j >= i) {
        return L"";
    }

    return value.substr(j + 1);
}

String BrazilianStemmer::getRV(const String& value) {
    if (value.empty()) {
        return L"";
    }

    int32_t i = (int32_t)(value.length() - 1);

    // RV - IF the second letter is a consonant, RV is the region after the next following vowel
    if (i > 0 && !isVowel(value[1])) {
        int32_t j = 2;
        // find 1st vowel
        for (; j < i; ++j) {
            if (isVowel(value[j])) {
                break;
            }
        }

        if (j < i) {
            return value.substr(j + 1);
        }
    }


    // RV - OR if the first two letters are vowels, RV is the region after the next consonant,
    if (i > 1 && isVowel(value[0]) && isVowel(value[1])) {
        int32_t j = 2;
        // find 1st consonant
        for (; j < i; ++j) {
            if (!isVowel(value[j])) {
                break;
            }
        }

        if (j < i) {
            return value.substr(j + 1);
        }
    }

    // RV - AND otherwise (consonant-vowel case) RV is the region after the third letter.
    if (i > 2) {
        return value.substr(3);
    }

    return L"";
}

String BrazilianStemmer::changeTerm(const String& value) {
    if (value.empty()) {
        return L"";
    }

    String lowerValue(StringUtils::toLower(value));
    String r;

    for (int32_t j = 0; j < (int32_t)value.length(); ++j) {
        if (value[j] == 0x00e1 || value[j] == 0x00e2 || value[j] == 0x00e3) {
            r += L"a";
            continue;
        }
        if (value[j] == 0x00e9 || value[j] == 0x00ea) {
            r += L"e";
            continue;
        }
        if (value[j] == 0x00ed) {
            r += L"i";
            continue;
        }
        if (value[j] == 0x00f3 || value[j] == 0x00f4 || value[j] == 0x00f5) {
            r += L"o";
            continue;
        }
        if (value[j] == 0x00fa || value[j] == 0x00fc) {
            r += L"u";
            continue;
        }
        if (value[j] == 0x00e7) {
            r += L"c";
            continue;
        }
        if (value[j] == 0x00f1) {
            r += L"n";
            continue;
        }

        r += value[j];
    }

    return r ;
}

bool BrazilianStemmer::checkSuffix(const String& value, const String& suffix) {
    if (value.empty() || suffix.empty()) {
        return false;
    }
    if (suffix.length() > value.length()) {
        return false;
    }
    return (value.substr(value.length() - suffix.length()) == suffix);
}

String BrazilianStemmer::replaceSuffix(const String& value, const String& toReplace, const String& changeTo) {
    if (value.empty() || toReplace.empty() || changeTo.empty()) {
        return value;
    }

    String vvalue = removeSuffix(value, toReplace);

    if (value == vvalue) {
        return value;
    } else {
        return vvalue + changeTo;
    }
}

String BrazilianStemmer::removeSuffix(const String& value, const String& toRemove) {
    if (value.empty() || toRemove.empty() || !checkSuffix(value, toRemove)) {
        return value;
    }
    return value.substr(0, value.length() - toRemove.length());
}

bool BrazilianStemmer::suffixPreceded(const String& value, const String& suffix, const String& preceded) {
    if (value.empty() || suffix.empty() || preceded.empty() || !checkSuffix(value, suffix)) {
        return false;
    }
    return checkSuffix(removeSuffix(value, suffix), preceded);
}

void BrazilianStemmer::createCT(const String& term) {
    CT = changeTerm(term);

    if (CT.length() < 2) {
        return;
    }

    // if the first character is ... , remove it
    if (CT[0] == L'"' || CT[0] == L'\'' || CT[0] == L'-' || CT[0] == L',' ||
            CT[0] == L';' || CT[0] == L'.' || CT[0] == L'?' || CT[0] == L'!') {
        CT = CT.substr(1);
    }

    if (CT.length() < 2) {
        return;
    }

    // if the last character is ... , remove it
    if (CT[CT.length() - 1] == L'-' || CT[CT.length() - 1] == L',' || CT[CT.length() - 1] == L';' ||
            CT[CT.length() - 1] == L'.' || CT[CT.length() - 1] == L'?' || CT[CT.length() - 1] == L'!' ||
            CT[CT.length() - 1] == L'\'' || CT[CT.length() - 1] == L'"') {
        CT = CT.substr(0, CT.length() - 1);
    }
}

bool BrazilianStemmer::step1() {
    if (CT.empty()) {
        return false;
    }

    // suffix length = 7
    if (checkSuffix(CT, L"uciones") && checkSuffix(R2, L"uciones")) {
        CT = replaceSuffix(CT, L"uciones", L"u");
        return true;
    }

    // suffix length = 6
    if (CT.length() >= 6) {
        if (checkSuffix(CT, L"imentos") && checkSuffix(R2, L"imentos")) {
            CT = removeSuffix(CT, L"imentos");
            return true;
        }
        if (checkSuffix(CT, L"amentos") && checkSuffix(R2, L"amentos")) {
            CT = removeSuffix(CT, L"amentos");
            return true;
        }
        if (checkSuffix(CT, L"adores") && checkSuffix(R2, L"adores")) {
            CT = removeSuffix(CT, L"adores");
            return true;
        }
        if (checkSuffix(CT, L"adoras") && checkSuffix(R2, L"adoras")) {
            CT = removeSuffix(CT, L"adoras");
            return true;
        }
        if (checkSuffix(CT, L"logias") && checkSuffix(R2, L"logias")) {
            replaceSuffix(CT, L"logias", L"log");
            return true;
        }
        if (checkSuffix(CT, L"encias") && checkSuffix(R2, L"encias")) {
            CT = replaceSuffix(CT, L"encias", L"ente");
            return true;
        }
        if (checkSuffix(CT, L"amente") && checkSuffix(R1, L"amente")) {
            CT = removeSuffix(CT, L"amente");
            return true;
        }
        if (checkSuffix(CT, L"idades") && checkSuffix(R2, L"idades")) {
            CT = removeSuffix(CT, L"idades");
            return true;
        }
    }

    // suffix length = 5
    if (CT.length() >= 5) {
        if (checkSuffix(CT, L"acoes") && checkSuffix(R2, L"acoes")) {
            CT = removeSuffix(CT, L"acoes");
            return true;
        }
        if (checkSuffix(CT, L"imento") && checkSuffix(R2, L"imento")) {
            CT = removeSuffix(CT, L"imento");
            return true;
        }
        if (checkSuffix(CT, L"amento") && checkSuffix(R2, L"amento")) {
            CT = removeSuffix(CT, L"amento");
            return true;
        }
        if (checkSuffix(CT, L"adora") && checkSuffix(R2, L"adora")) {
            CT = removeSuffix(CT, L"adora");
            return true;
        }
        if (checkSuffix(CT, L"ismos") && checkSuffix(R2, L"ismos")) {
            CT = removeSuffix(CT, L"ismos");
            return true;
        }
        if (checkSuffix(CT, L"istas") && checkSuffix(R2, L"istas")) {
            CT = removeSuffix(CT, L"istas");
            return true;
        }
        if (checkSuffix(CT, L"logia") && checkSuffix(R2, L"logia")) {
            CT = replaceSuffix(CT, L"logia", L"log");
            return true;
        }
        if (checkSuffix(CT, L"ucion") && checkSuffix(R2, L"ucion")) {
            CT = replaceSuffix(CT, L"ucion", L"u");
            return true;
        }
        if (checkSuffix(CT, L"encia") && checkSuffix(R2, L"encia")) {
            CT = replaceSuffix(CT, L"encia", L"ente");
            return true;
        }
        if (checkSuffix(CT, L"mente") && checkSuffix(R2, L"mente")) {
            CT = removeSuffix(CT, L"mente");
            return true;
        }
        if (checkSuffix(CT, L"idade") && checkSuffix(R2, L"idade")) {
            CT = removeSuffix(CT, L"idade");
            return true;
        }
    }

    // suffix length = 4
    if (CT.length() >= 4) {
        if (checkSuffix(CT, L"acao") && checkSuffix(R2, L"acao")) {
            CT = removeSuffix(CT, L"acao");
            return true;
        }
        if (checkSuffix(CT, L"ezas") && checkSuffix(R2, L"ezas")) {
            CT = removeSuffix(CT, L"ezas");
            return true;
        }
        if (checkSuffix(CT, L"icos") && checkSuffix(R2, L"icos")) {
            CT = removeSuffix(CT, L"icos");
            return true;
        }
        if (checkSuffix(CT, L"icas") && checkSuffix(R2, L"icas")) {
            CT = removeSuffix(CT, L"icas");
            return true;
        }
        if (checkSuffix(CT, L"ismo") && checkSuffix(R2, L"ismo")) {
            CT = removeSuffix(CT, L"ismo");
            return true;
        }
        if (checkSuffix(CT, L"avel") && checkSuffix(R2, L"avel")) {
            CT = removeSuffix(CT, L"avel");
            return true;
        }
        if (checkSuffix(CT, L"ivel") && checkSuffix(R2, L"ivel")) {
            CT = removeSuffix(CT, L"ivel");
            return true;
        }
        if (checkSuffix(CT, L"ista") && checkSuffix(R2, L"ista")) {
            CT = removeSuffix(CT, L"ista");
            return true;
        }
        if (checkSuffix(CT, L"osos") && checkSuffix(R2, L"osos")) {
            CT = removeSuffix(CT, L"osos");
            return true;
        }
        if (checkSuffix(CT, L"osas") && checkSuffix(R2, L"osas")) {
            CT = removeSuffix(CT, L"osas");
            return true;
        }
        if (checkSuffix(CT, L"ador") && checkSuffix(R2, L"ador")) {
            CT = removeSuffix(CT, L"ador");
            return true;
        }
        if (checkSuffix(CT, L"ivas") && checkSuffix(R2, L"ivas")) {
            CT = removeSuffix(CT, L"ivas");
            return true;
        }
        if (checkSuffix(CT, L"ivos") && checkSuffix(R2, L"ivos")) {
            CT = removeSuffix(CT, L"ivos");
            return true;
        }
        if (checkSuffix(CT, L"iras") && checkSuffix(RV, L"iras") && suffixPreceded(CT, L"iras", L"e")) {
            CT = replaceSuffix(CT, L"iras", L"ir");
            return true;
        }
    }

    // suffix length = 3
    if (CT.length() >= 3) {
        if (checkSuffix(CT, L"eza") && checkSuffix(R2, L"eza")) {
            CT = removeSuffix(CT, L"eza");
            return true;
        }
        if (checkSuffix(CT, L"ico") && checkSuffix(R2, L"ico")) {
            CT = removeSuffix(CT, L"ico");
            return true;
        }
        if (checkSuffix(CT, L"ica") && checkSuffix(R2, L"ica")) {
            CT = removeSuffix(CT, L"ica");
            return true;
        }
        if (checkSuffix(CT, L"oso") && checkSuffix(R2, L"oso")) {
            CT = removeSuffix(CT, L"oso");
            return true;
        }
        if (checkSuffix(CT, L"osa") && checkSuffix(R2, L"osa")) {
            CT = removeSuffix(CT, L"osa");
            return true;
        }
        if (checkSuffix(CT, L"iva") && checkSuffix(R2, L"iva")) {
            CT = removeSuffix(CT, L"iva");
            return true;
        }
        if (checkSuffix(CT, L"ivo") && checkSuffix(R2, L"ivo")) {
            CT = removeSuffix(CT, L"ivo");
            return true;
        }
        if (checkSuffix(CT, L"ira") && checkSuffix(RV, L"ira") && suffixPreceded(CT, L"ira", L"e")) {
            CT = replaceSuffix(CT, L"ira", L"ir");
            return true;
        }
    }

    // no ending was removed by step1
    return false;
}

bool BrazilianStemmer::step2() {
    if (RV.empty()) {
        return false;
    }

    // suffix lenght = 7
    if (RV.length() >= 7) {
        if (checkSuffix(RV, L"issemos")) {
            CT = removeSuffix(CT, L"issemos");
            return true;
        }
        if (checkSuffix(RV, L"essemos")) {
            CT = removeSuffix(CT, L"essemos");
            return true;
        }
        if (checkSuffix(RV, L"assemos")) {
            CT = removeSuffix(CT, L"assemos");
            return true;
        }
        if (checkSuffix(RV, L"ariamos")) {
            CT = removeSuffix(CT, L"ariamos");
            return true;
        }
        if (checkSuffix(RV, L"eriamos")) {
            CT = removeSuffix(CT, L"eriamos");
            return true;
        }
        if (checkSuffix(RV, L"iriamos")) {
            CT = removeSuffix(CT, L"iriamos");
            return true;
        }
    }

    // suffix length = 6
    if (RV.length() >= 6) {
        if (checkSuffix(RV, L"iremos")) {
            CT = removeSuffix(CT, L"iremos");
            return true;
        }
        if (checkSuffix(RV, L"eremos")) {
            CT = removeSuffix(CT, L"eremos");
            return true;
        }
        if (checkSuffix(RV, L"aremos")) {
            CT = removeSuffix(CT, L"aremos");
            return true;
        }
        if (checkSuffix(RV, L"avamos")) {
            CT = removeSuffix(CT, L"avamos");
            return true;
        }
        if (checkSuffix(RV, L"iramos")) {
            CT = removeSuffix(CT, L"iramos");
            return true;
        }
        if (checkSuffix(RV, L"eramos")) {
            CT = removeSuffix(CT, L"eramos");
            return true;
        }
        if (checkSuffix(RV, L"aramos")) {
            CT = removeSuffix(CT, L"aramos");
            return true;
        }
        if (checkSuffix(RV, L"asseis")) {
            CT = removeSuffix(CT, L"asseis");
            return true;
        }
        if (checkSuffix(RV, L"esseis")) {
            CT = removeSuffix(CT, L"esseis");
            return true;
        }
        if (checkSuffix(RV, L"isseis")) {
            CT = removeSuffix(CT, L"isseis");
            return true;
        }
        if (checkSuffix(RV, L"arieis")) {
            CT = removeSuffix(CT, L"arieis");
            return true;
        }
        if (checkSuffix(RV, L"erieis")) {
            CT = removeSuffix(CT, L"erieis");
            return true;
        }
        if (checkSuffix(RV, L"irieis")) {
            CT = removeSuffix(CT, L"irieis");
            return true;
        }
    }

    // suffix length = 5
    if (RV.length() >= 5) {
        if (checkSuffix(RV, L"irmos")) {
            CT = removeSuffix(CT, L"irmos");
            return true;
        }
        if (checkSuffix(RV, L"iamos")) {
            CT = removeSuffix(CT, L"iamos");
            return true;
        }
        if (checkSuffix(RV, L"armos")) {
            CT = removeSuffix(CT, L"armos");
            return true;
        }
        if (checkSuffix(RV, L"ermos")) {
            CT = removeSuffix(CT, L"ermos");
            return true;
        }
        if (checkSuffix(RV, L"areis")) {
            CT = removeSuffix(CT, L"areis");
            return true;
        }
        if (checkSuffix(RV, L"ereis")) {
            CT = removeSuffix(CT, L"ereis");
            return true;
        }
        if (checkSuffix(RV, L"ireis")) {
            CT = removeSuffix(CT, L"ireis");
            return true;
        }
        if (checkSuffix(RV, L"asses")) {
            CT = removeSuffix(CT, L"asses");
            return true;
        }
        if (checkSuffix(RV, L"esses")) {
            CT = removeSuffix(CT, L"esses");
            return true;
        }
        if (checkSuffix(RV, L"isses")) {
            CT = removeSuffix(CT, L"isses");
            return true;
        }
        if (checkSuffix(RV, L"astes")) {
            CT = removeSuffix(CT, L"astes");
            return true;
        }
        if (checkSuffix(RV, L"assem")) {
            CT = removeSuffix(CT, L"assem");
            return true;
        }
        if (checkSuffix(RV, L"essem")) {
            CT = removeSuffix(CT, L"essem");
            return true;
        }
        if (checkSuffix(RV, L"issem")) {
            CT = removeSuffix(CT, L"issem");
            return true;
        }
        if (checkSuffix(RV, L"ardes")) {
            CT = removeSuffix(CT, L"ardes");
            return true;
        }
        if (checkSuffix(RV, L"erdes")) {
            CT = removeSuffix(CT, L"erdes");
            return true;
        }
        if (checkSuffix(RV, L"irdes")) {
            CT = removeSuffix(CT, L"irdes");
            return true;
        }
        if (checkSuffix(RV, L"ariam")) {
            CT = removeSuffix(CT, L"ariam");
            return true;
        }
        if (checkSuffix(RV, L"eriam")) {
            CT = removeSuffix(CT, L"eriam");
            return true;
        }
        if (checkSuffix(RV, L"iriam")) {
            CT = removeSuffix(CT, L"iriam");
            return true;
        }
        if (checkSuffix(RV, L"arias")) {
            CT = removeSuffix(CT, L"arias");
            return true;
        }
        if (checkSuffix(RV, L"erias")) {
            CT = removeSuffix(CT, L"erias");
            return true;
        }
        if (checkSuffix(RV, L"irias")) {
            CT = removeSuffix(CT, L"irias");
            return true;
        }
        if (checkSuffix(RV, L"estes")) {
            CT = removeSuffix(CT, L"estes");
            return true;
        }
        if (checkSuffix(RV, L"istes")) {
            CT = removeSuffix(CT, L"istes");
            return true;
        }
        if (checkSuffix(RV, L"areis")) {
            CT = removeSuffix(CT, L"areis");
            return true;
        }
        if (checkSuffix(RV, L"aveis")) {
            CT = removeSuffix(CT, L"aveis");
            return true;
        }
    }

    // suffix length = 4
    if (RV.length() >= 4) {
        if (checkSuffix(RV, L"aria")) {
            CT = removeSuffix(CT, L"aria");
            return true;
        }
        if (checkSuffix(RV, L"eria")) {
            CT = removeSuffix(CT, L"eria");
            return true;
        }
        if (checkSuffix(RV, L"iria")) {
            CT = removeSuffix(CT, L"iria");
            return true;
        }
        if (checkSuffix(RV, L"asse")) {
            CT = removeSuffix(CT, L"asse");
            return true;
        }
        if (checkSuffix(RV, L"esse")) {
            CT = removeSuffix(CT, L"esse");
            return true;
        }
        if (checkSuffix(RV, L"isse")) {
            CT = removeSuffix(CT, L"isse");
            return true;
        }
        if (checkSuffix(RV, L"aste")) {
            CT = removeSuffix(CT, L"aste");
            return true;
        }
        if (checkSuffix(RV, L"este")) {
            CT = removeSuffix(CT, L"este");
            return true;
        }
        if (checkSuffix(RV, L"iste")) {
            CT = removeSuffix(CT, L"iste");
            return true;
        }
        if (checkSuffix(RV, L"arei")) {
            CT = removeSuffix(CT, L"arei");
            return true;
        }
        if (checkSuffix(RV, L"erei")) {
            CT = removeSuffix(CT, L"erei");
            return true;
        }
        if (checkSuffix(RV, L"irei")) {
            CT = removeSuffix(CT, L"irei");
            return true;
        }
        if (checkSuffix(RV, L"aram")) {
            CT = removeSuffix(CT, L"aram");
            return true;
        }
        if (checkSuffix(RV, L"eram")) {
            CT = removeSuffix(CT, L"eram");
            return true;
        }
        if (checkSuffix(RV, L"iram")) {
            CT = removeSuffix(CT, L"iram");
            return true;
        }
        if (checkSuffix(RV, L"avam")) {
            CT = removeSuffix(CT, L"avam");
            return true;
        }
        if (checkSuffix(RV, L"arem")) {
            CT = removeSuffix(CT, L"arem");
            return true;
        }
        if (checkSuffix(RV, L"erem")) {
            CT = removeSuffix(CT, L"erem");
            return true;
        }
        if (checkSuffix(RV, L"irem")) {
            CT = removeSuffix(CT, L"irem");
            return true;
        }
        if (checkSuffix(RV, L"ando")) {
            CT = removeSuffix(CT, L"ando");
            return true;
        }
        if (checkSuffix(RV, L"endo")) {
            CT = removeSuffix(CT, L"endo");
            return true;
        }
        if (checkSuffix(RV, L"indo")) {
            CT = removeSuffix(CT, L"indo");
            return true;
        }
        if (checkSuffix(RV, L"arao")) {
            CT = removeSuffix(CT, L"arao");
            return true;
        }
        if (checkSuffix(RV, L"erao")) {
            CT = removeSuffix(CT, L"erao");
            return true;
        }
        if (checkSuffix(RV, L"irao")) {
            CT = removeSuffix(CT, L"irao");
            return true;
        }
        if (checkSuffix(RV, L"adas")) {
            CT = removeSuffix(CT, L"adas");
            return true;
        }
        if (checkSuffix(RV, L"idas")) {
            CT = removeSuffix(CT, L"idas");
            return true;
        }
        if (checkSuffix(RV, L"aras")) {
            CT = removeSuffix(CT, L"aras");
            return true;
        }
        if (checkSuffix(RV, L"eras")) {
            CT = removeSuffix(CT, L"eras");
            return true;
        }
        if (checkSuffix(RV, L"iras")) {
            CT = removeSuffix(CT, L"iras");
            return true;
        }
        if (checkSuffix(RV, L"avas")) {
            CT = removeSuffix(CT, L"avas");
            return true;
        }
        if (checkSuffix(RV, L"ares")) {
            CT = removeSuffix(CT, L"ares");
            return true;
        }
        if (checkSuffix(RV, L"eres")) {
            CT = removeSuffix(CT, L"eres");
            return true;
        }
        if (checkSuffix(RV, L"ires")) {
            CT = removeSuffix(CT, L"ires");
            return true;
        }
        if (checkSuffix(RV, L"ados")) {
            CT = removeSuffix(CT, L"ados");
            return true;
        }
        if (checkSuffix(RV, L"idos")) {
            CT = removeSuffix(CT, L"idos");
            return true;
        }
        if (checkSuffix(RV, L"amos")) {
            CT = removeSuffix(CT, L"amos");
            return true;
        }
        if (checkSuffix(RV, L"emos")) {
            CT = removeSuffix(CT, L"emos");
            return true;
        }
        if (checkSuffix(RV, L"imos")) {
            CT = removeSuffix(CT, L"imos");
            return true;
        }
        if (checkSuffix(RV, L"iras")) {
            CT = removeSuffix(CT, L"iras");
            return true;
        }
        if (checkSuffix(RV, L"ieis")) {
            CT = removeSuffix(CT, L"ieis");
            return true;
        }
    }

    // suffix length = 3
    if (RV.length() >= 3) {
        if (checkSuffix(RV, L"ada")) {
            CT = removeSuffix(CT, L"ada");
            return true;
        }
        if (checkSuffix(RV, L"ida")) {
            CT = removeSuffix(CT, L"ida");
            return true;
        }
        if (checkSuffix(RV, L"ara")) {
            CT = removeSuffix(CT, L"ara");
            return true;
        }
        if (checkSuffix(RV, L"era")) {
            CT = removeSuffix(CT, L"era");
            return true;
        }
        if (checkSuffix(RV, L"ira")) {
            CT = removeSuffix(CT, L"ava");
            return true;
        }
        if (checkSuffix(RV, L"iam")) {
            CT = removeSuffix(CT, L"iam");
            return true;
        }
        if (checkSuffix(RV, L"ado")) {
            CT = removeSuffix(CT, L"ado");
            return true;
        }
        if (checkSuffix(RV, L"ido")) {
            CT = removeSuffix(CT, L"ido");
            return true;
        }
        if (checkSuffix(RV, L"ias")) {
            CT = removeSuffix(CT, L"ias");
            return true;
        }
        if (checkSuffix(RV, L"ais")) {
            CT = removeSuffix(CT, L"ais");
            return true;
        }
        if (checkSuffix(RV, L"eis")) {
            CT = removeSuffix(CT, L"eis");
            return true;
        }
        if (checkSuffix(RV, L"ira")) {
            CT = removeSuffix(CT, L"ira");
            return true;
        }
        if (checkSuffix(RV, L"ear")) {
            CT = removeSuffix(CT, L"ear");
            return true;
        }
    }

    // suffix length = 2
    if (RV.length() >= 2) {
        if (checkSuffix(RV, L"ia")) {
            CT = removeSuffix(CT, L"ia");
            return true;
        }
        if (checkSuffix(RV, L"ei")) {
            CT = removeSuffix(CT, L"ei");
            return true;
        }
        if (checkSuffix(RV, L"am")) {
            CT = removeSuffix(CT, L"am");
            return true;
        }
        if (checkSuffix(RV, L"em")) {
            CT = removeSuffix(CT, L"em");
            return true;
        }
        if (checkSuffix(RV, L"ar")) {
            CT = removeSuffix(CT, L"ar");
            return true;
        }
        if (checkSuffix(RV, L"er")) {
            CT = removeSuffix(CT, L"er");
            return true;
        }
        if (checkSuffix(RV, L"ir")) {
            CT = removeSuffix(CT, L"ir");
            return true;
        }
        if (checkSuffix(RV, L"as")) {
            CT = removeSuffix(CT, L"as");
            return true;
        }
        if (checkSuffix(RV, L"es")) {
            CT = removeSuffix(CT, L"es");
            return true;
        }
        if (checkSuffix(RV, L"is")) {
            CT = removeSuffix(CT, L"is");
            return true;
        }
        if (checkSuffix(RV, L"eu")) {
            CT = removeSuffix(CT, L"eu");
            return true;
        }
        if (checkSuffix(RV, L"iu")) {
            CT = removeSuffix(CT, L"iu");
            return true;
        }
        if (checkSuffix(RV, L"iu")) {
            CT = removeSuffix(CT, L"iu");
            return true;
        }
        if (checkSuffix(RV, L"ou")) {
            CT = removeSuffix(CT, L"ou");
            return true;
        }
    }

    // no ending was removed by step2
    return false;
}

void BrazilianStemmer::step3() {
    if (RV.empty()) {
        return;
    }

    if (checkSuffix(RV, L"i") && suffixPreceded(RV, L"i", L"c")) {
        CT = removeSuffix(CT, L"i");
    }
}

void BrazilianStemmer::step4() {
    if (RV.empty()) {
        return;
    }

    if (checkSuffix(RV, L"os")) {
        CT = removeSuffix(CT, L"os");
        return;
    }
    if (checkSuffix(RV, L"a")) {
        CT = removeSuffix(CT, L"a");
        return;
    }
    if (checkSuffix(RV, L"i")) {
        CT = removeSuffix(CT, L"i");
        return;
    }
    if (checkSuffix(RV, L"o")) {
        CT = removeSuffix(CT, L"o");
        return;
    }
}

void BrazilianStemmer::step5() {
    if (RV.empty()) {
        return;
    }

    if (checkSuffix(RV, L"e")) {
        if (suffixPreceded(RV, L"e", L"gu")) {
            CT = removeSuffix(CT, L"e");
            CT = removeSuffix(CT, L"u");
            return;
        }

        if (suffixPreceded(RV, L"e", L"ci")) {
            CT = removeSuffix(CT, L"e");
            CT = removeSuffix(CT, L"i");
            return;
        }

        CT = removeSuffix(CT, L"e");
        return;
    }
}

}
