/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>

#endif

#include "TestUtils.h"
#include "MiscUtils.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "LuceneGlobalFixture.h"

#include <gtest/gtest.h>

#include <boost/algorithm/string.hpp>

using namespace Lucene;

int main(int argc, char* argv[]) {
    String testDir;

    for (int32_t i = 0; i < argc; ++i) {
        if (strncmp(argv[i], "--test_dir", 9) == 0) {
            String testParam = StringUtils::toUnicode(argv[i]);
            Collection<String> vals = StringUtils::split(testParam, L"=");
            if (vals.size() == 2) {
                testDir = vals[1];
                boost::replace_all(testDir, L"\"", L"");
                boost::trim(testDir);
                break;
            }
        }
    }

    if (testDir.empty()) {
        testDir = L"../../src/test/testfiles";
        if (!FileUtils::isDirectory(testDir)) {
            testDir = L"../src/test/testfiles";
            if (!FileUtils::isDirectory(testDir)) {
                testDir = L"./src/test/testfiles";
            }
        }
    }

    if (!FileUtils::isDirectory(testDir)) {
        std::wcout << L"Test directory not found. (override default by using --test_dir=\"./src/test/testfiles\")\n";
        return 1;
    }

    setTestDir(testDir);

    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new LuceneGlobalFixture());

    return RUN_ALL_TESTS();
}
