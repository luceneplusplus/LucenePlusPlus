/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MMAPDIRECTORY_H
#define MMAPDIRECTORY_H

#include "FSDirectory.h"
#include "IndexInput.h"

namespace Lucene
{
    /// File-based {@link Directory} implementation that uses mmap for reading, and {@link SimpleFSIndexOutput} for writing.
    ///
    /// NOTE: memory mapping uses up a portion of the virtual memory address space in your process equal to the size of the 
    /// file being mapped.  Before using this class, be sure your have plenty of virtual address space.
    ///
    /// NOTE: Accessing this class either directly or indirectly from a thread while it's interrupted can close the 
    /// underlying channel immediately if at the same time the thread is blocked on IO.  The channel will remain closed and 
    /// subsequent access to {@link MMapDirectory} will throw an exception. 
    class LPPAPI MMapDirectory : public FSDirectory
    {
    public:
        /// Create a new MMapDirectory for the named location.
        /// @param path the path of the directory.
        /// @param lockFactory the lock factory to use, or null for the default ({@link NativeFSLockFactory})
        MMapDirectory(const String& path, LockFactoryPtr lockFactory = LockFactoryPtr());
        
        virtual ~MMapDirectory();
        
        LUCENE_CLASS(MMapDirectory);
            
    public:
        using FSDirectory::openInput;
        
        /// Creates an IndexInput for the file with the given name.
        virtual IndexInputPtr openInput(const String& name, int32_t bufferSize);
        
        /// Creates an IndexOutput for the file with the given name.
        virtual IndexOutputPtr createOutput(const String& name);
    };
    
    class LPPAPI MMapIndexInput : public IndexInput
    {
    public:
        MMapIndexInput(const String& path = L"");
        virtual ~MMapIndexInput();
        
        LUCENE_CLASS(MMapIndexInput);
                
    protected:
        int32_t _length;
        bool isClone;
        boost::iostreams::mapped_file_source file;
        int32_t bufferPosition; // next byte to read
    
    public:
        /// Reads and returns a single byte.
        /// @see IndexOutput#writeByte(uint8_t)
        virtual uint8_t readByte();
        
        /// Reads a specified number of bytes into an array at the specified offset.
        /// @param b the array to read bytes into.
        /// @param offset the offset in the array to start storing bytes.
        /// @param length the number of bytes to read.
        /// @see IndexOutput#writeBytes(const uint8_t*,int)
        virtual void readBytes(uint8_t* b, int32_t offset, int32_t length);
        
        /// Returns the current position in this file, where the next read will occur.
        /// @see #seek(int64_t)
        virtual int64_t getFilePointer();
        
        /// Sets current position in this file, where the next read will occur.
        /// @see #getFilePointer()
        virtual void seek(int64_t pos);
        
        /// The number of bytes in the file.
        virtual int64_t length();
        
        /// Closes the stream to further operations.
        virtual void close();
        
        /// Returns a clone of this stream.
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
    };
}

#endif
