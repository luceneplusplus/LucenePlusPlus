/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SEGMENTWRITESTATE_H
#define SEGMENTWRITESTATE_H

#include "LuceneObject.h"

namespace Lucene
{
    class SegmentWriteState : public LuceneObject
    {
    public:
        SegmentWriteState(InfoStreamPtr infoStream, DirectoryPtr directory, const String& segmentName, 
                          FieldInfosPtr fieldInfos, int32_t numDocs, int32_t termIndexInterval);
        virtual ~SegmentWriteState();
        
        LUCENE_CLASS(SegmentWriteState);
            
    public:
        InfoStreamPtr infoStream;
        DirectoryPtr directory;
        String segmentName;
        FieldInfosPtr fieldInfos;
        int32_t numDocs;
        bool hasVectors;

        /// The fraction of terms in the "dictionary" which should be stored in RAM.  Smaller values use 
        /// more memory, but make searching slightly faster, while larger values use less memory and make 
        /// searching slightly slower.  Searching is typically not dominated by dictionary lookup, so
        /// tweaking this is rarely useful.*/
        int32_t termIndexInterval;

        /// The fraction of TermDocs entries stored in skip tables, used to accelerate {@link 
        /// TermDocs#skipTo(int)}.  Larger values result in smaller indexes, greater acceleration, but 
        /// fewer accelerable cases, while smaller values result in bigger indexes, less acceleration and 
        /// more accelerable cases. More detailed experiments would be useful here.
        int32_t skipInterval;

        /// The maximum number of skip levels. Smaller values result in  slightly smaller indexes, but 
        /// slower skipping in big posting lists.
        int32_t maxSkipLevels;
    };
}

#endif
