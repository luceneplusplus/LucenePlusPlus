/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENEFACTORY_H
#define LUCENEFACTORY_H

#include "LucenePtr.h"
#include <boost/make_shared.hpp>
#include <boost/version.hpp>

namespace Lucene
{
    template <class T>
    LucenePtr<T> newInstance()
    {
        #ifdef LPP_USE_GC
        return new(GC) T;
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T);
        #else
        return boost::allocate_shared<T>(Allocator<T>());
        #endif
        #endif
    }

    template <class T, class A1>
    LucenePtr<T> newInstance(A1 const& a1)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1);
        #endif
        #endif
    }

    template <class T, class A1, class A2>
    LucenePtr<T> newInstance(A1 const& a1, A2 const& a2)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1, a2);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1, a2));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2);
        #endif
        #endif
    }

    template <class T, class A1, class A2, class A3>
    LucenePtr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1, a2, a3);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1, a2, a3));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3);
        #endif
        #endif
    }

    template <class T, class A1, class A2, class A3, class A4>
    LucenePtr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1, a2, a3, a4);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1, a2, a3, a4));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4);
        #endif
        #endif
    }

    template <class T, class A1, class A2, class A3, class A4, class A5>
    LucenePtr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1, a2, a3, a4, a5);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5);
        #endif
        #endif
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    LucenePtr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1, a2, a3, a4, a5, a6);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5, a6);
        #endif
        #endif
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    LucenePtr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1, a2, a3, a4, a5, a6, a7);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5, a6, a7);
        #endif
        #endif
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    LucenePtr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1, a2, a3, a4, a5, a6, a7, a8);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5, a6, a7, a8);
        #endif
        #endif
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    LucenePtr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        #ifdef LPP_USE_GC
        return new(GC) T(a1, a2, a3, a4, a5, a6, a7, a8, a9);
        #else
        #if BOOST_VERSION <= 103800
        return boost::shared_ptr<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9));
        #else
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5, a6, a7, a8, a9);
        #endif
        #endif
    }

    template <class T>
    LucenePtr<T> newLucene()
    {
        LucenePtr<T> instance(newInstance<T>());
        instance->initialize();
        return instance;
    }

    template <class T, class A1>
    LucenePtr<T> newLucene(A1 const& a1)
    {
        LucenePtr<T> instance(newInstance<T>(a1));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2>
    LucenePtr<T> newLucene(A1 const& a1, A2 const& a2)
    {
        LucenePtr<T> instance(newInstance<T>(a1, a2));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3>
    LucenePtr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3)
    {
        LucenePtr<T> instance(newInstance<T>(a1, a2, a3));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4>
    LucenePtr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        LucenePtr<T> instance(newInstance<T>(a1, a2, a3, a4));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5>
    LucenePtr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        LucenePtr<T> instance(newInstance<T>(a1, a2, a3, a4, a5));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    LucenePtr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        LucenePtr<T> instance(newInstance<T>(a1, a2, a3, a4, a5, a6));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    LucenePtr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        LucenePtr<T> instance(newInstance<T>(a1, a2, a3, a4, a5, a6, a7));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    LucenePtr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        LucenePtr<T> instance(newInstance<T>(a1, a2, a3, a4, a5, a6, a7, a8));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    LucenePtr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        LucenePtr<T> instance(newInstance<T>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
        instance->initialize();
        return instance;
    }
}

#endif
