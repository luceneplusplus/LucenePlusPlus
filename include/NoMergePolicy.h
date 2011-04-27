/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef NOMERGEPOLICY_H
#define NOMERGEPOLICY_H

#include "MergePolicy.h"

namespace Lucene
{
    /// A {@link MergePolicy} which never returns merges to execute (hence it's name). It is 
    /// also a singleton and can be accessed through {@link NoMergePolicy#NO_COMPOUND_FILES} 
    /// if you want to indicate the index does not use compound files, or through {@link 
    /// NoMergePolicy#COMPOUND_FILES} otherwise. Use it if you want to prevent an {@link 
    /// IndexWriter} from ever executing merges, without going through the hassle of tweaking 
    /// a merge policy's settings to achieve that, such as changing its merge factor.
    class LPPAPI NoMergePolicy : public MergePolicy
    {
    public:
        NoMergePolicy(bool useCompoundFile);
        virtual ~NoMergePolicy();
        
        LUCENE_CLASS(NoMergePolicy);
    
    private:
        bool _useCompoundFile;

    public:
        /// A singleton {@link NoMergePolicy} which indicates the index does not use compound files.
        static MergePolicyPtr NO_COMPOUND_FILES();
        
        /// A singleton {@link NoMergePolicy} which indicates the index uses compound files.
        static MergePolicyPtr COMPOUND_FILES();
        
        virtual void close();
        virtual MergeSpecificationPtr findMerges(SegmentInfosPtr segmentInfos);
        virtual MergeSpecificationPtr findMergesForOptimize(SegmentInfosPtr segmentInfos, int32_t maxSegmentCount, SetSegmentInfo segmentsToOptimize);
        virtual MergeSpecificationPtr findMergesToExpungeDeletes(SegmentInfosPtr segmentInfos);
        virtual bool useCompoundFile(SegmentInfosPtr segments, SegmentInfoPtr newSegment);
        virtual void setIndexWriter(IndexWriterPtr writer);
        virtual String toString();
    };
}

#endif
