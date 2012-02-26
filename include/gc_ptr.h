/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _LUTZE_GC_PTR
#define _LUTZE_GC_PTR

namespace lutze
{
    using boost::int32_t;
    using boost::uint32_t;
    using boost::int64_t;
    using boost::uint64_t;
    using boost::int8_t;
    using boost::uint8_t;

    namespace detail
    {
        struct static_cast_tag {};
        struct const_cast_tag {};
        struct dynamic_cast_tag {};
        struct reinterpret_cast_tag {};

        template <class Y, class T>
        struct gc_ptr_convertible
        {
            typedef char (&yes) [1];
            typedef char (&no) [2];

            static yes f(T*);
            static no f(...);

            enum _vt { value = sizeof((f)(static_cast<Y*>(0))) == sizeof(yes) };
        };

        struct gc_ptr_empty
        {
        };

        template <bool>
        struct gc_ptr_enable_if_convertible_impl;

        template <>
        struct gc_ptr_enable_if_convertible_impl<true>
        {
            typedef gc_ptr_empty type;
        };

        template <>
        struct gc_ptr_enable_if_convertible_impl<false>
        {
        };

        template <class Y, class T>
        struct gc_ptr_enable_if_convertible : public gc_ptr_enable_if_convertible_impl<gc_ptr_convertible<Y, T>::value>
        {
        };
    }

    template <class T>
    class gc_ptr
    {
    public:
        typedef gc_ptr this_type;
        typedef T element_type;

        gc_ptr(T* p = 0) : px(p), padding(0)
        {
        }

        template <class U>
        gc_ptr(const gc_ptr<U>& rhs, typename detail::gc_ptr_enable_if_convertible<U, T>::type = detail::gc_ptr_empty()) : px(rhs.get())
        {
        }

        template <class U>
        gc_ptr(const gc_ptr<U>& rhs, detail::static_cast_tag): px(static_cast<T*>(rhs.get()))
        {
        }

        template <class U>
        gc_ptr(const gc_ptr<U>& rhs, detail::const_cast_tag): px(const_cast<T*>(rhs.get()))
        {
        }

        template <class U>
        gc_ptr(const gc_ptr<U>& rhs, detail::dynamic_cast_tag): px(dynamic_cast<T*>(rhs.get()))
        {
        }

        template <class U>
        gc_ptr(const gc_ptr<U>& rhs, detail::reinterpret_cast_tag): px(reinterpret_cast<T*>(rhs.get()))
        {
        }

        ~gc_ptr()
        {
            px = 0;
        }

        gc_ptr& operator = (const gc_ptr& rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        template <class Y>
        gc_ptr& operator = (const gc_ptr<Y>& rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        void reset()
        {
            px = 0;
        }

        void reset(T* rhs)
        {
            this_type(rhs).swap(*this);
        }

        T* get() const
        {
            return px;
        }

        T& operator * () const
        {
            return *px;
        }

        T* operator -> () const
        {
            BOOST_ASSERT(px != 0);
            return px;
        }

        void swap(gc_ptr& rhs)
        {
            std::swap(px, rhs.px);
        }

        typedef T* this_type::*unspecified_bool_type;

        operator unspecified_bool_type() const
        {
            return px == 0 ? 0: &this_type::px;
        }

        bool operator ! () const
        {
            return px == 0;
        }

    protected:
        T* px;

        // padding is required for windows because of stack address space pollution
        uint8_t padding;
    };

    template <class T1, class T2>
    bool operator == (const gc_ptr<T1>& s1, const gc_ptr<T2>& s2)
    {
        return s1.get() == s2.get();
    }

    template <class T1, class T2>
    bool operator != (const gc_ptr<T1>& s1, const gc_ptr<T2>& s2)
    {
        return !(s1 == s2);
    }

    template <class T1, class T2>
    bool operator < (const gc_ptr<T1>& s1, const gc_ptr<T2>& s2)
    {
        return s1.get() < s2.get();
    }

    template <class T>
    T* get_pointer(const gc_ptr<T>& p) // mem_fn support
    {
        return p.get();
    }

    template <class T, class U>
    gc_ptr<T> gc_ptr_static_cast(const gc_ptr<U>& p)
    {
        return gc_ptr<T>(p, detail::static_cast_tag());
    }

    template <class T, class U>
    gc_ptr<T> gc_ptr_const_cast(const gc_ptr<U>& p)
    {
        return gc_ptr<T>(p, detail::const_cast_tag());
    }

    template <class T, class U>
    gc_ptr<T> gc_ptr_dynamic_cast(const gc_ptr<U>& p)
    {
        return gc_ptr<T>(p, detail::dynamic_cast_tag());
    }

    template <class T, class U>
    gc_ptr<T> gc_ptr_reinterpret_cast(const gc_ptr<U>& p)
    {
        return gc_ptr<T>(p, detail::reinterpret_cast_tag());
    }
}

#endif
