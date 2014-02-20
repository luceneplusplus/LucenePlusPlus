/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FRENCHSTEMMER_H
#define FRENCHSTEMMER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// A stemmer for French words.
///
/// The algorithm is based on the work of Dr Martin Porter on his snowball project refer to
/// http://snowball.sourceforge.net/french/stemmer.html (French stemming algorithm) for details.
class LPPCONTRIBAPI FrenchStemmer : public LuceneObject {
public:
    FrenchStemmer();
    virtual ~FrenchStemmer();

    LUCENE_CLASS(FrenchStemmer);

protected:
    /// Buffer for the terms while stemming them.
    String stringBuffer;

    /// A temporary buffer, used to reconstruct R2.
    String tempBuffer;

    /// Region R0 is equal to the whole buffer.
    String R0;

    /// Region RV
    ///
    /// "If the word begins with two vowels, RV is the region after the third letter, otherwise
    /// the region after the first vowel not at the beginning of the word, or the end of the
    /// word if these positions cannot be found."
    String RV;

    /// Region R1
    ///
    /// "R1 is the region after the first non-vowel following a vowel or is the null region at
    /// the end of the word if there is no such non-vowel"
    String R1;

    /// Region R2
    ///
    /// "R2 is the region after the first non-vowel in R1 following a vowel or is the null region
    /// at the end of the word if there is no such non-vowel"
    String R2;

    /// Set to true if we need to perform step 2
    bool suite;

    /// Set to true if the buffer was modified
    bool modified;

public:
    /// Stems the given term to a unique discriminator.
    ///
    /// @param term The term that should be stemmed.
    /// @return Discriminator for term.
    String stem(const String& term);

protected:
    /// Sets the search region Strings it needs to be done each time the buffer was modified.
    void setStrings();

    /// First step of the Porter Algorithm.
    /// Refer to http://snowball.sourceforge.net/french/stemmer.html for an explanation.
    void step1();

    /// Second step (A) of the Porter Algorithm.
    /// Will be performed if nothing changed from the first step or changed were done in the amment,
    /// emment, ments or ment suffixes.
    /// Refer to http://snowball.sourceforge.net/french/stemmer.html for an explanation.
    /// @return true if something changed in the buffer
    bool step2a();

    /// Second step (B) of the Porter Algorithm.
    /// Will be performed if step 2 A was performed unsuccessfully.
    /// Refer to http://snowball.sourceforge.net/french/stemmer.html for an explanation.
    void step2b();

    /// Third step of the Porter Algorithm.
    /// Refer to http://snowball.sourceforge.net/french/stemmer.html for an explanation.
    void step3();

    /// Fourth step of the Porter Algorithm.
    /// Refer to http://snowball.sourceforge.net/french/stemmer.html for an explanation.
    void step4();

    /// Fifth step of the Porter Algorithm.
    /// Refer to http://snowball.sourceforge.net/french/stemmer.html for an explanation.
    void step5();

    /// Sixth step of the Porter Algorithm.
    /// Refer to http://snowball.sourceforge.net/french/stemmer.html for an explanation.
    void step6();

    /// Delete a suffix searched in zone "source" if zone "from" contains prefix + search string.
    /// @param source String - the primary source zone for search.
    /// @param search String[] - the strings to search for suppression.
    /// @param from String - the secondary source zone for search.
    /// @param prefix String - the prefix to add to the search string to test.
    /// @return true if modified
    bool deleteFromIfPrecededIn(const String& source, Collection<String> search, const String& from, const String& prefix);

    /// Delete a suffix searched in zone "source" if the preceding letter is (or isn't) a vowel.
    /// @param source String - the primary source zone for search.
    /// @param search String[] - the strings to search for suppression.
    /// @param vowel boolean - true if we need a vowel before the search string.
    /// @param from String - the secondary source zone for search (where vowel could be).
    /// @return true if modified
    bool deleteFromIfTestVowelBeforeIn(const String& source, Collection<String> search, bool vowel, const String& from);

    /// Delete a suffix searched in zone "source" if preceded by the prefix.
    /// @param source String - the primary source zone for search.
    /// @param search String[] - the strings to search for suppression.
    /// @param prefix String - the prefix to add to the search string to test.
    /// @param without boolean - true if it will be deleted even without prefix found.
    void deleteButSuffixFrom(const String& source, Collection<String> search, const String& prefix, bool without);

    /// Delete a suffix searched in zone "source" if preceded by prefix or replace it with the
    /// replace string if preceded by the prefix in the zone "from" or delete the suffix if specified.
    /// @param source String - the primary source zone for search.
    /// @param search String[] - the strings to search for suppression.
    /// @param prefix String - the prefix to add to the search string to test.
    /// @param without boolean - true if it will be deleted even without prefix found.
    void deleteButSuffixFromElseReplace(const String& source, Collection<String> search, const String& prefix, bool without, const String& from, const String& replace);

    /// Replace a search string with another within the source zone.
    /// @param source String - the source zone for search.
    /// @param search String[] - the strings to search for replacement.
    /// @param replace String - the replacement string.
    bool replaceFrom(const String& source, Collection<String> search, const String& replace);

    /// Delete a search string within the source zone.
    /// @param source the source zone for search.
    /// @param suffix the strings to search for suppression.
    void deleteFrom(const String& source, Collection<String> suffix);

    /// Test if a char is a French vowel, including accentuated ones.
    /// @param ch the char to test.
    /// @return true if the char is a vowel
    bool isVowel(wchar_t ch);

    /// Retrieve the "R zone" (1 or 2 depending on the buffer) and return the corresponding string.
    /// "R is the region after the first non-vowel following a vowel or is the null region at the
    /// end of the word if there is no such non-vowel".
    /// @param buffer the in buffer.
    /// @return the resulting string.
    String retrieveR(const String& buffer);

    /// Retrieve the "RV zone" from a buffer an return the corresponding string.
    /// "If the word begins with two vowels, RV is the region after the third letter, otherwise the
    /// region after the first vowel not at the beginning of the word, or the end of the word if
    /// these positions cannot be found."
    /// @param buffer the in buffer
    /// @return the resulting string
    String retrieveRV(const String& buffer);

    /// Turns u and i preceded AND followed by a vowel to UpperCase<.
    /// Turns y preceded OR followed by a vowel to UpperCase.
    /// Turns u preceded by q to UpperCase.
    /// @param buffer the buffer to treat
    void treatVowels(String& buffer);

    /// Checks a term if it can be processed correctly.
    /// @return boolean - true if, and only if, the given term consists in letters.
    bool isStemmable(const String& term);
};

}

#endif
