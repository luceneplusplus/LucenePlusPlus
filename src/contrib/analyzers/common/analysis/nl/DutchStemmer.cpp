/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include <boost/algorithm/string.hpp>
#include "DutchStemmer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"
#include "StringUtils.h"

namespace Lucene {

DutchStemmer::DutchStemmer() {
    removedE = false;
    R1 = 0;
    R2 = 0;
}

DutchStemmer::~DutchStemmer() {
}

String DutchStemmer::stem(const String& term) {
    // Use lowercase for medium stemming.
    buffer = StringUtils::toLower(term);
    if (!isStemmable()) {
        return buffer;
    }

    if (stemDict && stemDict.contains(term)) {
        return stemDict.get(term);
    }

    // Stemming starts here...
    substitute();
    storeYandI();
    R1 = getRIndex(0);
    R1 = std::max((int32_t)3, R1);
    step1();
    step2();
    R2 = getRIndex(R1);
    step3a();
    step3b();
    step4();
    reStoreYandI();
    return buffer;
}

bool DutchStemmer::enEnding() {
    Collection<String> enend = newCollection<String>(L"ene", L"en");
    for (int32_t i = 0; i < enend.size(); ++i) {
        String end = enend[i];
        int32_t index = (int32_t)(buffer.length() - end.length());
        if (boost::ends_with(buffer, end) && index >= R1 && isValidEnEnding(index - 1)) {
            buffer.erase(index, end.length());
            unDouble(index);
            return true;
        }
    }
    return false;
}

void DutchStemmer::step1() {
    if (R1 >= (int32_t)buffer.length()) {
        return;
    }

    int32_t lengthR1 = (int32_t)(buffer.length() - R1);
    int32_t index;

    if (boost::ends_with(buffer, L"heden")) {
        buffer.replace(R1, lengthR1, boost::replace_all_copy(buffer.substr(R1, lengthR1), L"heden", L"heid"));
        return;
    }

    if (enEnding()) {
        return;
    }

    index = (int32_t)buffer.length() - 2;
    if (boost::ends_with(buffer, L"se") && index >= R1 && isValidSEnding(index - 1)) {
        buffer.erase(index, 2);
        return;
    }

    index = (int32_t)(buffer.length() - 1);
    if (boost::ends_with(buffer, L"s") && index >= R1 && isValidSEnding(index - 1)) {
        buffer.erase(index, 1);
    }
}

void DutchStemmer::step2() {
    removedE = false;
    if (R1 >= (int32_t)buffer.length()) {
        return;
    }
    int32_t index = (int32_t)(buffer.length() - 1);
    if (index >= R1 && boost::ends_with(buffer, L"e") && !isVowel(buffer[index - 1])) {
        buffer.erase(index, 1);
        unDouble();
        removedE = true;
    }
}

void DutchStemmer::step3a() {
    if (R2 >= (int32_t)buffer.length()) {
        return;
    }
    int32_t index = (int32_t)(buffer.length() - 4);
    if (boost::ends_with(buffer, L"heid") && index >= R2 && buffer[index - 1] != L'c') {
        buffer.erase(index, 4); // remove heid
        enEnding();
    }
}

void DutchStemmer::step3b() {
    if (R2 >= (int32_t)buffer.length()) {
        return;
    }

    int32_t index = (int32_t)(buffer.length() - 3);
    if ((boost::ends_with(buffer, L"end") || boost::ends_with(buffer, L"ing")) && index >= R2) {
        buffer.erase(index, 3);
        if (buffer[index - 2] == L'i' && buffer[index - 1] == L'g') {
            if (buffer[index - 3] != L'e' && index - 2 >= R2) {
                index -= 2;
                buffer.erase(index, 2);
            }
        } else {
            unDouble(index);
        }
        return;
    }
    index = (int32_t)(buffer.length() - 2);
    if (boost::ends_with(buffer, L"ig") && index >= R2) {
        if (buffer[index - 1] != L'e') {
            buffer.erase(index, 2);
        }
        return;
    }
    index = (int32_t)(buffer.length() - 4);
    if (boost::ends_with(buffer, L"lijk") && index >= R2) {
        buffer.erase(index, 4);
        step2();
        return;
    }
    index = (int32_t)(buffer.length() - 4);
    if (boost::ends_with(buffer, L"baar") && index >= R2) {
        buffer.erase(index, 4);
        return;
    }
    index = (int32_t)(buffer.length() - 3);
    if (boost::ends_with(buffer, L"bar") && index >= R2) {
        if (removedE) {
            buffer.erase(index, 3);
        }
        return;
    }
}

void DutchStemmer::step4() {
    if (buffer.length() < 4) {
        return;
    }
    String end(buffer.substr(buffer.length() - 4));
    if (end[1] == end[2] && end[3] != L'I' && end[1] != L'i' && isVowel(end[1]) && !isVowel(end[3]) && !isVowel(end[0])) {
        buffer.erase(buffer.length() - 2, 1);
    }
}

bool DutchStemmer::isStemmable() {
    for (int32_t c = 0; c < (int32_t)buffer.length(); ++c) {
        if (!UnicodeUtil::isAlnum(buffer[c])) {
            return false;
        }
    }
    return true;
}

void DutchStemmer::substitute() {
    for (int32_t i = 0; i < (int32_t)buffer.length(); ++i) {
        switch (buffer[i]) {
        case L'\x00e4':
        case L'\x00e1':
            buffer[i] = L'a';
            break;
        case L'\x00eb':
        case L'\x00e9':
            buffer[i] = L'e';
            break;
        case L'\x00fc':
        case L'\x00fa':
            buffer[i] = L'u';
            break;
        case L'\x00ef':
        case L'i':
            buffer[i] = L'i';
            break;
        case L'\x00f6':
        case L'\x00f3':
            buffer[i] = L'o';
            break;
        }
    }
}

bool DutchStemmer::isValidSEnding(int32_t index) {
    wchar_t c = buffer[index];
    if (isVowel(c) || c == L'j') {
        return false;
    }
    return true;
}

bool DutchStemmer::isValidEnEnding(int32_t index) {
    wchar_t c = buffer[index];
    if (isVowel(c)) {
        return false;
    }
    if (c < 3) {
        return false;
    }
    // ends with "gem"?
    if (c == L'm' && buffer[index - 2] == L'g' && buffer[index - 1] == L'e') {
        return false;
    }
    return true;
}

void DutchStemmer::unDouble() {
    unDouble((int32_t)buffer.length());
}

void DutchStemmer::unDouble(int32_t endIndex) {
    String s = buffer.substr(0, endIndex);
    if (boost::ends_with(s, L"kk") || boost::ends_with(s, L"tt") || boost::ends_with(s, L"dd") ||
            boost::ends_with(s, L"nn") || boost::ends_with(s, L"mm") || boost::ends_with(s, L"ff")) {
        buffer.resize(endIndex - 1);
    }
}

int32_t DutchStemmer::getRIndex(int32_t start) {
    if (start == 0) {
        start = 1;
    }
    int32_t i = start;
    for (; i < (int32_t)buffer.length(); ++i) {
        // first non-vowel preceded by a vowel
        if (!isVowel(buffer[i]) && isVowel(buffer[i - 1])) {
            return i + 1;
        }
    }
    return i + 1;
}

void DutchStemmer::storeYandI() {
    if (buffer[0] == L'y') {
        buffer[0] = L'Y';
    }

    int32_t last = (int32_t)(buffer.length() - 1);

    for (int32_t i = 1; i < last; i++) {
        switch (buffer[i]) {
        case L'i':
            if (isVowel(buffer[i - 1]) && isVowel(buffer[i + 1])) {
                buffer[i] = L'I';
            }
            break;
        case L'y':
            if (isVowel(buffer[i - 1])) {
                buffer[i] = L'Y';
            }
            break;
        }
    }
    if (last > 0 && buffer[last] == L'y' && isVowel(buffer[last - 1])) {
        buffer[last] = L'Y';
    }
}

void DutchStemmer::reStoreYandI() {
    boost::replace_all(buffer, L"I", L"i");
    boost::replace_all(buffer, L"Y", L"y");
}

bool DutchStemmer::isVowel(wchar_t c) {
    switch (c) {
    case L'e':
    case L'a':
    case L'o':
    case L'i':
    case L'u':
    case L'y':
    case L'\x00e8':
        return true;
    default:
        return false;
    }
}

void DutchStemmer::setStemDictionary(MapStringString dict) {
    stemDict = dict;
}

}
