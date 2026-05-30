/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef EDGENGRAMANALYZER_H
#define EDGENGRAMANALYZER_H

#include "Analyzer.h"

namespace Lucene {

/// An Analyzer that uses {@link EdgeNGramTokenizer}.
class LPPAPI EdgeNGramAnalyzer : public Analyzer {
public:
    EdgeNGramAnalyzer();
    EdgeNGramAnalyzer(int32_t minGram, int32_t maxGram);
    virtual ~EdgeNGramAnalyzer();

    LUCENE_CLASS(EdgeNGramAnalyzer);

protected:
    int32_t minGram;
    int32_t maxGram;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

}

#endif
