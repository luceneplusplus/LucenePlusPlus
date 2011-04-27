/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include <fstream>
#include "FSDirectory.h"
#include "_FSDirectory.h"
#include "NativeFSLockFactory.h"
#include "SimpleFSDirectory.h"
#include "_SimpleFSDirectory.h"
#include "MMapDirectory.h"
#include "BufferedIndexInput.h"
#include "LuceneThread.h"
#include "FileUtils.h"
#include "MiscUtils.h"
#include "StringUtils.h"

extern "C"
{
#include "../util/md5/md5.h"
}

namespace Lucene
{
    /// Default read chunk size.  This is a conditional default based on operating system.
    #ifdef LPP_BUILD_64
    const int32_t FSDirectory::DEFAULT_READ_CHUNK_SIZE = INT_MAX;
    #else
    const int32_t FSDirectory::DEFAULT_READ_CHUNK_SIZE = 100 * 1024 * 1024; // 100mb
    #endif

    FSDirectory::FSDirectory(const String& path, LockFactoryPtr lockFactory)
    {
        staleFiles = HashSet<String>::newInstance();
        chunkSize = DEFAULT_READ_CHUNK_SIZE;
        
        // new ctors use always NativeFSLockFactory as default
        if (!lockFactory)
            lockFactory = newLucene<NativeFSLockFactory>();
        directory = path;
        
        if (FileUtils::fileExists(directory) && !FileUtils::isDirectory(directory))
            boost::throw_exception(NoSuchDirectoryException(L"File '" + directory + L"' exists but is not a directory"));
        
        setLockFactory(lockFactory);
    }
    
    FSDirectory::~FSDirectory()
    {
    }
    
    FSDirectoryPtr FSDirectory::open(const String& path)
    {
        return open(path, LockFactoryPtr());
    }
    
    FSDirectoryPtr FSDirectory::open(const String& path, LockFactoryPtr lockFactory)
    {
        #if defined(_WIN32) && defined(LPP_BUILD_64)
        return newLucene<MMapDirectory>(path, lockFactory);
        #else
        return newLucene<SimpleFSDirectory>(path, lockFactory);
        #endif
    }
    
    void FSDirectory::setLockFactory(LockFactoryPtr lockFactory)
    {
        Directory::setLockFactory(lockFactory);
        
        // for filesystem based LockFactory, delete the lockPrefix, if the locks are placed
        // in index dir. If no index dir is given, set ourselves
        if (MiscUtils::typeOf<FSLockFactory>(lockFactory))
        {
            FSLockFactoryPtr lf(boost::static_pointer_cast<FSLockFactory>(lockFactory));
            String dir(lf->getLockDir());
            // if the lock factory has no lockDir set, use the this directory as lockDir
            if (dir.empty())
            {
                lf->setLockDir(directory);
                lf->setLockPrefix(L"");
            }
            else if (dir == directory)
                lf->setLockPrefix(L"");
        }
    }
    
    HashSet<String> FSDirectory::listAll(const String& dir)
    {
        if (!FileUtils::fileExists(dir))
            boost::throw_exception(NoSuchDirectoryException(L"Directory '" + dir + L"' does not exist"));
        else if (!FileUtils::isDirectory(dir))
            boost::throw_exception(NoSuchDirectoryException(L"File '" + dir + L"' exists but is not a directory"));
        
        HashSet<String> result(HashSet<String>::newInstance());
        
        // Exclude subdirs
        if (!FileUtils::listDirectory(dir, true, result))
            boost::throw_exception(IOException(L"Directory '" + dir + L"' exists and is a directory, but cannot be listed"));
        
        return result;
    }
    
    HashSet<String> FSDirectory::listAll()
    {
        ensureOpen();
        return listAll(directory);
    }
    
    bool FSDirectory::fileExists(const String& name)
    {
        ensureOpen();
        return FileUtils::fileExists(FileUtils::joinPath(directory, name));
    }
    
    uint64_t FSDirectory::fileModified(const String& name)
    {
        ensureOpen();
        return FileUtils::fileModified(FileUtils::joinPath(directory, name));
    }
    
    uint64_t FSDirectory::fileModified(const String& directory, const String& name)
    {
        return FileUtils::fileModified(FileUtils::joinPath(directory, name));
    }
    
    void FSDirectory::touchFile(const String& name)
    {
        ensureOpen();
        FileUtils::touchFile(FileUtils::joinPath(directory, name));
    }
    
    void FSDirectory::deleteFile(const String& name)
    {
        ensureOpen();
        if (!FileUtils::removeFile(FileUtils::joinPath(directory, name)))
            boost::throw_exception(IOException(L"Cannot delete: " + name));
        SyncLock staleLock(&staleFiles);
        staleFiles.remove(name);
    }
    
    int64_t FSDirectory::fileLength(const String& name)
    {
        ensureOpen();
        int64_t len = FileUtils::fileLength(FileUtils::joinPath(directory, name));
        if (len == 0 && !FileUtils::fileExists(FileUtils::joinPath(directory, name)))
            boost::throw_exception(FileNotFoundException(name));
        return len;
    }
    
    IndexOutputPtr FSDirectory::createOutput(const String& name)
    {
        ensureOpen();
        ensureCanWrite(name);
        return newLucene<FSIndexOutput>(shared_from_this(), name);
    }
    
