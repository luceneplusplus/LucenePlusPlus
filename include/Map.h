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
        typedef typename T::iterator iterator;
        typedef typename T::const_iterator const_iterator;

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

    template < class KEY, class VALUE, class HASH = boost::hash<KEY>, class EQUAL = std::equal_to<KEY> >
    class HashMap : public Map < boost::unordered_map<KEY, VALUE, HASH, EQUAL> >
    {
    public:
        typedef boost::unordered_map<KEY, VALUE, HASH, EQUAL> map_type;
        typedef typename map_type::key_type key_type;
        typedef typename map_type::mapped_type mapped_type;
        typedef typename map_type::hasher hasher;
        typedef typename map_type::key_equal key_equal;

        HashMap(gc_container<map_type, key_type, mapped_type>* p = 0) : Map<map_type>(p)
        {
        }

        HashMap(const HashMap& rhs) : Map<map_type>(rhs)
        {
        }

        static HashMap<KEY, VALUE, HASH, EQUAL> newInstance()
        {
            return HashMap<KEY, VALUE, HASH, EQUAL>(new(get_gc()) gc_container<boost::unordered_map<KEY, VALUE, HASH, EQUAL>, KEY, VALUE>());
        }

        static HashMap<KEY, VALUE, HASH, EQUAL> newStaticInstance()
        {
            return HashMap<KEY, VALUE, HASH, EQUAL>(new(get_static_gc()) gc_container<boost::unordered_map<KEY, VALUE, HASH, EQUAL>, KEY, VALUE>());
        }

        template <class ITER>
        static HashMap<KEY, VALUE, HASH, EQUAL> newInstance(ITER first, ITER last)
        {
            HashMap<KEY, VALUE, HASH, EQUAL> container(new(get_gc()) gc_container<boost::unordered_map<KEY, VALUE, HASH, EQUAL>, KEY, VALUE>());
            container.insert(first, last);
            return container;
        }

        template <class ITER>
        static HashMap<KEY, VALUE, HASH, EQUAL> newStaticInstance(ITER first, ITER last)
        {
            HashMap<KEY, VALUE, HASH, EQUAL> container(new(get_static_gc()) gc_container<boost::unordered_map<KEY, VALUE, HASH, EQUAL>, KEY, VALUE>());
            container.insert(first, last);
            return container;
        }
    };

    // todo
    // template <class KEY, class VALUE, class HASH, class EQUAL>
    // HashMap<KEY, VALUE, HASH, EQUAL> newHashMapPlaceholder(gc& gc)
    // {
    //     return HashMap<KEY, VALUE, HASH, EQUAL>(new(gc) gc_container<boost::unordered_map<KEY, VALUE, HASH, EQUAL>, KEY, VALUE>());
    // }

    // template <class T>
    // HashMap<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal> newHashMap()
    // {
    //     return newHashMapPlaceholder<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal>(get_gc());
    // }

    // template <class T>
    // HashMap<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal> newStaticHashMap()
    // {
    //     return newHashMapPlaceholder<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal>(get_static_gc());
    // }

    // template <class KEY, class VALUE, class HASH, class EQUAL, class ITER>
    // HashMap<KEY, VALUE, HASH, EQUAL> newHashMapPlaceholder(gc& gc, ITER first, ITER last)
    // {
    //     HashMap<KEY, VALUE, HASH, EQUAL> container(new(gc) gc_container<boost::unordered_map<KEY, VALUE, HASH, EQUAL>, KEY, VALUE>());
    //     container.insert(first, last);
    //     return container;
    // }

    // template <class T, class ITER>
    // HashMap<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal> newHashMap(ITER first, ITER last)
    // {
    //     return newHashMapPlaceholder<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal>(get_gc(), first, last);
    // }

    // template <class T, class ITER>
    // HashMap<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal> newStaticHashMap(ITER first, ITER last)
    // {
    //     return newHashMapPlaceholder<typename T::key_type, typename T::mapped_type, typename T::hasher, typename T::key_equal>(get_static_gc(), first, last);
    // }

    template < class KEY, class VALUE, class COMPARE = std::less<KEY> >
    class SortedMap : public Map < std::map<KEY, VALUE, COMPARE> >
    {
    public:
        typedef std::map<KEY, VALUE, COMPARE> map_type;
        typedef typename map_type::key_type key_type;
        typedef typename map_type::mapped_type mapped_type;
        typedef typename map_type::key_compare key_compare;

        SortedMap(gc_container<map_type, key_type, mapped_type>* p = 0) : Map<map_type>(p)
        {
        }

        SortedMap(const SortedMap& rhs) : Map<map_type>(rhs)
        {
        }

        static SortedMap<KEY, VALUE, COMPARE> newInstance()
        {
            return SortedMap<KEY, VALUE, COMPARE>(new(get_gc()) gc_container<std::map<KEY, VALUE, COMPARE>, KEY, VALUE>());
        }

        static SortedMap<KEY, VALUE, COMPARE> newStaticInstance()
        {
            return SortedMap<KEY, VALUE, COMPARE>(new(get_static_gc()) gc_container<std::map<KEY, VALUE, COMPARE>, KEY, VALUE>());
        }

        template <class ITER>
        static SortedMap<KEY, VALUE, COMPARE> newInstance(ITER first, ITER last)
        {
            SortedMap<KEY, VALUE, COMPARE> container(new(get_gc()) gc_container<std::map<KEY, VALUE, COMPARE>, KEY, VALUE>());
            container.insert(first, last);
            return container;
        }

        template <class ITER>
        static SortedMap<KEY, VALUE, COMPARE> newStaticInstance(ITER first, ITER last)
        {
            SortedMap<KEY, VALUE, COMPARE> container(new(get_static_gc()) gc_container<std::map<KEY, VALUE, COMPARE>, KEY, VALUE>());
            container.insert(first, last);
            return container;
        }
    };

    // todo
    // template <class KEY, class VALUE, class COMPARE>
    // SortedMap<KEY, VALUE, COMPARE> newSortedMapPlaceholder(gc& gc)
    // {
    //     return SortedMap<KEY, VALUE, COMPARE>(new(gc) gc_container<std::map<KEY, VALUE, COMPARE>, KEY, VALUE>());
    // }

    // template <class T>
    // SortedMap<typename T::key_type, typename T::mapped_type, typename T::key_compare> newSortedMap()
    // {
    //     return newSortedMapPlaceholder<typename T::key_type, typename T::mapped_type, typename T::key_compare>(get_gc());
    // }

    // template <class T>
    // SortedMap<typename T::key_type, typename T::mapped_type, typename T::key_compare> newStaticSortedMap()
    // {
    //     return newSortedMapPlaceholder<typename T::key_type, typename T::mapped_type, typename T::key_compare>(get_static_gc());
    // }

    // template <class KEY, class VALUE, class COMPARE, class ITER>
    // HashMap<KEY, VALUE, COMPARE> newSortedMapPlaceholder(gc& gc, ITER first, ITER last)
    // {
    //     SortedMap<KEY, VALUE, COMPARE> container(new(gc) gc_container<std::map<KEY, VALUE, COMPARE>, KEY, VALUE>());
    //     container.insert(first, last);
    //     return container;
    // }

    // template <class T, class ITER>
    // SortedMap<typename T::key_type, typename T::mapped_type, typename T::key_compare> newSortedMap(ITER first, ITER last)
    // {
    //     return newSortedMapPlaceholder<typename T::key_type, typename T::mapped_type, typename T::key_compare>(get_gc(), first, last);
    // }

    // template <class T, class ITER>
    // SortedMap<typename T::key_type, typename T::mapped_type, typename T::key_compare > newStaticSortedMap(ITER first, ITER last)
    // {
    //     return newSortedMapPlaceholder<typename T::key_type, typename T::mapped_type, typename T::key_compare>(get_static_gc(), first, last);
    // }
}

#endif
