/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef READONLYDIRECTORYREADER_H
#define READONLYDIRECTORYREADER_H

#include "DirectoryReader.h"

namespace Lucene
{
    class LPPAPI ReadOnlyDirectoryReader : public DirectoryReader
    {
    public:
        ReadOnlyDirectoryReader(DirectoryPtr directory, SegmentInfosPtr sis, IndexDeletionPolicyPtr deletionPolicy, int32_t termInfosIndexDivisor);
        ReadOnlyDirectoryReader(DirectoryPtr directory, SegmentInfosPtr infos, Collection<SegmentReaderPtr> oldReaders, 
                                Collection<int32_t> oldStarts, MapStringByteArray oldNormsCache, bool doClone, int32_t termInfosIndexDivisor);
        ReadOnlyDirectoryReader(IndexWriterPtr writer, SegmentInfosPtr infos, int32_t termInfosIndexDivisor);
        virtual ~ReadOnlyDirectoryReader();
        
        LUCENE_CLASS(ReadOnlyDirectoryReader);
            
    public:
        /// Tries to acquire the WriteLock on this directory. this method is only valid if this 
        /// IndexReader is directory owner.
        virtual void acquireWriteLock();
    };
}

#endif
