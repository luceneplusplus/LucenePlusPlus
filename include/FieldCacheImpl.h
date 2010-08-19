/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FieldCache.h"
#include "LuceneObject.h"

namespace Lucene
{
	/// The default cache implementation, storing all values in memory.  A WeakHashMap is used for storage.
	class LPPAPI FieldCacheImpl : public FieldCache, public LuceneObject
	{
	public:
		FieldCacheImpl();
		virtual ~FieldCacheImpl();
	
		LUCENE_CLASS(FieldCacheImpl);
	
	protected:
		MapStringCache caches;
		InfoStreamPtr infoStream;
	
	public:
		virtual void initialize();
		virtual void purgeAllCaches();
		virtual Collection<FieldCacheEntryPtr> getCacheEntries();
		
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
	
	class LPPAPI Entry : public LuceneObject
	{
	public:
		/// Creates one of these objects for a custom comparator/parser.
		Entry(const String& field, LuceneObjectPtr custom);
		virtual ~Entry();
	
		LUCENE_CLASS(Entry);
	
	public:
		String field; // which Fieldable
		LuceneObjectPtr custom; // which custom comparator or parser
	
	public:
		/// Two of these are equal if they reference the same field and type.
		virtual bool equals(LuceneObjectPtr other);
		
		/// Composes a hashcode based on the field and type.
		virtual int32_t hashCode();
	};
	
	/// Internal cache.
	class LPPAPI Cache : public LuceneObject
	{
	public:
		Cache(FieldCachePtr wrapper = FieldCachePtr());
		virtual ~Cache();
		
		LUCENE_CLASS(Cache);
	
	public:
		FieldCacheWeakPtr _wrapper;
		WeakMapLuceneObjectMapEntryLuceneObject readerCache;
	
	protected:
		virtual LuceneObjectPtr createValue(IndexReaderPtr reader, EntryPtr key) = 0;
	
	public:
		virtual LuceneObjectPtr get(IndexReaderPtr reader, EntryPtr key);
		virtual void printNewInsanity(InfoStreamPtr infoStream, LuceneObjectPtr value);
	};
	
	class LPPAPI ByteCache : public Cache
	{
	public:
		ByteCache(FieldCachePtr wrapper = FieldCachePtr());
		virtual ~ByteCache();
		
		LUCENE_CLASS(ByteCache);
	
	protected:
		virtual LuceneObjectPtr createValue(IndexReaderPtr reader, EntryPtr key);
	};
	
	class LPPAPI IntCache : public Cache
	{
	public:
		IntCache(FieldCachePtr wrapper = FieldCachePtr());
		virtual ~IntCache();
		
		LUCENE_CLASS(IntCache);
	
	protected:
		virtual LuceneObjectPtr createValue(IndexReaderPtr reader, EntryPtr key);
	};
	
	class LPPAPI LongCache : public Cache
	{
	public:
		LongCache(FieldCachePtr wrapper = FieldCachePtr());
		virtual ~LongCache();
		
		LUCENE_CLASS(LongCache);
	
	protected:
		virtual LuceneObjectPtr createValue(IndexReaderPtr reader, EntryPtr key);
	};
	
	class LPPAPI DoubleCache : public Cache
	{
	public:
		DoubleCache(FieldCachePtr wrapper = FieldCachePtr());
		virtual ~DoubleCache();
		
		LUCENE_CLASS(DoubleCache);
	
	protected:
		virtual LuceneObjectPtr createValue(IndexReaderPtr reader, EntryPtr key);
	};
	
	class LPPAPI StringCache : public Cache
	{
	public:
		StringCache(FieldCachePtr wrapper = FieldCachePtr());
		virtual ~StringCache();
		
		LUCENE_CLASS(StringCache);
	
	protected:
		virtual LuceneObjectPtr createValue(IndexReaderPtr reader, EntryPtr key);
	};
	
	class LPPAPI StringIndexCache : public Cache
	{
	public:
		StringIndexCache(FieldCachePtr wrapper = FieldCachePtr());
		virtual ~StringIndexCache();
		
		LUCENE_CLASS(StringIndexCache);
	
	protected:
		virtual LuceneObjectPtr createValue(IndexReaderPtr reader, EntryPtr key);
	};
	
	class LPPAPI FieldCacheEntryImpl : public FieldCacheEntry
	{
	public:
		FieldCacheEntryImpl(LuceneObjectPtr readerKey, const String& fieldName, const String& cacheType, LuceneObjectPtr custom, LuceneObjectPtr value);
		virtual ~FieldCacheEntryImpl();
		
		LUCENE_CLASS(FieldCacheEntryImpl);
	
	protected:
		LuceneObjectPtr readerKey;
		String fieldName;
		String cacheType;
		LuceneObjectPtr custom;
		LuceneObjectPtr value;
	
	public:
		virtual LuceneObjectPtr getReaderKey();
		virtual String getFieldName();
		virtual String getCacheType();
		virtual LuceneObjectPtr getCustom();
		virtual LuceneObjectPtr getValue();
	};
}
