/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef COLLECTION_H
#define COLLECTION_H

#include <vector>
#include "LuceneSync.h"

namespace Lucene
{
    template <class T>
    class Collection : public vector_container< std::vector<T> >, public LuceneSync
    {
    public:
        typedef std::vector<T> vector_type;
        typedef T value_type;
        typedef typename vector_type::iterator iterator;
        typedef typename vector_type::const_iterator const_iterator;

        Collection(gc_container<vector_type, value_type>* p = 0) : vector_container<vector_type>(p)
        {
        }

        Collection(const Collection& rhs) : vector_container<vector_type>(rhs)
        {
        }

        void add(const value_type& x)
        {
            this->push_back(type);
        }

        void add(int32_t pos, const value_type& x)
        {
            this->insert(container->begin() + pos, x);
        }

        template <class ITER>
        void add(ITER first, ITER last)
        {
            this->insert(this->end(), first, last);
        }

        iterator remove(iterator pos)
        {
            return this->erase(pos);
        }

        iterator remove(iterator first, iterator last)
        {
            return this->erase(first, last);
        }

        void remove(const value_type& x)
        {
            this->erase(std::remove(this->begin(), this->end(), x), this->end());
        }

        template <class PRED>
        void removeIf(PRED comp)
        {
            this->erase(std::remove_if(this->begin(), this->end(), comp), this->end());
        }

        value_type removeFirst()
        {
            value_type front = this->front();
            this->erase(this->begin());
            return front;
        }

        value_type removeLast()
        {
            value_type back = this->back();
            this->pop_back();
            return back;
        }

        iterator find(const value_type& x)
        {
            return std::find(this->begin(), this->end(), x);
        }

        const_iterator find(const value_type& x) const
        {
            return std::find(this->begin(), this->end(), x);
        }

        template <class PRED>
        iterator findIf(PRED comp)
        {
            return std::find_if(this->begin(), this->end(), comp);
        }

        bool contains(const value_type& x) const
        {
            return std::find(this->begin(), this->end(), x) != this->end();
        }

        template <class PRED>
        bool containsIf(PRED comp) const
        {
            return std::find_if(this->begin(), this->end(), comp) != this->end();
        }

        bool equals(const Collection& other) const
        {
            return equals(other, std::equal_to<value_type>());
        }

        template <class PRED>
        bool equals(const Collection& other, PRED comp) const
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
            return (int32_t)vector_container<T>::size();
        }

        static Collection<T> newInstance(typename std::vector<T>::size_type n = 0, const T& x = T())
        {
            Collection<T> container(new(get_gc()) gc_container<std::vector<T>, T>());
            container.resize(n, x);
            return container;
        }

        static Collection<T> newStaticInstance(typename std::vector<T>::size_type n = 0, const T& x = T())
        {
            Collection<T> container(new(get_static_gc()) gc_container<std::vector<T>, T>());
            container.resize(n, x);
            return container;
        }

        template <class ITER>
        static Collection<T> newInstance(ITER first, ITER last)
        {
            Collection<T> container(new(get_gc()) gc_container<std::vector<T>, T>());
            container.insert(first, last);
            return container;
        }

        template <class ITER>
        static Collection<T> newStaticInstance(ITER first, ITER last)
        {
            Collection<T> container(new(get_static_gc()) gc_container<std::vector<T>, T>());
            container.insert(first, last);
            return container;
        }
    };

    // todo
    // template <class T>
    // Collection<T> newCollectionPlaceholder(gc& gc, typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    // {
    //     Collection<T> container(new(gc) gc_container<std::vector<T>, T>());
    //     container.resize(n, x);
    //     return container;
    // }

    // template <class T>
    // Collection<typename T::value_type> newCollection(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    // {
    //     return newCollectionPlaceholder<typename T::value_type>(get_gc(), n, x);
    // }

    // template <class T>
    // Collection<typename T::value_type> newStaticCollection(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    // {
    //     return newCollectionPlaceholder<typename T::value_type>(get_static_gc(), n, x);
    // }

    // template <class T, class ITER>
    // Collection<T> newCollectionPlaceholder(gc& gc, ITER first, ITER last)
    // {
    //     Collection<T> container(new(gc) gc_container<std::vector<T>, T>());
    //     container.assign(first, last);
    //     return container;
    // }

    // template <class T, class ITER>
    // Collection<typename T::value_type> newCollection(ITER first, ITER last)
    // {
    //     return newCollectionPlaceholder<typename T::value_type>(get_gc(), first, last);
    // }

    // template <class T, class ITER>
    // Collection<typename T::value_type> newStaticCollection(ITER first, ITER last)
    // {
    //     return newCollectionPlaceholder<typename T::value_type>(get_static_gc(), first, last);
    // }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1)
    {
        Collection<typename T::value_type> collection(newCollection<T>());
        collection.push_back(a1);
        return collection;
    }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1, const typename T::value_type& a2)
    {
        Collection<T> collection(newCollection<T>(a1));
        collection.push_back(a2);
        return collection;
    }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3)
    {
        Collection<T> collection(newCollection<T>(a1, a2));
        collection.push_back(a3);
        return collection;
    }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                                           const typename T::value_type& a4)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3));
        collection.push_back(a4);
        return collection;
    }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                                           const typename T::value_type& a4, const typename T::value_type& a5)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4));
        collection.push_back(a5);
        return collection;
    }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                                           const typename T::value_type& a4, const typename T::value_type& a5, const typename T::value_type& a6)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5));
        collection.push_back(a6);
        return collection;
    }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                                           const typename T::value_type& a4, const typename T::value_type& a5, const typename T::value_type& a6,
                                                           const typename T::value_type& a7)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6));
        collection.push_back(a7);
        return collection;
    }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                                           const typename T::value_type& a4, const typename T::value_type& a5, const typename T::value_type& a6,
                                                           const typename T::value_type& a7, const typename T::value_type& a8)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6, a7));
        collection.push_back(a8);
        return collection;
    }

    template <class T>
    Collection<typename T::value_type> newCollection(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                                           const typename T::value_type& a4, const typename T::value_type& a5, const typename T::value_type& a6,
                                                           const typename T::value_type& a7, const typename T::value_type& a8, const typename T::value_type& a9)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6, a7, a8));
        collection.push_back(a9);
        return collection;
    }
}

#endif
