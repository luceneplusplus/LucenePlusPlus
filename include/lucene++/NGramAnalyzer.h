/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NGRAMANALYZER_H
#define NGRAMANALYZER_H

#include "Analyzer.h"

namespace Lucene {

/// An Analyzer that uses {@link NGramTokenizer}.
class LPPAPI NGramAnalyzer : public Analyzer {
public:
    NGramAnalyzer();
    NGramAnalyzer(int32_t minGram, int32_t maxGram);
    virtual ~NGramAnalyzer();

    LUCENE_CLASS(NGramAnalyzer);

protected:
    int32_t minGram;
    int32_t maxGram;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);
    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};

}

#endif
