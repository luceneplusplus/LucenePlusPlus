/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef RAMFILE_H
#define RAMFILE_H

#include "LuceneObject.h"

namespace Lucene
{
    /// File used as buffer in RAMDirectory
    class LPPAPI RAMFile : public LuceneObject
    {
    public:
        RAMFile(); // File used as buffer, in no RAMDirectory
        RAMFile(RAMDirectoryPtr directory);
        virtual ~RAMFile();

        LUCENE_CLASS(RAMFile);

    INTERNAL:
        int64_t length;
        RAMDirectoryPtr directory;

    protected:
        Collection<ByteArray> buffers;

        int64_t sizeInBytes;

        /// This is publicly modifiable via Directory.touchFile(), so direct access not supported
        int64_t lastModified;

    protected:
        virtual void mark_members(gc* gc) const
        {
            gc->mark(directory);
            gc->mark(buffers);
            LuceneObject::mark_members(gc);
        }

    public:
        /// For non-stream access from thread that might be concurrent with writing.
        int64_t getLength();
        void setLength(int64_t length);

        /// For non-stream access from thread that might be concurrent with writing
        int64_t getLastModified();
        void setLastModified(int64_t lastModified);

        int64_t getSizeInBytes();

        ByteArray addBuffer(int32_t size);
        ByteArray getBuffer(int32_t index);
        int32_t numBuffers();

    protected:
        /// Allocate a new buffer.  Subclasses can allocate differently.
        virtual ByteArray newBuffer(int32_t size);
    };
}

#endif
