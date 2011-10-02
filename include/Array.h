/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ARRAY_H
#define ARRAY_H

#include <cstring>
#include "LuceneSync.h"

namespace Lucene
{
    template <typename TYPE> 
    class ArrayData
    {
    public:
        ArrayData(int32_t size)
        {
            data = NULL;
            resize(size);
        }
        
        ~ArrayData()
        {
            resize(0);
        }
    
    public:
        TYPE* data;
        int32_t size;
    
    public:
        void resize(int32_t size)
        {
            if (size == 0)
            {
                FreeMemory(data);
                data = NULL;
            }
            else if (data == NULL)
                data = (TYPE*)AllocMemory(size * sizeof(TYPE));
            else
                data = (TYPE*)ReallocMemory(data, size * sizeof(TYPE));
            this->size = size;
        }
    };
    
    /// Utility template class to handle sharable arrays of simple data types
    template <typename TYPE>
    class Array : public LuceneSync
    {
    public:
        typedef Array<TYPE> this_type;
        typedef ArrayData<TYPE> array_type;

        Array()
        {
            array = NULL;
        }
        
    protected:
        //LucenePtr<array_type> container;
        //array_type* array;
        TYPE* array;
        int32_t arraySize;
        
    public:
        static this_type newInstance(int32_t size)
        {
            this_type instance;
            //instance.container = Lucene::newInstance<array_type>(size);
            //instance.array = instance.container.get();
            instance.array = (TYPE*)GC_MALLOC(size * sizeof(TYPE));
            instance.arraySize = size;
            return instance;
        }
        
        void reset()
        {
            resize(0);
        }
        
        void resize(int32_t size)
        {
            /*if (size == 0)
                container.reset();
            else if (!container)
                container = Lucene::newInstance<array_type>(size);
            else
                container->resize(size);
            array = container.get();*/
            array = (TYPE*)GC_REALLOC(array, size * sizeof(TYPE));
            arraySize = size;
        }
                
        TYPE* get() const
        {
            //return array->data;
            return array;
        }
        
        int32_t size() const
        {
            //return array->size;
            return arraySize;
        }
        
        bool equals(const this_type& other) const
        {
            //if (array->size != other.array->size)
            //    return false;
            //return (std::memcmp(array->data, other.array->data, array->size) == 0);
            if (arraySize != other.arraySize)
                return false;
            return (std::memcmp(array, other.array, arraySize) == 0);
        }
        
        int32_t hashCode() const
        {
            return (int32_t)(int64_t)array;
        }
        
        TYPE& operator[] (int32_t i) const
        {
            //BOOST_ASSERT(i >= 0 && i < array->size);
            //return array->data[i];
            BOOST_ASSERT(i >= 0 && i < arraySize);
            return array[i];
        }
        
        operator bool () const
        {
            //return container;
            return array != NULL;
        }
        
        bool operator! () const
        {
            //return !container;
            return array == NULL;
        }
        
        bool operator== (const Array<TYPE>& other)
        {
            //return (container == other.container);
            return array == other.array;
        }
        
        bool operator!= (const Array<TYPE>& other)
        {
            //return (container != other.container);
            return array != other.array;
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
        return (value1.hashCode() == value2.hashCode());
    }
}

#endif
