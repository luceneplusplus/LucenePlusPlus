/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

#ifndef _GC_CONTAINER
#define _GC_CONTAINER

#include "gc.h"

namespace lutze
{
    template <class C, class A = void, class B = void, class enableA = void, class enableB = void>
    class gc_container : public C, public gc_object
    {
    protected:
        virtual void mark_members(gc* gc) const
        {
        }
    };

    template <class C, class A>
    class gc_container <C, A, void, typename boost::enable_if_c< boost::is_base_of<gc_object, typename A::element_type>::value >::type> : public C, public gc_object
    {
    protected:
        virtual void mark_members(gc* gc) const
        {
            for (typename C::const_iterator elem = this->begin(), last = this->end(); elem != last; ++elem)
                gc->mark(*elem);
        }
    };

    template <class C, class A, class B>
    class gc_container <C, A, B, typename boost::enable_if_c< boost::is_base_of<gc_object, typename A::element_type>::value >::type, typename boost::enable_if_c< boost::is_pod< B >::value >::type> : public C, public gc_object
    {
    protected:
        virtual void mark_members(gc* gc) const
        {
            for (typename C::const_iterator elem = this->begin(), last = this->end(); elem != last; ++elem)
                gc->mark(elem->first);
        }
    };

    template <class C, class A, class B>
    class gc_container <C, A, B, typename boost::enable_if_c< boost::is_pod< A >::value >::type, typename boost::enable_if_c< boost::is_base_of<gc_object, typename B::element_type>::value >::type> : public C, public gc_object
    {
    protected:
        virtual void mark_members(gc* gc) const
        {
            for (typename C::const_iterator elem = this->begin(), last = this->end(); elem != last; ++elem)
                gc->mark(elem->second);
        }
    };

    template <class C, class A, class B>
    class gc_container <C, A, B, typename boost::enable_if_c< boost::is_base_of<gc_object, typename A::element_type>::value >::type, typename boost::enable_if_c< boost::is_base_of<gc_object, typename B::element_type>::value >::type> : public C, public gc_object
    {
    protected:
        virtual void mark_members(gc* gc) const
        {
            for (typename C::const_iterator elem = this->begin(), last = this->end(); elem != last; ++elem)
            {
                gc->mark(elem->first);
                gc->mark(elem->second);
            }
        }
    };

    template <class T>
    class container_ptr : public gc_ptr<T>
    {
    public:
        typedef T container_type;
        typedef typename T::size_type size_type;
        typedef typename T::iterator iterator;
        typedef typename T::const_iterator const_iterator;
        typedef typename T::reverse_iterator reverse_iterator;
        typedef typename T::const_reverse_iterator const_reverse_iterator;

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

        size_type size() const
        {
            return this->px->size();
        }
    };

    template <class T>
    class vector_container : public container_ptr< gc_container<T, typename T::value_type> >
    {
    public:
        typedef T vector_type;
        typedef typename T::size_type size_type;
        typedef typename T::value_type value_type;
        typedef typename T::reference reference;
        typedef typename T::const_reference const_reference;
        typedef typename gc_container<T, value_type>::iterator iterator;
        typedef typename gc_container<T, value_type>::const_iterator const_iterator;

        vector_container(gc_container<T, value_type>* p = 0) : container_ptr< gc_container<T, value_type> >(p)
        {
        }

        vector_container(const vector_container& rhs) : container_ptr< gc_container<T, value_type> >(rhs)
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
    };

