/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "gc.h"
#include "gc_object.h"

namespace lutze
{
    gc_object::~gc_object()
    {
    }

    void gc_object::mark_members(gc* gc) const
    {
        // override
    }

    void* gc_object::operator new (size_t size, gc& gc)
    {
        void* pobj = ::operator new(size);
        gc.register_object(static_cast<gc_object*>(pobj));
        return pobj;
    }

    void* gc_object::operator new (size_t size, void* p)
    {
        return gc_object::operator new(size, get_gc());
    }

    void gc_object::operator delete (void* p, gc& gc)
    {
        gc_object* pobj = static_cast<gc_object*>(p);
        gc.unregister_object(pobj);
        ::operator delete(pobj);
    }

    void gc_object::operator delete (void* p)
    {
        gc_object::operator delete(p, get_gc());
    }
}
