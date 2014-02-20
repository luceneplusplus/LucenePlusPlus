/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef DUTCHSTEMMER_H
#define DUTCHSTEMMER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// A stemmer for Dutch words.
///
/// The algorithm is an implementation of the
/// <a href="http://snowball.tartarus.org/algorithms/dutch/stemmer.html">dutch stemming</a>
/// algorithm in Martin Porter's snowball project.
class LPPCONTRIBAPI DutchStemmer : public LuceneObject {
public:
    DutchStemmer();
    virtual ~DutchStemmer();

    LUCENE_CLASS(DutchStemmer);

protected:
    /// Buffer for the terms while stemming them.
    String buffer;

    bool removedE;
    MapStringString stemDict;

    int32_t R1;
    int32_t R2;

public:
    /// Stems the given term to a unique discriminator.
    ///
    /// @param term The term that should be stemmed.
    /// @return Discriminator for term.
    String stem(const String& term);

    void setStemDictionary(MapStringString dict);

protected:
    bool enEnding();

    void step1();

    /// Delete suffix e if in R1 and preceded by a non-vowel, and then undouble the ending.
    void step2();

    /// Delete "heid"
    void step3a();

    /// A d-suffix, or derivational suffix, enables a new word, often with a different grammatical
    /// category, or with a different sense, to be built from another word. Whether a d-suffix can
    /// be attached is discovered not from the rules of grammar, but by referring to a dictionary.
    /// So in English, ness can be added to certain adjectives to form corresponding nouns
    /// (littleness, kindness, foolishness ...) but not to all adjectives (not for example, to big,
    /// cruel, wise ...) d-suffixes can be used to change meaning, often in rather exotic ways.
    /// Remove "ing", "end", "ig", "lijk", "baar" and "bar"
    void step3b();

    /// Undouble vowel.  If the words ends CVD, where C is a non-vowel, D is a non-vowel other than
    /// I, and V is double a, e, o or u, remove one of the vowels from V (for example, maan -> man,
    /// brood -> brod).
    void step4();

    /// Checks if a term could be stemmed.
    bool isStemmable();

    /// Substitute ä, ë, ï, ö, ü, á , é, í, ó, ú
    void substitute();

    bool isValidSEnding(int32_t index);
    bool isValidEnEnding(int32_t index);

    void unDouble();
    void unDouble(int32_t endIndex);

    int32_t getRIndex(int32_t start);

    void storeYandI();
    void reStoreYandI();

    bool isVowel(wchar_t c);
};

}

#endif