    void FSDirectory::ensureCanWrite(const String& name)
    {
        if (!FileUtils::fileExists(directory))
        {
            if (!FileUtils::createDirectory(directory))
                boost::throw_exception(IOException(L"Cannot create directory: " + directory));
        }
        String path(FileUtils::joinPath(directory, name));
        if (FileUtils::fileExists(path) && !FileUtils::removeFile(path)) // delete existing, if any
            boost::throw_exception(IOException(L"Cannot overwrite: " + path));
    }
    
    void FSDirectory::onIndexOutputClosed(FSIndexOutputPtr io)
    {
        SyncLock staleLock(&staleFiles);
        staleFiles.add(io->name);
    }
    
    void FSDirectory::sync(const String& name)
    {
        HashSet<String> names(HashSet<String>::newInstance());
        names.add(name);
        sync(names);
    }
    
    void FSDirectory::sync(HashSet<String> names)
    {
        ensureOpen();
        HashSet<String> uniqueNames(HashSet<String>::newInstance(names.begin(), names.end()));
        HashSet<String> toSync(HashSet<String>::newInstance());
        
        {
            SyncLock staleLock(&staleFiles);
            for (HashSet<String>::iterator unique = uniqueNames.begin(); unique != uniqueNames.end(); ++unique)
            {
                if (staleFiles.contains(*unique))
                    toSync.add(*unique);
            }
        }
        
        for (HashSet<String>::iterator name = toSync.begin(); name != toSync.end(); ++name)
            fsync(*name);

        {
            SyncLock staleLock(&staleFiles);
            for (HashSet<String>::iterator name = toSync.begin(); name != toSync.end(); ++name)
                staleFiles.remove(*name);
        }
    }
    
    IndexInputPtr FSDirectory::openInput(const String& name)
    {
        ensureOpen();
        return openInput(name, BufferedIndexInput::BUFFER_SIZE);
    }
    
    IndexInputPtr FSDirectory::openInput(const String& name, int32_t bufferSize)
    {
        return Directory::openInput(name, bufferSize);
    }
    
    String FSDirectory::getLockID()
    {
        ensureOpen();
        md5_state_t state;
        md5_byte_t digest[16];
        
        md5_init(&state);
        md5_append(&state, (const md5_byte_t *)StringUtils::toUTF8(directory).c_str(), directory.size());
        md5_finish(&state, digest);
        
        static const wchar_t* hexDigits = L"0123456789abcdef";
        
        String lockID(L"lucene-");
        for (int32_t i = 0; i < 16; ++i)
        {
            lockID += hexDigits[(digest[i] >> 4) & 0x0f];
            lockID += hexDigits[digest[i] & 0x0f];
        }
        
        return lockID;
    }
    
    void FSDirectory::close()
    {
        SyncLock syncLock(this);
        isOpen = false;
    }
    
    String FSDirectory::toString()
    {
        return getClassName() + L"@" + directory + L" lockFactory=" + getLockFactory()->toString();
    }
    
    String FSDirectory::getFile()
    {
        return getDirectory();
    }
    
    String FSDirectory::getDirectory()
    {
        ensureOpen();
        return directory;
    }
    
    void FSDirectory::setReadChunkSize(int32_t chunkSize)
    {
        #ifndef LPP_BUILD_64
        this->chunkSize = chunkSize;
        #endif
    }
    
    int32_t FSDirectory::getReadChunkSize()
    {
        return chunkSize;
    }
    
    void FSDirectory::fsync(const String& name)
    {
        String path(FileUtils::joinPath(directory, name));
        bool success = false;
        
        for (int32_t retryCount = 0; retryCount < 5; ++retryCount)
        {
            std::ofstream syncFile;
            try
            {
                syncFile.open(StringUtils::toUTF8(path).c_str(), std::ios::binary | std::ios::in | std::ios::out);
            }
            catch (...)
            {
            }
            
            if (syncFile.is_open())
            {
                syncFile.close();
                success = true;
                break;
            }
            
            LuceneThread::threadSleep(5); // pause 5 msec
        }

        if (!success)
            boost::throw_exception(IOException(L"Sync failure: " + path));
    }
    
    FSIndexOutput::FSIndexOutput(FSDirectoryPtr parent, const String& name)
    {
        this->_parent = parent;
        this->name = name;
        this->file = newLucene<OutputFile>(FileUtils::joinPath(parent->directory, name));
        this->isOpen = true;
    }
    
    FSIndexOutput::~FSIndexOutput()
    {
    }
    
    void FSIndexOutput::flushBuffer(const uint8_t* b, int32_t offset, int32_t length)
    {
        file->write(b, offset, length);
    }
    
    void FSIndexOutput::close()
    {
        // only close the file if it has not been closed yet
        if (isOpen)
        {
            bool success = false;
            LuceneException finally;
            try
            {
                BufferedIndexOutput::close();
                success = true;
            }
            catch (LuceneException& e)
            {
                finally = e;
            }
            isOpen = false;
            if (!success)
            {
                try
                {
                    file->close();
                    FSDirectoryPtr(_parent)->onIndexOutputClosed(shared_from_this());
                }
                catch (...)
                {
                    // Suppress so we don't mask original exception
                }
            }
            else
                file->close();
            finally.throwException();
        }
    }
    
    void FSIndexOutput::seek(int64_t pos)
    {
        BufferedIndexOutput::seek(pos);
        file->setPosition(pos);
    }
    
    int64_t FSIndexOutput::length()
    {
        return file->getLength();
    }
    
    void FSIndexOutput::setLength(int64_t length)
    {
        file->setLength(length);
    }
}
