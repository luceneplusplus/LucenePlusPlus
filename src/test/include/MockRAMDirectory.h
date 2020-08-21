/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MOCKRAMDIRECTORY_H
#define MOCKRAMDIRECTORY_H

#include "test_lucene.h"
#include "RAMDirectory.h"

namespace Lucene {

/// This is a subclass of RAMDirectory that adds methods intended to be used only by unit tests.
class MockRAMDirectory : public RAMDirectory {
public:
    MockRAMDirectory();
    MockRAMDirectory(const DirectoryPtr& dir);
    virtual ~MockRAMDirectory();

    LUCENE_CLASS(MockRAMDirectory);

public:
    int64_t maxSize;
    RandomPtr randomState;

    // Max actual bytes used. This is set by MockRAMOutputStream
    int64_t maxUsedSize;
    double randomIOExceptionRate;
    bool noDeleteOpenFile;
    bool preventDoubleWrite;
    bool crashed;

    MapStringInt openFiles;

    // Only tracked if noDeleteOpenFile is true: if an attempt is made to delete an
    // open file, we enroll it here.
    HashSet<String> openFilesDeleted;

    Collection<MockDirectoryFailurePtr> failures;

protected:
    HashSet<String> unSyncedFiles;
    HashSet<String> createdFiles;

public:
    /// If set to true, we throw an IO exception if the same file is opened by createOutput, ever.
    void setPreventDoubleWrite(bool value);

    virtual void sync(const String& name);

    /// Simulates a crash of OS or machine by overwriting unsynced files.
    void crash();

    void clearCrash();
    void setMaxSizeInBytes(int64_t maxSize);
    int64_t getMaxSizeInBytes();

    /// Returns the peek actual storage used (bytes) in this directory.
    int64_t getMaxUsedSizeInBytes();
    void resetMaxUsedSizeInBytes();

    /// Emulate windows whereby deleting an open file is not allowed (raise IO exception)
    void setNoDeleteOpenFile(bool value);
    bool getNoDeleteOpenFile();

    /// If 0.0, no exceptions will be thrown.  Else this should be a double 0.0 - 1.0.  We will randomly throw an
    /// IO exception on the first write to an OutputStream based on this probability.
    void setRandomIOExceptionRate(double rate, int64_t seed);
    double getRandomIOExceptionRate();

    void maybeThrowIOException();

    virtual void deleteFile(const String& name);

    virtual HashSet<String> getOpenDeletedFiles();

    virtual IndexOutputPtr createOutput(const String& name);
    virtual IndexInputPtr openInput(const String& name);

    /// Provided for testing purposes.  Use sizeInBytes() instead.
    int64_t getRecomputedSizeInBytes();

    /// Like getRecomputedSizeInBytes(), but, uses actual file lengths rather than buffer allocations (which are
    /// quantized up to nearest RAMOutputStream::BUFFER_SIZE (now 1024) bytes.
    int64_t getRecomputedActualSizeInBytes();

    virtual void close();

    /// Add a Failure object to the list of objects to be evaluated at every potential failure point
    void failOn(const MockDirectoryFailurePtr& fail);

    /// Iterate through the failures list, giving each object a chance to throw an IO exception.
    void maybeThrowDeterministicException();

protected:
    void init();

    void deleteFile(const String& name, bool forced);
};

/// Objects that represent fail-able conditions. Objects of a derived class are created and registered with the
/// mock directory. After register, each object will be invoked once for each first write of a file, giving the
/// object a chance to throw an IO exception.
class MockDirectoryFailure : public LuceneObject {
public:
    MockDirectoryFailure();
    virtual ~MockDirectoryFailure();

    LUCENE_CLASS(MockDirectoryFailure);

public:
    /// eval is called on the first write of every new file.
    virtual void eval(const MockRAMDirectoryPtr& dir);

    /// reset should set the state of the failure to its default (freshly constructed) state. Reset is convenient
    /// for tests that want to create one failure object and then reuse it in multiple cases. This, combined with
    /// the fact that MockDirectoryFailure subclasses are often anonymous classes makes reset difficult to do otherwise.
    virtual MockDirectoryFailurePtr reset();

    virtual void setDoFail();
    virtual void clearDoFail();

protected:
    bool doFail;
};

}

#endif
