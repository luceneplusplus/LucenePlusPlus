/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _FSDIRECTORY_H
#define _FSDIRECTORY_H

#include "BufferedIndexOutput.h"

namespace Lucene
{
    class FSIndexOutput : public BufferedIndexOutput
    {
    public:
        FSIndexOutput(FSDirectoryPtr parent, const String& name);
        virtual ~FSIndexOutput();
        
        LUCENE_CLASS(FSIndexOutput);

    private:
        FSDirectoryWeakPtr _parent;
        String name;
        OutputFilePtr file;
        bool isOpen; // remember if the file is open, so that we don't try to close it more than once
    
    public:
        virtual void flushBuffer(const uint8_t* b, int32_t offset, int32_t length);
        virtual void close();
        virtual void seek(int64_t pos);
        virtual int64_t length();
        virtual void setLength(int64_t length);
        
        friend class FSDirectory;
    };
}

#endif
