/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "gc.h"

#define _GC_VERSION "2.0.1"

#if defined(_WIN32) || defined(_WIN64) || defined(BOOST_THREAD_WIN32)
#define GC_PLATFORM_WINDOWS
#endif
#if defined(__SVR4) && defined(__sun) || defined(BOOST_THREAD_SOLARIS)
#define GC_PLATFORM_SOLARIS
#endif
#if defined(__sparc__) || defined(__sparc)
#define GC_PLATFORM_SPARC
#endif
#if defined(__ppc__) || defined(__powerpc__) || (__ppc64__) || (__powerpc64__)
#define GC_PLATFORM_POWERPC
#endif
#if defined(_MAC) || defined(BOOST_THREAD_MACOS)
#define GC_PLATFORM_MAC
#endif
#if defined(LINUX) || defined(BOOST_THREAD_LINUX)
#define GC_PLATFORM_LINUX
#endif

#if defined(GC_PLATFORM_WINDOWS)

#include <setjmp.h>
#include <windows.h>

#define GC_GET_STACK_EXTENTS(_gc, _stack, _size) \
    jmp_buf __env; \
    ::setjmp(__env); \
    __asm { mov _stack, esp }; \
    _size = (uint32_t)(_gc->stack_top() - (uintptr_t)_stack);

#elif defined(GC_PLATFORM_SPARC)

#define GC_GET_STACK_EXTENTS(_gc, _stack, _size) \
    jmp_buf __env; \
    ::setjmp(__env); \
    asm ("mov %%sp, %0":"=r" (_stack)); \
    _size = (uint32_t)(_gc->stack_top() - (uintptr_t)_stack);

#elif defined(GC_PLATFORM_POWERPC)

#include <pthread.h>
#include <setjmp.h>

register void* __sp __asm__("r1");

#define GC_GET_STACK_EXTENTS(_gc, _stack, _size) \
    jmp_buf __env; \
    ::setjmp(__env); \
    _stack = (void*)__sp; \
    _size = (uint32_t)(_gc->stack_top() - (uintptr_t)_stack);

#else

#include <pthread.h>
#include <setjmp.h>

#define GC_GET_STACK_EXTENTS(_gc, _stack, _size) \
    jmp_buf __env; \
    ::setjmp(__env); \
    _stack = &__env; \
    _size = (uint32_t)(_gc->stack_top() - (uintptr_t)_stack); \

#endif

#if defined(GC_PLATFORM_SOLARIS)

#include <ucontext.h>
#include <sys/frame.h>
#include <sys/stack.h>

#endif

namespace lutze
{
    static const uint32_t register_threshold = 200;
    static const uint32_t transfer_threshold = 100;

    gc::gc(bool alloc_only) : alloc_only(alloc_only), mark_token(0), register_count(0)
    {
    }

    gc::~gc()
    {
        final_collect();
    }

    std::string gc::gc_version()
    {
        return _GC_VERSION;
    }

    bool gc::gc_init()
    {
        static bool initialized = false;
        bool prev_init = initialized;
        initialized = true;
        return prev_init;
    }

    void gc::gc_term()
    {
        unregister_gc(&get_static_gc());
    }

    void gc::register_gc(gc* pgc)
    {
        if (!gc_init())
            boost::throw_exception(std::runtime_error("gc_init() must be called"));
        boost::mutex::scoped_lock lock(gc_registry_mutex);
        gc_registry.insert(pgc);
    }

    void gc::unregister_gc(gc* pgc)
    {
        {
            boost::mutex::scoped_lock lock(gc_registry_mutex);
            gc_registry.erase(pgc);
        }
        delete pgc;
    }

    gc& gc::get_gc()
    {
        static boost::thread_specific_ptr<gc> thread_gc(gc::unregister_gc);
        if (thread_gc.get() == NULL)
        {
            thread_gc.reset(new gc);
            gc::register_gc(thread_gc.get());
        }
        return *thread_gc.get();
    }

    gc& gc::get_static_gc()
    {
        static gc* static_gc = NULL;
        if (static_gc == NULL)
            static_gc = new gc(true);
        return *static_gc;
    }

    void gc::collect(bool force)
    {
        // have we reached threshold before collection is necessary?
        if (alloc_only || (!force && !check_threshold()))
            return;

        // 1) prepare release queue
        init_collect();

        // 2) scan stack for roots
        node_map roots;
        scan_stack(roots);

        // 3) mark phase
        mark_objects(roots);

        // 4) sweep phase
        sweep_objects();

        // 5) destroy or transfer released objects
        dispose_objects();
    }

    void gc::final_collect()
    {
        // 1) prepare release queue
        init_collect();

        // 2) sweep phase
        sweep_objects();

        // 3) destroy or transfer released objects
        dispose_objects();
    }

    #if defined(GC_PLATFORM_WINDOWS)

