/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef GERMANSTEMMER_H
#define GERMANSTEMMER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// A stemmer for German words.
///
/// The algorithm is based on the report "A Fast and Simple Stemming Algorithm for German Words" by J&ouml;rg
/// Caumanns (joerg.caumanns at isst.fhg.de).
class LPPCONTRIBAPI GermanStemmer : public LuceneObject {
public:
    GermanStemmer();
    virtual ~GermanStemmer();

    LUCENE_CLASS(GermanStemmer);

protected:
    /// Buffer for the terms while stemming them.
    String buffer;

    /// Amount of characters that are removed with substitute() while stemming.
    int32_t substCount;

public:
    /// Stems the given term to a unique discriminator.
    ///
    /// @param term The term that should be stemmed.
    /// @return Discriminator for term.
    String stem(const String& term);

protected:
    /// Checks if a term could be stemmed.
    /// @return true if, and only if, the given term consists in letters.
    bool isStemmable();

    /// Suffix stripping (stemming) on the current term. The stripping is reduced to the seven "base"
    /// suffixes "e", "s", "n", "t", "em", "er" and * "nd", from which all regular suffixes are build
    /// of. The simplification causes some overstemming, and way more irregular stems, but still
    /// provides unique.
    /// Discriminators in the most of those cases.
    /// The algorithm is context free, except of the length restrictions.
    void strip();

    /// Does some optimizations on the term. This optimisations are contextual.
    void optimize();

    /// Removes a particle denotion ("ge") from a term.
    void removeParticleDenotion();

    /// Do some substitutions for the term to reduce overstemming:
    ///
    /// - Substitute Umlauts with their corresponding vowel: äöü -> aou, "ß" is substituted by "ss"
    /// - Substitute a second char of a pair of equal characters with an asterisk: ?? -> ?*
    /// - Substitute some common character combinations with a token: sch/ch/ei/ie/ig/st -> $/§/%/&/#/!
    void substitute();

    /// Undoes the changes made by substitute(). That are character pairs and character combinations.
    /// Umlauts will remain as their corresponding vowel, as "ß" remains as "ss".
    void resubstitute();
};

}

#endif
