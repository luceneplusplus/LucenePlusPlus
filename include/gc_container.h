/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#ifndef _LUTZE_GC_CONTAINER
#define _LUTZE_GC_CONTAINER

#include "gc.h"

namespace lutze
{
    template <class T>
    class container_ptr : public gc_ptr<T>
    {
    public:
        typedef T container_type;
        typedef typename T::size_type size_type;
        typedef typename T::iterator iterator;
        typedef typename T::const_iterator const_iterator;

        container_ptr(T* p = 0) : gc_ptr<T>(p)
        {
        }

        container_ptr(const container_ptr& rhs) : gc_ptr<T>(rhs.px)
        {
        }

        iterator begin()
        {
            return this->px->begin();
        }

        const_iterator begin() const
        {
            return this->px->begin();
        }

        void clear()
        {
            this->px->clear();
        }

        bool empty() const
        {
            return this->px->empty();
        }

        iterator end()
        {
            return this->px->end();
        }

        const_iterator end() const
        {
            return this->px->end();
        }

        size_type size() const
        {
            return this->px->size();
        }
    };

    template <class T>
    class single_container : public T, public gc_object
    {
    protected:
        virtual void mark_members(gc* gc) const
        {
            for (typename T::const_iterator obj = this->begin(), last = this->end(); obj != last; ++obj)
                gc->mark(*obj);
        }
    };

    template <class T>
    class vector_ptr : public container_ptr< single_container<T> >
    {
    public:
        typedef T vector_type;
        typedef typename T::size_type size_type;
        typedef typename T::value_type value_type;
        typedef typename T::reference reference;
        typedef typename T::const_reference const_reference;
        typedef typename T::iterator iterator;
        typedef typename T::const_iterator const_iterator;
        typedef typename T::reverse_iterator reverse_iterator;
        typedef typename T::const_reverse_iterator const_reverse_iterator;

        vector_ptr(single_container<T>* p = 0) : container_ptr< single_container<T> >(p)
        {
        }

        vector_ptr(const vector_ptr& rhs) : container_ptr< single_container<T> >(rhs)
        {
        }

        template <class Iter>
        void assign(Iter first, Iter last)
        {
            this->px->assign(first, last);
        }

        void assign(size_type n, const value_type& x)
        {
            this->px->assign(n, x);
        }

        reference at(size_type n)
        {
            return this->px->at(n);
        }

        const_reference at(size_type n) const
        {
            return this->px->at(n);
        }

        reference back()
        {
            return this->px->back();
        }

        const_reference back() const
        {
            return this->px->back();
        }

        iterator erase(iterator position)
        {
            return this->px->erase(position);
        }

        iterator erase(iterator first, iterator last)
        {
            return this->px->erase(first, last);
        }

        reference front()
        {
            return this->px->front();
        }

        const_reference front() const
        {
            return this->px->front();
        }

        iterator insert(iterator position, const value_type& x)
        {
            return this->px->insert(position, x);
        }

        void insert(iterator position, size_type n, const value_type& x)
        {
            return this->px->insert(position, n, x);
        }

        template <class Iter>
        void insert(iterator position, Iter first, Iter last)
        {
            this->px->insert(position, first, last);
        }

        reference operator [] (size_type n)
        {
            return (*this->px)[n];
        }

        const_reference operator [] (size_type n) const
        {
            return (*this->px)[n];
        }

        void pop_back()
        {
            this->px->pop_back();
        }

        void push_back(const value_type& x)
        {
            this->px->push_back(x);
        }

        void reserve(size_type n)
        {
            this->px->reserve(n);
        }

        void resize(size_type n, const value_type& x = value_type())
        {
            this->px->resize(n, x);
        }

        reverse_iterator rbegin()
        {
            return this->px->rbegin();
        }

        const_reverse_iterator rbegin() const
        {
            return this->px->rbegin();
        }

        reverse_iterator rend()
        {
            return this->px->rend();
        }

        const_reverse_iterator rend() const
        {
            return this->px->rend();
        }
    };

    template <class T>
    vector_ptr<T> new_vector_placeholder(gc& gc, typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        vector_ptr<T> container(new(gc) single_container<T>());
        container.resize(n, x);
        return container;
    }

    template <class T>
    vector_ptr<T> new_vector(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_vector_placeholder<T>(get_gc(), n, x);
    }

    template <class T>
    vector_ptr<T> new_static_vector(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_vector_placeholder<T>(get_static_gc(), n, x);
    }

    template <class T, class Iter>
    vector_ptr<T> new_vector_placeholder(gc& gc, Iter first, Iter last)
    {
        vector_ptr<T> container(new(gc) single_container<T>());
        container.assign(first, last);
        return container;
    }

