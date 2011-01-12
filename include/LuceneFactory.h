/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENEFACTORY_H
#define LUCENEFACTORY_H

namespace Lucene
{
    template <class T>
    boost::shared_ptr<T> newInstance()
    {
        return boost::allocate_shared<T>(Allocator<T>());
    }
    
    template <class T, class A1>
    boost::shared_ptr<T> newInstance(A1 const& a1)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1);
    }
    
    template <class T, class A1, class A2>
    boost::shared_ptr<T> newInstance(A1 const& a1, A2 const& a2)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2);
    }
    
    template <class T, class A1, class A2, class A3>
    boost::shared_ptr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3);
    }
    
    template <class T, class A1, class A2, class A3, class A4>
    boost::shared_ptr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4);
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5>
    boost::shared_ptr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5);
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    boost::shared_ptr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5, a6);
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    boost::shared_ptr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5, a6, a7);
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    boost::shared_ptr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5, a6, a7, a8);
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    boost::shared_ptr<T> newInstance(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        return boost::allocate_shared<T>(Allocator<T>(), a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
        
    template <class T>
    boost::shared_ptr<T> newLucene()
    {
        boost::shared_ptr<T> instance(newInstance<T>());
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1>
    boost::shared_ptr<T> newLucene(A1 const& a1)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1));
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1, class A2>
    boost::shared_ptr<T> newLucene(A1 const& a1, A2 const& a2)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1, a2));
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1, class A2, class A3>
    boost::shared_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1, a2, a3));
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1, class A2, class A3, class A4>
    boost::shared_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1, a2, a3, a4));
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5>
    boost::shared_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1, a2, a3, a4, a5));
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5, class A6>
    boost::shared_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1, a2, a3, a4, a5, a6));
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
    boost::shared_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1, a2, a3, a4, a5, a6, a7));
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
    boost::shared_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1, a2, a3, a4, a5, a6, a7, a8));
        instance->initialize();
        return instance;
    }
    
    template <class T, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
    boost::shared_ptr<T> newLucene(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5, A6 const& a6, A7 const& a7, A8 const& a8, A9 const& a9)
    {
        boost::shared_ptr<T> instance(newInstance<T>(a1, a2, a3, a4, a5, a6, a7, a8, a9));
        instance->initialize();
        return instance;
    }
}

#endif
