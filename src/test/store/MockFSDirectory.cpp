/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "MockFSDirectory.h"
#include "NoLockFactory.h"
#include "SimpleFSDirectory.h"
#include "BufferedIndexInput.h"
#include "Random.h"

namespace Lucene {

MockFSDirectory::MockFSDirectory(const String& path) {
    allIndexInputs = Collection<IndexInputPtr>::newInstance();
    lockFactory = newLucene<NoLockFactory>();
    dir = newLucene<SimpleFSDirectory>(path);
    rand = newLucene<Random>();
}

MockFSDirectory::~MockFSDirectory() {
}

IndexInputPtr MockFSDirectory::openInput(const String& name) {
    return openInput(name, BufferedIndexInput::BUFFER_SIZE);
}

void MockFSDirectory::tweakBufferSizes() {
    for (Collection<IndexInputPtr>::iterator ii = allIndexInputs.begin(); ii != allIndexInputs.end(); ++ii) {
        BufferedIndexInputPtr bii(boost::dynamic_pointer_cast<BufferedIndexInput>(*ii));
        int32_t bufferSize = 1024 + (int32_t)std::abs(rand->nextInt() % 32768);
        bii->setBufferSize(bufferSize);
    }
}

IndexInputPtr MockFSDirectory::openInput(const String& name, int32_t bufferSize) {
    // Make random changes to buffer size
    bufferSize = 1 + (int32_t)std::abs(rand->nextInt() % 10);
    IndexInputPtr f(dir->openInput(name, bufferSize));
    allIndexInputs.add(f);
    return f;
}

IndexOutputPtr MockFSDirectory::createOutput(const String& name) {
    return dir->createOutput(name);
}

void MockFSDirectory::close() {
    dir->close();
}

void MockFSDirectory::deleteFile(const String& name) {
    dir->deleteFile(name);
}

void MockFSDirectory::touchFile(const String& name) {
    dir->touchFile(name);
}

uint64_t MockFSDirectory::fileModified(const String& name) {
    return dir->fileModified(name);
}

bool MockFSDirectory::fileExists(const String& name) {
    return dir->fileExists(name);
}

HashSet<String> MockFSDirectory::listAll() {
    return dir->listAll();
}

int64_t MockFSDirectory::fileLength(const String& name) {
    return dir->fileLength(name);
}

}
