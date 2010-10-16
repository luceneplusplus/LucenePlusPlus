/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LuceneObject.h"

namespace Lucene
{
	template <typename TYPE> 
	class ArrayData
	{
	public:
		ArrayData(int32_t n)
		{
			array = NULL;
			resize(n);
		}
		
		~ArrayData()
		{
			resize(0);
		}
	
	public:
		TYPE* array;
		int32_t size;
	
	public:
		void resize(int32_t n)
		{
			if (n == 0)
			{
				FreeMemory(array);
				array = NULL;
			}
			else if (array == NULL)
				array = (TYPE*)AllocMemory(n * sizeof(TYPE));
			else
				array = (TYPE*)ReallocMemory(array, n * sizeof(TYPE));
			size = n;
		}
	};
	
	/// Utility template class to handle sharable arrays of simple data types
	template <typename TYPE>
	class Array : public LuceneObject
	{
	public:
		typedef Array<TYPE> this_type;
		typedef ArrayData<TYPE> array_type;
		
	protected:
		boost::shared_ptr<array_type> container;
		
	public:
		static this_type newInstance(int32_t size)
		{
			this_type instance;
			instance.container = Lucene::newInstance<array_type>(size);
			return instance;
		}
		
		void reset()
		{
			resize(0);
		}
		
		void resize(int32_t size)
		{
			if (size == 0)
				container.reset();
			else if (!container)
				container = Lucene::newInstance<array_type>(size);
			else
				container->resize(size);
		}
				
		TYPE* get() const
		{
			return container->array;
		}
		
		int32_t length() const
		{
			return container->size;
		}
		
		bool equals(const this_type& other) const
		{
			if (container->size != other.container->size)
				return false;
			return (std::memcmp(container->array, other.container->array, container->size) == 0);
		}
		
		int32_t hashCode() const
		{
			return (int32_t)(int64_t)container.get();
		}
		
		TYPE& operator[] (int32_t i) const
		{
			BOOST_ASSERT(i >= 0 && i < container->size);
			return container->array[i];
		}
		
		operator bool () const
		{
			return container;
		}
		
		bool operator! () const
		{
			return !container;
		}
		
		bool operator== (const Array<TYPE>& other)
		{
			return (container == other.container);
		}
		
		bool operator!= (const Array<TYPE>& other)
		{
			return (container != other.container);
		}
	};
	
	template <class TYPE>
	inline std::size_t hash_value(const Array<TYPE>& value)
	{
		return (std::size_t)value.hashCode();
	}

	template <class TYPE>
	inline bool operator== (const Array<TYPE>& value1, const Array<TYPE>& value2)
	{
		return value1.hashCode() == value2.hashCode();
	}
}
