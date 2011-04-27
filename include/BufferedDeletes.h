/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef BUFFEREDDELETES_H
#define BUFFEREDDELETES_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Holds a {@link SegmentDeletes} for each segment in the index.
    class BufferedDeletes : public LuceneObject
    {
    public:
        BufferedDeletes(int32_t messageID);
        virtual ~BufferedDeletes();
        
        LUCENE_CLASS(BufferedDeletes);
        
    private:
        /// Deletes for all flushed/merged segments
        MapSegmentInfoSegmentDeletes deletesMap;
        
        // used only by assert
        TermPtr lastDeleteTerm;
        
        InfoStreamPtr infoStream;
        AtomicLongPtr _bytesUsed;
        AtomicLongPtr _numTerms;
        int32_t messageID;
    
    public:
        void setInfoStream(InfoStreamPtr infoStream);
        
        void pushDeletes(SegmentDeletesPtr newDeletes, SegmentInfoPtr info);
        
        /// Moves all pending deletes onto the provided segment, then clears the pending deletes
        void pushDeletes(SegmentDeletesPtr newDeletes, SegmentInfoPtr info, bool noLimit);
        
        void clear();
        bool any();
        int32_t numTerms();
        int64_t bytesUsed();
        
        /// IW calls this on finishing a merge.  While the merge was running, it's possible new 
        /// deletes were pushed onto our last (and only our last) segment.  In this case we must 
        /// carry forward those deletes onto the merged segment.
        void commitMerge(OneMergePtr merge);
        
        void clear(SegmentDeletesPtr deletes);
        
        bool applyDeletes(ReaderPoolPtr readerPool, SegmentInfosPtr segmentInfos, SegmentInfosPtr applyInfos);
        int64_t applyDeletes(ReaderPoolPtr readerPool, SegmentInfoPtr info, SegmentDeletesPtr coalescedDeletes, SegmentDeletesPtr segmentDeletes);
        int64_t applyDeletes(SegmentDeletesPtr deletes, SegmentReaderPtr reader);
        
        SegmentDeletesPtr getDeletes(SegmentInfoPtr info);
        
        void remove(SegmentInfosPtr infos);
    
    private:
        void message(const String& message);
        
        /// used only by assert
        bool anyDeletes(SegmentInfosPtr infos);
        
        /// used only by assert
        bool checkDeleteTerm(TermPtr term);
        
        /// used only by assert
        bool checkDeleteStats();
    };
}

#endif
