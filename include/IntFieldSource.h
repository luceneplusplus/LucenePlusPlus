/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "FieldCacheSource.h"
#include "DocValues.h"

namespace Lucene
{
	/// Obtains int field values from the {@link FieldCache} using getInts() and makes those values available 
	/// as other numeric types, casting as needed.
	///
	/// @see FieldCacheSource for requirements on the field.
	///
	/// NOTE: with the switch in 2.9 to segment-based searching, if {@link #getValues} is invoked with a composite 
	/// (multi-segment) reader, this can easily cause double RAM usage for the values in the FieldCache.  It's
	/// best to switch your application to pass only atomic (single segment) readers to this API.  Alternatively, 
	/// for a short-term fix, you could wrap your ValueSource using {@link MultiValueSource}, which costs more CPU 
	/// per lookup but will not consume double the FieldCache RAM.
	class LPPAPI IntFieldSource : public FieldCacheSource
	{
	public:
		/// Create a cached int field source with a specific string-to-int parser.
		IntFieldSource(const String& field, IntParserPtr parser = IntParserPtr());
		virtual ~IntFieldSource();
	
		LUCENE_CLASS(IntFieldSource);
	
	protected:
		IntParserPtr parser;
	
	public:
		virtual String description();
		virtual DocValuesPtr getCachedFieldValues(FieldCachePtr cache, const String& field, IndexReaderPtr reader);
		virtual bool cachedFieldSourceEquals(FieldCacheSourcePtr other);
		virtual int32_t cachedFieldSourceHashCode();
	};
	
	class LPPAPI IntDocValues : public DocValues
	{
	public:
		IntDocValues(IntFieldSourcePtr source, Collection<int32_t> arr);
		virtual ~IntDocValues();
	
		LUCENE_CLASS(IntDocValues);
	
	protected:
		IntFieldSourceWeakPtr _source;
		Collection<int32_t> arr;
	
	public:
		virtual double doubleVal(int32_t doc);
		virtual int32_t intVal(int32_t doc);
		virtual String toString(int32_t doc);
		virtual CollectionValue getInnerArray();
	};
}
