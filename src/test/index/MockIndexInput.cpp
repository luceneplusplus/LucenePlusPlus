/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "TestInc.h"
#include "MockIndexInput.h"
#include "MiscUtils.h"

namespace Lucene {

MockIndexInput::MockIndexInput(ByteArray bytes) {
    buffer = bytes;
    _length = bytes.size();
    pointer = 0;
}

MockIndexInput::~MockIndexInput() {
}

void MockIndexInput::readInternal(uint8_t* b, int32_t offset, int32_t length) {
    int32_t remainder = length;
    int32_t start = pointer;
    while (remainder != 0) {
        int32_t bufferOffset = start % buffer.size();
        int32_t bytesInBuffer = buffer.size() - bufferOffset;
        int32_t bytesToCopy = bytesInBuffer >= remainder ? remainder : bytesInBuffer;
        MiscUtils::arrayCopy(buffer.get(), bufferOffset, b, offset, bytesToCopy);
        offset += bytesToCopy;
        start += bytesToCopy;
        remainder -= bytesToCopy;
    }
    pointer += length;
}

void MockIndexInput::close() {
    // ignore
}

void MockIndexInput::seekInternal(int64_t pos) {
    pointer = (int32_t)pos;
}

int64_t MockIndexInput::length() {
    return _length;
}

}
