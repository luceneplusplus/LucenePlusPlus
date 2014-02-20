/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include "ArabicStemmer.h"
#include "MiscUtils.h"

namespace Lucene {

const wchar_t ArabicStemmer::ALEF = (wchar_t)0x0627;
const wchar_t ArabicStemmer::BEH = (wchar_t)0x0628;
const wchar_t ArabicStemmer::TEH_MARBUTA = (wchar_t)0x0629;
const wchar_t ArabicStemmer::TEH = (wchar_t)0x062a;
const wchar_t ArabicStemmer::FEH = (wchar_t)0x0641;
const wchar_t ArabicStemmer::KAF = (wchar_t)0x0643;
const wchar_t ArabicStemmer::LAM = (wchar_t)0x0644;
const wchar_t ArabicStemmer::NOON = (wchar_t)0x0646;
const wchar_t ArabicStemmer::HEH = (wchar_t)0x0647;
const wchar_t ArabicStemmer::WAW = (wchar_t)0x0648;
const wchar_t ArabicStemmer::YEH = (wchar_t)0x064a;

ArabicStemmer::~ArabicStemmer() {
}

const Collection<String> ArabicStemmer::prefixes() {
    static Collection<String> _prefixes;
    if (!_prefixes) {
        _prefixes = Collection<String>::newInstance();
        _prefixes.add(String(L"") + ALEF + LAM);
        _prefixes.add(String(L"") + WAW + ALEF + LAM);
        _prefixes.add(String(L"") + BEH + ALEF + LAM);
        _prefixes.add(String(L"") + KAF + ALEF + LAM);
        _prefixes.add(String(L"") + FEH + ALEF + LAM);
        _prefixes.add(String(L"") + LAM + LAM);
        _prefixes.add(String(L"") + WAW);
    }
    return _prefixes;
}

const Collection<String> ArabicStemmer::suffixes() {
    static Collection<String> _suffixes;
    if (!_suffixes) {
        _suffixes = Collection<String>::newInstance();
        _suffixes.add(String(L"") + HEH + ALEF);
        _suffixes.add(String(L"") + ALEF + NOON);
        _suffixes.add(String(L"") + ALEF + TEH);
        _suffixes.add(String(L"") + WAW + NOON);
        _suffixes.add(String(L"") + YEH + NOON);
        _suffixes.add(String(L"") + YEH + HEH);
        _suffixes.add(String(L"") + YEH + TEH_MARBUTA);
        _suffixes.add(String(L"") + HEH);
        _suffixes.add(String(L"") + TEH_MARBUTA);
        _suffixes.add(String(L"") + YEH);
    }
    return _suffixes;
}

int32_t ArabicStemmer::stem(wchar_t* s, int32_t len) {
    len = stemPrefix(s, len);
    len = stemSuffix(s, len);
    return len;
}

int32_t ArabicStemmer::stemPrefix(wchar_t* s, int32_t len) {
    Collection<String> stemPrefixes(prefixes());
    for (int32_t i = 0; i < stemPrefixes.size(); ++i) {
        if (startsWith(s, len, stemPrefixes[i])) {
            return deleteChars(s, 0, len, (int32_t)stemPrefixes[i].length());
        }
    }
    return len;
}

int32_t ArabicStemmer::stemSuffix(wchar_t* s, int32_t len) {
    Collection<String> stemSuffixes(suffixes());
    for (int32_t i = 0; i < stemSuffixes.size(); ++i) {
        if (endsWith(s, len, stemSuffixes[i])) {
            len = (int32_t)deleteChars(s, (int32_t)(len - stemSuffixes[i].length()), len, (int32_t)stemSuffixes[i].length());
        }
    }
    return len;
}

bool ArabicStemmer::startsWith(wchar_t* s, int32_t len, const String& prefix) {
    if (prefix.length() == 1 && len < 4) { // wa- prefix requires at least 3 characters
        return false;
    } else if (len < (int32_t)prefix.length() + 2) { // other prefixes require only 2
        return false;
    } else {
        for (int32_t i = 0; i < (int32_t)prefix.length(); ++i) {
            if (s[i] != prefix[i]) {
                return false;
            }
        }
        return true;
    }
}

bool ArabicStemmer::endsWith(wchar_t* s, int32_t len, const String& suffix) {
    if (len < (int32_t)suffix.length() + 2) { // all suffixes require at least 2 characters after stemming
        return false;
    } else {
        for (int32_t i = 0; i < (int32_t)suffix.length(); ++i) {
            if (s[len - suffix.length() + i] != suffix[i]) {
                return false;
            }
        }
        return true;
    }
}

int32_t ArabicStemmer::deleteChars(wchar_t* s, int32_t pos, int32_t len, int32_t chars) {
    for (int32_t i = 0; i < chars; ++i) {
        len = deleteChar(s, pos, len);
    }
    return len;
}

int32_t ArabicStemmer::deleteChar(wchar_t* s, int32_t pos, int32_t len) {
    if (pos < len) {
        MiscUtils::arrayCopy(s, pos + 1, s, pos, len - pos - 1);
    }
    return len - 1;
}

}
