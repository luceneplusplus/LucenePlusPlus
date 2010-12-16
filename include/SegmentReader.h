/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SEGMENTREADER_H
#define SEGMENTREADER_H

#include "IndexReader.h"
#include "CloseableThreadLocal.h"

namespace Lucene
{
    class LPPAPI SegmentReader : public IndexReader
    {
    public:
        SegmentReader();
        virtual ~SegmentReader();
        
        LUCENE_CLASS(SegmentReader);
            
    protected:
        SegmentInfoPtr si;
        bool readOnly;
        int32_t readBufferSize;
        
        bool deletedDocsDirty;
        bool normsDirty;
        int32_t pendingDeleteCount;
        
        bool rollbackHasChanges;
        bool rollbackDeletedDocsDirty;
        bool rollbackNormsDirty;
        SegmentInfoPtr rollbackSegmentInfo;
        int32_t rollbackPendingDeleteCount;
        
        // optionally used for the .nrm file shared by multiple norms
        IndexInputPtr singleNormStream;
        SegmentReaderRefPtr singleNormRef;
        
    public:
        FieldsReaderLocalPtr fieldsReaderLocal;
        CloseableThreadLocal<TermVectorsReader> termVectorsLocal;
    
        BitVectorPtr deletedDocs;
        SegmentReaderRefPtr deletedDocsRef;
        
        CoreReadersPtr core;
        MapStringNorm _norms;
        
    public:
        virtual void initialize();
        
        using IndexReader::document;
        using IndexReader::termPositions;
    
        static SegmentReaderPtr get(bool readOnly, SegmentInfoPtr si, int32_t termInfosIndexDivisor);
        static SegmentReaderPtr get(bool readOnly, DirectoryPtr dir, SegmentInfoPtr si, int32_t readBufferSize, bool doOpenStores, int32_t termInfosIndexDivisor);
        
        void openDocStores();
        
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        virtual LuceneObjectPtr clone(bool openReadOnly, LuceneObjectPtr other = LuceneObjectPtr());
        SegmentReaderPtr reopenSegment(SegmentInfoPtr si, bool doClone, bool openReadOnly);
        FieldsReaderPtr getFieldsReader();
        
        static bool hasDeletions(SegmentInfoPtr si);
        
        /// Returns true if any documents have been deleted
        virtual bool hasDeletions();
        
        static bool usesCompoundFile(SegmentInfoPtr si);
        static bool hasSeparateNorms(SegmentInfoPtr si);
        
        HashSet<String> files();
        
        /// Returns an enumeration of all the terms in the index.
        virtual TermEnumPtr terms();
        
        /// Returns an enumeration of all terms starting at a given term.
        virtual TermEnumPtr terms(TermPtr t);
        
        FieldInfosPtr fieldInfos();
        
        /// Get the {@link Document} at the n'th position.
        virtual DocumentPtr document(int32_t n, FieldSelectorPtr fieldSelector);
        
        /// Returns true if document n has been deleted 
        virtual bool isDeleted(int32_t n);
        
        /// Returns an enumeration of all the documents which contain term.
        virtual TermDocsPtr termDocs(TermPtr term);
        
        /// Returns an unpositioned {@link TermDocs} enumerator.
        virtual TermDocsPtr termDocs();
        
        /// Returns an unpositioned {@link TermPositions} enumerator.
        virtual TermPositionsPtr termPositions();
        
        /// Returns the number of documents containing the term t.
        virtual int32_t docFreq(TermPtr t);
        
        /// Returns the number of documents in this index.
        virtual int32_t numDocs();
        
        /// Returns one greater than the largest possible document number.
        virtual int32_t maxDoc();
        
        /// Get a list of unique field names that exist in this index and have the specified field option information.
        virtual HashSet<String> getFieldNames(FieldOption fieldOption);
        
        /// Returns true if there are norms stored for this field.
        virtual bool hasNorms(const String& field);
        
        /// Returns the byte-encoded normalization factor for the named field of every document.
        virtual ByteArray norms(const String& field);
        
        /// Read norms into a pre-allocated array.
        virtual void norms(const String& field, ByteArray norms, int32_t offset);
        
        bool termsIndexLoaded();
        
        /// NOTE: only called from IndexWriter when a near real-time reader is opened, or applyDeletes is run, sharing a 
        /// segment that's still being merged.  This method is not thread safe, and relies on the synchronization in IndexWriter
        void loadTermsIndex(int32_t termsIndexDivisor);
        
