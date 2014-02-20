/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include <boost/algorithm/string.hpp>
#include "FrenchStemmer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"
#include "StringUtils.h"

namespace Lucene {

FrenchStemmer::FrenchStemmer() {
    suite = false;
    modified = false;
}

FrenchStemmer::~FrenchStemmer() {
}

String FrenchStemmer::stem(const String& term) {
    if (!isStemmable(term)) {
        return term;
    }

    // Use lowercase for medium stemming.
    stringBuffer = StringUtils::toLower(term);

    // reset the booleans
    modified = false;
    suite = false;

    treatVowels(stringBuffer);

    setStrings();

    step1();

    if (!modified || suite) {
        if (!RV.empty()) {
            suite = step2a();
            if (!suite) {
                step2b();
            }
        }
    }

    if (modified || suite) {
        step3();
    } else {
        step4();
    }

    step5();

    step6();

    return stringBuffer;
}

void FrenchStemmer::setStrings() {
    // set the strings
    R0 = stringBuffer;
    RV = retrieveRV(stringBuffer);
    R1 = retrieveR(stringBuffer);
    if (!R1.empty()) {
        tempBuffer = R1;
        R2 = retrieveR(tempBuffer);
    } else {
        R2.clear();
    }
}

void FrenchStemmer::step1() {
    Collection<String> suffix = newCollection<String>(L"ances", L"iqUes", L"ismes", L"ables", L"istes", L"ance", L"iqUe", L"isme", L"able", L"iste");
    deleteFrom(R2, suffix);

    replaceFrom(R2, newCollection<String>(L"logies", L"logie"), L"log");
    replaceFrom(R2, newCollection<String>(L"usions", L"utions", L"usion", L"ution"), L"u");
    replaceFrom(R2, newCollection<String>(L"ences", L"ence"), L"ent");

    Collection<String> search = newCollection<String>(L"atrices", L"ateurs", L"ations", L"atrice", L"ateur", L"ation");
    deleteButSuffixFromElseReplace(R2, search, L"ic",  true, R0, L"iqU");

    deleteButSuffixFromElseReplace(R2, newCollection<String>(L"ements", L"ement"), L"eus", false, R0, L"eux");
    deleteButSuffixFrom(R2, newCollection<String>(L"ements", L"ement"), L"ativ", false);
    deleteButSuffixFrom(R2, newCollection<String>(L"ements", L"ement"), L"iv", false);
    deleteButSuffixFrom(R2, newCollection<String>(L"ements", L"ement"), L"abl", false);
    deleteButSuffixFrom(R2, newCollection<String>(L"ements", L"ement"), L"iqU", false);

    deleteFromIfTestVowelBeforeIn(R1, newCollection<String>(L"issements", L"issement"), false, R0);
    deleteFrom(RV, newCollection<String>(L"ements", L"ement"));

    deleteButSuffixFromElseReplace(R2, newCollection<String>(L"it\x00e9s", L"it\x00e9"), L"abil", false, R0, L"abl");
    deleteButSuffixFromElseReplace(R2, newCollection<String>(L"it\x00e9s", L"it\x00e9"), L"ic", false, R0, L"iqU");
    deleteButSuffixFrom(R2, newCollection<String>(L"it\x00e9s", L"it\x00e9"), L"iv", true);

    Collection<String> autre = newCollection<String>(L"ifs", L"ives", L"if", L"ive");
    deleteButSuffixFromElseReplace(R2, autre, L"icat", false, R0, L"iqU");
    deleteButSuffixFromElseReplace(R2, autre, L"at", true, R2, L"iqU");

    replaceFrom(R0, newCollection<String>(L"eaux"), L"eau");

    replaceFrom(R1, newCollection<String>(L"aux"), L"al");

    deleteButSuffixFromElseReplace(R2, newCollection<String>(L"euses", L"euse"), L"", true, R1, L"eux");

    deleteFrom(R2, newCollection<String>(L"eux"));

    // if one of the next steps is performed, we will need to perform step2a
    if (replaceFrom(RV, newCollection<String>(L"amment"), L"ant")) {
        suite = true;
    }
    if (replaceFrom(RV, newCollection<String>(L"emment"), L"ent")) {
        suite = true;
    }
    if (deleteFromIfTestVowelBeforeIn(RV, newCollection<String>(L"ments", L"ment"), true, RV)) {
        suite = true;
    }
}

bool FrenchStemmer::step2a() {
    static Collection<String> search;
    if (!search) {
        static const wchar_t* _search[] = {
            L"\x00eemes", L"\x00eetes", L"iraIent", L"irait", L"irais", L"irai", L"iras", L"ira",
            L"irent", L"iriez", L"irez", L"irions", L"irons", L"iront", L"issaIent",
            L"issais", L"issantes", L"issante", L"issants", L"issant", L"issait",
            L"issais", L"issions", L"issons", L"issiez", L"issez", L"issent", L"isses",
            L"isse", L"ir", L"is", L"\x00eet", L"it", L"ies", L"ie", L"i"
        };
        search = Collection<String>::newInstance(_search, _search + SIZEOF_ARRAY(_search));
    }
    return deleteFromIfTestVowelBeforeIn(RV, search, false, RV);
}

void FrenchStemmer::step2b() {
    static Collection<String> suffix;
    if (!suffix) {
        static const wchar_t* _suffix[] = {
            L"eraIent", L"erais", L"erait", L"erai", L"eras", L"erions", L"eriez",
            L"erons", L"eront", L"erez", L"\x00e8rent", L"era", L"\x00e9es", L"iez", L"\x00e9e", L"\x00e9s",
            L"er", L"ez", L"\x00e9"
        };
        suffix = Collection<String>::newInstance(_suffix, _suffix + SIZEOF_ARRAY(_suffix));
    }
    deleteFrom(RV, suffix);

    static Collection<String> search;
    if (!search) {
        static const wchar_t* _search[] = {
            L"assions", L"assiez", L"assent", L"asses", L"asse", L"aIent", L"antes",
            L"aIent", L"Aient", L"ante", L"\x00e2mes", L"\x00e2tes", L"ants", L"ant", L"ait",
            L"a\x00eet", L"ais", L"Ait", L"A\x00eet", L"Ais", L"\x00e2t", L"as", L"ai", L"Ai", L"a"
        };
        search = Collection<String>::newInstance(_search, _search + SIZEOF_ARRAY(_search));
    }
    deleteButSuffixFrom(RV, search, L"e", true);

    deleteFrom(R2, newCollection<String>(L"ions"));
}

void FrenchStemmer::step3() {
    if (!stringBuffer.empty()) {
        wchar_t ch = stringBuffer[stringBuffer.length() - 1];
        if (ch == L'Y') {
            stringBuffer[stringBuffer.length() - 1] = L'i';
            setStrings();
        } else if (ch == L'\x00e7') {
            stringBuffer[stringBuffer.length() - 1] = L'c';
            setStrings();
        }
    }
}

void FrenchStemmer::step4() {
    if (stringBuffer.length() > 1) {
        wchar_t ch = stringBuffer[stringBuffer.length() - 1];
        if (ch == L's') {
            wchar_t b = stringBuffer[stringBuffer.length() - 2];
            if (b != L'a' && b != L'i' && b != L'o' && b != L'u' && b != L'\x00e8' && b != L's') {
                stringBuffer.resize(stringBuffer.length() - 1);
                setStrings();
            }
        }
    }
    if (!deleteFromIfPrecededIn(R2, newCollection<String>(L"ion"), RV, L"s")) {
        deleteFromIfPrecededIn(R2, newCollection<String>(L"ion"), RV, L"t");
    }

    replaceFrom(RV, newCollection<String>(L"I\x00e8re", L"i\x00e8re", L"Ier", L"ier"), L"i");
    deleteFrom(RV, newCollection<String>(L"e"));
    deleteFromIfPrecededIn(RV, newCollection<String>(L"\x00eb"), R0, L"gu");
}

void FrenchStemmer::step5() {
    if (!R0.empty()) {
        if (boost::ends_with(R0, L"enn") || boost::ends_with(R0, L"onn") ||
                boost::ends_with(R0, L"ett") || boost::ends_with(R0, L"ell") || boost::ends_with(R0, L"eill")) {
            stringBuffer.resize(stringBuffer.length() - 1);
            setStrings();
        }
    }
}

void FrenchStemmer::step6() {
    if (!R0.empty()) {
        bool seenVowel = false;
        bool seenConson = false;
        int32_t pos = -1;
        for (int32_t i = (int32_t)(R0.length() - 1); i > -1; --i) {
            wchar_t ch = R0[i];
            if (isVowel(ch)) {
                if (!seenVowel) {
                    if (ch == L'\x00e9' || ch == L'\x00e8') {
                        pos = i;
                        break;
                    }
                }
                seenVowel = true;
            } else {
                if (seenVowel) {
                    break;
                } else {
                    seenConson = true;
                }
            }
        }
        if (pos > -1 && seenConson && !seenVowel) {
            stringBuffer[pos] = L'e';
        }
    }
}

bool FrenchStemmer::deleteFromIfPrecededIn(const String& source, Collection<String> search, const String& from, const String& prefix) {
    bool found = false;
    if (!source.empty()) {
        for (int32_t i = 0; i < search.size(); ++i) {
            if (boost::ends_with(source, search[i])) {
                if (!from.empty() && boost::ends_with(from, prefix + search[i])) {
                    stringBuffer.resize(stringBuffer.length() - search[i].length());
                    found = true;
                    setStrings();
                    break;
                }
            }
        }
    }
    return found;
}

bool FrenchStemmer::deleteFromIfTestVowelBeforeIn(const String& source, Collection<String> search, bool vowel, const String& from) {
    bool found = false;
    if (!source.empty() && !from.empty()) {
        for (int32_t i = 0; i < search.size(); ++i) {
            if (boost::ends_with(source, search[i])) {
                if ((search[i].length() + 1) <= from.length()) {
                    bool test = isVowel(stringBuffer[stringBuffer.length() - (search[i].length() + 1)]);
                    if (test == vowel) {
                        stringBuffer.resize(stringBuffer.length() - search[i].length());
                        modified = true;
                        found = true;
                        setStrings();
                        break;
                    }
                }
            }
        }
    }
    return found;
}

void FrenchStemmer::deleteButSuffixFrom(const String& source, Collection<String> search, const String& prefix, bool without) {
    if (!source.empty()) {
        for (int32_t i = 0; i < search.size(); ++i) {
            if (boost::ends_with(source, prefix + search[i])) {
                stringBuffer.resize(stringBuffer.length() - (prefix.length() + search[i].length()));
                modified = true;
                setStrings();
                break;
            } else if (without && boost::ends_with(source, search[i])) {
                stringBuffer.resize(stringBuffer.length() - search[i].length());
                modified = true;
                setStrings();
                break;
            }
        }
    }
}

void FrenchStemmer::deleteButSuffixFromElseReplace(const String& source, Collection<String> search, const String& prefix, bool without, const String& from, const String& replace) {
    if (!source.empty()) {
        for (int32_t i = 0; i < search.size(); ++i) {
            if (boost::ends_with(source, prefix + search[i])) {
                stringBuffer.resize(stringBuffer.length() - (prefix.length() + search[i].length()));
                modified = true;
                setStrings();
                break;
            } else if (!from.empty() && boost::ends_with(from, prefix + search[i])) {
                stringBuffer.resize(stringBuffer.length() - (prefix.length() + search[i].length()));
                stringBuffer += replace;
                modified = true;
                setStrings();
                break;
            } else if (without && boost::ends_with(source, search[i])) {
                stringBuffer.resize(stringBuffer.length() - search[i].length());
                modified = true;
                setStrings();
                break;
            }
        }
    }
}

bool FrenchStemmer::replaceFrom(const String& source, Collection<String> search, const String& replace) {
    bool found = false;
    if (!source.empty()) {
        for (int32_t i = 0; i < search.size(); ++i) {
            if (boost::ends_with(source, search[i])) {
                stringBuffer.resize(stringBuffer.length() - search[i].length());
                stringBuffer += replace;
                modified = true;
                found = true;
                setStrings();
                break;
            }
        }
    }
    return found;
}

void FrenchStemmer::deleteFrom(const String& source, Collection<String> suffix) {
    if (!source.empty()) {
        for (int32_t i = 0; i < suffix.size(); ++i) {
            if (boost::ends_with(source, suffix[i])) {
                stringBuffer.resize(stringBuffer.length() - suffix[i].length());
                modified = true;
                setStrings();
                break;
            }
        }
    }
}

bool FrenchStemmer::isVowel(wchar_t ch) {
    switch (ch) {
    case L'a':
    case L'e':
    case L'i':
    case L'o':
    case L'u':
    case L'y':
    case L'\x00e2':
    case L'\x00e0':
    case L'\x00eb':
    case L'\x00e9':
    case L'\x00ea':
    case L'\x00e8':
    case L'\x00ef':
    case L'\x00ee':
    case L'\x00f4':
    case L'\x00fc':
    case L'\x00f9':
    case L'\x00fb':
        return true;
    default:
        return false;
    }
}

String FrenchStemmer::retrieveR(const String& buffer) {
    int32_t len = (int32_t)buffer.length();
    int32_t pos = -1;
    for (int32_t c = 0; c < len; ++c) {
        if (isVowel(buffer[c])) {
            pos = c;
            break;
        }
    }
    if (pos > -1) {
        int32_t consonne = -1;
        for (int32_t c = pos; c < len; ++c) {
            if (!isVowel(buffer[c])) {
                consonne = c;
                break;
            }
        }
        if (consonne > -1 && (consonne + 1) < len) {
            return buffer.substr(consonne + 1);
        } else {
            return L"";
        }
    } else {
        return L"";
    }
}

String FrenchStemmer::retrieveRV(const String& buffer) {
    int32_t len = (int32_t)buffer.length();
    if (buffer.length() > 3) {
        if (isVowel(buffer[0]) && isVowel(buffer[1])) {
            return buffer.substr(3);
        } else {
            int32_t pos = 0;
            for (int32_t c = 1; c < len; ++c) {
                if (isVowel(buffer[c])) {
                    pos = c;
                    break;
                }
            }
            if (pos + 1 < len) {
                return buffer.substr(pos + 1);
            } else {
                return L"";
            }
        }
    } else {
        return L"";
    }
}

void FrenchStemmer::treatVowels(String& buffer) {

    for (int32_t c = 0; c < (int32_t)buffer.length(); ++c) {
        wchar_t ch = buffer[c];

        if (c == 0) { // first char
            if (buffer.length() > 1) {
                if (ch == L'y' && isVowel(buffer[c + 1])) {
                    buffer[c] = L'Y';
                }
            }
        } else if (c == buffer.length() - 1) { // last char
            if (ch == L'u' && buffer[c - 1] == L'q') {
                buffer[c] = L'U';
            }
            if (ch == L'y' && isVowel(buffer[c - 1])) {
                buffer[c] = L'Y';
            }
        } else { // other cases
            if (ch == L'u') {
                if (buffer[c - 1] == L'q') {
                    buffer[c] = L'U';
                } else if (isVowel(buffer[c - 1]) && isVowel(buffer[c + 1])) {
                    buffer[c] = L'U';
                }
            }
            if (ch == L'i') {
                if (isVowel(buffer[c - 1]) && isVowel(buffer[c + 1])) {
                    buffer[c] = L'I';
                }
            }
            if (ch == L'y') {
                if (isVowel(buffer[c - 1]) || isVowel(buffer[c + 1])) {
                    buffer[c] = L'Y';
                }
            }
        }
    }
}

bool FrenchStemmer::isStemmable(const String& term) {
    bool upper = false;
    int32_t first = -1;
    for (int32_t c = 0; c < (int32_t)term.length(); ++c) {
        // Discard terms that contain non-letter characters.
        if (!UnicodeUtil::isAlpha(term[c])) {
            return false;
        }
        // Discard terms that contain multiple uppercase letters.
        if (UnicodeUtil::isUpper(term[c])) {
            if (upper) {
                return false;
            } else { // First encountered uppercase letter, set flag and save position.
                first = c;
                upper = true;
            }
        }
    }
    // Discard the term if it contains a single uppercase letter that
    // is not starting the term.
    if (first > 0) {
        return false;
    }
    return true;
}

}
