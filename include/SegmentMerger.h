/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SEGMENTMERGER_H
#define SEGMENTMERGER_H

#include "LuceneObject.h"

namespace Lucene
{
    /// The SegmentMerger class combines two or more Segments, represented by an IndexReader ({@link #add}, into a single 
    /// Segment.  After adding the appropriate readers, call the merge method to combine the segments.
    ///
    /// @see #merge
    /// @see #add
    class SegmentMerger : public LuceneObject
    {
    public:
        SegmentMerger(DirectoryPtr dir, int32_t termIndexInterval, const String& name, OneMergePtr merge, 
                      PayloadProcessorProviderPtr payloadProcessorProvider, FieldInfosPtr fieldInfos);
        virtual ~SegmentMerger();
        
        LUCENE_CLASS(SegmentMerger);
            
    protected:
        DirectoryPtr directory;
        String segment;
        int32_t termIndexInterval;
        
        Collection<IndexReaderPtr> readers;
        FieldInfosPtr _fieldInfos;
        
        int32_t mergedDocs;
        CheckAbortPtr checkAbort;
        
        /// Maximum number of contiguous documents to bulk-copy when merging stored fields
        static const int32_t MAX_RAW_MERGE_DOCS;
        
        SegmentWriteStatePtr segmentWriteState;
        PayloadProcessorProviderPtr payloadProcessorProvider;
        
        Collection<SegmentReaderPtr> matchingSegmentReaders;
        Collection<int32_t> rawDocLengths;
        Collection<int32_t> rawDocLengths2;
        int32_t matchedCount;
        
        SegmentMergeQueuePtr queue;
        bool omitTermFreqAndPositions;
        
        ByteArray payloadBuffer;
        Collection< Collection<int32_t> > docMaps;
        Collection<int32_t> delCounts;
        
    public:
        /// norms header placeholder
        static const uint8_t NORMS_HEADER[];
        static const int32_t NORMS_HEADER_LENGTH;
    
    public:
        FieldInfosPtr fieldInfos();
        
        /// Add an IndexReader to the collection of readers that are to be merged
        void add(IndexReaderPtr reader);
        
        /// Merges the readers specified by the {@link #add} method into the directory passed to the constructor.
        /// @return The number of documents that were merged
        int32_t merge();
        
        HashSet<String> createCompoundFile(const String& fileName, SegmentInfoPtr info);
        
        int32_t getMatchedSubReaderCount();
        
        /// @return The number of documents in all of the readers
        int32_t mergeFields();
        
        Collection< Collection<int32_t> > getDocMaps();
        Collection<int32_t> getDelCounts();
    
    protected:
        static void addIndexed(IndexReaderPtr reader, FieldInfosPtr fInfos, HashSet<String> names, bool storeTermVectors,
                               bool storePositionWithTermVector, bool storeOffsetWithTermVector, bool storePayloads, 
                               bool omitTFAndPositions);
      
        void setMatchingSegmentReaders();
        int32_t copyFieldsWithDeletions(FieldsWriterPtr fieldsWriter, IndexReaderPtr reader, FieldsReaderPtr matchingFieldsReader);
        int32_t copyFieldsNoDeletions(FieldsWriterPtr fieldsWriter, IndexReaderPtr reader, FieldsReaderPtr matchingFieldsReader);
        
        /// Merge the TermVectors from each of the segments into the new one.
        void mergeVectors();
        
        void copyVectorsWithDeletions(TermVectorsWriterPtr termVectorsWriter, TermVectorsReaderPtr matchingVectorsReader, IndexReaderPtr reader);
        void copyVectorsNoDeletions(TermVectorsWriterPtr termVectorsWriter, TermVectorsReaderPtr matchingVectorsReader, IndexReaderPtr reader);
        
        void mergeTerms();
        
        void mergeTermInfos(FormatPostingsFieldsConsumerPtr consumer);
        
        /// Process postings from multiple segments all positioned on the same term. Writes out merged entries 
        /// into freqOutput and the proxOutput streams.
        /// @param smis array of segments
        /// @param n number of cells in the array actually occupied
        /// @return number of documents across all segments where this term was found
        int32_t appendPostings(FormatPostingsTermsConsumerPtr termsConsumer, Collection<SegmentMergeInfoPtr> smis, int32_t n);
        
        void mergeNorms();
    };
    
    class CheckAbort : public LuceneObject
    {
    public:
        CheckAbort(OneMergePtr merge, DirectoryPtr dir);
        virtual ~CheckAbort();
        
        LUCENE_CLASS(CheckAbort);
            
    protected:
        double workCount;
        OneMergePtr merge;
        DirectoryWeakPtr _dir;
    
    public:
        /// Records the fact that roughly units amount of work have been done since this method was last called.
        /// When adding time-consuming code into SegmentMerger, you should test different values for units to 
        /// ensure that the time in between calls to merge.checkAborted is up to ~ 1 second.
        virtual void work(double units);
    };
    
    class CheckAbortNull : public CheckAbort
    {
    public:
        CheckAbortNull();
        virtual ~CheckAbortNull();
        
        LUCENE_CLASS(CheckAbortNull);
            
    public:
        /// do nothing
        virtual void work(double units);
    };
}

#endif
