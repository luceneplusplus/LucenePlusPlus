/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TESTUTILS_H
#define TESTUTILS_H

#include "Lucene.h"

namespace Lucene {

/// Initialise unit test files directory
void setTestDir(const String& dir);

/// Return unit test files directory
String getTestDir();

/// Return temporary directory
String getTempDir();

/// Return temporary directory (randomly generated)
String getTempDir(const String& desc);

/// Wait for concurrent merge to finish
void syncConcurrentMerges(const IndexWriterPtr& writer);

/// Wait for concurrent merge to finish
void syncConcurrentMerges(const MergeSchedulerPtr& ms);

/// Return English representation of given integer
String intToEnglish(int32_t i);

/// Return English representation of given integer (recursive)
String _intToEnglish(int32_t i);

/// This runs the CheckIndex tool on the index in.
/// If any issues are hit, a RuntimeException is thrown; else, true is returned.
bool checkIndex(const DirectoryPtr& dir);
}

#endif