        bool normsClosed(); // for testing only
        bool normsClosed(const String& field); // for testing only
        
        /// Create a clone from the initial TermVectorsReader and store it in the ThreadLocal.
        /// @return TermVectorsReader
        TermVectorsReaderPtr getTermVectorsReader();
        
        TermVectorsReaderPtr getTermVectorsReaderOrig();
        
        /// Return a term frequency vector for the specified document and field. The vector returned contains term 
        /// numbers and frequencies for all terms in the specified field of this document, if the field had 
        /// storeTermVector flag set.  If the flag was not set, the method returns null.
        virtual TermFreqVectorPtr getTermFreqVector(int32_t docNumber, const String& field);
        
        /// Load the Term Vector into a user-defined data structure instead of relying on the parallel arrays 
        /// of the {@link TermFreqVector}.
        virtual void getTermFreqVector(int32_t docNumber, const String& field, TermVectorMapperPtr mapper);
        
        /// Map all the term vectors for all fields in a Document
        virtual void getTermFreqVector(int32_t docNumber, TermVectorMapperPtr mapper);
        
        /// Return an array of term frequency vectors for the specified document.  The array contains a vector for 
        /// each vectorized field in the document.  Each vector vector contains term numbers and frequencies for all 
        /// terms in a given vectorized field.  If no such fields existed, the method returns null.
        virtual Collection<TermFreqVectorPtr> getTermFreqVectors(int32_t docNumber);
        
        /// Return the name of the segment this reader is reading.
        String getSegmentName();
        
        /// Return the SegmentInfo of the segment this reader is reading.
        SegmentInfoPtr getSegmentInfo();
        void setSegmentInfo(SegmentInfoPtr info);
        
        void startCommit();
        void rollbackCommit();
        
        /// Returns the directory this index resides in.
        virtual DirectoryPtr directory();
        
        /// This is necessary so that cloned SegmentReaders (which share the underlying postings data) 
        /// will map to the same entry in the FieldCache.
        virtual LuceneObjectPtr getFieldCacheKey();
        virtual LuceneObjectPtr getDeletesCacheKey();
        
        /// Returns the number of unique terms (across all fields) in this reader.
        virtual int64_t getUniqueTermCount();
        
        static SegmentReaderPtr getOnlySegmentReader(DirectoryPtr dir);
        static SegmentReaderPtr getOnlySegmentReader(IndexReaderPtr reader);
        
        virtual int32_t getTermInfosIndexDivisor();
            
    protected:
        bool checkDeletedCounts();
        void loadDeletedDocs();
        
        /// Clones the norm bytes.  May be overridden by subclasses.
        /// @param bytes Byte array to clone
        /// @return New BitVector
        virtual ByteArray cloneNormBytes(ByteArray bytes);
        
        /// Clones the deleteDocs BitVector.  May be overridden by subclasses.
        /// @param bv BitVector to clone
        /// @return New BitVector
        virtual BitVectorPtr cloneDeletedDocs(BitVectorPtr bv);
        
        /// Implements commit.
        virtual void doCommit(MapStringString commitUserData);
        
        virtual void commitChanges(MapStringString commitUserData);
        
        /// Implements close.
        virtual void doClose();
        
        /// Implements deletion of the document numbered docNum.
        /// Applications should call {@link #deleteDocument(int)} or {@link #deleteDocuments(Term)}.
        virtual void doDelete(int32_t docNum);
        
        /// Implements actual undeleteAll() in subclass.
        virtual void doUndeleteAll();
        
        /// can return null if norms aren't stored
        ByteArray getNorms(const String& field);
        
        /// Implements setNorm in subclass.
        virtual void doSetNorm(int32_t doc, const String& field, uint8_t value);
        
        void openNorms(DirectoryPtr cfsDir, int32_t readBufferSize);
        
        friend class ReaderPool;
        friend class IndexWriter;
        friend class Norm;
    };
    
    /// Holds core readers that are shared (unchanged) when SegmentReader is cloned or reopened
    class LPPAPI CoreReaders : public LuceneObject
    {
    public:
        CoreReaders(SegmentReaderPtr origInstance, DirectoryPtr dir, SegmentInfoPtr si, int32_t readBufferSize, int32_t termsIndexDivisor);
        virtual ~CoreReaders();
        
        LUCENE_CLASS(CoreReaders);
            
