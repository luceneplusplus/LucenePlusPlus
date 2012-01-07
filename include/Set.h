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
    class Set : public set_ptr<T>, public LuceneSync
    {
    public:
        typedef typename T::value_type value_type;

        Set(single_container<T>* p = 0) : set_ptr<T>(p)
        {
        }

        Set(const Set& rhs) : set_ptr<T>(rhs)
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
            return (int32_t)(int64_t)this->get();
        }

        int32_t size() const
        {
            return (int32_t)set_ptr<T>::size();
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

        HashSet(single_container<set_type>* p = 0) : Set<set_type>(p)
        {
        }

        HashSet(const HashSet& rhs) : Set<set_type>(rhs)
        {
        }

        static HashSet<VALUE, HASH, EQUAL> newInstance()
        {
            return HashSet<VALUE, HASH, EQUAL>(new(get_gc()) single_container<set_type>());
        }

        static HashSet<VALUE, HASH, EQUAL> newStaticInstance()
        {
            return HashSet<VALUE, HASH, EQUAL>(new(get_static_gc()) single_container<set_type>());
        }

        template <class ITER>
        static HashSet<VALUE, HASH, EQUAL> newInstance(ITER first, ITER last)
        {
            HashSet<VALUE, HASH, EQUAL> container(new(get_gc()) single_container<set_type>());
            container.insert(first, last);
            return container;
        }

        template <class ITER>
        static HashSet<VALUE, HASH, EQUAL> newStaticInstance(ITER first, ITER last)
        {
            HashSet<VALUE, HASH, EQUAL> container(new(get_static_gc()) single_container<set_type>());
            container.insert(first, last);
            return container;
        }
    };

    template < class VALUE, class COMPARE = std::less<VALUE> >
    class SortedSet : public Set < std::set<VALUE, COMPARE> >
    {
    public:
        typedef std::set<VALUE, COMPARE> set_type;
        typedef typename set_type::value_type value_type;
        typedef typename set_type::key_compare key_compare;

        SortedSet(single_container<set_type>* p = 0) : Set<set_type>(p)
        {
        }

        SortedSet(const SortedSet& rhs) : Set<set_type>(rhs)
        {
        }

        static SortedSet<VALUE, COMPARE> newInstance()
        {
            return SortedSet<VALUE, COMPARE>(new(get_gc()) single_container<set_type>());
        }

        static SortedSet<VALUE, COMPARE> newStaticInstance()
        {
            return SortedSet<VALUE, COMPARE>(new(get_static_gc()) single_container<set_type>());
        }

        template <class ITER>
        static SortedSet<VALUE, COMPARE> newInstance(ITER first, ITER last)
        {
            SortedSet<VALUE, COMPARE> container(new(get_gc()) single_container<set_type>());
            container.insert(first, last);
            return container;
        }

        template <class ITER>
        static SortedSet<VALUE, COMPARE> newStaticInstance(ITER first, ITER last)
        {
            SortedSet<VALUE, COMPARE> container(new(get_static_gc()) single_container<set_type>());
            container.insert(first, last);
            return container;
        }
    };
}

#endif
