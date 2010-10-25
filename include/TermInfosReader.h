/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef TERMINFOSREADER_H
#define TERMINFOSREADER_H

#include "LuceneObject.h"
#include "CloseableThreadLocal.h"
#include "SimpleLRUCache.h"

namespace Lucene
{
    /// This stores a monotonically increasing set of <Term, TermInfo> pairs in a  Directory.  Pairs are 
    /// accessed either by Term or by ordinal position the set.
    class LPPAPI TermInfosReader : public LuceneObject
    {
    public:
        TermInfosReader(DirectoryPtr dir, const String& seg, FieldInfosPtr fis, int32_t readBufferSize, int32_t indexDivisor);
        virtual ~TermInfosReader();
        
        LUCENE_CLASS(TermInfosReader);
            
    protected:
        DirectoryPtr directory;
        String segment;
        FieldInfosPtr fieldInfos;
        CloseableThreadLocal<TermInfosReaderThreadResources> threadResources;
        SegmentTermEnumPtr origEnum;
        int64_t _size;

        Collection<TermPtr> indexTerms;
        Collection<TermInfoPtr> indexInfos;
        Collection<int64_t> indexPointers;
        
        int32_t totalIndexInterval;
        
        static const int32_t DEFAULT_CACHE_SIZE;
    
    public:
        int32_t getSkipInterval();
        int32_t getMaxSkipLevels();
        void close();
        
        /// Returns the number of term/value pairs in the set.
        int64_t size();
        
        /// Returns the TermInfo for a Term in the set, or null.
        TermInfoPtr get(TermPtr term);
        
        /// Returns the position of a Term in the set or -1.
        int64_t getPosition(TermPtr term);
        
        /// Returns an enumeration of all the Terms and TermInfos in the set.
        SegmentTermEnumPtr terms();
        
        /// Returns an enumeration of terms starting at or after the named term.
        SegmentTermEnumPtr terms(TermPtr term);
        
    protected:
        TermInfosReaderThreadResourcesPtr getThreadResources();
        
        /// Returns the offset of the greatest index entry which is less than or equal to term.
        int32_t getIndexOffset(TermPtr term);
        
        void seekEnum(SegmentTermEnumPtr enumerator, int32_t indexOffset);
        
        /// Returns the TermInfo for a Term in the set, or null.
        TermInfoPtr get(TermPtr term, bool useCache);
        
        void ensureIndexIsRead();
    };
    
    class LPPAPI TermInfosReaderThreadResources : public LuceneObject
    {
    public:
        virtual ~TermInfosReaderThreadResources();
        
        LUCENE_CLASS(TermInfosReaderThreadResources);
            
    public:
        SegmentTermEnumPtr termEnum;
    
        // Used for caching the least recently looked-up Terms
        TermInfoCachePtr termInfoCache;
    };
}

#endif
