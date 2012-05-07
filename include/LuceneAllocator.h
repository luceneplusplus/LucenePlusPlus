/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "Config.h"

namespace Lucene
{
    /// Allocate block of memory.
    LPPAPI void* AllocMemory(size_t size);

    /// Reallocate a given block of memory.
    LPPAPI void* ReallocMemory(void* memory, size_t size);

    /// Release a given block of memory.
    LPPAPI void FreeMemory(void* memory);

    /// Release thread cache.  Note: should be called whenever a thread
    /// exits and using nedmalloc.
    LPPAPI void ReleaseThreadCache();

    #ifdef LPP_USE_ALLOCATOR

    /// Custom stl allocator used to help exporting stl container across process
    /// borders.  It can also calls custom memory allocation functions that can
    /// help track memory leaks and/or improve performance over standard allocators.
    /// @see #AllocMemory(size_t)
    /// @see #FreeMemory(void*)
    template <typename TYPE>
    class LuceneAllocator
    {
    public:
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef TYPE* pointer;
        typedef const TYPE* const_pointer;
        typedef TYPE& reference;
        typedef const TYPE& const_reference;
        typedef TYPE value_type;

        LuceneAllocator()
        {
        }

        LuceneAllocator(const LuceneAllocator&)
        {
        }

        pointer allocate(size_type n, const void* = 0)
        {
            return (TYPE*)AllocMemory((size_t)(n * sizeof(TYPE)));
        }

        void deallocate(void* p, size_type)
        {
            if (p != NULL)
                FreeMemory(p);
        }

        pointer address(reference x) const
        {
            return &x;
        }

        const_pointer address(const_reference x) const
        {
            return &x;
        }

        LuceneAllocator<TYPE>& operator= (const LuceneAllocator&)
        {
            return *this;
        }

        void construct(pointer p, const TYPE& val)
        {
            new ((TYPE*)p) TYPE(val);
        }

        void destroy(pointer p)
        {
            p->~TYPE();
        }

        size_type max_size() const
        {
            return size_t(-1);
        }

        template <class U>
        struct rebind
        {
            typedef LuceneAllocator<U> other;
        };

        template <class U>
        LuceneAllocator(const LuceneAllocator<U>&)
        {
        }
    };

    template <typename TYPE>
    inline bool operator== (const LuceneAllocator<TYPE>&, const LuceneAllocator<TYPE>&)
    {
        return true;
    }

    template <typename TYPE>
    inline bool operator!= (const LuceneAllocator<TYPE>&, const LuceneAllocator<TYPE>&)
    {
        return false;
    }

    template <>
    class LuceneAllocator<void>
    {
    public:
        typedef void* pointer;
        typedef const void* const_pointer;
        typedef void value_type;

        LuceneAllocator()
        {
        }

        LuceneAllocator(const LuceneAllocator&)
        {
        }

        template <class U>
        struct rebind
        {
            typedef LuceneAllocator<U> other;
        };

        template <class U>
        LuceneAllocator(const LuceneAllocator<U>&)
        {
        }
    };

    #endif
}

#ifndef LPP_USE_ALLOCATOR
#define LuceneAllocator std::allocator
#endif

#endif
