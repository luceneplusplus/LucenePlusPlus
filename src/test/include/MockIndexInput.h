/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MOCKINDEXINPUT_H
#define MOCKINDEXINPUT_H

#include "test_lucene.h"
#include "BufferedIndexInput.h"

namespace Lucene {

class MockIndexInput : public BufferedIndexInput {
public:
    MockIndexInput(ByteArray bytes);
    virtual ~MockIndexInput();

    LUCENE_CLASS(MockIndexInput);

protected:
    ByteArray buffer;
    int32_t pointer;
    int64_t _length;

public:
    virtual void close();
    virtual int64_t length();

protected:
    virtual void readInternal(uint8_t* b, int32_t offset, int32_t length);
    virtual void seekInternal(int64_t pos);
};

}

#endif
