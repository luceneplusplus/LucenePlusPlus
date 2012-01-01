/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENEFACTORY_H
#define LUCENEFACTORY_H

#include "gc_ptr.h"

namespace Lucene
{
    template <class T>
    gc_ptr<T> newLucene()
    {
        gc_ptr<T> instance(new_gc<T>());
        instance->initialize();
        return instance;
    }

    template <class T, class A1>
    gc_ptr<T> newLucene(A1 const& a1)
    {
        gc_ptr<T> instance(new_gc<T>(a1));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2>
    gc_ptr<T> newLucene(A1 const& a1, A2 const& a2)
    {
        gc_ptr<T> instance(new_gc<T>(a1, a2));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3>
    gc_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3)
    {
        gc_ptr<T> instance(new_gc<T>(a1, a2, a3));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4>
    gc_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        gc_ptr<T> instance(new_gc<T>(a1, a2, a3, a4));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5>
    gc_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        gc_ptr<T> instance(new_gc<T>(a1, a2, a3, a4, a5));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    gc_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        gc_ptr<T> instance(new_gc<T>(a1, a2, a3, a4, a5, a6));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    gc_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        gc_ptr<T> instance(new_gc<T>(a1, a2, a3, a4, a5, a6, a7));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    gc_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        gc_ptr<T> instance(new_gc<T>(a1, a2, a3, a4, a5, a6, a7, a8));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    gc_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        gc_ptr<T> instance(new_gc<T>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
        instance->initialize();
        return instance;
    }

    template <class T>
    gc_ptr<T> newStaticLucene()
    {
        gc_ptr<T> instance(new_static_gc<T>());
        instance->initialize();
        return instance;
    }

    template <class T, class A1>
    gc_ptr<T> newStaticLucene(A1 const& a1)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2>
    gc_ptr<T> newStaticLucene(A1 const& a1, A2 const& a2)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1, a2));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3>
    gc_ptr<T> newStaticLucene(A1 const& a1, A2 const& a2, A3 const& a3)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1, a2, a3));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4>
    gc_ptr<T> newStaticLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1, a2, a3, a4));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5>
    gc_ptr<T> newStaticLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1, a2, a3, a4, a5));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    gc_ptr<T> newStaticLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1, a2, a3, a4, a5, a6));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    gc_ptr<T> newStaticLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1, a2, a3, a4, a5, a6, a7));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    gc_ptr<T> newStaticLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1, a2, a3, a4, a5, a6, a7, a8));
        instance->initialize();
        return instance;
    }

    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    gc_ptr<T> newStaticLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        gc_ptr<T> instance(new_static_gc<T>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
        instance->initialize();
        return instance;
    }
}

#endif
