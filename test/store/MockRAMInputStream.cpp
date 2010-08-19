/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MockRAMInputStream.h"
#include "MockRAMDirectory.h"

namespace Lucene
{
    MockRAMInputStream::MockRAMInputStream()
    {
        this->isClone = false;
    }
    
    MockRAMInputStream::MockRAMInputStream(MockRAMDirectoryPtr dir, const String& name, RAMFilePtr f) : RAMInputStream(f)
    {
        this->isClone = false;
        this->name = name;
        this->_dir = dir;
    }
    
    MockRAMInputStream::~MockRAMInputStream()
    {
    }
    
    void MockRAMInputStream::close()
    {
        RAMInputStream::close();
        if (!isClone)
        {
            MockRAMDirectoryPtr dir(_dir);
            SyncLock dirLock(dir);
            MapStringInt::iterator openFile = dir->openFiles.find(name);
            // Could be null when MockRAMDirectory.crash() was called
            if (openFile != dir->openFiles.end())
            {
                if (openFile->second == 1)
                    dir->openFiles.remove(name);
                else
                    --openFile->second;
            }
        }
    }
    
    LuceneObjectPtr MockRAMInputStream::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = RAMInputStream::clone(other ? other : newLucene<MockRAMInputStream>());
        MockRAMInputStreamPtr cloneInputStream(boost::dynamic_pointer_cast<MockRAMInputStream>(clone));
        cloneInputStream->_dir = _dir;
        cloneInputStream->name = name;
        cloneInputStream->isClone = true;
        return cloneInputStream;
    }
}
