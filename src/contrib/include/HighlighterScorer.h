/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef HIGHLIGHTERSCORER_H
#define HIGHLIGHTERSCORER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// A HighlighterScorer is responsible for scoring a stream of tokens.  These token scores
/// can then be used to compute {@link TextFragment} scores.
class LPPCONTRIBAPI HighlighterScorer {
public:
    virtual ~HighlighterScorer();
    LUCENE_INTERFACE(HighlighterScorer);

public:
    /// Called to init the Scorer with a {@link TokenStream}. You can grab references to the
    /// attributes you are interested in here and access them from {@link #getTokenScore()}.
    ///
    /// @param tokenStream the {@link TokenStream} that will be scored.
    /// @return either a {@link TokenStream} that the Highlighter should continue using (eg
    /// if you read the tokenSream in this method) or null to continue using the same {@link
    /// TokenStream} that was passed in.
    virtual TokenStreamPtr init(const TokenStreamPtr& tokenStream);

    /// Called when a new fragment is started for consideration.
    ///
    /// @param newFragment the fragment that will be scored next
    virtual void startFragment(const TextFragmentPtr& newFragment);

    /// Called for each token in the current fragment. The {@link Highlighter} will increment
    /// the {@link TokenStream} passed to init on every call.
    ///
    /// @return a score which is passed to the {@link Highlighter} class to influence the
    /// mark-up of the text (this return value is NOT used to score the fragment)
    virtual double getTokenScore();

    /// Called when the {@link Highlighter} has no more tokens for the current fragment - the
    /// Scorer returns the weighting it has derived for the most recent fragment, typically
    /// based on the results of {@link #getTokenScore()}.
    virtual double getFragmentScore();
};

}

#endif
