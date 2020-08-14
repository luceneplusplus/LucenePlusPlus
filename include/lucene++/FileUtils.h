/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include "Lucene.h"

namespace Lucene {

namespace FileUtils {

/// Return true if given file or directory exists.
LPPAPI bool fileExists(const String& path);

/// Return file last modified date and time.
LPPAPI uint64_t fileModified(const String& path);

/// Set file last modified date and time to now.
LPPAPI bool touchFile(const String& path);

/// Return file length in bytes.
LPPAPI int64_t fileLength(const String& path);

/// Set new file length, truncating or expanding as required.
LPPAPI bool setFileLength(const String& path, int64_t length);

/// Delete file from file system.
LPPAPI bool removeFile(const String& path);

/// Copy a file to/from file system.
LPPAPI bool copyFile(const String& source, const String& dest);

/// Create new directory under given location.
LPPAPI bool createDirectory(const String& path);

/// Delete directory from file system.
LPPAPI bool removeDirectory(const String& path);

/// Return true if given path points to a directory.
LPPAPI bool isDirectory(const String& path);

/// Return list of files (and/or directories) under given directory.
/// @param path path to list directory.
/// @param filesOnly if true the exclude sub-directories.
/// @param dirList list of files to return.
LPPAPI bool listDirectory(const String& path, bool filesOnly, HashSet<String> dirList);

/// Copy a directory to/from file system.
LPPAPI bool copyDirectory(const String& source, const String& dest);

/// Return complete path after joining given directory and file name.
LPPAPI String joinPath(const String& path, const String& file);

/// Extract parent path from given path.
LPPAPI String extractPath(const String& path);

/// Extract file name from given path.
LPPAPI String extractFile(const String& path);
}

}

#endif
