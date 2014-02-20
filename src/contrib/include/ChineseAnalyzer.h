/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CHINESEANALYZER_H
#define CHINESEANALYZER_H

#include "LuceneContrib.h"
#include "Analyzer.h"

namespace Lucene {

/// An {@link Analyzer} that tokenizes text with {@link ChineseTokenizer} and filters with {@link ChineseFilter}
class LPPCONTRIBAPI ChineseAnalyzer : public Analyzer {
public:
    virtual ~ChineseAnalyzer();

    LUCENE_CLASS(ChineseAnalyzer);

public:
    /// Creates a {@link TokenStream} which tokenizes all the text in the provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from {@link ChineseTokenizer}, filtered with {@link ChineseFilter}
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

    /// Returns a (possibly reused) {@link TokenStream} which tokenizes all the text  in the
    /// provided {@link Reader}.
    ///
    /// @return A {@link TokenStream} built from {@link ChineseTokenizer}, filtered with {@link ChineseFilter}
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

class LPPCONTRIBAPI ChineseAnalyzerSavedStreams : public LuceneObject {
public:
    virtual ~ChineseAnalyzerSavedStreams();

    LUCENE_CLASS(ChineseAnalyzerSavedStreams);

public:
    TokenizerPtr source;
    TokenStreamPtr result;
};

}

#endif
