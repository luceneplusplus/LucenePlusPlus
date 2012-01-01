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

    template < class VALUE, class HASH = boost::hash<VALUE>, class EQUAL = std::equal_to<VALUE> >
    class HashSet : public Set < boost::unordered_set<VALUE, HASH, EQUAL> >
    {
    public:
        typedef boost::unordered_set<VALUE, HASH, EQUAL> set_type;
        typedef typename set_type::value_type value_type;
        typedef typename set_type::hasher hasher;
        typedef typename set_type::key_equal key_equal;

        HashSet(gc_container<set_type, value_type>* p = 0) : Set<set_type>(p)
        {
        }

        HashSet(const HashSet& rhs) : Set<set_type>(rhs)
        {
        }

        static HashSet<VALUE, HASH, EQUAL> newInstance()
        {
            return HashSet<VALUE, HASH, EQUAL>(new(get_gc()) gc_container<boost::unordered_set<VALUE, HASH, EQUAL>, VALUE>());
        }

        static HashSet<VALUE, HASH, EQUAL> newStaticInstance()
        {
            return HashSet<VALUE, HASH, EQUAL>(new(get_static_gc()) gc_container<boost::unordered_set<VALUE, HASH, EQUAL>, VALUE>());
        }

        template <class ITER>
        static HashSet<VALUE, HASH, EQUAL> newInstance(ITER first, ITER last)
        {
            HashSet<VALUE, HASH, EQUAL> container(new(get_gc()) gc_container<boost::unordered_set<VALUE, HASH, EQUAL>, VALUE>());
            container.insert(first, last);
            return container;
        }

        template <class ITER>
        static HashSet<VALUE, HASH, EQUAL> newStaticInstance(ITER first, ITER last)
        {
            HashSet<VALUE, HASH, EQUAL> container(new(get_static_gc()) gc_container<boost::unordered_set<VALUE, HASH, EQUAL>, VALUE>());
            container.insert(first, last);
            return container;
        }
    };

    // todo
    // template <class VALUE, class HASH, class EQUAL>
    // HashSet<VALUE, HASH, EQUAL> newHashSetPlaceholder(gc& gc)
    // {
    //     return HashSet<VALUE, HASH, EQUAL>(new(gc) gc_container<boost::unordered_set<VALUE, HASH, EQUAL>, VALUE>());
    // }

    // template <class T>
    // HashSet<typename T::value_type, typename T::hasher, typename T::key_equal> newHashSet()
    // {
    //     return newHashSetPlaceholder<typename T::value_type, typename T::hasher, typename T::key_equal>(get_gc());
    // }

    // template <class T>
    // HashSet<typename T::value_type, typename T::hasher, typename T::key_equal> newStaticHashSet()
    // {
    //     return newHashSetPlaceholder<typename T::value_type, typename T::hasher, typename T::key_equal>(get_static_gc());
    // }

    // template <class VALUE, class HASH, class EQUAL, class ITER>
    // HashSet<VALUE, HASH, EQUAL> newHashSetPlaceholder(gc& gc, ITER first, ITER last)
    // {
    //     HashSet<VALUE, HASH, EQUAL> container(new(gc) gc_container<boost::unordered_set<VALUE, HASH, EQUAL>, VALUE>());
    //     container.insert(first, last);
    //     return container;
    // }

    // template <class T, class ITER>
    // HashSet<typename T::value_type, typename T::hasher, typename T::key_equal> newHashSet(ITER first, ITER last)
    // {
    //     return newHashSetPlaceholder<typename T::value_type, typename T::hasher, typename T::key_equal>(get_gc(), first, last);
    // }

    // template <class T, class ITER>
    // HashSet<typename T::value_type, typename T::hasher, typename T::key_equal> newStaticHashSet(ITER first, ITER last)
    // {
    //     return newHashSetPlaceholder<typename T::value_type, typename T::hasher, typename T::key_equal>(get_static_gc(), first, last);
    // }

    template < class VALUE, class COMPARE = std::less<VALUE> >
    class SortedSet : public Set < std::set<VALUE, COMPARE> >
    {
    public:
        typedef std::set<VALUE, COMPARE> set_type;
        typedef typename set_type::value_type value_type;
        typedef typename set_type::key_compare key_compare;

        SortedSet(gc_container<set_type, value_type>* p = 0) : Set<set_type>(p)
        {
        }

        SortedSet(const SortedSet& rhs) : Set<set_type>(rhs)
        {
        }

        static SortedSet<VALUE, COMPARE> newInstance()
        {
            return SortedSet<VALUE, COMPARE>(new(get_gc()) gc_container<std::set<VALUE, COMPARE>, VALUE>());
        }

        static SortedSet<VALUE, COMPARE> newStaticInstance()
        {
            return SortedSet<VALUE, COMPARE>(new(get_static_gc()) gc_container<std::set<VALUE, COMPARE>, VALUE>());
        }

        template <class ITER>
        static SortedSet<VALUE, COMPARE> newInstance(ITER first, ITER last)
        {
            SortedSet<VALUE, COMPARE> container(new(get_gc()) gc_container<std::set<VALUE, COMPARE>, VALUE>());
            container.insert(first, last);
            return container;
        }

        template <class ITER>
        static SortedSet<VALUE, COMPARE> newStaticInstance(ITER first, ITER last)
        {
            SortedSet<VALUE, COMPARE> container(new(get_static_gc()) gc_container<std::set<VALUE, COMPARE>, VALUE>());
            container.insert(first, last);
            return container;
        }
    };

    // todo
    // template <class VALUE, class COMPARE>
    // SortedSet<VALUE, COMPARE> newSortedSetPlaceholder(gc& gc)
    // {
    //     return SortedSet<VALUE, COMPARE>(new(gc) gc_container<std::set<VALUE, COMPARE>, VALUE>());
    // }

    // template <class T>
    // SortedSet<typename T::value_type, typename T::key_compare> newSortedSet()
    // {
    //     return newSortedSetPlaceholder<typename T::value_type, typename T::key_compare>(get_gc());
    // }

    // template <class T>
    // SortedSet<typename T::value_type, typename T::key_compare> newStaticSortedSet()
    // {
    //     return newSortedSetPlaceholder<typename T::value_type, typename T::key_compare>(get_static_gc());
    // }

    // template <class VALUE, class COMPARE, class ITER>
    // HashSet<VALUE, COMPARE> newSortedSetPlaceholder(gc& gc, ITER first, ITER last)
    // {
    //     SortedSet<VALUE, COMPARE> container(new(gc) gc_container<std::set<VALUE, COMPARE>, VALUE>());
    //     container.insert(first, last);
    //     return container;
    // }

    // template <class T, class ITER>
    // SortedSet<typename T::value_type, typename T::key_compare> newSortedSet(ITER first, ITER last)
    // {
    //     return newSortedSetPlaceholder<typename T::value_type, typename T::key_compare>(get_gc(), first, last);
    // }

    // template <class T, class ITER>
    // SortedSet<typename T::value_type, typename T::key_compare > newStaticSortedSet(ITER first, ITER last)
    // {
    //     return newSortedSetPlaceholder<typename T::value_type, typename T::key_compare>(get_static_gc(), first, last);
    // }
}

#endif
