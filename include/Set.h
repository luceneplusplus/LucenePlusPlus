/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef SET_H
#define SET_H

#include <set>
#include <boost/unordered_set.hpp>
#include "LuceneSync.h"

namespace Lucene
{
    template <class T>
    class Set : public set_container<T>, public LuceneSync
    {
    public:
        typedef typename T::value_type value_type;

        Set(gc_container<T, value_type>* p = 0) : set_container<T>(p)
        {
        }

        Set(const Set& rhs) : set_container<T>(rhs)
        {
        }

        bool add(const value_type& x)
        {
            return this->insert(x).second;
        }

        template <class ITER>
        void add(ITER first, ITER last)
        {
            this->insert(first, last);
        }

        bool remove(const value_type& x)
        {
            return this->erase(x) > 0;
        }

        bool contains(const value_type& x) const
        {
            return this->find(x) != this->end();
        }

        bool equals(const Set& other) const
        {
            return equals(other, std::equal_to<value_type>());
        }

        template <class PRED>
        bool equals(const Set& other, PRED comp) const
        {
            if (size() != other.size())
                return false;
            return std::equal(this->begin(), this->end(), other.begin(), comp);
        }

        int32_t hashCode()
        {
            return (int32_t)(int64_t)get();
        }

        int32_t size() const
        {
            return (int32_t)set_container<T>::size();
        }
    };

    template <class T>
    Set<T> newSetPlaceholder(gc& gc)
    {
        return Set<T>(new(gc) gc_container<T, typename T::value_type>());
    }

    template <class T>
    Set<T> newSet()
    {
        return newSetPlaceholder<T>(get_gc());
    }

    template <class T>
    Set<T> newStaticSet()
    {
        return newSetPlaceholder<T>(get_static_gc());
    }

    template <class T, class Iter>
    Set<T> newSetPlaceholder(gc& gc, Iter first, Iter last)
    {
        Set<T> container(Set<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.insert(first, last);
        return container;
    }

    template <class T, class Iter>
    Set<T> newSet(Iter first, Iter last)
    {
        return newSetPlaceholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    Set<T> newStaticSet(Iter first, Iter last)
    {
        return newSetPlaceholder<T>(get_static_gc(), first, last);
    }
}

#endif
