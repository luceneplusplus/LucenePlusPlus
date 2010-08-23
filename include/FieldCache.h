/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	/// Maintains caches of term values.
	/// @see FieldCacheSanityChecker
	class LPPAPI FieldCache
	{
	public:
		virtual ~FieldCache();
		LUCENE_INTERFACE(FieldCache);
	
	public:
		/// Indicator for StringIndex values in the cache.
		/// NOTE: the value assigned to this constant must not be the same as any of those in SortField
		static const int32_t STRING_INDEX;
	
	public:
		/// The cache used internally by sorting and range query classes.
		static FieldCachePtr DEFAULT();
		
		/// The default parser for byte values, which are encoded by StringUtils::toInt
		static ByteParserPtr DEFAULT_BYTE_PARSER();
		
		/// The default parser for int values, which are encoded by StringUtils::toInt
		static IntParserPtr DEFAULT_INT_PARSER();
		
		/// The default parser for int values, which are encoded by StringUtils::toLong
		static LongParserPtr DEFAULT_LONG_PARSER();
		
		/// The default parser for double values, which are encoded by StringUtils::toDouble
		static DoubleParserPtr DEFAULT_DOUBLE_PARSER();
		
		/// A parser instance for int values encoded by {@link NumericUtils#prefixCodedToInt(String)}, 
		/// eg. when indexed via {@link NumericField}/{@link NumericTokenStream}.
		static IntParserPtr NUMERIC_UTILS_INT_PARSER();
		
		/// A parser instance for long values encoded by {@link NumericUtils#prefixCodedToLong(String)}, 
		/// eg. when indexed via {@link NumericField}/{@link NumericTokenStream}.
		static LongParserPtr NUMERIC_UTILS_LONG_PARSER();
		
		/// A parser instance for double values encoded by {@link NumericUtils}, 
		/// eg. when indexed via {@link NumericField}/{@link NumericTokenStream}.
		static DoubleParserPtr NUMERIC_UTILS_DOUBLE_PARSER();
	
		/// Checks the internal cache for an appropriate entry, and if none are found, reads the terms in 
		/// field as a single byte and returns an array of size reader.maxDoc() of the value each document
		/// has in the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the single byte values.
		/// @return The values in the given field for each document.
		virtual Collection<uint8_t> getBytes(IndexReaderPtr reader, const String& field);
		
		/// Checks the internal cache for an appropriate entry, and if none are found, reads the terms in 
		/// field as bytes and returns an array of size reader.maxDoc() of the value each document has in 
		/// the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the bytes.
		/// @param parser Computes byte for string values.
		/// @return The values in the given field for each document.
		virtual Collection<uint8_t> getBytes(IndexReaderPtr reader, const String& field, ByteParserPtr parser);
		
		/// Checks the internal cache for an appropriate entry, and if none are found, reads the terms in 
		/// field as integers and returns an array of size reader.maxDoc() of the value each document has in 
		/// the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the integers.
		/// @return The values in the given field for each document.
		virtual Collection<int32_t> getInts(IndexReaderPtr reader, const String& field);
		
		/// Checks the internal cache for an appropriate entry, and if none are found, reads the terms in 
		/// field as integers and returns an array of size reader.maxDoc() of the value each document has in 
		/// the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the integers.
		/// @param parser Computes integer for string values.
		/// @return The values in the given field for each document.
		virtual Collection<int32_t> getInts(IndexReaderPtr reader, const String& field, IntParserPtr parser);
		
		/// Checks the internal cache for an appropriate entry, and if none are found, reads the terms in 
		/// field as longs and returns an array of size reader.maxDoc() of the value each document has in 
		/// the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the longs.
		/// @return The values in the given field for each document.
		virtual Collection<int64_t> getLongs(IndexReaderPtr reader, const String& field);

		/// Checks the internal cache for an appropriate entry, and if none are found, reads the terms in 
		/// field as longs and returns an array of size reader.maxDoc() of the value each document has in 
		/// the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the longs.
		/// @param parser Computes long for string values.
		/// @return The values in the given field for each document.
		virtual Collection<int64_t> getLongs(IndexReaderPtr reader, const String& field, LongParserPtr parser);
		
		/// Checks the internal cache for an appropriate entry, and if none are found, reads the terms in 
		/// field as integers and returns an array of size reader.maxDoc() of the value each document has in 
		/// the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the doubles.
		/// @return The values in the given field for each document.
		virtual Collection<double> getDoubles(IndexReaderPtr reader, const String& field);
		
		/// Checks the internal cache for an appropriate entry, and if none are found, reads the terms in 
		/// field as doubles and returns an array of size reader.maxDoc() of the value each document has in 
		/// the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the doubles.
		/// @param parser Computes double for string values.
		/// @return The values in the given field for each document.
		virtual Collection<double> getDoubles(IndexReaderPtr reader, const String& field, DoubleParserPtr parser);
		
		/// Checks the internal cache for an appropriate entry, and if none are found, reads the term values in 
		/// field and returns an array of size reader.maxDoc() containing the value each document has in 
		/// the given field.
		/// @param reader Used to get field values.
		/// @param field Which field contains the strings.
		/// @return The values in the given field for each document.
		virtual Collection<String> getStrings(IndexReaderPtr reader, const String& field);
		
		/// Checks the internal cache for an appropriate entry, and if none are found reads the term values in 
		/// field and returns an array of them in natural order, along with an array telling which element in 
		/// the term array each document uses.
		/// @param reader Used to get field values.
		/// @param field Which field contains the strings.
		/// @return Array of terms and index into the array for each document.
		virtual StringIndexPtr getStringIndex(IndexReaderPtr reader, const String& field);
		
		/// Generates an array of CacheEntry objects representing all items currently in the FieldCache.
		virtual Collection<FieldCacheEntryPtr> getCacheEntries() = 0;
		
		/// Instructs the FieldCache to forcibly expunge all entries from the underlying caches.  This is intended 
		/// only to be used for test methods as a way to ensure a known base state of the Cache.  It should not be 
		/// relied on for "Cache maintenance" in general application code.
		virtual void purgeAllCaches() = 0;
		
		/// Drops all cache entries associated with this reader.  NOTE: this reader must precisely match the reader 
		/// that the cache entry is keyed on. If you pass a top-level reader, it usually will have no effect as
		/// Lucene now caches at the segment reader level.
		virtual void purge(IndexReaderPtr r) = 0;
		
		/// If non-null, FieldCacheImpl will warn whenever entries are created that are not sane according to 
		/// {@link FieldCacheSanityChecker}.
		virtual void setInfoStream(InfoStreamPtr stream);
		
		/// @see #setInfoStream
		virtual InfoStreamPtr getInfoStream();
	};
	
	class LPPAPI CreationPlaceholder : public LuceneObject
	{
	public:
		CreationPlaceholder();
		virtual ~CreationPlaceholder();
	
		LUCENE_CLASS(CreationPlaceholder);
	
	public:
		LuceneObjectPtr value;
	};
	
	/// Stores term text values and document ordering data.
	class LPPAPI StringIndex : public LuceneObject
	{
	public:
		StringIndex(Collection<int32_t> values, Collection<String> lookup);
		virtual ~StringIndex();
	
		LUCENE_CLASS(StringIndex);
	
	public:
		/// All the term values, in natural order.
		Collection<String> lookup;
		
		/// For each document, an index into the lookup array.
		Collection<int32_t> order;
		
	public:
		int32_t binarySearchLookup(const String& key);
	};
	
	/// Marker interface as super-interface to all parsers.  It is used to specify a custom parser to {@link
	/// SortField#SortField(String, Parser)}.
	class LPPAPI Parser : public LuceneObject
	{
	public:
		virtual ~Parser();
		LUCENE_CLASS(Parser);
	};
	
	/// Interface to parse bytes from document fields.
	/// @see FieldCache#getBytes(IndexReaderPtr, String, ByteParserPtr)
	class LPPAPI ByteParser : public Parser
	{
	public:
		virtual ~ByteParser();
		LUCENE_CLASS(ByteParser);
	
	public:
		/// Return a single Byte representation of this field's value.
		virtual uint8_t parseByte(const String& string);
	};
	
	/// @see FieldCache#DEFAULT_BYTE_PARSER()
	class LPPAPI DefaultByteParser : public ByteParser
	{
	public:
		virtual ~DefaultByteParser();
		LUCENE_CLASS(DefaultByteParser);
	
	public:
		virtual uint8_t parseByte(const String& string);
		virtual String toString();
	};
	
	/// Interface to parse ints from document fields.
	/// @see FieldCache#getInts(IndexReaderPtr, String, IntParserPtr)
	class LPPAPI IntParser : public Parser
	{
	public:
		virtual ~IntParser();
		LUCENE_CLASS(IntParser);
	
	public:
		/// Return a integer representation of this field's value.
		virtual int32_t parseInt(const String& string);
	};
	
	/// @see FieldCache#DEFAULT_INT_PARSER()
	class LPPAPI DefaultIntParser : public IntParser
	{
	public:
		virtual ~DefaultIntParser();
		LUCENE_CLASS(DefaultIntParser);
	
	public:
		virtual int32_t parseInt(const String& string);
		virtual String toString();
	};
	
	/// @see FieldCache#NUMERIC_UTILS_INT_PARSER()
	class LPPAPI NumericUtilsIntParser : public IntParser
	{
	public:
		virtual ~NumericUtilsIntParser();
		LUCENE_CLASS(NumericUtilsIntParser);
	
	public:
		virtual int32_t parseInt(const String& string);
		virtual String toString();
	};
	
	/// Interface to parse longs from document fields.
	/// @see FieldCache#getLongs(IndexReaderPtr, String, LongParserPtr)
	class LPPAPI LongParser : public Parser
	{
	public:
		virtual ~LongParser();
		LUCENE_CLASS(LongParser);
	
	public:
		/// Return a long representation of this field's value.
		virtual int64_t parseLong(const String& string);
	};
	
	/// @see FieldCache#DEFAULT_LONG_PARSER()
	class LPPAPI DefaultLongParser : public LongParser
	{
	public:
		virtual ~DefaultLongParser();
		LUCENE_CLASS(DefaultLongParser);
	
	public:
		virtual int64_t parseLong(const String& string);
		virtual String toString();
	};
	
	/// @see FieldCache#NUMERIC_UTILS_LONG_PARSER()
	class LPPAPI NumericUtilsLongParser : public LongParser
	{
	public:
		virtual ~NumericUtilsLongParser();
		LUCENE_CLASS(NumericUtilsLongParser);
	
	public:
		virtual int64_t parseLong(const String& string);
		virtual String toString();
	};
	
	/// Interface to parse doubles from document fields.
	/// @see FieldCache#getDoubles(IndexReaderPtr, String, DoubleParserPtr)
	class LPPAPI DoubleParser : public Parser
	{
	public:
		virtual ~DoubleParser();
		LUCENE_CLASS(DoubleParser);
	
	public:
		/// Return a double representation of this field's value.
		virtual double parseDouble(const String& string);
	};
	
	/// @see FieldCache#DEFAULT_DOUBLE_PARSER()
	class LPPAPI DefaultDoubleParser : public DoubleParser
	{
	public:
		virtual ~DefaultDoubleParser();
		LUCENE_CLASS(DefaultDoubleParser);
	
	public:
		virtual double parseDouble(const String& string);
		virtual String toString();
	};
	
	/// @see FieldCache#NUMERIC_UTILS_DOUBLE_PARSER()
	class LPPAPI NumericUtilsDoubleParser : public DoubleParser
	{
	public:
		virtual ~NumericUtilsDoubleParser();
		LUCENE_CLASS(NumericUtilsDoubleParser);
	
	public:
		virtual double parseDouble(const String& string);
		virtual String toString();
	};
	
	/// A unique Identifier/Description for each item in the FieldCache.  Can be useful for logging/debugging.
	class LPPAPI FieldCacheEntry : public LuceneObject
	{
	public:
		virtual ~FieldCacheEntry();
		LUCENE_CLASS(FieldCacheEntry);
	
	public:
		virtual LuceneObjectPtr getReaderKey() = 0;
		virtual String getFieldName() = 0;
		virtual String getCacheType() = 0;
		virtual LuceneObjectPtr getCustom() = 0;
		virtual LuceneObjectPtr getValue() = 0;
		
		virtual String toString();
	};
}
