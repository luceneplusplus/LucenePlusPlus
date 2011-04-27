/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "NoMergePolicy.h"

namespace Lucene
{
    NoMergePolicy::NoMergePolicy(bool useCompoundFile)
    {
        this->_useCompoundFile = useCompoundFile;
    }
    
    NoMergePolicy::~NoMergePolicy()
    {
    }
    
    MergePolicyPtr NoMergePolicy::NO_COMPOUND_FILES()
    {
        static MergePolicyPtr _NO_COMPOUND_FILES;
        if (!_NO_COMPOUND_FILES)
        {
            _NO_COMPOUND_FILES = newLucene<NoMergePolicy>(false);
            CycleCheck::addStatic(_NO_COMPOUND_FILES);
        }
        return _NO_COMPOUND_FILES;
    }
    
    MergePolicyPtr NoMergePolicy::COMPOUND_FILES()
    {
        static MergePolicyPtr _COMPOUND_FILES;
        if (!_COMPOUND_FILES)
        {
            _COMPOUND_FILES = newLucene<NoMergePolicy>(true);
            CycleCheck::addStatic(_COMPOUND_FILES);
        }
        return _COMPOUND_FILES;
    }
    
    void NoMergePolicy::close()
    {
    }
    
    MergeSpecificationPtr NoMergePolicy::findMerges(SegmentInfosPtr segmentInfos)
    {
        return MergeSpecificationPtr();
    }
    
    MergeSpecificationPtr NoMergePolicy::findMergesForOptimize(SegmentInfosPtr segmentInfos, int32_t maxSegmentCount, SetSegmentInfo segmentsToOptimize)
    {
        return MergeSpecificationPtr();
    }
    
    MergeSpecificationPtr NoMergePolicy::findMergesToExpungeDeletes(SegmentInfosPtr segmentInfos)
    {
        return MergeSpecificationPtr();
    }
    
    bool NoMergePolicy::useCompoundFile(SegmentInfosPtr segments, SegmentInfoPtr newSegment)
    {
        return _useCompoundFile;
    }
    
    void NoMergePolicy::setIndexWriter(IndexWriterPtr writer)
    {
    }
    
    String NoMergePolicy::toString()
    {
        return getClassName();
    }
}
