/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuceneTestFixture.h"
#include "TestUtils.h"
#include "DocHelper.h"
#include "Document.h"
#include "FieldInfos.h"
#include "FieldInfo.h"
#include "RAMDirectory.h"
#include "IndexOutput.h"

using namespace Lucene;

class FieldInfosTestFixture : public LuceneTestFixture, public DocHelper
{
public:
    virtual ~FieldInfosTestFixture()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE(FieldInfosTest, FieldInfosTestFixture)

BOOST_AUTO_TEST_CASE(testFieldInfos)
{
    DocumentPtr testDoc = newLucene<Document>();
    DocHelper::setupDoc(testDoc);
    
    // Positive test of FieldInfos
    BOOST_CHECK(testDoc);
    FieldInfosPtr fieldInfos = newLucene<FieldInfos>();
    fieldInfos->add(testDoc);
    // Since the complement is stored as well in the fields map
    BOOST_CHECK(fieldInfos->size() == DocHelper::all.size()); // this is all because we are using the no-arg constructor
    RAMDirectoryPtr dir = newLucene<RAMDirectory>();
    String name = L"testFile";
    IndexOutputPtr output = dir->createOutput(name);
    BOOST_CHECK(output);
    // Use a RAMOutputStream

    fieldInfos->write(output);
    output->close();
    BOOST_CHECK(output->length() > 0);
    FieldInfosPtr readIn = newLucene<FieldInfos>(dir, name);
    BOOST_CHECK(fieldInfos->size() == readIn->size());
    FieldInfoPtr info = readIn->fieldInfo(L"textField1");
    BOOST_CHECK(info);
    BOOST_CHECK(!info->storeTermVector);
    BOOST_CHECK(!info->omitNorms);

    info = readIn->fieldInfo(L"textField2");
    BOOST_CHECK(info);
    BOOST_CHECK(info->storeTermVector);
    BOOST_CHECK(!info->omitNorms);

    info = readIn->fieldInfo(L"textField3");
    BOOST_CHECK(info);
    BOOST_CHECK(!info->storeTermVector);
    BOOST_CHECK(info->omitNorms);

    info = readIn->fieldInfo(L"omitNorms");
    BOOST_CHECK(info);
    BOOST_CHECK(!info->storeTermVector);
    BOOST_CHECK(info->omitNorms);

    dir->close();
}

BOOST_AUTO_TEST_SUITE_END()
