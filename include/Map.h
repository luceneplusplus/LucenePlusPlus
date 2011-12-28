/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef MAP_H
#define MAP_H

#include <map>
#include <boost/unordered_map.hpp>
#include "LuceneSync.h"

namespace Lucene
{
    template <class T>
    class Map : public map_container<T>, public LuceneSync
    {
    public:
        typedef typename T::key_type key_type;
        typedef typename T::mapped_type mapped_type;
        typedef typename map_container<T>::iterator iterator;
        typedef typename map_container<T>::const_iterator const_iterator;

        Map(gc_container<T, key_type, mapped_type>* p = 0) : map_container<T>(p)
        {
        }

        Map(const Map& rhs) : map_container<T>(rhs)
        {
        }

        void put(const key_type& key, const mapped_type& value)
        {
            (*this)[key] = value;
        }

        template <class ITER>
        void put(ITER first, ITER last)
        {
            for (; first != last; ++first)
                (*this)[first->first] = first->second;
        }

        iterator remove(iterator pos)
        {
            return this->erase(pos);
        }

        iterator remove(iterator first, iterator last)
        {
            return this->erase(first, last);
        }

        bool remove(const key_type& key)
        {
            return this->erase(key) > 0;
        }

        mapped_type get(const key_type& key) const
        {
            const_iterator findValue = this->find(key);
            return findValue == this->end() ? mapped_type() : findValue->second;
        }

        bool contains(const key_type& key) const
        {
            return this->find(key) != this->end();
        }

        int32_t hashCode()
        {
            return (int32_t)(int64_t)get();
        }

        int32_t size() const
        {
            return (int32_t)map_container<T>::size();
        }
    };

    template <class T>
    Map<T> newMapPlaceholder(gc& gc)
    {
        return Map<T>(new(gc) gc_container<T, typename T::key_type, typename T::mapped_type>());
    }

    template <class T>
    Map<T> newMap()
    {
        return newMapPlaceholder<T>(get_gc());
    }

    template <class T>
    Map<T> newStaticMap()
    {
        return newMapPlaceholder<T>(get_static_gc());
    }

    template <class T, class Iter>
    Map<T> newMapPlaceholder(gc& gc, Iter first, Iter last)
    {
        Map<T> container(Map<T>(new(gc) gc_container<T, typename T::key_type, typename T::mapped_type>()));
        container.insert(first, last);
        return container;
    }

    template <class T, class Iter>
    Map<T> newMap(Iter first, Iter last)
    {
        return newMapPlaceholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    Map<T> newStaticMap(Iter first, Iter last)
    {
        return newMapPlaceholder<T>(get_static_gc(), first, last);
    }
}

#endif
