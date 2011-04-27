/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "CompoundFileReader.h"
#include "CompoundFileWriter.h"
#include "IndexFileNames.h"
#include "StringUtils.h"

namespace Lucene
{
    CompoundFileReader::CompoundFileReader(DirectoryPtr dir, const String& name)
    {
        ConstructReader(dir, name, BufferedIndexInput::BUFFER_SIZE);
    }
    
    CompoundFileReader::CompoundFileReader(DirectoryPtr dir, const String& name, int32_t readBufferSize)
    {
        ConstructReader(dir, name, readBufferSize);
    }
    
    CompoundFileReader::~CompoundFileReader()
    {
    }
    
    void CompoundFileReader::ConstructReader(DirectoryPtr dir, const String& name, int32_t readBufferSize)
    {
        directory = dir;
        fileName = name;
        this->readBufferSize = readBufferSize;
        this->entries = MapStringFileEntryPtr::newInstance();
        
        bool success = false;
        
        LuceneException finally;
        try
        {
            stream = dir->openInput(name, readBufferSize);
            
            // read the first VInt. If it is negative, it's the version number otherwise it's 
            // the count (pre-3.1 indexes)
            int32_t firstInt = stream->readVInt();

            int32_t count = 0;
            bool stripSegmentName = false;
            if (firstInt < CompoundFileWriter::FORMAT_PRE_VERSION)
            {
                if (firstInt < CompoundFileWriter::FORMAT_CURRENT)
                {
                    boost::throw_exception(CorruptIndexException(L"Incompatible format version: " + StringUtils::toString(firstInt) + 
                                                                 L" expected " + StringUtils::toString(CompoundFileWriter::FORMAT_CURRENT)));
                }
                // It's a post-3.1 index, read the count.
                count = stream->readVInt();
                stripSegmentName = false;
            }
            else
            {
                count = firstInt;
                stripSegmentName = true;
            }
            
            // read the directory and init files
            FileEntryPtr entry;
            for (int32_t i = 0; i < count; ++i)
            {
                int64_t offset = stream->readLong();
                String id(stream->readString());
                
                if (stripSegmentName)
                {
                    // Fix the id to not include the segment names. This is relevant for pre-3.1 indexes.
                    id = IndexFileNames::stripSegmentName(id);
                }

                if (entry)
                {
                    // set length of the previous entry
                    entry->length = offset - entry->offset;
                }
                
                entry = newInstance<FileEntry>();
                entry->offset = offset;
                entries.put(id, entry);
            }
            
            // set the length of the final entry
            if (entry)
                entry->length = stream->length() - entry->offset;
            
            success = true;
        }
        catch (LuceneException& e)
        {
            finally = e;
        }
        
        if (!success && stream)
        {
            try
            {
                stream->close();
            }
            catch (...)
            {
            }
        }
        
        finally.throwException();
    }
    
    DirectoryPtr CompoundFileReader::getDirectory()
    {
        return directory;
    }
    
    String CompoundFileReader::getName()
    {
        return fileName;
    }
    
    void CompoundFileReader::close()
    {
        SyncLock syncLock(this);
        if (!stream)
            boost::throw_exception(IOException(L"Already closed"));
        
        entries.clear();
        stream->close();
        stream.reset();
    }
    
    IndexInputPtr CompoundFileReader::openInput(const String& name)
    {
        SyncLock syncLock(this);
        // Default to readBufferSize passed in when we were opened
        return openInput(name, readBufferSize);
    }
    
    IndexInputPtr CompoundFileReader::openInput(const String& name, int32_t bufferSize)
    {
        SyncLock syncLock(this);
        if (!stream)
            boost::throw_exception(IOException(L"Stream closed"));
        
        String id(IndexFileNames::stripSegmentName(name));
        MapStringFileEntryPtr::iterator entry = entries.find(id);
        if (entry == entries.end())
            boost::throw_exception(IOException(L"No sub-file with id " + id + L" found"));
        
        return newLucene<CSIndexInput>(stream, entry->second->offset, entry->second->length, readBufferSize);
    }
    
