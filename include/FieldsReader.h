/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FIELDSREADER_H
#define FIELDSREADER_H

#include "AbstractField.h"
#include "CloseableThreadLocal.h"

namespace Lucene
{
    /// Class responsible for access to stored document fields.  It uses <segment>.fdt and <segment>.fdx; files.
    class FieldsReader : public LuceneObject
    {
    public:
        /// Used only by clone
        FieldsReader(FieldInfosPtr fieldInfos, int32_t numTotalDocs, int32_t size, int32_t format, int32_t formatSize,
                     int32_t docStoreOffset, IndexInputPtr cloneableFieldsStream, IndexInputPtr cloneableIndexStream);
        FieldsReader(DirectoryPtr d, const String& segment, FieldInfosPtr fn);
        FieldsReader(DirectoryPtr d, const String& segment, FieldInfosPtr fn, int32_t readBufferSize, int32_t docStoreOffset = -1, int32_t size = 0);
        
        virtual ~FieldsReader();
        
        LUCENE_CLASS(FieldsReader);
            
    protected:
        FieldInfosPtr fieldInfos;
        
        // The main fieldStream, used only for cloning.
        IndexInputPtr cloneableFieldsStream;
        
        // This is a clone of cloneableFieldsStream used for reading documents.  It should not be cloned outside of a 
        // synchronized context.
        IndexInputPtr fieldsStream;
        
        IndexInputPtr cloneableIndexStream;
        IndexInputPtr indexStream;
        int32_t numTotalDocs;
        int32_t _size;
        bool closed;
        int32_t format;
        int32_t formatSize;
        
        // The docID offset where our docs begin in the index file.  This will be 0 if we have our own private file.
        int32_t docStoreOffset;
        
        CloseableThreadLocal<IndexInput> fieldsStreamTL;
        bool isOriginal;
        
    public:
        /// Detects the code version this segment was written with. Returns either "2.x" for all pre-3.0 segments, 
        /// or "3.0" for 3.0 segments. This method should not be called for 3.1+ segments since they already record 
        /// their code version.
        static String detectCodeVersion(DirectoryPtr dir, const String& segment);
        
        /// Returns a cloned FieldsReader that shares open IndexInputs with the original one.  It is the caller's job not to 
        /// close the original FieldsReader until all clones are called (eg, currently SegmentReader manages this logic).
        virtual LuceneObjectPtr clone(LuceneObjectPtr other = LuceneObjectPtr());
        
        /// Closes the underlying {@link IndexInput} streams, including any ones associated with a lazy implementation of a 
        /// Field.  This means that the Fields values will not be accessible.
        void close();
        
        int32_t size();
        
        bool canReadRawDocs();
        
        DocumentPtr doc(int32_t n, FieldSelectorPtr fieldSelector);
        
        /// Returns the length in bytes of each raw document in a contiguous range of length numDocs starting with startDocID.  
        /// Returns the IndexInput (the fieldStream), already seeked to the starting point for startDocID.
        IndexInputPtr rawDocs(Collection<int32_t> lengths, int32_t startDocID, int32_t numDocs);
    
    protected:
        void ConstructReader(DirectoryPtr d, const String& segment, FieldInfosPtr fn, int32_t readBufferSize, int32_t docStoreOffset, int32_t size);
        
        void ensureOpen();
        
        void seekIndex(int32_t docID);
        
        /// Skip the field.  We still have to read some of the information about the field, but can skip past the actual content.  
        /// This will have the most payoff on large fields.
        void skipField(bool binary, bool compressed);
        void skipField(bool binary, bool compressed, int32_t toRead);
        
        void addFieldLazy(DocumentPtr doc, FieldInfoPtr fi, bool binary, bool compressed, bool tokenize, bool cacheResult);
        void addField(DocumentPtr doc, FieldInfoPtr fi, bool binary, bool compressed, bool tokenize);
        
        /// Add the size of field as a byte[] containing the 4 bytes of the integer byte size (high order byte first; char = 2 bytes).  
        /// Read just the size - caller must skip the field content to continue reading fields.  Return the size in bytes or chars, 
        /// depending on field type.
        int32_t addFieldSize(DocumentPtr doc, FieldInfoPtr fi, bool binary, bool compressed);
        
        ByteArray uncompress(ByteArray b);
        String uncompressString(ByteArray b);
        
        friend class LazyField;
    };
}

#endif
