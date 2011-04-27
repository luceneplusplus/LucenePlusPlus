/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MERGEPOLICY_H
#define MERGEPOLICY_H

#include "SegmentInfos.h"

namespace Lucene
{
    /// A MergePolicy determines the sequence of primitive merge operations to be used for overall merge
    /// and optimize operations.
    ///
    /// Whenever the segments in an index have been altered by {@link IndexWriter}, either the addition of 
    /// a newly flushed segment, addition of many segments from addIndexes* calls, or a previous merge that 
    /// may now need to cascade, {@link IndexWriter} invokes {@link #findMerges} to give the MergePolicy a 
    /// chance to pick merges that are now required.  This method returns a {@link MergeSpecification} 
    /// instance describing the set of merges that should be done, or null if no merges are necessary.  
    /// When IndexWriter.optimize is called, it calls {@link #findMergesForOptimize} and the MergePolicy 
    /// should then return the necessary merges.
    ///
    /// Note that the policy can return more than one merge at a time.  In this case, if the writer is using 
    /// {@link SerialMergeScheduler}, the merges will be run sequentially but if it is using {@link
    /// ConcurrentMergeScheduler} they will be run concurrently.
    ///
    /// The default MergePolicy is {@link LogByteSizeMergePolicy}.
    ///
    /// NOTE: This API is new and still experimental (subject to change suddenly in the next release)
    class LPPAPI MergePolicy : public LuceneObject
    {
    public:
        /// Creates a new merge policy instance. Note that if you intend to use it without passing it to 
        /// {@link IndexWriter}, you should call {@link #setIndexWriter(IndexWriter)}.
        MergePolicy();
        
        virtual ~MergePolicy();
        
        LUCENE_CLASS(MergePolicy);
    
    protected:
        IndexWriterWeakPtr _writer;
    
    public:
        /// Sets the {@link IndexWriter} to use by this merge policy. This method is allowed to be called 
        /// only once, and is usually set by IndexWriter. If it is called more than once, {@link 
        /// RuntimeException} is thrown.
        virtual void setIndexWriter(IndexWriterPtr writer);
        
        /// Determine what set of merge operations are now necessary on the index. {@link IndexWriter} calls 
        /// this whenever there is a change to the segments. This call is always synchronized on the {@link 
        /// IndexWriter} instance so only one thread at a time will call this method.
        /// @param segmentInfos the total set of segments in the index
        virtual MergeSpecificationPtr findMerges(SegmentInfosPtr segmentInfos) = 0;
        
        /// Determine what set of merge operations is necessary in order to optimize the index. {@link 
        /// IndexWriter} calls this when its {@link IndexWriter#optimize()} method is called. This call is 
        /// always synchronized on the {@link IndexWriter} instance so only one thread at a time will call 
        /// this method.
        /// @param segmentInfos the total set of segments in the index
        /// @param maxSegmentCount requested maximum number of segments in the index (currently this is always 1)
        /// @param segmentsToOptimize contains the specific SegmentInfo instances that must be merged away. 
        /// This may be a subset of all SegmentInfos.
        virtual MergeSpecificationPtr findMergesForOptimize(SegmentInfosPtr segmentInfos, int32_t maxSegmentCount, SetSegmentInfo segmentsToOptimize) = 0;
        
        /// Determine what set of merge operations is necessary in order to expunge all deletes from the index.
        /// @param segmentInfos the total set of segments in the index
        virtual MergeSpecificationPtr findMergesToExpungeDeletes(SegmentInfosPtr segmentInfos) = 0;
        
        /// Release all resources for the policy.
        virtual void close() = 0;
        
        /// Returns true if a new segment (regardless of its origin) should use the compound file format.
        virtual bool useCompoundFile(SegmentInfosPtr segments, SegmentInfoPtr newSegment) = 0;
    };
    
    /// OneMerge provides the information necessary to perform  an individual primitive merge operation,
    /// resulting in a single new segment.  The merge spec includes the subset of segments to be merged 
    /// as well as whether the new segment should use the compound file format.    
    class LPPAPI OneMerge : public LuceneObject
    {
    public:
        OneMerge(SegmentInfosPtr segments);
        virtual ~OneMerge();
        
        LUCENE_CLASS(OneMerge);
                
    public:
        SegmentInfoPtr info; // used by IndexWriter
        bool optimize; // used by IndexWriter
        bool registerDone; // used by IndexWriter
        int64_t mergeGen; // used by IndexWriter
        bool isExternal; // used by IndexWriter
        int32_t maxNumSegmentsOptimize; // used by IndexWriter
        Collection<SegmentReaderPtr> readers; // used by IndexWriter
        Collection<SegmentReaderPtr> readersClone; // used by IndexWriter
        
        SegmentInfosPtr segments;
        bool aborted;
        LuceneException error;
        bool paused;
        
    public:
        /// Record that an exception occurred while executing this merge
        void setException(const LuceneException& error);
        
        /// Retrieve previous exception set by {@link #setException}.
        LuceneException getException();
        
        /// Mark this merge as aborted.  If this is called before the merge is committed then the merge will not be committed.
        void abort();
        
        /// Returns true if this merge was aborted.
        bool isAborted();
        
        void checkAborted(DirectoryPtr dir);
        
        void setPause(bool paused);
        bool getPause();
        
        String segString(DirectoryPtr dir);
        
        /// Returns the total size in bytes of this merge. Note that this does not indicate the size of the merged segment, 
        /// but the input total size.
        int64_t totalBytesSize();
        
        /// Returns the total number of documents that are included with this merge. Note that this does not indicate the 
        /// number of documents after the merge.
        int32_t totalNumDocs();
    };
    
    /// A MergeSpecification instance provides the information necessary to perform multiple merges.  
    /// It simply contains a list of {@link OneMerge} instances.
    class LPPAPI MergeSpecification : public LuceneObject
    {
    public:
        MergeSpecification();
        virtual ~MergeSpecification();
        
        LUCENE_CLASS(MergeSpecification);
                
    public:
        Collection<OneMergePtr> merges;
    
    public:
        void add(OneMergePtr merge);
        String segString(DirectoryPtr dir);
    };
}

#endif