    uintptr_t gc::stack_top() const
    {
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery(&mbi, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
        // now mbi.AllocationBase = reserved stack memory base address
        VirtualQuery(mbi.AllocationBase, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
        // now (mbi.BaseAddress, mbi.RegionSize) describe reserved (uncommitted) portion of the stack
        VirtualQuery((uint8_t*)mbi.BaseAddress + mbi.RegionSize, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
        // now (mbi.BaseAddress, mbi.RegionSize) describe the guard page
        VirtualQuery((uint8_t*)mbi.BaseAddress + mbi.RegionSize, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
        // now (mbi.BaseAddress, mbi.RegionSize) describe the committed (i.e. accessed) portion of the stack
        return (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
    }

    #elif defined(GC_PLATFORM_SOLARIS)

    uintptr_t gc::stack_top() const
    {
        #if defined(GC_PLATFORM_SPARC)
        asm("ta 3");
        #endif

        struct frame *sp;
        int i;
        int *iptr;

        stack_t st;
        stack_getbounds(&st);

        return (uintptr_t)st.ss_sp + st.ss_size;
    }

    #elif defined(GC_PLATFORM_LINUX)

    uintptr_t gc::stack_top() const
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);

        size_t sz;
        void* s_base;

        pthread_getattr_np(pthread_self(), &attr);
        pthread_attr_getstack(&attr, &s_base, &sz);
        pthread_attr_destroy(&attr);

        return (uintptr_t)((size_t)s_base + sz);
    }

    #elif defined(GC_PLATFORM_MAC)

    uintptr_t gc::stack_top() const
    {
        return (uintptr_t)pthread_get_stackaddr_np(pthread_self());
    }

    #else

    uintptr_t gc::stack_top() const
    {
        pthread_attr_t attr;
        pthread_getattr_np(pthread_self(), &attr);

        void* sp;
        size_t sz;
        pthread_attr_getstack(&attr, &sp, &sz);

        return (uintptr_t)sp;
    }

    #endif

    bool gc::check_threshold()
    {
        /* todo
        if (register_count > register_threshold)
            return true;
        {
            boost::mutex::scoped_lock lock(transfer_mutex);
            if (transfer_queue.size() > transfer_threshold)
                return true;
        }*/
        return false;
    }

    void gc::init_collect()
    {
        register_count = 0;
        ++mark_token;

        // take snapshot of transfer queue
        {
            boost::mutex::scoped_lock lock(transfer_mutex);
            release_queue.clear();
            release_queue.swap(transfer_queue);
        }
    }

    void gc::scan_stack(node_map& roots)
    {
        void* stack;
        size_t stack_size;
        GC_GET_STACK_EXTENTS(this, stack, stack_size);

        uint8_t* ptr = (uint8_t*)stack;
        uint8_t* last = ptr + (stack_size - sizeof(gc_object*));

        // scan stack for roots
        while (ptr < last)
        {
            gc_object** ppobj = reinterpret_cast<gc_object**>(ptr);
            node_map::iterator obj = object_registry.find(normalize_ptr(*ppobj));
            if (obj == object_registry.end())
                ++ptr;
            else
            {
                roots.insert(*obj);
                ptr += sizeof(gc_object*);
            }
        }
    }

    void gc::mark_objects(const node_map& roots)
    {
        for (node_map::const_iterator node = roots.begin(), last = roots.end(); node != last; ++node)
        {
            if (unmark_objects.find(node->first) == unmark_objects.end())
                mark_object(node->second.object);
        }
        unmark_objects.clear();
    }

    void gc::mark_object(const gc_object* pobj)
    {
        if (!pobj)
            return;
        void* ptr = normalize_ptr(pobj);
        node_map::iterator node = object_registry.find(ptr);
        if (node == object_registry.end()) // object does not belong to this gc registry
        {
            node_map::iterator input = release_queue.find(ptr);
            if (input == release_queue.end())
                return;
            node = object_registry.insert(std::make_pair(ptr, gc_node(pobj))).first;
            release_queue.erase(input); // take ownership
        }
        if (mark_token != node->second.mark_token)
        {
            node->second.mark_token = mark_token;
            node->second.object->mark_members(this);
        }
    }

    void gc::unmark_object(const gc_object* pobj)
    {
        node_map::iterator node = object_registry.find(normalize_ptr(pobj));
        if (node != object_registry.end())
            unmark_objects.insert(*node);
    }

    void gc::sweep_objects()
    {
        for (node_map::iterator node = object_registry.begin(), last = object_registry.end(); node != last; ++node)
        {
            if (node->second.mark_token != mark_token)
                release_queue.insert(*node);
        }
    }

    void gc::dispose_objects()
    {
        boost::mutex::scoped_lock lock(gc_registry_mutex);

        // take snapshot of currently running gc set
        gc_set gc_running(gc_registry);
        gc_running.erase(this);

        typedef boost::unordered_map<gc*, node_map> transfer_map;
        transfer_map transfer;

        // clean up phase
        for (node_map::iterator node = release_queue.begin(), last = release_queue.end(); node != last; ++node)
        {
            gc_set remaining;
            std::set_difference(gc_running.begin(), gc_running.end(), node->second.history.begin(), node->second.history.end(), std::inserter(remaining, remaining.end()));
            if (alloc_only || remaining.empty())
            {
                // destroy object after unregistering from this gc
                unregister_object(node->second.object);
                delete const_cast<gc_object*>(node->second.object);
            }
            else
            {
                // append object to first remaining gc transfer map
                node->second.history.insert(this);
                std::pair<transfer_map::iterator, bool> transfer_gc = transfer.insert(std::make_pair(*remaining.begin(), node_map()));
                transfer_gc.first->second.insert(*node);
            }
        }

        // transfer all remaining objects to other gc instances
        for (transfer_map::iterator transfer_gc = transfer.begin(), last = transfer.end(); transfer_gc != last; ++transfer_gc)
            transfer_gc->first->transfer(transfer_gc->second);
    }

    void gc::transfer(const node_map& transfer_nodes)
    {
        boost::mutex::scoped_lock lock(transfer_mutex);
        transfer_queue.insert(transfer_nodes.begin(), transfer_nodes.end());
    }

    boost::mutex gc::gc_registry_mutex;
    gc::gc_set gc::gc_registry;
}