    template <class T, class Iter>
    vector_ptr<T> new_vector(Iter first, Iter last)
    {
        return new_vector_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    vector_ptr<T> new_static_vector(Iter first, Iter last)
    {
        return new_vector_placeholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    class deque_ptr : public vector_ptr<T>
    {
    public:
        typedef T deque_type;
        typedef typename T::value_type value_type;

        deque_ptr(single_container<T>* p = 0) : vector_ptr<T>(p)
        {
        }

        deque_ptr(const deque_ptr& rhs) : vector_ptr<T>(rhs)
        {
        }

        void pop_front()
        {
            this->px->pop_front();
        }

        void push_front(const value_type& x)
        {
            this->px->push_front(x);
        }
    };

    template <class T>
    deque_ptr<T> new_deque_placeholder(gc& gc, typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        deque_ptr<T> container(new(gc) single_container<T>());
        container.resize(n, x);
        return container;
    }

    template <class T>
    deque_ptr<T> new_deque(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_deque_placeholder<T>(get_gc(), n, x);
    }

    template <class T>
    deque_ptr<T> new_static_deque(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_deque_placeholder<T>(get_static_gc(), n, x);
    }

    template <class T, class Iter>
    deque_ptr<T> new_deque_placeholder(gc& gc, Iter first, Iter last)
    {
        deque_ptr<T> container(new(gc) single_container<T>());
        container.assign(first, last);
        return container;
    }

    template <class T, class Iter>
    deque_ptr<T> new_deque(Iter first, Iter last)
    {
        return new_deque_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    deque_ptr<T> new_static_deque(Iter first, Iter last)
    {
        return new_deque_placeholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    class list_ptr : public deque_ptr<T>
    {
    public:
        typedef T list_type;
        typedef typename T::value_type value_type;
        typedef typename T::iterator iterator;
        typedef typename T::const_iterator const_iterator;

        list_ptr(single_container<T>* p = 0) : deque_ptr<T>(p)
        {
        }

        list_ptr(const list_ptr& rhs) : deque_ptr<T>(rhs)
        {
        }

        void merge(list_ptr& x)
        {
            this->px->merge(*x);
        }

        template <class Comp>
        void merge(list_ptr& x, Comp comp)
        {
            this->px->merge(*x, comp);
        }

        void remove(const value_type& x)
        {
            this->px->remove(x);
        }

        template <class Pred>
        void remove_if(const value_type& x, Pred pred)
        {
            this->px->remove_if(x, pred);
        }

        void sort()
        {
            this->px->sort();
        }

        template <class Comp>
        void sort(Comp comp)
        {
            this->px->sort(comp);
        }

        void splice(iterator position, list_ptr& x)
        {
            this->px->splice(position, *x);
        }

        void splice(iterator position, list_ptr& x, iterator i)
        {
            this->px->splice(position, *x, i);
        }

        void splice(iterator position, list_ptr& x, iterator first, iterator last)
        {
            this->px->splice(position, *x, first, last);
        }

        void unique()
        {
            this->px->unique();
        }

        template <class Pred>
        void unique(Pred pred)
        {
            this->px->unique(pred);
        }
    };

    template <class T>
    list_ptr<T> new_list_placeholder(gc& gc, typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        list_ptr<T> container(new(gc) single_container<T>());
        container.resize(n, x);
        return container;
    }

    template <class T>
    list_ptr<T> new_list(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_list_placeholder<T>(get_gc(), n, x);
    }

    template <class T>
    list_ptr<T> new_static_list(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_list_placeholder<T>(get_static_gc(), n, x);
    }

    template <class T, class Iter>
    list_ptr<T> new_list_placeholder(gc& gc, Iter first, Iter last)
    {
        list_ptr<T> container(new(gc) single_container<T>());
        container.assign(first, last);
        return container;
    }

    template <class T, class Iter>
    list_ptr<T> new_list(Iter first, Iter last)
    {
        return new_list_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    list_ptr<T> new_static_list(Iter first, Iter last)
    {
        return new_list_placeholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    class set_ptr : public container_ptr< single_container<T> >
    {
    public:
        typedef T set_type;
        typedef typename T::size_type size_type;
        typedef typename T::value_type value_type;
        typedef typename T::reference reference;
        typedef typename T::const_reference const_reference;
        typedef typename T::iterator iterator;
        typedef typename T::const_iterator const_iterator;

        set_ptr(single_container<T>* p = 0) : container_ptr< single_container<T> >(p)
        {
        }

        set_ptr(const set_ptr& rhs) : container_ptr< single_container<T> >(rhs)
        {
        }

        std::pair<iterator, iterator> equal_range(const value_type& x) const
        {
            return this->px->equal_range(x);
        }

        void erase(iterator position)
        {
            this->px->erase(position);
        }

        size_type erase(const value_type& x)
        {
            return this->px->erase(x);
        }

        void erase(iterator first, iterator last)
        {
            this->px->erase(first, last);
        }

        iterator find(const value_type& x) const
        {
            return this->px->find(x);
        }

        std::pair<iterator, bool> insert(const value_type& x)
        {
            return this->px->insert(x);
        }

        iterator insert(iterator position, const value_type& x)
        {
            return this->px->insert(position, x);
        }

        template <class Iter>
        void insert(Iter first, Iter last)
        {
            this->px->insert(first, last);
        }

        iterator lower_bound(const value_type& x) const
        {
            return this->px->lower_bound(x);
        }

        iterator upper_bound(const value_type& x) const
        {
            return this->px->upper_bound(x);
        }
    };

    template <class T>
    set_ptr<T> new_set_placeholder(gc& gc)
    {
        return set_ptr<T>(new(gc) single_container<T>());
    }

    template <class T>
    set_ptr<T> new_set()
    {
        return new_set_placeholder<T>(get_gc());
    }

    template <class T>
    set_ptr<T> new_static_set()
    {
        return new_set_placeholder<T>(get_static_gc());
    }

    template <class T, class Iter>
    set_ptr<T> new_set_placeholder(gc& gc, Iter first, Iter last)
    {
        set_ptr<T> container(new(gc) single_container<T>());
        container.insert(first, last);
        return container;
    }

    template <class T, class Iter>
    set_ptr<T> new_set(Iter first, Iter last)
    {
        return new_set_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    set_ptr<T> new_static_set(Iter first, Iter last)
    {
        return new_set_placeholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    class pair_container : public T, public gc_object
    {
    protected:
        virtual void mark_members(gc* gc) const
        {
            for (typename T::const_iterator obj = this->begin(), last = this->end(); obj != last; ++obj)
            {
                gc->mark(obj->first);
                gc->mark(obj->second);
            }
        }
    };

    template <class T>
    class map_ptr : public container_ptr< pair_container<T> >
    {
    public:
        typedef T map_type;
        typedef typename T::size_type size_type;
        typedef typename T::key_type key_type;
        typedef typename T::mapped_type mapped_type;
        typedef typename T::value_type value_type;
        typedef typename T::iterator iterator;
        typedef typename T::const_iterator const_iterator;

        map_ptr(pair_container<T>* p = 0) : container_ptr< pair_container<T> >(p)
        {
        }

        map_ptr(const map_ptr& rhs) : container_ptr< pair_container<T> >(rhs)
        {
        }

        size_type count(const key_type& x) const
        {
            return this->px->count(x);
        }

        std::pair<iterator, iterator> equal_range(const key_type& x)
        {
            return this->px->equal_range(x);
        }

        std::pair<const_iterator, const_iterator> equal_range(const key_type& x) const
        {
            return this->px->equal_range(x);
        }

        void erase(iterator position)
        {
            this->px->erase(position);
        }

        size_type erase(const key_type& x)
        {
            return this->px->erase(x);
        }

        void erase(iterator first, iterator last)
        {
            this->px->erase(first, last);
        }

        iterator find(const key_type& x)
        {
            return this->px->find(x);
        }

        const_iterator find(const key_type& x) const
        {
            return this->px->find(x);
        }

        std::pair<iterator, bool> insert(const value_type& x)
        {
            return this->px->insert(x);
        }

        iterator insert(iterator position, const value_type& x)
        {
            return this->px->insert(position, x);
        }

        template <class Iter>
        void insert(Iter first, Iter last)
        {
            this->px->insert(first, last);
        }

        iterator lower_bound(const key_type& x)
        {
            return this->px->lower_bound(x);
        }

        const_iterator lower_bound(const key_type& x) const
        {
            return this->px->lower_bound(x);
        }

        iterator upper_bound(const key_type& x)
        {
            return this->px->upper_bound(x);
        }

        mapped_type& operator [] (const key_type &x)
        {
            return (*this->px)[x];
        }

        const_iterator upper_bound(const key_type& x) const
        {
            return this->px->upper_bound(x);
        }
    };

    template <class T>
    map_ptr<T> new_map_placeholder(gc& gc)
    {
        return map_ptr<T>(new(gc) pair_container<T>());
    }

    template <class T>
    map_ptr<T> new_map()
    {
        return new_map_placeholder<T>(get_gc());
    }

    template <class T>
    map_ptr<T> new_static_map()
    {
        return new_map_placeholder<T>(get_static_gc());
    }

    template <class T, class Iter>
    map_ptr<T> new_map_placeholder(gc& gc, Iter first, Iter last)
    {
        map_ptr<T> container(new(gc) pair_container<T>());
        container.insert(first, last);
        return container;
    }

    template <class T, class Iter>
    map_ptr<T> new_map(Iter first, Iter last)
    {
        return new_map_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    map_ptr<T> new_static_map(Iter first, Iter last)
    {
        return new_map_placeholder<T>(get_static_gc(), first, last);
    }
}

#endif
