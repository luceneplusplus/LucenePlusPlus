/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MOCKRAMOUTPUTSTREAM_H
#define MOCKRAMOUTPUTSTREAM_H

#include "test_lucene.h"
#include "RAMOutputStream.h"

namespace Lucene {

/// Used by MockRAMDirectory to create an output stream that will throw an IOException on fake disk full, track max
/// disk space actually used, and maybe throw random IOExceptions.
class MockRAMOutputStream : public RAMOutputStream {
public:
    /// Construct an empty output buffer.
    MockRAMOutputStream(const MockRAMDirectoryPtr& dir, const RAMFilePtr& f, const String& name);
    virtual ~MockRAMOutputStream();

    LUCENE_CLASS(MockRAMOutputStream);

protected:
    MockRAMDirectoryWeakPtr _dir;
    bool first;
    String name;

public:
    ByteArray singleByte;

public:
    virtual void close();
    virtual void flush();
    virtual void writeByte(uint8_t b);
    virtual void writeBytes(const uint8_t* b, int32_t offset, int32_t length);
};

}

#endif
