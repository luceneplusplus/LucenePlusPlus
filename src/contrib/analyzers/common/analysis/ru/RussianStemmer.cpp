/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "RussianStemmer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"

namespace Lucene {

const wchar_t RussianStemmer::A = L'\x0430';
const wchar_t RussianStemmer::V = L'\x0432';
const wchar_t RussianStemmer::G = L'\x0433';
const wchar_t RussianStemmer::E = L'\x0435';
const wchar_t RussianStemmer::I = L'\x0438';
const wchar_t RussianStemmer::I_ = L'\x0439';
const wchar_t RussianStemmer::L = L'\x043b';
const wchar_t RussianStemmer::M = L'\x043c';
const wchar_t RussianStemmer::N = L'\x043d';
const wchar_t RussianStemmer::O = L'\x043e';
const wchar_t RussianStemmer::S = L'\x0441';
const wchar_t RussianStemmer::T = L'\x0442';
const wchar_t RussianStemmer::U = L'\x0443';
const wchar_t RussianStemmer::X = L'\x0445';
const wchar_t RussianStemmer::SH = L'\x0448';
const wchar_t RussianStemmer::SHCH = L'\x0449';
const wchar_t RussianStemmer::Y = L'\x044b';
const wchar_t RussianStemmer::SOFT = L'\x044c';
const wchar_t RussianStemmer::AE = L'\x044d';
const wchar_t RussianStemmer::IU = L'\x044e';
const wchar_t RussianStemmer::IA = L'\x044f';

const wchar_t RussianStemmer::vowels[] = {A, E, I, O, U, Y, AE, IU, IA};

RussianStemmer::RussianStemmer() {
    RV = 0;
    R1 = 0;
    R2 = 0;
}

RussianStemmer::~RussianStemmer() {
}

Collection<String> RussianStemmer::perfectiveGerundEndings1() {
    static Collection<String> _perfectiveGerundEndings1;
    if (!_perfectiveGerundEndings1) {
        _perfectiveGerundEndings1 = Collection<String>::newInstance();
        _perfectiveGerundEndings1.add(String(L"") + V);
        _perfectiveGerundEndings1.add(String(L"") + V + SH + I);
        _perfectiveGerundEndings1.add(String(L"") + V + SH + I + S + SOFT);
    }
    return _perfectiveGerundEndings1;
}

Collection<String> RussianStemmer::perfectiveGerund1Predessors() {
    static Collection<String> _perfectiveGerund1Predessors;
    if (!_perfectiveGerund1Predessors) {
        _perfectiveGerund1Predessors = Collection<String>::newInstance();
        _perfectiveGerund1Predessors.add(String(L"") + A);
        _perfectiveGerund1Predessors.add(String(L"") + IA);
    }
    return _perfectiveGerund1Predessors;
}

Collection<String> RussianStemmer::perfectiveGerundEndings2() {
    static Collection<String> _perfectiveGerundEndings2;
    if (!_perfectiveGerundEndings2) {
        _perfectiveGerundEndings2 = Collection<String>::newInstance();
        _perfectiveGerundEndings2.add(String(L"") + I + V);
        _perfectiveGerundEndings2.add(String(L"") + Y + V);
        _perfectiveGerundEndings2.add(String(L"") + I + V + SH + I);
        _perfectiveGerundEndings2.add(String(L"") + Y + V + SH + I);
        _perfectiveGerundEndings2.add(String(L"") + I + V + SH + I + S + SOFT);
        _perfectiveGerundEndings2.add(String(L"") + Y + V + SH + I + S + SOFT);
    }
    return _perfectiveGerundEndings2;
}

Collection<String> RussianStemmer::adjectiveEndings() {
    static Collection<String> _adjectiveEndings;
    if (!_adjectiveEndings) {
        _adjectiveEndings = Collection<String>::newInstance();
        _adjectiveEndings.add(String(L"") + E + E);
        _adjectiveEndings.add(String(L"") + I + E);
        _adjectiveEndings.add(String(L"") + Y + E);
        _adjectiveEndings.add(String(L"") + O + E);
        _adjectiveEndings.add(String(L"") + E + I_);
        _adjectiveEndings.add(String(L"") + I + I_);
        _adjectiveEndings.add(String(L"") + Y + I_);
        _adjectiveEndings.add(String(L"") + O + I_);
        _adjectiveEndings.add(String(L"") + E + M);
        _adjectiveEndings.add(String(L"") + I + M);
        _adjectiveEndings.add(String(L"") + Y + M);
        _adjectiveEndings.add(String(L"") + O + M);
        _adjectiveEndings.add(String(L"") + I + X);
        _adjectiveEndings.add(String(L"") + Y + X);
        _adjectiveEndings.add(String(L"") + U + IU);
        _adjectiveEndings.add(String(L"") + IU + IU);
        _adjectiveEndings.add(String(L"") + A + IA);
        _adjectiveEndings.add(String(L"") + IA + IA);
        _adjectiveEndings.add(String(L"") + O + IU);
        _adjectiveEndings.add(String(L"") + E + IU);
        _adjectiveEndings.add(String(L"") + I + M + I);
        _adjectiveEndings.add(String(L"") + Y + M + I);
        _adjectiveEndings.add(String(L"") + E + G + O);
        _adjectiveEndings.add(String(L"") + O + G + O);
        _adjectiveEndings.add(String(L"") + E + M + U);
        _adjectiveEndings.add(String(L"") + O + M + U);
    }
    return _adjectiveEndings;
}

Collection<String> RussianStemmer::participleEndings1() {
    static Collection<String> _participleEndings1;
    if (!_participleEndings1) {
        _participleEndings1 = Collection<String>::newInstance();
        _participleEndings1.add(String(L"") + SHCH);
        _participleEndings1.add(String(L"") + E + M);
        _participleEndings1.add(String(L"") + N + N);
        _participleEndings1.add(String(L"") + V + SH);
        _participleEndings1.add(String(L"") + IU + SHCH);
    }
    return _participleEndings1;
}

Collection<String> RussianStemmer::participleEndings2() {
    static Collection<String> _participleEndings2;
    if (!_participleEndings2) {
        _participleEndings2 = Collection<String>::newInstance();
        _participleEndings2.add(String(L"") + I + V + SH);
        _participleEndings2.add(String(L"") + Y + V + SH);
        _participleEndings2.add(String(L"") + U + IU + SHCH);
    }
    return _participleEndings2;
}

Collection<String> RussianStemmer::participle1Predessors() {
    static Collection<String> _participle1Predessors;
    if (!_participle1Predessors) {
        _participle1Predessors = Collection<String>::newInstance();
        _participle1Predessors.add(String(L"") + A);
        _participle1Predessors.add(String(L"") + IA);
    }
    return _participle1Predessors;
}

Collection<String> RussianStemmer::reflexiveEndings() {
    static Collection<String> _participle1Predessors;
    if (!_participle1Predessors) {
        _participle1Predessors = Collection<String>::newInstance();
        _participle1Predessors.add(String(L"") + S + IA);
        _participle1Predessors.add(String(L"") + S + SOFT);
    }
    return _participle1Predessors;
}

Collection<String> RussianStemmer::verbEndings1() {
    static Collection<String> _verbEndings1;
    if (!_verbEndings1) {
        _verbEndings1 = Collection<String>::newInstance();
        _verbEndings1.add(String(L"") + I_);
        _verbEndings1.add(String(L"") + L);
        _verbEndings1.add(String(L"") + N);
        _verbEndings1.add(String(L"") + L + O);
        _verbEndings1.add(String(L"") + N + O);
        _verbEndings1.add(String(L"") + E + T);
        _verbEndings1.add(String(L"") + IU + T);
        _verbEndings1.add(String(L"") + L + A);
        _verbEndings1.add(String(L"") + N + A);
        _verbEndings1.add(String(L"") + L + I);
        _verbEndings1.add(String(L"") + E + M);
        _verbEndings1.add(String(L"") + N + Y);
        _verbEndings1.add(String(L"") + E + T + E);
        _verbEndings1.add(String(L"") + I_ + T + E);
        _verbEndings1.add(String(L"") + T + SOFT);
        _verbEndings1.add(String(L"") + E + SH + SOFT);
        _verbEndings1.add(String(L"") + N + N + O);
    }
    return _verbEndings1;
}

Collection<String> RussianStemmer::verbEndings2() {
    static Collection<String> _verbEndings2;
    if (!_verbEndings2) {
        _verbEndings2 = Collection<String>::newInstance();
        _verbEndings2.add(String(L"") + IU);
        _verbEndings2.add(String(L"") + U + IU);
        _verbEndings2.add(String(L"") + E + N);
        _verbEndings2.add(String(L"") + E + I_);
        _verbEndings2.add(String(L"") + IA + T);
        _verbEndings2.add(String(L"") + U + I_);
        _verbEndings2.add(String(L"") + I + L);
        _verbEndings2.add(String(L"") + Y + L);
        _verbEndings2.add(String(L"") + I + M);
        _verbEndings2.add(String(L"") + Y + M);
        _verbEndings2.add(String(L"") + I + T);
        _verbEndings2.add(String(L"") + Y + T);
        _verbEndings2.add(String(L"") + I + L + A);
        _verbEndings2.add(String(L"") + Y + L + A);
        _verbEndings2.add(String(L"") + E + N + A);
        _verbEndings2.add(String(L"") + I + T + E);
        _verbEndings2.add(String(L"") + I + L + I);
        _verbEndings2.add(String(L"") + Y + L + I);
        _verbEndings2.add(String(L"") + I + L + O);
        _verbEndings2.add(String(L"") + Y + L + O);
        _verbEndings2.add(String(L"") + E + N + O);
        _verbEndings2.add(String(L"") + U + E + T);
        _verbEndings2.add(String(L"") + U + IU + T);
        _verbEndings2.add(String(L"") + E + N + Y);
        _verbEndings2.add(String(L"") + I + T + SOFT);
        _verbEndings2.add(String(L"") + Y + T + SOFT);
        _verbEndings2.add(String(L"") + I + SH + SOFT);
        _verbEndings2.add(String(L"") + E + I_ + T + E);
        _verbEndings2.add(String(L"") + U + I_ + T + E);
    }
    return _verbEndings2;
}

Collection<String> RussianStemmer::verb1Predessors() {
    static Collection<String> _verb1Predessors;
    if (!_verb1Predessors) {
        _verb1Predessors = Collection<String>::newInstance();
        _verb1Predessors.add(String(L"") + A);
        _verb1Predessors.add(String(L"") + IA);
    }
    return _verb1Predessors;
}

Collection<String> RussianStemmer::nounEndings() {
    static Collection<String> _nounEndings;
    if (!_nounEndings) {
        _nounEndings = Collection<String>::newInstance();
        _nounEndings.add(String(L"") + A);
        _nounEndings.add(String(L"") + U);
        _nounEndings.add(String(L"") + I_);
        _nounEndings.add(String(L"") + O);
        _nounEndings.add(String(L"") + U);
        _nounEndings.add(String(L"") + E);
        _nounEndings.add(String(L"") + Y);
        _nounEndings.add(String(L"") + I);
        _nounEndings.add(String(L"") + SOFT);
        _nounEndings.add(String(L"") + IA);
        _nounEndings.add(String(L"") + E + V);
        _nounEndings.add(String(L"") + O + V);
        _nounEndings.add(String(L"") + I + E);
        _nounEndings.add(String(L"") + SOFT + E);
        _nounEndings.add(String(L"") + IA + X);
        _nounEndings.add(String(L"") + I + IU);
        _nounEndings.add(String(L"") + E + I);
        _nounEndings.add(String(L"") + I + I);
        _nounEndings.add(String(L"") + E + I_);
        _nounEndings.add(String(L"") + O + I_);
        _nounEndings.add(String(L"") + E + M);
        _nounEndings.add(String(L"") + A + M);
        _nounEndings.add(String(L"") + O + M);
        _nounEndings.add(String(L"") + A + X);
        _nounEndings.add(String(L"") + SOFT + IU);
        _nounEndings.add(String(L"") + I + IA);
        _nounEndings.add(String(L"") + SOFT + IA);
        _nounEndings.add(String(L"") + I + I_);
        _nounEndings.add(String(L"") + IA + M);
        _nounEndings.add(String(L"") + IA + M + I);
        _nounEndings.add(String(L"") + A + M + I);
        _nounEndings.add(String(L"") + I + E + I_);
        _nounEndings.add(String(L"") + I + IA + M);
        _nounEndings.add(String(L"") + I + E + M);
        _nounEndings.add(String(L"") + I + IA + X);
        _nounEndings.add(String(L"") + I + IA + M + I);
    }
    return _nounEndings;
}

Collection<String> RussianStemmer::superlativeEndings() {
    static Collection<String> _superlativeEndings;
    if (!_superlativeEndings) {
        _superlativeEndings = Collection<String>::newInstance();
        _superlativeEndings.add(String(L"") + E + I_ + SH);
        _superlativeEndings.add(String(L"") + E + I_ + SH + E);
    }
    return _superlativeEndings;
}

Collection<String> RussianStemmer::derivationalEndings() {
    static Collection<String> _derivationalEndings;
    if (!_derivationalEndings) {
        _derivationalEndings = Collection<String>::newInstance();
        _derivationalEndings.add(String(L"") + O + S + T);
        _derivationalEndings.add(String(L"") + O + S + T + SOFT);
    }
    return _derivationalEndings;
}

Collection<String> RussianStemmer::doubleN() {
    static Collection<String> _doubleN;
    if (!_doubleN) {
        _doubleN = Collection<String>::newInstance();
        _doubleN.add(String(L"") + N + N);
    }
    return _doubleN;
}

String RussianStemmer::stem(const String& input) {
    markPositions(input);
    if (RV == 0) {
        return input;    // RV wasn't detected, nothing to stem
    }

    String stemmingZone(input.substr(RV));

    // stemming goes on in RV

    // Step 1
    if (!perfectiveGerund(stemmingZone)) {
        reflexive(stemmingZone);

        if (!adjectival(stemmingZone)) {
            if (!verb(stemmingZone)) {
                noun(stemmingZone);
            }
        }
    }

    // Step 2
    removeI(stemmingZone);

    // Step 3
    derivational(stemmingZone);

    // Step 4
    superlative(stemmingZone);
    undoubleN(stemmingZone);
    removeSoft(stemmingZone);

    // return result
    return input.substr(0, RV) + stemmingZone;
}

String RussianStemmer::stemWord(const String& word) {
    return newLucene<RussianStemmer>()->stem(word);
}

bool RussianStemmer::adjectival(String& stemmingZone) {
    // look for adjective ending in a stemming zone
    if (!findAndRemoveEnding(stemmingZone, adjectiveEndings())) {
        return false;
    }

    if (!findAndRemoveEnding(stemmingZone, participleEndings1(), participle1Predessors())) {
        findAndRemoveEnding(stemmingZone, participleEndings2());
    }

    return true;
}

bool RussianStemmer::derivational(String& stemmingZone) {
    int32_t endingLength = findEnding(stemmingZone, derivationalEndings());
    if (endingLength == 0) {
        return false;    // no derivational ending found
    } else {
        // Ensure that the ending locates in R2
        if (R2 - RV <= (int32_t)stemmingZone.length() - endingLength) {
            stemmingZone.resize(stemmingZone.length() - endingLength);
            return true;
        } else {
            return false;
        }
    }
}

int32_t RussianStemmer::findEnding(String& stemmingZone, int32_t startIndex, Collection<String> theEndingClass) {
    bool match = false;
    for (int32_t i = theEndingClass.size() - 1; i >= 0; --i) {
        String theEnding(theEndingClass[i]);
        // check if the ending is bigger than stemming zone
        if (startIndex < (int32_t)theEnding.length() - 1) {
            match = false;
            continue;
        }
        match = true;
        int32_t stemmingIndex = startIndex;
        for (int32_t j = (int32_t)theEnding.length() - 1; j >= 0; --j) {
            if (stemmingZone[stemmingIndex--] != theEnding[j]) {
                match = false;
                break;
            }
        }
        // check if ending was found
        if (match) {
            return (int32_t)theEndingClass[i].size();    // cut ending
        }
    }
    return 0;
}

int32_t RussianStemmer::findEnding(String& stemmingZone, Collection<String> theEndingClass) {
    return findEnding(stemmingZone, (int32_t)(stemmingZone.length() - 1), theEndingClass);
}

bool RussianStemmer::findAndRemoveEnding(String& stemmingZone, Collection<String> theEndingClass) {
    int32_t endingLength = findEnding(stemmingZone, theEndingClass);
    if (endingLength == 0) {
        return false;    // not found
    } else {
        stemmingZone.resize(stemmingZone.length() - endingLength);
        return true; // cut the ending found
    }
}

bool RussianStemmer::findAndRemoveEnding(String& stemmingZone, Collection<String> theEndingClass, Collection<String> thePredessors) {
    int32_t endingLength = findEnding(stemmingZone, theEndingClass);
    if (endingLength == 0) {
        return false;    // not found
    } else {
        int32_t predessorLength = findEnding(stemmingZone, (int32_t)(stemmingZone.length() - endingLength - 1), thePredessors);
        if (predessorLength == 0) {
            return false;
        } else {
            stemmingZone.resize(stemmingZone.length() - endingLength);
            return true; // cut the ending found
        }
    }
}

void RussianStemmer::markPositions(const String& word) {
    RV = 0;
    R1 = 0;
    R2 = 0;
    int32_t i = 0;
    // find RV
    while ((int32_t)word.length() > i && !isVowel(word[i])) {
        ++i;
    }
    if ((int32_t)word.length() - 1 < ++i) {
        return;    // RV zone is empty
    }
    RV = i;
    // find R1
    while ((int32_t)word.length() > i && isVowel(word[i])) {
        ++i;
    }
    if ((int32_t)word.length() - 1 < ++i) {
        return;    // R1 zone is empty
    }
    R1 = i;
    // find R2
    while ((int32_t)word.length() > i && !isVowel(word[i])) {
        ++i;
    }
    if ((int32_t)word.length() - 1 < ++i) {
        return;    // R2 zone is empty
    }
    while ((int32_t)word.length() > i && isVowel(word[i])) {
        ++i;
    }
    if ((int32_t)word.length() - 1 < ++i) {
        return;    // R2 zone is empty
    }
    R2 = i;
}

bool RussianStemmer::isVowel(wchar_t letter) {
    for (int32_t i = 0; i < SIZEOF_ARRAY(vowels); ++i) {
        if (letter == vowels[i]) {
            return true;
        }
    }
    return false;
}

bool RussianStemmer::noun(String& stemmingZone) {
    return findAndRemoveEnding(stemmingZone, nounEndings());
}

bool RussianStemmer::perfectiveGerund(String& stemmingZone) {
    return findAndRemoveEnding(stemmingZone, perfectiveGerundEndings1(), perfectiveGerund1Predessors()) ||
           findAndRemoveEnding(stemmingZone, perfectiveGerundEndings2());
}

bool RussianStemmer::reflexive(String& stemmingZone) {
    return findAndRemoveEnding(stemmingZone, reflexiveEndings());
}

bool RussianStemmer::removeI(String& stemmingZone) {
    if ((int32_t)stemmingZone.length() > 0 && stemmingZone[stemmingZone.length() - 1] == I) {
        stemmingZone.resize(stemmingZone.length() - 1);
        return true;
    } else {
        return false;
    }
}

bool RussianStemmer::removeSoft(String& stemmingZone) {
    if ((int32_t)stemmingZone.length() > 0 && stemmingZone[stemmingZone.length() - 1] == SOFT) {
        stemmingZone.resize(stemmingZone.length() - 1);
        return true;
    }
    return false;
}

bool RussianStemmer::superlative(String& stemmingZone) {
    return findAndRemoveEnding(stemmingZone, superlativeEndings());
}

bool RussianStemmer::undoubleN(String& stemmingZone) {
    if (findEnding(stemmingZone, doubleN()) != 0) {
        stemmingZone.resize(stemmingZone.length() - 1);
        return true;
    } else {
        return false;
    }
}

bool RussianStemmer::verb(String& stemmingZone) {
    return findAndRemoveEnding(stemmingZone, verbEndings1(), verb1Predessors()) ||
           findAndRemoveEnding(stemmingZone, verbEndings2());
}

}
