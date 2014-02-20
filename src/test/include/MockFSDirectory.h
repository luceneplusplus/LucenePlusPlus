/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MOCKFSDIRECTORY_H
#define MOCKFSDIRECTORY_H

#include "test_lucene.h"
#include "Directory.h"

namespace Lucene {

class MockFSDirectory : public Directory {
public:
    MockFSDirectory(const String& path);
    virtual ~MockFSDirectory();

    LUCENE_CLASS(MockFSDirectory);

public:
    Collection<IndexInputPtr> allIndexInputs;

protected:
    DirectoryPtr dir;
    RandomPtr rand;

public:
    virtual IndexInputPtr openInput(const String& name);
    virtual IndexInputPtr openInput(const String& name, int32_t bufferSize);

    void tweakBufferSizes();

    virtual IndexOutputPtr createOutput(const String& name);
    virtual void close();
    virtual void deleteFile(const String& name);
    virtual void touchFile(const String& name);
    virtual uint64_t fileModified(const String& name);
    virtual bool fileExists(const String& name);
    virtual HashSet<String> listAll();
    virtual int64_t fileLength(const String& name);
};

}

#endif
