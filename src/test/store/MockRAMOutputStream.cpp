/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "MockRAMOutputStream.h"
#include "MockRAMDirectory.h"

namespace Lucene {

MockRAMOutputStream::MockRAMOutputStream(const MockRAMDirectoryPtr& dir, const RAMFilePtr& f, const String& name) : RAMOutputStream(f) {
    this->first = true;
    this->singleByte = ByteArray::newInstance(1);
    this->_dir = dir;
    this->name = name;
}

MockRAMOutputStream::~MockRAMOutputStream() {
}

void MockRAMOutputStream::close() {
    RAMOutputStream::close();
    MockRAMDirectoryPtr dir(_dir);

    // Now compute actual disk usage & track the maxUsedSize in the MockRAMDirectory
    int64_t size = dir->getRecomputedActualSizeInBytes();
    if (size > dir->maxUsedSize) {
        dir->maxUsedSize = size;
    }
}

void MockRAMOutputStream::flush() {
    MockRAMDirectoryPtr(_dir)->maybeThrowDeterministicException();
    RAMOutputStream::flush();
}

void MockRAMOutputStream::writeByte(uint8_t b) {
    singleByte[0] = b;
    writeBytes(singleByte.get(), 0, 1);
}

void MockRAMOutputStream::writeBytes(const uint8_t* b, int32_t offset, int32_t length) {
    MockRAMDirectoryPtr dir(_dir);
    int64_t freeSpace = dir->maxSize - dir->sizeInBytes();
    int64_t realUsage = 0;

    // If MockRAMDir crashed since we were opened, then don't write anything
    if (dir->crashed) {
        boost::throw_exception(IOException(L"MockRAMDirectory was crashed; cannot write to " + name));
    }

    // Enforce disk full
    if (dir->maxSize != 0 && freeSpace <= length) {
        // Compute the real disk free.  This will greatly slow down our test but makes it more accurate
        realUsage = dir->getRecomputedActualSizeInBytes();
        freeSpace = dir->maxSize - realUsage;
    }

    if (dir->maxSize != 0 && freeSpace <= length) {
        if (freeSpace > 0 && freeSpace < length) {
            realUsage += freeSpace;
            RAMOutputStream::writeBytes(b, offset, (int32_t)freeSpace);
        }
        if (realUsage > dir->maxUsedSize) {
            dir->maxUsedSize = realUsage;
        }
        boost::throw_exception(IOException(L"fake disk full at " + StringUtils::toString(dir->getRecomputedActualSizeInBytes()) + L" bytes when writing " + name));
    } else {
        RAMOutputStream::writeBytes(b, offset, length);
    }

    dir->maybeThrowDeterministicException();

    if (first) {
        // Maybe throw random exception; only do this on first write to a new file
        first = false;
        dir->maybeThrowIOException();
    }
}

}