    template <class T>
    vector_container<T> new_vector_placeholder(gc& gc, typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        vector_container<T> container(vector_container<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.resize(n, x);
        return container;
    }

    template <class T>
    vector_container<T> new_vector(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_vector_placeholder<T>(get_gc(), n, x);
    }

    template <class T>
    vector_container<T> new_static_vector(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_vector_placeholder<T>(get_static_gc(), n, x);
    }

    template <class T, class Iter>
    vector_container<T> new_vector_placeholder(gc& gc, Iter first, Iter last)
    {
        vector_container<T> container(vector_container<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.assign(first, last);
        return container;
    }

    template <class T, class Iter>
    vector_container<T> new_vector(Iter first, Iter last)
    {
        return new_vector_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    vector_container<T> new_static_vector(Iter first, Iter last)
    {
        return new_vector_placeholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    class deque_container : public vector_container<T>
    {
    public:
        typedef T deque_type;
        typedef typename T::value_type value_type;

        deque_container(gc_container<T, value_type>* p = 0) : vector_container<T>(p)
        {
        }

        deque_container(const deque_container& rhs) : vector_container<T>(rhs)
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
    deque_container<T> new_deque_placeholder(gc& gc, typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        deque_container<T> container(deque_container<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.resize(n, x);
        return container;
    }

    template <class T>
    deque_container<T> new_deque(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_deque_placeholder<T>(get_gc(), n, x);
    }

    template <class T>
    deque_container<T> new_static_deque(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_deque_placeholder<T>(get_static_gc(), n, x);
    }

    template <class T, class Iter>
    deque_container<T> new_deque_placeholder(gc& gc, Iter first, Iter last)
    {
        deque_container<T> container(deque_container<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.assign(first, last);
        return container;
    }

    template <class T, class Iter>
    deque_container<T> new_deque(Iter first, Iter last)
    {
        return new_deque_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    deque_container<T> new_static_deque(Iter first, Iter last)
    {
        return new_deque_placeholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    class list_container : public deque_container<T>
    {
    public:
        typedef T list_type;
        typedef typename T::value_type value_type;
        typedef typename gc_container<T, value_type>::iterator iterator;
        typedef typename gc_container<T, value_type>::const_iterator const_iterator;

        list_container(gc_container<T, value_type>* p = 0) : deque_container<T>(p)
        {
        }

        list_container(const list_container& rhs) : deque_container<T>(rhs)
        {
        }

        void merge(list_container& x)
        {
            this->px->merge(*x);
        }

        template <class Comp>
        void merge(list_container& x, Comp comp)
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

        void splice(iterator position, list_container& x)
        {
            this->px->splice(position, *x);
        }

        void splice(iterator position, list_container& x, iterator i)
        {
            this->px->splice(position, *x, i);
        }

        void splice(iterator position, list_container& x, iterator first, iterator last)
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
    list_container<T> new_list_placeholder(gc& gc, typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        list_container<T> container(list_container<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.resize(n, x);
        return container;
    }

    template <class T>
    list_container<T> new_list(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_list_placeholder<T>(get_gc(), n, x);
    }

    template <class T>
    list_container<T> new_static_list(typename T::size_type n = 0, const typename T::value_type& x = typename T::value_type())
    {
        return new_list_placeholder<T>(get_static_gc(), n, x);
    }

    template <class T, class Iter>
    list_container<T> new_list_placeholder(gc& gc, Iter first, Iter last)
    {
        list_container<T> container(list_container<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.assign(first, last);
        return container;
    }

    template <class T, class Iter>
    list_container<T> new_list(Iter first, Iter last)
    {
        return new_list_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    list_container<T> new_static_list(Iter first, Iter last)
    {
        return new_list_placeholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    class set_container : public container_ptr< gc_container<T, typename T::value_type> >
    {
    public:
        typedef T set_type;
        typedef typename T::size_type size_type;
        typedef typename T::value_type value_type;
        typedef typename T::reference reference;
        typedef typename T::const_reference const_reference;
        typedef typename gc_container<T, value_type>::iterator iterator;
        typedef typename gc_container<T, value_type>::const_iterator const_iterator;

        set_container(gc_container<T, value_type>* p = 0) : container_ptr< gc_container<T, value_type> >(p)
        {
        }

        set_container(const set_container& rhs) : container_ptr< gc_container<T, value_type> >(rhs)
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
    set_container<T> new_set_placeholder(gc& gc)
    {
        return set_container<T>(new(gc) gc_container<T, typename T::value_type>());
    }

    template <class T>
    set_container<T> new_set()
    {
        return new_set_placeholder<T>(get_gc());
    }

    template <class T>
    set_container<T> new_static_set()
    {
        return new_set_placeholder<T>(get_static_gc());
    }

    template <class T, class Iter>
    set_container<T> new_set_placeholder(gc& gc, Iter first, Iter last)
    {
        set_container<T> container(set_container<T>(new(gc) gc_container<T, typename T::value_type>()));
        container.insert(first, last);
        return container;
    }

    template <class T, class Iter>
    set_container<T> new_set(Iter first, Iter last)
    {
        return new_set_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    set_container<T> new_static_set(Iter first, Iter last)
    {
        return new_set_placeholder<T>(get_static_gc(), first, last);
    }

    template <class T>
    class map_container : public container_ptr< gc_container<T, typename T::key_type, typename T::mapped_type> >
    {
    public:
        typedef T map_type;
        typedef typename T::size_type size_type;
        typedef typename T::key_type key_type;
        typedef typename T::mapped_type mapped_type;
        typedef typename T::value_type value_type;
        typedef typename T::reference reference;
        typedef typename T::const_reference const_reference;
        typedef typename gc_container<T, key_type, typename T::mapped_type>::iterator iterator;
        typedef typename gc_container<T, key_type, typename T::mapped_type>::const_iterator const_iterator;

        map_container(gc_container<T, key_type, typename T::mapped_type>* p = 0) : container_ptr< gc_container<T, key_type, typename T::mapped_type> >(p)
        {
        }

        map_container(const map_container& rhs) : container_ptr< gc_container<T, key_type, typename T::mapped_type> >(rhs)
        {
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

        reference operator [] (const key_type& x)
        {
            return (*this->px)[x];
        }

        const_iterator upper_bound(const key_type& x) const
        {
            return this->px->upper_bound(x);
        }
    };

    template <class T>
    map_container<T> new_map_placeholder(gc& gc)
    {
        return map_container<T>(new(gc) gc_container<T, typename T::key_type, typename T::mapped_type>());
    }

    template <class T>
    map_container<T> new_map()
    {
        return new_map_placeholder<T>(get_gc());
    }

    template <class T>
    map_container<T> new_static_map()
    {
        return new_map_placeholder<T>(get_static_gc());
    }

    template <class T, class Iter>
    map_container<T> new_map_placeholder(gc& gc, Iter first, Iter last)
    {
        map_container<T> container(map_container<T>(new(gc) gc_container<T, typename T::key_type, typename T::mapped_type>()));
        container.insert(first, last);
        return container;
    }

    template <class T, class Iter>
    map_container<T> new_map(Iter first, Iter last)
    {
        return new_map_placeholder<T>(get_gc(), first, last);
    }

    template <class T, class Iter>
    map_container<T> new_static_map(Iter first, Iter last)
    {
        return new_map_placeholder<T>(get_static_gc(), first, last);
    }
}

#endif
