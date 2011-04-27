/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "SegmentWriteState.h"

namespace Lucene
{
    SegmentWriteState::SegmentWriteState(InfoStreamPtr infoStream, DirectoryPtr directory, const String& segmentName, 
                                         FieldInfosPtr fieldInfos, int32_t numDocs, int32_t termIndexInterval)
    {
        this->infoStream = infoStream;
        this->directory = directory;
        this->segmentName = segmentName;
        this->fieldInfos = fieldInfos;
        this->numDocs = numDocs;
        this->hasVectors = false;
        this->termIndexInterval = termIndexInterval;
        this->skipInterval = 16;
        this->maxSkipLevels = 10;
    }
    
    SegmentWriteState::~SegmentWriteState()
    {
    }
}
