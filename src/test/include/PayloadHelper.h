/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef PAYLOADHELPER_H
#define PAYLOADHELPER_H

#include "test_lucene.h"

namespace Lucene {

class PayloadHelper {
public:
    virtual ~PayloadHelper();

public:
    static const String NO_PAYLOAD_FIELD;
    static const String MULTI_FIELD;
    static const String FIELD;

public:
    static const ByteArray payloadField();
    static const ByteArray payloadMultiField1();
    static const ByteArray payloadMultiField2();

    /// Sets up a RAMDirectory, and adds documents (using intToEnglish()) with two fields: field and multiField
    /// and analyzes them using the PayloadHelperAnalyzer
    static IndexSearcherPtr setUp(const SimilarityPtr& similarity, int32_t numDocs);
};

}

#endif
