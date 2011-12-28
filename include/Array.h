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
    template <class T>
    class Array : public vector_container<T>, public LuceneSync
    {
    public:
        typedef typename T::value_type value_type;
        typedef typename vector_container::iterator iterator;
        typedef typename vector_container::const_iterator const_iterator;

        Array(gc_container<T, value_type>* p = 0) : vector_container<T>(p)
        {
        }

        Array(const Array& rhs) : vector_container<T>(rhs)
        {
        }

        value_type* get() const
        {
            return &(*this)[0]
        }

        int32_t size() const
        {
            return (int32_t)vector_container<T>::size();
        }

        bool equals(const Collection& other) const
        {
            if (size() != other.size())
                return false;
            return std::equal(this->begin(), this->end(), other.begin());
        }

        int32_t hashCode() const
        {
            return (int32_t)(int64_t)vector_container<T>::get();
        }
    };

    template <class T>
    Array<T> newArrayPlaceholder(gc& gc, typename T::size_type n = 0)
    {
        Array<T> container(Array<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.resize(n);
        return container;
    }

    template <class T>
    Array<T> newArray(typename T::size_type n = 0)
    {
        return newArrayPlaceholder<T>(get_gc(), n);
    }

    template <class T>
    Array<T> newStaticArray(typename T::size_type n = 0)
    {
        return newArrayPlaceholder<T>(get_static_gc(), n);
    }

    template <class T>
    inline std::size_t hash_value(const Array<T>& value)
    {
        return (std::size_t)value.hashCode();
    }

    template <class T>
    inline bool operator== (const Array<T>& value1, const Array<T>& value2)
    {
        return value1.hashCode() == value2.hashCode();
    }
}

#endif
