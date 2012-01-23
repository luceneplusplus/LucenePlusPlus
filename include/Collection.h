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
    class Collection : public vector_ptr< std::vector<T> >, public LuceneSync
    {
    public:
        typedef std::vector<T> vector_type;
        typedef T value_type;
        typedef typename vector_type::iterator iterator;
        typedef typename vector_type::const_iterator const_iterator;

        Collection(single_container<vector_type>* p = 0) : vector_ptr<vector_type>(p)
        {
        }

        Collection(const Collection& rhs) : vector_ptr<vector_type>(rhs)
        {
        }

        void add(const value_type& x)
        {
            this->push_back(x);
        }

        void add(int32_t pos, const value_type& x)
        {
            this->insert(this->begin() + pos, x);
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
            return (int32_t)(int64_t)this->get();
        }

        int32_t size() const
        {
            return (int32_t)vector_ptr<vector_type>::size();
        }

        static Collection<T> newInstance(typename vector_type::size_type n = 0, const T& x = T())
        {
            single_container<vector_type>* container = new single_container<vector_type>();
            gc::get_gc().register_object(static_cast<gc_object*>(container));
            container->resize(n, x);
            return Collection<T>(container);
        }

        static Collection<T> newStaticInstance(typename vector_type::size_type n = 0, const T& x = T())
        {
            single_container<vector_type>* container = new single_container<vector_type>();
            gc::get_static_gc().register_object(static_cast<gc_object*>(container));
            container->resize(n, x);
            return Collection<T>(container);
        }

        template <class ITER>
        static Collection<T> newInstance(ITER first, ITER last)
        {
            single_container<vector_type>* container = new single_container<vector_type>();
            gc::get_gc().register_object(static_cast<gc_object*>(container));
            container->assign(first, last);
            return Collection<T>(container);
        }

        template <class ITER>
        static Collection<T> newStaticInstance(ITER first, ITER last)
        {
            single_container<vector_type>* container = new single_container<vector_type>();
            gc::get_static_gc().register_object(static_cast<gc_object*>(container));
            container->assign(first, last);
            return Collection<T>(container);
        }
    };

    template <class T>
    Collection<T> newCollection(const T& a1)
    {
        return Collection<T>::newInstance(1, a1);
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2)
    {
        Collection<T> collection(newCollection<T>(a1));
        collection.push_back(a2);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3)
    {
        Collection<T> collection(newCollection<T>(a1, a2));
        collection.push_back(a3);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3));
        collection.push_back(a4);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4, const T& a5)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4));
        collection.push_back(a5);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4, const T& a5, const T& a6)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5));
        collection.push_back(a6);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4, const T& a5, const T& a6,
                                const T& a7)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6));
        collection.push_back(a7);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4, const T& a5, const T& a6,
                                const T& a7, const T& a8)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6, a7));
        collection.push_back(a8);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4, const T& a5, const T& a6,
                                const T& a7, const T& a8, const T& a9)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6, a7, a8));
        collection.push_back(a9);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4, const T& a5, const T& a6,
                                const T& a7, const T& a8, const T& a9,
                                const T& a10)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
        collection.push_back(a10);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4, const T& a5, const T& a6,
                                const T& a7, const T& a8, const T& a9,
                                const T& a10, const T& a11)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10));
        collection.push_back(a11);
        return collection;
    }

    template <class T>
    Collection<T> newCollection(const T& a1, const T& a2, const T& a3,
                                const T& a4, const T& a5, const T& a6,
                                const T& a7, const T& a8, const T& a9,
                                const T& a10, const T& a11, const T& a12)
    {
        Collection<T> collection(newCollection<T>(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11));
        collection.push_back(a12);
        return collection;
    }
}

#endif
