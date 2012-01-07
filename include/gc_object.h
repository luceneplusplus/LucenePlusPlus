/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _LUTZE_GC_OBJECT
#define _LUTZE_GC_OBJECT

namespace lutze
{
    class gc;

    // all garbage collected classes must be derived from this base class
    class gc_object
    {
    public:
        virtual ~gc_object();

    protected:
        virtual void mark_members(gc* gc) const;

    public:
        void* operator new (size_t size, gc& gc);
        void* operator new (size_t size, void* p = 0);
        void operator delete (void* p, gc& gc);
        void operator delete (void* p);

        friend class gc;
    };
}

#endif
