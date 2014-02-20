/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FUNCTIONFIXTURE_H
#define FUNCTIONFIXTURE_H

#include "LuceneTestFixture.h"

namespace Lucene {

class FunctionFixture : public LuceneTestFixture {
public:
    FunctionFixture(bool doMultiSegment);
    virtual ~FunctionFixture();

public:
    static const double TEST_SCORE_TOLERANCE_DELTA;

public:
    static const int32_t N_DOCS;

    static const String ID_FIELD;
    static const String TEXT_FIELD;
    static const String INT_FIELD;
    static const String DOUBLE_FIELD;

    bool doMultiSegment;
    DirectoryPtr dir;
    AnalyzerPtr anlzr;

protected:
    static const Collection<String> DOC_TEXT_LINES();

    void addDoc(const IndexWriterPtr& iw, int32_t i);
    String id2String(int32_t scoreAndID);
    String textLine(int32_t docNum);

    double expectedFieldScore(const String& docIDFieldVal);

    bool equalCollectionValues(CollectionValue first, CollectionValue second);
};

}

#endif
