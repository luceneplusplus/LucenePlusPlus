/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BRAZILIANSTEMMER_H
#define BRAZILIANSTEMMER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// A stemmer for Brazilian Portuguese words.
class LPPCONTRIBAPI BrazilianStemmer : public LuceneObject {
public:
    virtual ~BrazilianStemmer();

    LUCENE_CLASS(BrazilianStemmer);

protected:
    String TERM;
    String CT;
    String R1;
    String R2;
    String RV;

public:
    /// Stems the given term to a unique discriminator.
    ///
    /// @param term The term that should be stemmed.
    /// @return Discriminator for term.
    String stem(const String& term);

protected:
    /// Checks a term if it can be processed correctly.
    /// @return true if, and only if, the given term consists in letters.
    bool isStemmable(const String& term);

    /// Checks a term if it can be processed indexed.
    /// @return true if it can be indexed
    bool isIndexable(const String& term);

    /// See if string is 'a','e','i','o','u'
    /// @return true if is vowel
    bool isVowel(wchar_t value);

    /// Gets R1.
    /// R1 - is the region after the first non-vowel following a vowel, or is the null region at the end of the
    /// word if there is no such non-vowel.
    /// @return null or a string representing R1
    String getR1(const String& value);

    /// Gets RV.
    /// RV - if the second letter is a consonant, RV is the region after the next following vowel,
    ///
    /// OR if the first two letters are vowels, RV is the region after the next consonant,
    ///
    /// AND otherwise (consonant-vowel case) RV is the region after the third letter.
    ///
    /// BUT RV is the end of the word if this positions cannot be found.
    /// @return null or a string representing RV
    String getRV(const String& value);

    /// 1) Turn to lowercase
    /// 2) Remove accents
    /// 3) ã -> a ; õ -> o
    /// 4) ç -> c
    /// @return null or a string transformed
    String changeTerm(const String& value);

    /// Check if a string ends with a suffix.
    /// @return true if the string ends with the specified suffix.
    bool checkSuffix(const String& value, const String& suffix);

    /// Replace a string suffix by another
    /// @return the replaced String
    String replaceSuffix(const String& value, const String& toReplace, const String& changeTo);

    /// Remove a string suffix.
    /// @return the String without the suffix;
    String removeSuffix(const String& value, const String& toRemove);

    /// See if a suffix is preceded by a String.
    /// @return true if the suffix is preceded.
    bool suffixPreceded(const String& value, const String& suffix, const String& preceded);

    /// Creates CT (changed term) , substituting * 'ã' and 'õ' for 'a~' and 'o~'.
    void createCT(const String& term);

    /// Standard suffix removal.
    /// @return false if no ending was removed
    bool step1();

    /// Verb suffixes.
    /// Search for the longest among the following suffixes in RV, and if found, delete.
    /// @return false if no ending was removed
    bool step2();

    /// Delete suffix 'i' if in RV and preceded by 'c'
    void step3();

    /// Residual suffix
    /// If the word ends with one of the suffixes (os a i o á í ó) in RV, delete it.
    void step4();

    /// If the word ends with one of (e é ê) in RV,delete it, and if preceded by 'gu' (or 'ci') with
    /// the 'u' (or 'i') in RV, delete the 'u' (or 'i')
    ///
    /// Or if the word ends ç remove the cedilha.
    void step5();
};

}

#endif
