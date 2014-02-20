/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef RUSSIANSTEMMER_H
#define RUSSIANSTEMMER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Russian stemming algorithm implementation (see http://snowball.sourceforge.net for
/// detailed description).
class LPPCONTRIBAPI RussianStemmer : public LuceneObject {
public:
    RussianStemmer();
    virtual ~RussianStemmer();

    LUCENE_CLASS(RussianStemmer);

protected:
    /// positions of RV, R1 and R2 respectively
    int32_t RV;
    int32_t R1;
    int32_t R2;

    static const wchar_t A;
    static const wchar_t V;
    static const wchar_t G;
    static const wchar_t E;
    static const wchar_t I;
    static const wchar_t I_;
    static const wchar_t L;
    static const wchar_t M;
    static const wchar_t N;
    static const wchar_t O;
    static const wchar_t S;
    static const wchar_t T;
    static const wchar_t U;
    static const wchar_t X;
    static const wchar_t SH;
    static const wchar_t SHCH;
    static const wchar_t Y;
    static const wchar_t SOFT;
    static const wchar_t AE;
    static const wchar_t IU;
    static const wchar_t IA;

    /// stem definitions
    static const wchar_t vowels[];

    Collection<String> perfectiveGerundEndings1();
    Collection<String> perfectiveGerund1Predessors();
    Collection<String> perfectiveGerundEndings2();
    Collection<String> adjectiveEndings();
    Collection<String> participleEndings1();
    Collection<String> participleEndings2();
    Collection<String> participle1Predessors();
    Collection<String> reflexiveEndings();
    Collection<String> verbEndings1();
    Collection<String> verbEndings2();
    Collection<String> verb1Predessors();
    Collection<String> nounEndings();
    Collection<String> superlativeEndings();
    Collection<String> derivationalEndings();
    Collection<String> doubleN();

public:
    /// Finds the stem for given Russian word.
    String stem(const String& input);

    /// Static method for stemming.
    static String stemWord(const String& word);

protected:
    /// Adjectival ending is an adjective ending, optionally preceded by participle ending.
    bool adjectival(String& stemmingZone);

    /// Derivational endings
    bool derivational(String& stemmingZone);

    /// Finds ending among given ending class and returns the length of ending found(0, if not found).
    int32_t findEnding(String& stemmingZone, int32_t startIndex, Collection<String> theEndingClass);
    int32_t findEnding(String& stemmingZone, Collection<String> theEndingClass);

    /// Finds the ending among the given class of endings and removes it from stemming zone.
    bool findAndRemoveEnding(String& stemmingZone, Collection<String> theEndingClass);

    /// Finds the ending among the given class of endings, then checks if this ending was
    /// preceded by any of given predecessors, and if so, removes it from stemming zone.
    bool findAndRemoveEnding(String& stemmingZone, Collection<String> theEndingClass, Collection<String> thePredessors);

    /// Marks positions of RV, R1 and R2 in a given word.
    void markPositions(const String& word);

    /// Checks if character is a vowel.
    bool isVowel(wchar_t letter);

    /// Noun endings.
    bool noun(String& stemmingZone);

    /// Perfective gerund endings.
    bool perfectiveGerund(String& stemmingZone);

    /// Reflexive endings.
    bool reflexive(String& stemmingZone);

    bool removeI(String& stemmingZone);
    bool removeSoft(String& stemmingZone);

    /// Superlative endings.
    bool superlative(String& stemmingZone);

    /// Undoubles N.
    bool undoubleN(String& stemmingZone);

    /// Verb endings.
    bool verb(String& stemmingZone);
};

}

#endif
