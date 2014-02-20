/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "DocHelper.h"
#include "Document.h"
#include "FieldInfos.h"
#include "FieldInfo.h"
#include "RAMDirectory.h"
#include "IndexOutput.h"

using namespace Lucene;

class FieldInfosTest : public LuceneTestFixture, public DocHelper {
public:
    virtual ~FieldInfosTest() {
    }
};

TEST_F(FieldInfosTest, testFieldInfos) {
    DocumentPtr testDoc = newLucene<Document>();
    DocHelper::setupDoc(testDoc);

    // Positive test of FieldInfos
    EXPECT_TRUE(testDoc);
    FieldInfosPtr fieldInfos = newLucene<FieldInfos>();
    fieldInfos->add(testDoc);
    // Since the complement is stored as well in the fields map
    EXPECT_TRUE(fieldInfos->size() == DocHelper::all.size()); // this is all because we are using the no-arg constructor
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    String name = L"testFile";
    IndexOutputPtr output = dir->createOutput(name);
    EXPECT_TRUE(output);
    // Use a RAMOutputStream

    fieldInfos->write(output);
    output->close();
    EXPECT_TRUE(output->length() > 0);
    FieldInfosPtr readIn = newLucene<FieldInfos>(dir, name);
    EXPECT_TRUE(fieldInfos->size() == readIn->size());
    FieldInfoPtr info = readIn->fieldInfo(L"textField1");
    EXPECT_TRUE(info);
    EXPECT_TRUE(!info->storeTermVector);
    EXPECT_TRUE(!info->omitNorms);

    info = readIn->fieldInfo(L"textField2");
    EXPECT_TRUE(info);
    EXPECT_TRUE(info->storeTermVector);
    EXPECT_TRUE(!info->omitNorms);

    info = readIn->fieldInfo(L"textField3");
    EXPECT_TRUE(info);
    EXPECT_TRUE(!info->storeTermVector);
    EXPECT_TRUE(info->omitNorms);

    info = readIn->fieldInfo(L"omitNorms");
    EXPECT_TRUE(info);
    EXPECT_TRUE(!info->storeTermVector);
    EXPECT_TRUE(info->omitNorms);

    dir->close();
}
