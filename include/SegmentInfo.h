/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SEGMENTINFO_H
#define SEGMENTINFO_H

#include "LuceneObject.h"

namespace Lucene
{
    /// Information about a segment such as it's name, directory, and files 
    /// related to the segment.
    class LPPAPI SegmentInfo : public LuceneObject
    {
    public:
        SegmentInfo(const String& name, int32_t docCount, DirectoryPtr dir, bool isCompoundFile, bool hasSingleNormFile, 
                    bool hasProx, bool hasVectors);
                    
        /// Construct a new SegmentInfo instance by reading a previously saved SegmentInfo from input.
        /// @param dir directory to load from.
        /// @param format format of the segments info file.
        /// @param input input handle to read segment info from.
        SegmentInfo(DirectoryPtr dir, int32_t format, IndexInputPtr input);
        
        virtual ~SegmentInfo();
        
        LUCENE_CLASS(SegmentInfo);
            
    public:        
        static const int32_t NO; // no norms; no deletes;
        static const int32_t YES; // have norms; have deletes;
        static const int32_t CHECK_DIR; // must check dir to see if there are norms/deletions
        static const int32_t WITHOUT_GEN; // a file name that has no GEN in it. 
    
    protected:
        // true if this is a segments file written before lock-less commits (2.1)
        bool preLockless;
        
        // current generation of del file; NO if there are no deletes; CHECK_DIR if  it's a pre-2.1 segment 
        // (and we must check filesystem); YES or higher if there are deletes at generation N
        int64_t delGen; 

        // current generation of each field's norm file.  If this array is null, for lockLess this means no 
        // separate norms.  For preLockLess this means we must check filesystem. If this array is not null, 
        // its values mean: NO says this field has no separate norms; CHECK_DIR says it is a preLockLess 
        // segment and filesystem must be checked; >= YES says this field has separate norms with the 
        // specified generation
        Collection<int64_t> normGen;
                                     
        // NO if it is not; YES if it is; CHECK_DIR if it's pre-2.1 (ie, must check file system to see if 
        // <name>.cfs and <name>.nrm exist)
        uint8_t isCompoundFile;
        
        // true if this segment maintains norms in a single file; false otherwise this is currently false for 
        // segments populated by DocumentWriter and true for newly created merged segments (both compound and 
        // non compound).
        bool hasSingleNormFile;
        
        // cached list of files that this segment uses in the Directory
        HashSet<String> _files;
        
        // total byte size of all but the store files (computed on demand)
        int64_t sizeInBytesNoStore;
        
        // total byte size of all of our files (computed on demand)
        int64_t sizeInBytesWithStore;
        
        // if this segment shares stored fields & vectors, this offset is where in that file this segment's 
        // docs begin
        int32_t docStoreOffset;
        
        // name used to derive fields/vectors file we share with other segments
        String docStoreSegment;
        
        // whether doc store files are stored in compound file (*.cfx)
        bool docStoreIsCompoundFile;
        
        // How many deleted docs in this segment, or -1 if not yet known (if it's an older index)
        int32_t delCount;
        
        // True if this segment has any fields with omitTermFreqAndPositions == false
        bool hasProx;
        
        // True if this segment wrote term vectors
        bool hasVectors;
        
        MapStringString diagnostics;
        
        // Tracks the Lucene version this segment was created with, since 3.1. The format expected is 
        // "x.y" - "2.x" for pre-3.0 indexes, and specific versions afterwards ("3.0", "3.1" etc.).
        // see Constants::LUCENE_MAIN_VERSION.
        String version;
                            
    public:
        String name; // unique name in dir
        int32_t docCount; // number of docs in seg
        DirectoryPtr dir; // where segment resides
    
    public:
        /// Copy everything from src SegmentInfo into our instance.
        void reset(SegmentInfoPtr src);
        
        void setDiagnostics(MapStringString diagnostics);
        MapStringString getDiagnostics();
        
        void setNumFields(int32_t numFields);
        
        /// Returns total size in bytes of all of files used by this segment (if {@code includeDocStores} 
        /// is true), or the size of all files except the store files otherwise.
        int64_t sizeInBytes(bool includeDocStores);
        
        bool getHasVectors();
        void setHasVectors(bool v);
        
        bool hasDeletions();
        void advanceDelGen();
        void clearDelGen();
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        String getDelFileName();
        
        /// Returns true if this field for this segment has saved a separate norms file (_<segment>_N.sX).
        /// @param fieldNumber the field index to check
        bool hasSeparateNorms(int32_t fieldNumber);
        
        /// Returns true if any fields in this segment have separate norms.
        bool hasSeparateNorms();
        
        /// Increment the generation count for the norms file for this field.
        /// @param fieldIndex field whose norm file will be rewritten
        void advanceNormGen(int32_t fieldIndex);
        
        /// Get the file name for the norms file for this field.
        /// @param number field index
        String getNormFileName(int32_t number);
        
        /// Mark whether this segment is stored as a compound file.
        /// @param isCompoundFile true if this is a compound file; else, false
        void setUseCompoundFile(bool isCompoundFile);
        
        /// Returns true if this segment is stored as a compound file; else, false.
        bool getUseCompoundFile();
        
        int32_t getDelCount();
        void setDelCount(int32_t delCount);
        int32_t getDocStoreOffset();
        bool getDocStoreIsCompoundFile();
        void setDocStoreIsCompoundFile(bool v);
        String getDocStoreSegment();
        void setDocStoreSegment(const String& segment);
        void setDocStoreOffset(int32_t offset);
        void setDocStore(int32_t offset, const String& segment, bool isCompoundFile);
        
        /// Save this segment's info.
        void write(IndexOutputPtr output);
        
        void setHasProx(bool hasProx);
        bool getHasProx();
    
        /// Return all files referenced by this SegmentInfo.  The returns List is a locally cached List so 
        /// you should not modify it.
        HashSet<String> files();
        
        virtual String toString();
        
        /// Used for debugging.  Format may suddenly change.
        ///
        /// Current format looks like _a(3.1):c45/4->_1, which means the segment's name is _a; it was created 
        /// with Lucene 3.1 (or '?' if it's unknown); it's using compound file format (would be C if not compound); 
        /// it has 45 documents; it has 4 deletions (this part is left off when there are no deletions); it's 
        /// using the shared doc stores named _1 (this part is left off if doc stores are private).
        String toString(DirectoryPtr dir, int32_t pendingDelCount);
        
        /// We consider another SegmentInfo instance equal if it has the same dir and same name.
        virtual bool equals(LuceneObjectPtr other);        
        
        virtual int32_t hashCode();
        
        /// Used by SegmentInfos to upgrade segments that do not record their code version (either "2.x" or "3.0").
        ///
        /// NOTE: this method is used for internal purposes only - you should not modify the version of a 
        /// SegmentInfo, or it may result in unexpected exceptions thrown when you attempt to open the index.
        void setVersion(const String& version);
        
        /// Returns the version of the code which wrote the segment.
        String getVersion();
        
    protected:
        void addIfExists(HashSet<String> files, const String& fileName);
        
        /// Called whenever any change is made that affects which files this segment has.
        void clearFiles();
    };
}

#endif
