/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ARRAY_H
#define ARRAY_H

#include <vector>
#include "LuceneSync.h"

namespace Lucene
{
    template <class T>
    class Array : public vector_ptr< std::vector<T> >, public LuceneSync
    {
    public:
        typedef std::vector<T> vector_type;
        typedef T value_type;
        typedef typename vector_type::iterator iterator;
        typedef typename vector_type::const_iterator const_iterator;

        Array(single_container<vector_type>* p = 0) : vector_ptr<vector_type>(p)
        {
        }

        Array(const Array& rhs) : vector_ptr<vector_type>(rhs)
        {
        }

        value_type* get() const
        {
            return vector_ptr<vector_type>::get() == NULL ? (value_type*)NULL : &(*const_cast<Array<T>*>(this))[0];
        }

        int32_t size() const
        {
            return (int32_t)vector_ptr<vector_type>::size();
        }

        bool equals(const Array& other) const
        {
            if (size() != other.size())
                return false;
            return std::equal(this->begin(), this->end(), other.begin());
        }

        int32_t hashCode() const
        {
            return (int32_t)(int64_t)vector_ptr<vector_type>::get();
        }

        static Array<T> newInstance(typename vector_type::size_type n = 0)
        {
            single_container<vector_type>* container = new single_container<vector_type>();
            gc::get_gc().register_object(static_cast<gc_object*>(container));
            container->resize(n);
            return Array<T>(container);
        }

        static Array<T> newStaticInstance(typename vector_type::size_type n = 0)
        {
            single_container<vector_type>* container = new single_container<vector_type>();
            gc::get_static_gc().register_object(static_cast<gc_object*>(container));
            container->resize(n);
            return Array<T>(container);
        }

    protected:
        virtual void mark_members(gc* gc) const
        {
            // do not mark array elements
        }
    };

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
