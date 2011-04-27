/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include <fstream>
#include "SimpleFSDirectory.h"
#include "_SimpleFSDirectory.h"
#include "IndexOutput.h"
#include "FileReader.h"
#include "FileUtils.h"
#include "StringUtils.h"

namespace Lucene
{
    SimpleFSDirectory::SimpleFSDirectory(const String& path, LockFactoryPtr lockFactory) : FSDirectory(path, lockFactory)
    {
    }
    
    SimpleFSDirectory::~SimpleFSDirectory()
    {
    }
    
    IndexInputPtr SimpleFSDirectory::openInput(const String& name)
    {
        return FSDirectory::openInput(name);
    }
    
    IndexInputPtr SimpleFSDirectory::openInput(const String& name, int32_t bufferSize)
    {
        ensureOpen();
        return newLucene<SimpleFSIndexInput>(FileUtils::joinPath(directory, name), bufferSize, getReadChunkSize());
    }
    
    const int32_t InputFile::FILE_EOF = FileReader::FILE_EOF;
    const int32_t InputFile::FILE_ERROR = FileReader::FILE_ERROR;
    
    InputFile::InputFile(const String& path)
    {
        file = newInstance<std::ifstream>(StringUtils::toUTF8(path).c_str(), std::ios::binary | std::ios::in);
        if (!file->is_open())
            boost::throw_exception(FileNotFoundException(path));
        position = 0;
        length = FileUtils::fileLength(path);
    }
    
    InputFile::~InputFile()
    {
    }
    
    void InputFile::setPosition(int64_t position)
    {
        this->position = position;
        file->seekg((std::streamoff)position);
        if (!file->good())
            boost::throw_exception(IOException());
    }
    
    int64_t InputFile::getPosition()
    {
        return position;
    }
    
    int64_t InputFile::getLength()
    {
        return length;
    }
    
    int32_t InputFile::read(uint8_t* b, int32_t offset, int32_t length)
    {
        try
        {
            if (file->eof())
                return FILE_EOF;
            file->read((char*)b + offset, length);
            int32_t readCount = file->gcount();
            position += readCount;
            return readCount;
        }
        catch (...)
        {
            return FILE_ERROR;
        }
    }
    
    void InputFile::close()
    {
        if (file->is_open())
            file->close();
    }
    
    bool InputFile::isValid()
    {
        return (file && file->is_open() && file->good());
    }
    
    SimpleFSIndexInput::SimpleFSIndexInput()
    {
        this->chunkSize = 0;
        this->isClone = false;
    }
    
    SimpleFSIndexInput::SimpleFSIndexInput(const String& path, int32_t bufferSize, int32_t chunkSize) : BufferedIndexInput(bufferSize)
    {
        this->file = newLucene<InputFile>(path);
        this->path = path;
        this->chunkSize = chunkSize;
        this->isClone = false;
    }
    
    SimpleFSIndexInput::~SimpleFSIndexInput()
    {
    }
    
    void SimpleFSIndexInput::readInternal(uint8_t* b, int32_t offset, int32_t length)
    {
        SyncLock fileLock(file);
        
        int64_t position = getFilePointer();
        if (position != file->getPosition())
            file->setPosition(position);
        
        int32_t total = 0;
        
        while (total < length)
        {
            int32_t readLength = total + chunkSize > length ? length - total : chunkSize;
            
            int32_t i = file->read(b, offset + total, readLength);
            if (i == InputFile::FILE_EOF)
                boost::throw_exception(IOException(L"Read past EOF"));
            total += i;
        }
    }
    
    void SimpleFSIndexInput::seekInternal(int64_t pos)
    {
    }
    
    int64_t SimpleFSIndexInput::length()
    {
        return file->getLength();
    }
    
    void SimpleFSIndexInput::close()
    {
        if (!isClone)
            file->close();
    }
    
    bool SimpleFSIndexInput::isValid()
    {
        return file->isValid();
    }
    
    void SimpleFSIndexInput::copyBytes(IndexOutputPtr out, int64_t numBytes)
    {
        numBytes -= flushBuffer(out, numBytes);
        // If out is FSIndexOutput, the copy will be optimized
        out->copyBytes(shared_from_this(), numBytes);
    }
    
    LuceneObjectPtr SimpleFSIndexInput::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = BufferedIndexInput::clone(other ? other : newLucene<SimpleFSIndexInput>());
        SimpleFSIndexInputPtr cloneIndexInput(boost::static_pointer_cast<SimpleFSIndexInput>(clone));
        cloneIndexInput->path = path;
        cloneIndexInput->file = file;
        cloneIndexInput->chunkSize = chunkSize;
        cloneIndexInput->isClone = true;
        return cloneIndexInput;
    }
    
    OutputFile::OutputFile(const String& path)
    {
        this->path = path;
        file = newInstance<std::ofstream>(StringUtils::toUTF8(path).c_str(), std::ios::binary | std::ios::out);
    }
    
    OutputFile::~OutputFile()
    {
    }
    
    bool OutputFile::write(const uint8_t* b, int32_t offset, int32_t length)
    {
        if (!file->is_open())
            return false;
        try
        {
            file->write((char*)b + offset, length);
            return file->good();
        }
        catch (...)
        {
            return false;
        }
    }
    
    void OutputFile::close()
    {
        file.reset();
    }
    
    void OutputFile::setPosition(int64_t position)
    {
        file->seekp((std::streamoff)position);
        if (!file->good())
            boost::throw_exception(IOException());
    }
    
    int64_t OutputFile::getLength()
    {
        return FileUtils::fileLength(path);
    }
    
    void OutputFile::setLength(int64_t length)
    {
        FileUtils::setFileLength(path, length);
    }
    
    void OutputFile::flush()
    {
        if (file->is_open())
            file->flush();
    }
    
    bool OutputFile::isValid()
    {
        return (file && file->is_open() && file->good());
    }
}
