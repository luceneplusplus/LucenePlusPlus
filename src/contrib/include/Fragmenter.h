/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FRAGMENTER_H
#define FRAGMENTER_H

#include "LuceneContrib.h"
#include "LuceneObject.h"

namespace Lucene {

/// Implements the policy for breaking text into multiple fragments for consideration by the
/// {@link Highlighter} class. A sophisticated implementation may do this on the basis of
/// detecting end of sentences in the text.
class LPPCONTRIBAPI Fragmenter {
public:
    virtual ~Fragmenter();
    LUCENE_INTERFACE(Fragmenter);

public:
    /// Initializes the Fragmenter. You can grab references to the Attributes you are
    /// interested in from tokenStream and then access the values in {@link #isNewFragment()}.
    /// @param originalText the original source text.
    /// @param tokenStream the {@link TokenStream} to be fragmented.
    virtual void start(const String& originalText, const TokenStreamPtr& tokenStream);

    /// Test to see if this token from the stream should be held in a new TextFragment.
    /// Every time this is called, the TokenStream passed to start(String, TokenStream)
    /// will have been incremented.
    virtual bool isNewFragment();
};

}

#endif
