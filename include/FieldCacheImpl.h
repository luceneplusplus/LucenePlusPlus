/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef FIELDCACHEIMPL_H
#define FIELDCACHEIMPL_H

#include "FieldCache.h"

namespace Lucene
{
    /// The default cache implementation, storing all values in memory.  A WeakHashMap is used for storage.
    class FieldCacheImpl : public FieldCache, public LuceneObject
    {
    public:
        FieldCacheImpl();
        virtual ~FieldCacheImpl();
    
        LUCENE_CLASS(FieldCacheImpl);
    
    protected:
        MapIntCache caches;
        InfoStreamPtr infoStream;
    
    public:
        virtual void initialize();
        virtual void purgeAllCaches();
        virtual void purge(IndexReaderPtr r);
        virtual Collection<FieldCacheEntryPtr> getCacheEntries();
        
        static ReaderFinishedListenerPtr purgeReader();
        
        virtual Collection<uint8_t> getBytes(IndexReaderPtr reader, const String& field);
        virtual Collection<uint8_t> getBytes(IndexReaderPtr reader, const String& field, ByteParserPtr parser);
        
        virtual Collection<int32_t> getInts(IndexReaderPtr reader, const String& field);
        virtual Collection<int32_t> getInts(IndexReaderPtr reader, const String& field, IntParserPtr parser);
        
        virtual Collection<int64_t> getLongs(IndexReaderPtr reader, const String& field);
        virtual Collection<int64_t> getLongs(IndexReaderPtr reader, const String& field, LongParserPtr parser);
        
        virtual Collection<double> getDoubles(IndexReaderPtr reader, const String& field);
        virtual Collection<double> getDoubles(IndexReaderPtr reader, const String& field, DoubleParserPtr parser);
        
        virtual Collection<String> getStrings(IndexReaderPtr reader, const String& field);
        virtual StringIndexPtr getStringIndex(IndexReaderPtr reader, const String& field);
        
        virtual void setInfoStream(InfoStreamPtr stream);
        virtual InfoStreamPtr getInfoStream();
    };
}

#endif
