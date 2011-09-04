/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENEPTR_H
#define LUCENEPTR_H

#include "Config.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#ifdef LPP_USE_GC
#define GC_NOT_DLL
#include "gc_cpp.h"

#include <boost/config.hpp>
#include <boost/assert.hpp>

#ifdef BOOST_MSVC  // moved here to work around VC++ compiler crash
# pragma warning(push)
# pragma warning(disable:4284) // odd return type for operator->
#endif

namespace Lucene
{
    namespace detail
    {
        template<class Y, class T> struct sp_convertible
        {
            typedef char (&yes) [1];
            typedef char (&no) [2];

            static yes f(T*);
            static no  f(...);

            enum _vt { value = sizeof((f)(static_cast<Y*>(0))) == sizeof(yes) };
        };
        
        struct sp_empty
        {
        };
        
        template<bool> struct sp_enable_if_convertible_impl;

        template<> struct sp_enable_if_convertible_impl<true>
        {
            typedef sp_empty type;
        };

        template<> struct sp_enable_if_convertible_impl<false>
        {
        };

        template<class Y, class T> struct sp_enable_if_convertible: public sp_enable_if_convertible_impl<sp_convertible<Y, T>::value>
        {
        };
    }
    
    template<class T> class gc_ptr
    {
    private:
        typedef gc_ptr this_type;

    public:
        typedef T element_type;

        gc_ptr() : px(0)
        {
        }

        gc_ptr(T* p) : px(p)
        {
        }
        
        #if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)
        template<class U>
        #if !defined(BOOST_SP_NO_SP_CONVERTIBLE)
        gc_ptr(gc_ptr<U> const& rhs, typename Lucene::detail::sp_enable_if_convertible<U, T>::type = Lucene::detail::sp_empty())
        #else
        gc_ptr(gc_ptr<U> const& rhs)
        #endif
        : px(rhs.get())
        {
        }
        #endif

        gc_ptr(gc_ptr const& rhs) : px(rhs.px)
        {
        }

        #if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)
        template<class U> gc_ptr& operator= (gc_ptr<U> const& rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }
        #endif

        // Move support
        #if defined(BOOST_HAS_RVALUE_REFS)
        gc_ptr(gc_ptr&& rhs): px(rhs.px)
        {
            rhs.px = 0;
        }

        gc_ptr& operator=(gc_ptr&& rhs)
        {
            this_type(static_cast<gc_ptr &&>(rhs)).swap(*this);
            return *this;
        }
        #endif

        gc_ptr& operator=(gc_ptr const& rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        gc_ptr& operator=(T* rhs)
        {
            this_type(rhs).swap(*this);
            return *this;
        }

        void reset()
        {
            this_type().swap(*this);
        }

        void reset(T* rhs)
        {
            this_type(rhs).swap(*this);
        }

        T* get() const
        {
            return px;
        }

        T& operator*() const
        {
            BOOST_ASSERT(px != 0);
            return *px;
        }

        T* operator->() const
        {
            BOOST_ASSERT(px != 0);
            return px;
        }
        
        typedef T* this_type::*unspecified_bool_type;
 
        operator unspecified_bool_type() const
        {
            return px == 0 ? 0: &this_type::px;
        }

        bool operator! () const
        {
            return px == 0;
        }

        void swap(gc_ptr& rhs)
        {
            T* tmp = px;
            px = rhs.px;
            rhs.px = tmp;
        }
        
        gc_ptr<T> lock() const
        {
            return gc_ptr<element_type>(*this);
        }
        
        bool expired() const
        {
            return px == 0;
        }

    private:
        T* px;
    };

    template<class T, class U> inline bool operator==(gc_ptr<T> const& a, gc_ptr<U> const& b)
    {
        return a.get() == b.get();
    }

    template<class T, class U> inline bool operator!=(gc_ptr<T> const& a, gc_ptr<U> const& b)
    {
        return a.get() != b.get();
    }

    template<class T, class U> inline bool operator==(gc_ptr<T> const& a, U* b)
    {
        return a.get() == b;
    }

    template<class T, class U> inline bool operator!=(gc_ptr<T> const& a, U* b)
    {
        return a.get() != b;
    }

    template<class T, class U> inline bool operator==(T* a, gc_ptr<U> const& b)
    {
        return a == b.get();
    }

    template<class T, class U> inline bool operator!=(T* a, gc_ptr<U> const& b)
    {
        return a != b.get();
    }

    #if __GNUC__ == 2 && __GNUC_MINOR__ <= 96
    // Resolve the ambiguity between our op!= and the one in rel_ops
    template<class T> inline bool operator!=(gc_ptr<T> const& a, gc_ptr<T> const& b)
    {
        return a.get() != b.get();
    }
    #endif

    template<class T> inline bool operator<(gc_ptr<T> const& a, gc_ptr<T> const& b)
    {
        return std::less<T *>()(a.get(), b.get());
    }

    template<class T> void swap(gc_ptr<T>& lhs, gc_ptr<T>& rhs)
    {
        lhs.swap(rhs);
    }

    // mem_fn support
    template<class T> T* get_pointer(gc_ptr<T> const& p)
    {
        return p.get();
    }

    template<class T, class U> gc_ptr<T> LuceneStaticCast(gc_ptr<U> const& p)
    {
        return static_cast<T *>(p.get());
    }

    template<class T, class U> gc_ptr<T> LuceneConstCast(gc_ptr<U> const& p)
    {
        return const_cast<T *>(p.get());
    }

    template<class T, class U> gc_ptr<T> LuceneDynamicCast(gc_ptr<U> const& p)
    {
        return dynamic_cast<T *>(p.get());
    }
}

#define LucenePtr Lucene::gc_ptr
#define LuceneWeakPtr Lucene::gc_ptr

#else
#define LucenePtr boost::shared_ptr
#define LuceneWeakPtr boost::weak_ptr

#define LuceneDynamicCast boost::dynamic_pointer_cast
#define LuceneStaticCast boost::static_pointer_cast
#define LuceneConstCast boost::const_pointer_cast

#endif

#endif