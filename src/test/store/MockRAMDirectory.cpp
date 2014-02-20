/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "MockRAMDirectory.h"
#include "RAMFile.h"
#include "MockRAMOutputStream.h"
#include "MockRAMInputStream.h"
#include "TestPoint.h"
#include "Random.h"
#include "MiscUtils.h"

namespace Lucene {

MockRAMDirectory::MockRAMDirectory() {
    maxSize = 0;
    maxUsedSize = 0;
    randomIOExceptionRate = 0;
    noDeleteOpenFile = true;
    preventDoubleWrite = true;
    crashed = false;
    init();
}

MockRAMDirectory::MockRAMDirectory(const DirectoryPtr& dir) : RAMDirectory(dir) {
    maxSize = 0;
    maxUsedSize = 0;
    randomIOExceptionRate = 0;
    noDeleteOpenFile = true;
    preventDoubleWrite = true;
    crashed = false;
    init();
}

MockRAMDirectory::~MockRAMDirectory() {
}

void MockRAMDirectory::init() {
    SyncLock syncLock(this);
    if (!openFiles) {
        openFiles = MapStringInt::newInstance();
        openFilesDeleted = HashSet<String>::newInstance();
    }
    if (!createdFiles) {
        createdFiles = HashSet<String>::newInstance();
    }
    if (!unSyncedFiles) {
        unSyncedFiles = HashSet<String>::newInstance();
    }
}

void MockRAMDirectory::setPreventDoubleWrite(bool value) {
    preventDoubleWrite = value;
}

void MockRAMDirectory::sync(const String& name) {
    TestScope testScope(L"MockRAMDirectory", L"sync");
    SyncLock syncLock(this);
    maybeThrowDeterministicException();
    if (crashed) {
        boost::throw_exception(IOException(L"cannot sync after crash"));
    }
    unSyncedFiles.remove(name);
}

void MockRAMDirectory::crash() {
    SyncLock syncLock(this);
    crashed = true;
    openFiles = MapStringInt::newInstance();
    openFilesDeleted = HashSet<String>::newInstance();
    HashSet<String> crashFiles(unSyncedFiles);
    unSyncedFiles.clear();
    int32_t count = 0;
    for (HashSet<String>::iterator it = crashFiles.begin(); it != crashFiles.end(); ++it) {
        RAMFilePtr file(fileMap.get(*it));
        if (count % 3 == 0) {
            deleteFile(*it, true);
        } else if (count % 3 == 1) {
            // Zero out file entirely
            int32_t numBuffers = file->numBuffers();
            for (int32_t i = 0; i < numBuffers; ++i) {
                MiscUtils::arrayFill(file->getBuffer(i).get(), 0, file->getBuffer(i).size(), 0);
            }
        } else if (count % 3 == 2) {
            // Truncate the file
            file->setLength(file->getLength() / 2);
        }
        ++count;
    }
}

void MockRAMDirectory::clearCrash() {
    SyncLock syncLock(this);
    crashed = false;
}

void MockRAMDirectory::setMaxSizeInBytes(int64_t maxSize) {
    this->maxSize = maxSize;
}

int64_t MockRAMDirectory::getMaxSizeInBytes() {
    return maxSize;
}

int64_t MockRAMDirectory::getMaxUsedSizeInBytes() {
    return maxUsedSize;
}

void MockRAMDirectory::resetMaxUsedSizeInBytes() {
    maxUsedSize = getRecomputedActualSizeInBytes();
}

void MockRAMDirectory::setNoDeleteOpenFile(bool value) {
    noDeleteOpenFile = value;
}

bool MockRAMDirectory::getNoDeleteOpenFile() {
    return noDeleteOpenFile;
}

void MockRAMDirectory::setRandomIOExceptionRate(double rate, int64_t seed) {
    randomIOExceptionRate = rate;
    // seed so we have deterministic behaviour
    randomState = newLucene<Random>(seed);
}

double MockRAMDirectory::getRandomIOExceptionRate() {
    return randomIOExceptionRate;
}

void MockRAMDirectory::maybeThrowIOException() {
    if (randomIOExceptionRate > 0.0) {
        int32_t number = std::abs(randomState->nextInt() % 1000);
        if (number < randomIOExceptionRate * 1000) {
            boost::throw_exception(IOException(L"a random IO exception"));
        }
    }
}

void MockRAMDirectory::deleteFile(const String& name) {
    deleteFile(name, false);
}

void MockRAMDirectory::deleteFile(const String& name, bool forced) {
    TestScope testScope(L"MockRAMDirectory", L"deleteFile");
    SyncLock syncLock(this);
    maybeThrowDeterministicException();

    if (crashed && !forced) {
        boost::throw_exception(IOException(L"cannot delete after crash"));
    }

    unSyncedFiles.remove(name);

    if (!forced && noDeleteOpenFile) {
        if (openFiles.contains(name)) {
            openFilesDeleted.add(name);
            boost::throw_exception(IOException(L"MockRAMDirectory: file \"" + name + L"\" is still open: cannot delete"));
        } else {
            openFilesDeleted.remove(name);
        }
    }

    RAMDirectory::deleteFile(name);
}

HashSet<String> MockRAMDirectory::getOpenDeletedFiles() {
    SyncLock syncLock(this);
    HashSet<String> openFilesDeleted = HashSet<String>::newInstance(this->openFilesDeleted.begin(), this->openFilesDeleted.end());
    return openFilesDeleted;
}

IndexOutputPtr MockRAMDirectory::createOutput(const String& name) {
    SyncLock syncLock(this);
    if (crashed) {
        boost::throw_exception(IOException(L"cannot createOutput after crash"));
    }
    init();
    if (preventDoubleWrite && createdFiles.contains(name) && name != L"segments.gen") {
        boost::throw_exception(IOException(L"file \"" + name + L"\" was already written to"));
    }
    if (noDeleteOpenFile && openFiles.contains(name)) {
        boost::throw_exception(IOException(L"MockRAMDirectory: file \"" + name + L"\" is still open: cannot overwrite"));
    }
    RAMFilePtr file(newLucene<RAMFile>(shared_from_this()));
    if (crashed) {
        boost::throw_exception(IOException(L"cannot createOutput after crash"));
    }
    unSyncedFiles.add(name);
    createdFiles.add(name);
    RAMFilePtr existing(fileMap.get(name));
    // Enforce write once
    if (existing && name != L"segments.gen" && preventDoubleWrite) {
        boost::throw_exception(IOException(L"file " + name + L" already exists"));
    } else {
        if (existing) {
            _sizeInBytes -= existing->getSizeInBytes();
            existing->_directory.reset();
        }
        fileMap.put(name, file);
    }

    return newLucene<MockRAMOutputStream>(shared_from_this(), file, name);
}

IndexInputPtr MockRAMDirectory::openInput(const String& name) {
    SyncLock syncLock(this);
    MapStringRAMFile::iterator file = fileMap.find(name);
    if (file == fileMap.end()) {
        boost::throw_exception(FileNotFoundException(name));
    } else {
        MapStringInt::iterator openFile = openFiles.find(name);
        if (openFile != openFiles.end()) {
            ++openFile->second;
        } else {
            openFiles.put(name, 1);
        }
    }
    return newLucene<MockRAMInputStream>(shared_from_this(), name, file->second);
}

int64_t MockRAMDirectory::getRecomputedSizeInBytes() {
    SyncLock syncLock(this);
    int64_t size = 0;
    for (MapStringRAMFile::iterator file = fileMap.begin(); file != fileMap.end(); ++file) {
        size += file->second->getSizeInBytes();
    }
    return size;
}

int64_t MockRAMDirectory::getRecomputedActualSizeInBytes() {
    SyncLock syncLock(this);
    int64_t size = 0;
    for (MapStringRAMFile::iterator file = fileMap.begin(); file != fileMap.end(); ++file) {
        size += file->second->length;
    }
    return size;
}

void MockRAMDirectory::close() {
    SyncLock syncLock(this);
    if (!openFiles) {
        openFiles = MapStringInt::newInstance();
        openFilesDeleted = HashSet<String>::newInstance();
    }
    if (noDeleteOpenFile && !openFiles.empty()) {
        // RuntimeException instead of IOException because RAMDirectory does not throw IOException currently
        boost::throw_exception(RuntimeException(L"MockRAMDirectory: cannot close: there are still open files"));
    }
}

void MockRAMDirectory::failOn(const MockDirectoryFailurePtr& fail) {
    SyncLock syncLock(this);
    if (!failures) {
        failures = Collection<MockDirectoryFailurePtr>::newInstance();
    }
    failures.add(fail);
}

void MockRAMDirectory::maybeThrowDeterministicException() {
    SyncLock syncLock(this);
    if (failures) {
        for (Collection<MockDirectoryFailurePtr>::iterator failure = failures.begin(); failure != failures.end(); ++failure) {
            (*failure)->eval(shared_from_this());
        }
    }
}

MockDirectoryFailure::MockDirectoryFailure() {
    doFail = false;
}

MockDirectoryFailure::~MockDirectoryFailure() {
}

void MockDirectoryFailure::eval(const MockRAMDirectoryPtr& dir) {
}

MockDirectoryFailurePtr MockDirectoryFailure::reset() {
    return shared_from_this();
}

void MockDirectoryFailure::setDoFail() {
    doFail = true;
}

void MockDirectoryFailure::clearDoFail() {
    doFail = false;
}

}
