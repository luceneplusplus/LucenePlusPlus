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
    class Collection : public vector_container<T>, public LuceneSync
    {
    public:
        typedef typename T::value_type value_type;
        typedef typename vector_container::iterator iterator;
        typedef typename vector_container::const_iterator const_iterator;

        Collection(gc_container<T, value_type>* p = 0) : vector_container<T>(p)
        {
        }

        Collection(const Collection& rhs) : vector_container<T>(rhs)
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
    };

    template <class T>
    Collection<T> newCollectionPlaceholder(gc& gc, typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        Collection<T> container(Collection<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.resize(n, x);
        return container;
    }

    template <class T>
    Collection<T> newCollection(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return newCollectionPlaceholder<T>(get_gc(), n, x);
    }

    template <class T>
    Collection<T> newStaticCollection(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return newCollectionPlaceholder<T>(get_static_gc(), n, x);
    }

    template <class T, class Iter>
    Collection<T> newCollectionPlaceholder(gc& gc, Iter first, Iter last)
    {
        Collection<T> container(Collection<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.assign(first, last);
        return container;
    }

    template <class T, class Iter>
    Collection<T> newCollection(Iter first, Iter last)
    {
        return newCollectionPlaceholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    Collection<T> newStaticCollection(Iter first, Iter last)
    {
        return newCollectionPlaceholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1)
    {
        Collection<T> collection(newCollection<Collection<T>::vector_type>());
        collection.push_back(a1);
        return collection;
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1, const typename T::value_type& a2)
    {
        Collection<T> collection(newCollectionAssign<Collection<T>::vector_type>(a1));
        collection.push_back(a2);
        return collection;
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3)
    {
        Collection<T> collection(newCollectionAssign<Collection<T>::vector_type>(a1, a2));
        collection.push_back(a3);
        return collection;
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                      const typename T::value_type& a4)
    {
        Collection<T> collection(newCollectionAssign<Collection<T>::vector_type>(a1, a2, a3));
        collection.push_back(a4);
        return collection;
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                      const typename T::value_type& a4, const typename T::value_type& a5)
    {
        Collection<T> collection(newCollectionAssign<Collection<T>::vector_type>(a1, a2, a3, a4));
        collection.push_back(a5);
        return collection;
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                      const typename T::value_type& a4, const typename T::value_type& a5, const typename T::value_type& a6)
    {
        Collection<T> collection(newCollectionAssign<Collection<T>::vector_type>(a1, a2, a3, a4, a5));
        collection.push_back(a6);
        return collection;
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                      const typename T::value_type& a4, const typename T::value_type& a5, const typename T::value_type& a6,
                                      const typename T::value_type& a7)
    {
        Collection<T> collection(newCollectionAssign<Collection<T>::vector_type>(a1, a2, a3, a4, a5, a6));
        collection.push_back(a7);
        return collection;
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                      const typename T::value_type& a4, const typename T::value_type& a5, const typename T::value_type& a6,
                                      const typename T::value_type& a7, const typename T::value_type& a8)
    {
        Collection<T> collection(newCollectionAssign<Collection<T>::vector_type>(a1, a2, a3, a4, a5, a6, a7));
        collection.push_back(a8);
        return collection;
    }

    template <class T>
    Collection<T> newCollectionAssign(const typename T::value_type& a1, const typename T::value_type& a2, const typename T::value_type& a3,
                                      const typename T::value_type& a4, const typename T::value_type& a5, const typename T::value_type& a6,
                                      const typename T::value_type& a7, const typename T::value_type& a8, const typename T::value_type& a9)
    {
        Collection<T> collection(newCollectionAssign<Collection<T>::vector_type>(a1, a2, a3, a4, a5, a6, a7, a8));
        collection.push_back(a9);
        return collection;
    }
}

#endif
