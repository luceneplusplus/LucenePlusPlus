/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SEGMENTINFOCOLLECTION_H
#define SEGMENTINFOCOLLECTION_H

#include "LuceneObject.h"

namespace Lucene
{
    /// A collection of SegmentInfo objects to be used as a base class for {@link SegmentInfos}
    class LPPAPI SegmentInfoCollection : public LuceneObject
    {
    public:
        SegmentInfoCollection();
        virtual ~SegmentInfoCollection();
        
        LUCENE_CLASS(SegmentInfoCollection);
    
    protected:
        Collection<SegmentInfoPtr> segmentInfos;
    
    public:
        int32_t size();
        bool empty();
        void clear();
        void add(SegmentInfoPtr info);
        void add(int32_t pos, SegmentInfoPtr info);
        void addAll(SegmentInfoCollectionPtr segmentInfos);
        bool equals(SegmentInfoCollectionPtr other);
        int32_t find(SegmentInfoPtr info);
        bool contains(SegmentInfoPtr info);
        void remove(int32_t pos);
        void remove(int32_t start, int32_t end);
                
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
    };
}

#endif