    protected:
        /// Counts how many other reader share the core objects (freqStream, proxStream, tis, etc.) of this reader;
        /// when coreRef drops to 0, these core objects may be closed.  A given instance of SegmentReader may be
        /// closed, even those it shares core objects with other SegmentReaders
        SegmentReaderRefPtr ref;
        
        SegmentReaderWeakPtr _origInstance;
    
    public:
        String segment;
        FieldInfosPtr fieldInfos;
        IndexInputPtr freqStream;
        IndexInputPtr proxStream;
        TermInfosReaderPtr tisNoIndex;

        DirectoryPtr dir;
        DirectoryPtr cfsDir;
        int32_t readBufferSize;
        int32_t termsIndexDivisor;

        TermInfosReaderPtr tis;
        FieldsReaderPtr fieldsReaderOrig;
        TermVectorsReaderPtr termVectorsReaderOrig;
        CompoundFileReaderPtr cfsReader;
        CompoundFileReaderPtr storeCFSReader;
    
    public:
        TermVectorsReaderPtr getTermVectorsReaderOrig();
        FieldsReaderPtr getFieldsReaderOrig();
        void incRef();
        DirectoryPtr getCFSReader();
        TermInfosReaderPtr getTermsReader();
        bool termsIndexIsLoaded();
        
        /// NOTE: only called from IndexWriter when a near real-time reader is opened, or applyDeletes is run,
        /// sharing a segment that's still being merged.  This method is not fully thread safe, and relies on the
        /// synchronization in IndexWriter
        void loadTermsIndex(SegmentInfoPtr si, int32_t termsIndexDivisor);
        
        void openDocStores(SegmentInfoPtr si);
        
        void decRef();
        
        friend class SegmentReader;
    };
    
    /// Sets the initial value
    class LPPAPI FieldsReaderLocal : public CloseableThreadLocal<FieldsReader>
    {
    public:
        FieldsReaderLocal(SegmentReaderPtr reader);
    
    protected:
        SegmentReaderWeakPtr _reader;
    
    protected:
        virtual FieldsReaderPtr initialValue();
    };
    
    class LPPAPI SegmentReaderRef : public LuceneObject
    {
    public:
        SegmentReaderRef();
        virtual ~SegmentReaderRef();
        
        LUCENE_CLASS(SegmentReaderRef);
            
    protected:
        int32_t _refCount;
    
    public:
        virtual String toString();
        int32_t refCount();
        int32_t incRef();
        int32_t decRef();
        
        friend class SegmentReader;
    };
    
    /// Byte[] referencing is used because a new norm object needs to be created for each clone, and the byte 
    /// array is all that is needed for sharing between cloned readers.  The current norm referencing is for 
    /// sharing between readers whereas the byte[] referencing is for copy on write which is independent of 
    /// reader references (i.e. incRef, decRef).
    class LPPAPI Norm : public LuceneObject
    {
    public:
        Norm();
        Norm(SegmentReaderPtr reader, IndexInputPtr in, int32_t number, int64_t normSeek);
        virtual ~Norm();
        
        LUCENE_CLASS(Norm);
                
    protected:
        SegmentReaderWeakPtr _reader;
        int32_t refCount;
        
        /// If this instance is a clone, the originalNorm references the Norm that has a real open IndexInput
        NormPtr origNorm;
        SegmentReaderPtr origReader;
        
        IndexInputPtr in;
        int64_t normSeek;
        
        SegmentReaderRefPtr _bytesRef;
        ByteArray _bytes;
        bool dirty;
        int32_t number;
        bool rollbackDirty;
    
    public:
        void incRef();
        void decRef();
        
        /// Load bytes but do not cache them if they were not already cached
        void bytes(uint8_t* bytesOut, int32_t offset, int32_t length);
        
        /// Load & cache full bytes array.  Returns bytes.
        ByteArray bytes();
        
        /// Only for testing
        SegmentReaderRefPtr bytesRef();
        
        /// Called if we intend to change a norm value.  We make a private copy of bytes if it's shared 
        // with others
        ByteArray copyOnWrite();
        
        /// Returns a copy of this Norm instance that shares IndexInput & bytes with the original one
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        /// Flush all pending changes to the next generation separate norms file.
        void reWrite(SegmentInfoPtr si);
        
    protected:
        void closeInput();
        
        friend class SegmentReader;
    };
}

#endif
