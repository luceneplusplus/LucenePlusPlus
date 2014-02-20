/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "ContribInc.h"
#include <boost/algorithm/string.hpp>
#include "GermanStemmer.h"
#include "MiscUtils.h"
#include "UnicodeUtils.h"
#include "StringUtils.h"

namespace Lucene {

GermanStemmer::GermanStemmer() {
    substCount = 0;
}

GermanStemmer::~GermanStemmer() {
}

String GermanStemmer::stem(const String& term) {
    // Use lowercase for medium stemming.
    buffer = StringUtils::toLower(term);
    if (!isStemmable()) {
        return buffer;
    }

    // Stemming starts here
    substitute();
    strip();
    optimize();
    resubstitute();
    removeParticleDenotion();

    return buffer;
}

bool GermanStemmer::isStemmable() {
    for (int32_t c = 0; c < (int32_t)buffer.length(); ++c) {
        if (!UnicodeUtil::isAlpha(buffer[c])) {
            return false;
        }
    }
    return true;
}

void GermanStemmer::strip() {
    bool doMore = true;
    while (doMore && buffer.length() > 3) {
        if (buffer.length() + substCount > 5 && boost::ends_with(buffer, L"nd")) {
            buffer.resize(buffer.length() - 2);
        } else if (buffer.length() + substCount > 4 && boost::ends_with(buffer, L"em")) {
            buffer.resize(buffer.length() - 2);
        } else if (buffer.length() + substCount > 4 && boost::ends_with(buffer, L"er")) {
            buffer.resize(buffer.length() - 2);
        } else if (buffer[buffer.length() - 1] == L'e') {
            buffer.resize(buffer.length() - 1);
        } else if (buffer[buffer.length() - 1] == L's') {
            buffer.resize(buffer.length() - 1);
        } else if (buffer[buffer.length() - 1] == L'n') {
            buffer.resize(buffer.length() - 1);
        }
        // "t" occurs only as suffix of verbs.
        else if (buffer[buffer.length() - 1] == L't') {
            buffer.resize(buffer.length() - 1);
        } else {
            doMore = false;
        }
    }
}

void GermanStemmer::optimize() {
    // Additional step for female plurals of professions and inhabitants.
    if (buffer.length() > 5 && boost::ends_with(buffer, L"erin*")) {
        buffer.resize(buffer.length() - 1);
        strip();
    }

    // Additional step for irregular plural nouns like "Matrizen -> Matrix".
    if (buffer[buffer.length() - 1] == L'z') {
        buffer[buffer.length() - 1] = L'x';
    }
}

void GermanStemmer::removeParticleDenotion() {
    if (buffer.length() > 4) {
        for (int32_t c = 0; c < (int32_t)buffer.length() - 3; ++c) {
            if (buffer.substr(c, 4) == L"gege") {
                buffer.erase(c, 2);
                return;
            }
        }
    }
}

void GermanStemmer::substitute() {
    substCount = 0;
    for (int32_t c = 0; c < (int32_t)buffer.length(); ++c) {
        // Replace the second char of a pair of the equal characters with an asterisk
        if (c > 0 && buffer[c] == buffer[c - 1]) {
            buffer[c] = L'*';
        }
        // Substitute Umlauts.
        else if (buffer[c] == L'\x00e4') {
            buffer[c] = L'a';
        } else if (buffer[c] == L'\x00f6') {
            buffer[c] = L'o';
        } else if (buffer[c] == L'\x00fc') {
            buffer[c] = L'u';
        }
        // Fix bug so that 'ß' at the end of a word is replaced.
        else if (buffer[c] == L'\x00df') {
            buffer[c] = L's';
            buffer.insert(c + 1, 1, L's');
            ++substCount;
        }
        // Take care that at least one character is left left side from the current one
        if (c < (int32_t)buffer.length() - 1) {
            // Masking several common character combinations with an token
            if (c < (int32_t)buffer.length() - 2 && buffer[c] == L's' && buffer[c + 1] == L'c' && buffer[c + 2] == L'h') {
                buffer[c] = L'$';
                buffer.erase(c + 1, 2);
                substCount += 2;
            } else if (buffer[c] == L'c' && buffer[c + 1] == L'h') {
                buffer[c] = L'\x00a7';
                buffer.erase(c + 1, 1);
                ++substCount;
            } else if (buffer[c] == L'e' && buffer[c + 1] == L'i') {
                buffer[c] = L'%';
                buffer.erase(c + 1, 1);
                ++substCount;
            } else if (buffer[c] == L'i' && buffer[c + 1] == L'e') {
                buffer[c] = L'&';
                buffer.erase(c + 1, 1);
                ++substCount;
            } else if (buffer[c] == L'i' && buffer[c + 1] == L'g') {
                buffer[c] = L'#';
                buffer.erase(c + 1, 1);
                ++substCount;
            } else if (buffer[c] == L's' && buffer[c + 1] == L't') {
                buffer[c] = L'!';
                buffer.erase(c + 1, 1);
                ++substCount;
            }
        }
    }
}

void GermanStemmer::resubstitute() {
    for (int32_t c = 0; c < (int32_t)buffer.length(); ++c) {
        if (buffer[c] == L'*') {
            buffer[c] = buffer[c - 1];
        } else if (buffer[c] == L'$') {
            buffer[c] = L's';
            buffer.insert(c + 1, L"ch");
        } else if (buffer[c] == L'\x00a7') {
            buffer[c] = L'c';
            buffer.insert(c + 1, 1, L'h');
        } else if (buffer[c] == L'%') {
            buffer[c] = L'e';
            buffer.insert(c + 1, 1, L'i');
        } else if (buffer[c] == L'&') {
            buffer[c] = L'i';
            buffer.insert(c + 1, 1, L'e');
        } else if (buffer[c] == L'#') {
            buffer[c] = L'i';
            buffer.insert(c + 1, 1, L'g');
        } else if (buffer[c] == L'!') {
            buffer[c] = L's';
            buffer.insert(c + 1, 1, L't');
        }
    }
}

}
