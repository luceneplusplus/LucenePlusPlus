/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "RAMDirectory.h"
#include "RAMFile.h"
#include "RAMInputStream.h"
#include "RAMOutputStream.h"
#include "SingleInstanceLockFactory.h"
#include "LuceneThread.h"
#include "MiscUtils.h"
#include "AtomicLong.h"
#include "IndexFileNameFilter.h"

namespace Lucene
{
    RAMDirectory::RAMDirectory()
    {
        this->fileMap = MapStringRAMFile::newInstance();
        this->_sizeInBytes = newLucene<AtomicLong>();
        this->copyDirectory = false;
        this->closeDir = false;
        try
        {
            setLockFactory(newLucene<SingleInstanceLockFactory>());
        }
        catch (IOException&)
        { // Cannot happen
        }
    }
    
    RAMDirectory::RAMDirectory(DirectoryPtr dir)
    {
        this->fileMap = MapStringRAMFile::newInstance();
        this->_sizeInBytes = newLucene<AtomicLong>();
        this->copyDirectory = true;
        this->_dirSource = dir;
        this->closeDir = false;
        try
        {
            setLockFactory(newLucene<SingleInstanceLockFactory>());
        }
        catch (IOException&)
        { // Cannot happen
        }
    }
    
    RAMDirectory::RAMDirectory(DirectoryPtr dir, bool closeDir)
    {
        this->fileMap = MapStringRAMFile::newInstance();
        this->_sizeInBytes = newLucene<AtomicLong>();
        this->copyDirectory = true;
        this->_dirSource = dir;
        this->closeDir = closeDir;
        try
        {
            setLockFactory(newLucene<SingleInstanceLockFactory>());
        }
        catch (IOException&)
        { // Cannot happen
        }
    }
    
    RAMDirectory::~RAMDirectory()
    {
    }
    
    void RAMDirectory::initialize()
    {
        if (copyDirectory)
        {
            IndexFileNameFilterPtr filter(IndexFileNameFilter::getFilter());
            DirectoryPtr dir(_dirSource);
            HashSet<String> files(dir->listAll());
            for (HashSet<String>::iterator file = files.begin(); file != files.end(); ++file)
            {
                if (filter->accept(L"", *file))
                    dir->copy(shared_from_this(), *file, *file);
            }
            if (closeDir)
                dir->close();
        }
    }
    
    HashSet<String> RAMDirectory::listAll()
    {
        ensureOpen();
        HashSet<String> result(HashSet<String>::newInstance());
        {
            SyncLock fileLock(&fileMap);
            for (MapStringRAMFile::iterator fileName = fileMap.begin(); fileName != fileMap.end(); ++fileName)
                result.add(fileName->first);
        }
        return result;
    }
    
    bool RAMDirectory::fileExists(const String& name)
    {
        ensureOpen();
        SyncLock fileLock(&fileMap);
        return fileMap.contains(name);
    }
    
    uint64_t RAMDirectory::fileModified(const String& name)
    {
        ensureOpen();
        SyncLock fileLock(&fileMap);
        MapStringRAMFile::iterator ramFile = fileMap.find(name);
        if (ramFile == fileMap.end())
            boost::throw_exception(FileNotFoundException(name));
        return ramFile->second->getLastModified();
    }
    
    void RAMDirectory::touchFile(const String& name)
    {
        ensureOpen();
        RAMFilePtr file;
        {
            SyncLock fileLock(&fileMap);
            MapStringRAMFile::iterator ramFile = fileMap.find(name);
            if (ramFile == fileMap.end())
                boost::throw_exception(FileNotFoundException(name));
            file = ramFile->second;
        }
        int64_t ts1 = MiscUtils::currentTimeMillis();
        while (ts1 == MiscUtils::currentTimeMillis())
            LuceneThread::threadSleep(1);
        file->setLastModified(MiscUtils::currentTimeMillis());
    }
    
    int64_t RAMDirectory::fileLength(const String& name)
    {
        ensureOpen();
        SyncLock fileLock(&fileMap);
        MapStringRAMFile::iterator ramFile = fileMap.find(name);
        if (ramFile == fileMap.end())
            boost::throw_exception(FileNotFoundException(name));
        return ramFile->second->getLength();
    }
    
    int64_t RAMDirectory::sizeInBytes()
    {
        ensureOpen();
        SyncLock fileLock(&fileMap);
        return _sizeInBytes->get();
    }
    
    void RAMDirectory::deleteFile(const String& name)
    {
        ensureOpen();
        SyncLock fileLock(&fileMap);
        MapStringRAMFile::iterator ramFile = fileMap.find(name);
        if (ramFile == fileMap.end())
            boost::throw_exception(FileNotFoundException(name));
        _sizeInBytes->addAndGet(-ramFile->second->getSizeInBytes());
        fileMap.remove(name);        
    }
    
    IndexOutputPtr RAMDirectory::createOutput(const String& name)
    {
        ensureOpen();
        RAMFilePtr file(newRAMFile());
        {
            SyncLock fileLock(&fileMap);
            MapStringRAMFile::iterator existing = fileMap.find(name);
            if (existing != fileMap.end())
            {
                _sizeInBytes->addAndGet(-existing->second->getSizeInBytes());
                existing->second->_directory.reset();
            }
            fileMap.put(name, file);
        }
        return newLucene<RAMOutputStream>(file);
    }
    
    RAMFilePtr RAMDirectory::newRAMFile()
    {
        return newLucene<RAMFile>(shared_from_this());
    }
    
    IndexInputPtr RAMDirectory::openInput(const String& name)
    {
        ensureOpen();
        RAMFilePtr file;
        {
            SyncLock fileLock(&fileMap);
            MapStringRAMFile::iterator ramFile = fileMap.find(name);
            if (ramFile == fileMap.end())
                boost::throw_exception(FileNotFoundException(name));
            file = ramFile->second;
        }
        return newLucene<RAMInputStream>(file);
    }
    
    void RAMDirectory::close()
    {
        isOpen = false;
        fileMap.clear();
    }
}
