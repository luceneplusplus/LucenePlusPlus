/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include <float.h>
#include "LogByteSizeMergePolicy.h"

namespace Lucene
{
    /// Default minimum segment size.
    const double LogByteSizeMergePolicy::DEFAULT_MIN_MERGE_MB = 1.6;
    
    /// Default maximum segment size.  A segment of this size or larger will never be merged.
    const double LogByteSizeMergePolicy::DEFAULT_MAX_MERGE_MB = 2048.0;
    
    /// Default maximum segment size.  A segment of this size or larger will never be merged 
    /// during optimize.  @see setMaxMergeMBForOptimize
    const double LogByteSizeMergePolicy::DEFAULT_MAX_MERGE_MB_FOR_OPTIMIZE = DBL_MAX;
    
    LogByteSizeMergePolicy::LogByteSizeMergePolicy()
    {
        minMergeSize = (int64_t)(DEFAULT_MIN_MERGE_MB * 1024 * 1024);
        maxMergeSize = DEFAULT_MAX_MERGE_MB == DBL_MAX ? LLONG_MAX : (int64_t)(DEFAULT_MAX_MERGE_MB * 1024 * 1024);
        maxMergeSizeForOptimize = (int64_t)(DEFAULT_MAX_MERGE_MB_FOR_OPTIMIZE * 1024 * 1024);
    }
    
    LogByteSizeMergePolicy::~LogByteSizeMergePolicy()
    {
    }
    
    int64_t LogByteSizeMergePolicy::size(SegmentInfoPtr info)
    {
        return sizeBytes(info);
    }
    
    void LogByteSizeMergePolicy::setMaxMergeMB(double mb)
    {
        maxMergeSize = (int64_t)(mb * 1024 * 1024);
    }
    
    double LogByteSizeMergePolicy::getMaxMergeMB()
    {
        return ((double)maxMergeSize) / 1024 / 1024;
    }
    
    void LogByteSizeMergePolicy::setMaxMergeMBForOptimize(double mb)
    {
        maxMergeSizeForOptimize = (int64_t)(mb * 1024 * 1024);
    }
    
    double LogByteSizeMergePolicy::getMaxMergeMBForOptimize()
    {
        return ((double)maxMergeSizeForOptimize) / 1024 / 1024;
    }
    
    void LogByteSizeMergePolicy::setMinMergeMB(double mb)
    {
        minMergeSize = (int64_t)(mb * 1024 * 1024);
    }
    
    double LogByteSizeMergePolicy::getMinMergeMB()
    {
        return ((double)minMergeSize) / 1024 / 1024;
    }
}
