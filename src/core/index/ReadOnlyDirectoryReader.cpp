/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "ReadOnlyDirectoryReader.h"
#include "ReadOnlySegmentReader.h"

namespace Lucene
{
    ReadOnlyDirectoryReader::ReadOnlyDirectoryReader(DirectoryPtr directory, SegmentInfosPtr sis, IndexDeletionPolicyPtr deletionPolicy, 
                                                     int32_t termInfosIndexDivisor, SetReaderFinishedListener readerFinishedListeners) : 
        DirectoryReader(directory, sis, deletionPolicy, true, termInfosIndexDivisor, readerFinishedListeners)
    {
    }
    
    ReadOnlyDirectoryReader::ReadOnlyDirectoryReader(DirectoryPtr directory, SegmentInfosPtr infos, Collection<SegmentReaderPtr> oldReaders, 
                                                     Collection<int32_t> oldStarts, MapStringByteArray oldNormsCache, bool doClone, 
                                                     int32_t termInfosIndexDivisor, SetReaderFinishedListener readerFinishedListeners) :
        DirectoryReader(directory, infos, oldReaders, oldStarts, oldNormsCache, true, doClone, termInfosIndexDivisor, readerFinishedListeners)
    {
    }
    
    ReadOnlyDirectoryReader::ReadOnlyDirectoryReader(IndexWriterPtr writer, SegmentInfosPtr infos, int32_t termInfosIndexDivisor, bool applyAllDeletes) :
        DirectoryReader(writer, infos, termInfosIndexDivisor, applyAllDeletes)
    {
    }
    
    ReadOnlyDirectoryReader::~ReadOnlyDirectoryReader()
    {
    }
    
    void ReadOnlyDirectoryReader::acquireWriteLock()
    {
        ReadOnlySegmentReader::noWrite();
    }
}