    HashSet<String> CompoundFileReader::listAll()
    {
        HashSet<String> res(HashSet<String>::newInstance());
        // Add the segment name
        String seg(fileName.substr(0, fileName.find(L".")));
        for (MapStringFileEntryPtr::iterator entry = entries.begin(); entry != entries.end(); ++entry)
            res.add(seg + entry->first);
        return res;
    }
    
    bool CompoundFileReader::fileExists(const String& name)
    {
        return entries.contains(IndexFileNames::stripSegmentName(name));
    }
    
    uint64_t CompoundFileReader::fileModified(const String& name)
    {
        return directory->fileModified(fileName);
    }
    
    void CompoundFileReader::touchFile(const String& name)
    {
        directory->touchFile(fileName);
    }
    
    void CompoundFileReader::deleteFile(const String& name)
    {
        boost::throw_exception(UnsupportedOperationException());
    }
    
    void CompoundFileReader::renameFile(const String& from, const String& to)
    {
        boost::throw_exception(UnsupportedOperationException());
    }
    
    int64_t CompoundFileReader::fileLength(const String& name)
    {
        MapStringFileEntryPtr::iterator entry = entries.find(IndexFileNames::stripSegmentName(name));
        if (entry == entries.end())
            boost::throw_exception(FileNotFoundException(name));
        return entry->second->length;
    }
    
    IndexOutputPtr CompoundFileReader::createOutput(const String& name)
    {
        boost::throw_exception(UnsupportedOperationException());
        return IndexOutputPtr();
    }
    
    LockPtr CompoundFileReader::makeLock(const String& name)
    {
        boost::throw_exception(UnsupportedOperationException());
        return LockPtr();
    }
    
    CSIndexInput::CSIndexInput()
    {
        fileOffset = 0;
        _length = 0;
    }
    
    CSIndexInput::CSIndexInput(IndexInputPtr base, int64_t fileOffset, int64_t length) : BufferedIndexInput(BufferedIndexInput::BUFFER_SIZE)
    {
        this->base = boost::static_pointer_cast<IndexInput>(base->clone());
        this->fileOffset = fileOffset;
        this->_length = length;
    }
    
    CSIndexInput::CSIndexInput(IndexInputPtr base, int64_t fileOffset, int64_t length, int32_t readBufferSize) : BufferedIndexInput(readBufferSize)
    {
        this->base = boost::static_pointer_cast<IndexInput>(base->clone());
        this->fileOffset = fileOffset;
        this->_length = length;
    }
    
    CSIndexInput::~CSIndexInput()
    {
    }
    
    void CSIndexInput::readInternal(uint8_t* b, int32_t offset, int32_t length)
    {
        int64_t start = getFilePointer();
        if (start + length > _length)
            boost::throw_exception(IOException(L"read past EOF"));
        base->seek(fileOffset + start);
        base->readBytes(b, offset, length, false);
    }
    
    void CSIndexInput::seekInternal(int64_t pos)
    {
    }
    
    void CSIndexInput::close()
    {
        base->close();
    }
    
    int64_t CSIndexInput::length()
    {
        return _length;
    }
    
    LuceneObjectPtr CSIndexInput::clone(LuceneObjectPtr other)
    {
        LuceneObjectPtr clone = other ? other : newLucene<CSIndexInput>();
        CSIndexInputPtr cloneIndexInput(boost::static_pointer_cast<CSIndexInput>(BufferedIndexInput::clone(clone)));
        cloneIndexInput->base = boost::static_pointer_cast<IndexInput>(this->base->clone());
        cloneIndexInput->fileOffset = fileOffset;
        cloneIndexInput->_length = _length;
        return cloneIndexInput;
    }
    
    void CSIndexInput::copyBytes(IndexOutputPtr out, int64_t numBytes)
    {
        // Copy first whatever is in the buffer
        numBytes -= flushBuffer(out, numBytes);

        // If there are more bytes left to copy, delegate the copy task to the base IndexInput, 
        // in case it can do an optimized copy.
        if (numBytes > 0)
        {
            int64_t start = getFilePointer();
            if (start + numBytes > _length)
                boost::throw_exception(IOException(L"read past EOF"));
            base->seek(fileOffset + start);
            base->copyBytes(out, numBytes);
        }
    }
}
