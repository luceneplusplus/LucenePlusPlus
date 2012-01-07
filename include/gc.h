/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _LUTZE_GC
#define _LUTZE_GC

#include <set>
#include <boost/cstdint.hpp>
#include <boost/unordered_map.hpp>
#include "boost/thread/tss.hpp"
#include "boost/thread/mutex.hpp"
#include <boost/preprocessor/punctuation.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic.hpp>
#include "gc_ptr.h"

#define _GC_VERSION "1.0.2"

#if defined(_WIN32) || defined(_WIN64)
#define GC_PLATFORM_WINDOWS
#endif
#if defined(__SVR4) && defined(__sun)
#define GC_PLATFORM_SOLARIS
#endif
#if defined(__sparc__) || defined(__sparc)
#define GC_PLATFORM_SPARC
#endif
#if defined(__ppc__) || defined(__powerpc__) || (__ppc64__) || (__powerpc64__)
#define GC_PLATFORM_POWERPC
#endif
#if defined(_MAC)
#define GC_PLATFORM_MAC
#endif
#if defined(LINUX)
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
    using boost::int32_t;
    using boost::uint32_t;
    using boost::int64_t;
    using boost::uint64_t;
    using boost::uint8_t;

    static const uint32_t register_threshold = 200;
    static const uint32_t transfer_threshold = 100;

    class gc;
    typedef std::set<gc*> gc_set;

    // all garbage collected classes must be derived from this base class
    class gc_object
    {
    public:
        virtual ~gc_object()
        {
        }

    protected:
        virtual void mark_members(gc* gc) const
        {
            // override
        }

    public:
        void* operator new (size_t size, gc& gc);
        void* operator new (size_t size, void* p = 0);
        void operator delete (void* p, gc& gc);
        void operator delete (void* p);

        friend class gc;
    };

    class gc
    {
    public:
        gc(bool alloc_only = false) : alloc_only(alloc_only), mark_token(0), register_count(0)
        {
        }

        ~gc()
        {
            final_collect();
        }

    private:
        struct gc_node
        {
            gc_node(const gc_object* obj = NULL) : mark_token(0), object(obj) {}
            const gc_object* object;
            uint32_t mark_token;
            gc_set history;
        };

        typedef boost::unordered_map<const void*, gc_node> node_map;

        node_map object_registry;
        node_map unmark_objects;
        node_map release_queue;

        boost::mutex transfer_mutex;
        node_map transfer_queue;

        static boost::mutex gc_registry_mutex;
        static gc_set gc_registry;

        bool alloc_only;
        uint32_t mark_token;
        uint32_t register_count;

    public:
        static std::string gc_version()
        {
            return _GC_VERSION;
        }

        static void register_gc(gc* pgc)
        {
            boost::mutex::scoped_lock lock(gc_registry_mutex);
            gc_registry.insert(pgc);
        }

        static void unregister_gc(gc* pgc)
        {
            {
                boost::mutex::scoped_lock lock(gc_registry_mutex);
                gc_registry.erase(pgc);
            }
            delete pgc;
        }

        inline void register_object(const gc_object* pobj)
        {
            ++register_count;
            object_registry.insert(std::make_pair(normalize_ptr(pobj), gc_node(pobj)));
        }

        inline void unregister_object(const gc_object* pobj)
        {
            object_registry.erase(normalize_ptr(pobj));
        }

        template <class OBJ>
        void mark(OBJ obj)
        {
            // do nothing
        }

        template <class OBJ>
        void mark(OBJ* obj)
        {
            mark_object(reinterpret_cast<gc_object*>(obj));
        }

        template <class OBJ>
        void mark(const gc_ptr<OBJ>& obj)
        {
            mark_object(obj.get());
        }

        template <class OBJ>
        void unmark(OBJ obj)
        {
            // do nothing
        }

        template <class OBJ>
        void unmark(OBJ* obj)
        {
            unmark_object(reinterpret_cast<gc_object*>(obj));
        }

        template <class OBJ>
        void unmark(const gc_ptr<OBJ>& obj)
        {
            unmark_object(obj.get());
        }

        void collect(bool force = false)
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

        void final_collect()
        {
            // 1) prepare release queue
            init_collect();

            // 2) sweep phase
            sweep_objects();

            // 3) destroy or transfer released objects
            dispose_objects();
        }

    private:
        inline void* normalize_ptr(const gc_object* pobj)
        {
            return (void*)((uintptr_t)pobj & ~0xf);
        }

        #if defined(GC_PLATFORM_WINDOWS)

        uintptr_t stack_top() const
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

        uintptr_t stack_top() const
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

        uintptr_t stack_top() const
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

        uintptr_t stack_top() const
        {
            return (uintptr_t)pthread_get_stackaddr_np(pthread_self());
        }

        #else

        uintptr_t stack_top() const
        {
            pthread_attr_t attr;
            pthread_getattr_np(pthread_self(), &attr);

            void* sp;
            size_t sz;
            pthread_attr_getstack(&attr, &sp, &sz);

            return (uintptr_t)sp;
        }

        #endif

        bool check_threshold()
        {
            if (register_count > register_threshold)
                return true;
            {
                boost::mutex::scoped_lock lock(transfer_mutex);
                if (transfer_queue.size() > transfer_threshold)
                    return true;
            }
            return false;
        }

        void init_collect()
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

        void scan_stack(node_map& roots)
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

        void mark_objects(const node_map& roots)
        {
            for (node_map::const_iterator node = roots.begin(), last = roots.end(); node != last; ++node)
            {
                if (unmark_objects.find(node->first) == unmark_objects.end())
                    mark_object(node->second.object);
            }
            unmark_objects.clear();
        }

        void mark_object(const gc_object* pobj)
        {
            if (!pobj)
                return;
            void* ptr = normalize_ptr(pobj);
            node_map::iterator node = object_registry.find(ptr);
            if (node == object_registry.end()) // object does not belong to this gc registry
            {
                node_map::iterator input = release_queue.find(ptr);
                if (node == object_registry.end())
                    return;
                node = object_registry.insert(std::make_pair(ptr, gc_node(pobj))).first;
                release_queue.erase(input); // take ownership
            }
            if (mark_token != node->second.mark_token)
            {
                node->second.mark_token = mark_token;
                pobj->mark_members(this);
            }
        }

        void unmark_object(const gc_object* pobj)
        {
            node_map::iterator node = object_registry.find(normalize_ptr(pobj));
            if (node != object_registry.end())
                unmark_objects.insert(*node);
        }

        void sweep_objects()
        {
            for (node_map::iterator node = object_registry.begin(), last = object_registry.end(); node != last; ++node)
            {
                if (node->second.mark_token != mark_token)
                    release_queue.insert(*node);
            }
        }

        void dispose_objects()
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
                    // manually destroy object before unregistering from this gc
                    node->second.object->~gc_object();
                    gc_object::operator delete(const_cast<gc_object*>(node->second.object), *this);
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

        void transfer(const node_map& transfer_nodes)
        {
            boost::mutex::scoped_lock lock(transfer_mutex);
            transfer_queue.insert(transfer_nodes.begin(), transfer_nodes.end());
        }
    };

    boost::mutex gc::gc_registry_mutex;
    gc_set gc::gc_registry;

    static gc& get_gc()
    {
        static boost::thread_specific_ptr<gc> thread_gc(gc::unregister_gc);
        if (thread_gc.get() == NULL)
        {
            thread_gc.reset(new gc);
            gc::register_gc(thread_gc.get());
        }
        return *thread_gc.get();
    }

    static gc& get_static_gc()
    {
        static gc* static_gc = NULL;
        if (static_gc == NULL)
            static_gc = new gc(true);
        return *static_gc;
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

    // This expands to...
    // template <class T, class A1, ...etc>
    // gc_ptr<T> new_gc(A1 const& a1, ...etc)
    // {
    //     return new_gc_placeholder<T>(get_gc(), a1, ...etc);
    // }

    // template <class T, class A1, ...etc>
    // gc_ptr<T> new_static_gc(A1 const& a1, ...etc)
    // {
    //     return new_gc_placeholder<T>(get_static_gc(), a1, ...etc);
    // }
    #define NEW_GC(Z, N, _) \
    template<class T BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, class A)> \
    gc_ptr<T> new_gc_placeholder(gc& gc BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_BINARY_PARAMS(N, const A, & a)) \
    { gc_ptr<T> obj(new(gc) T(BOOST_PP_ENUM_PARAMS(N, a))); gc.collect(); return obj; } \
    template<class T BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, class A)> \
    gc_ptr<T> new_gc(BOOST_PP_ENUM_BINARY_PARAMS(N, const A, & a)) \
    { return new_gc_placeholder<T>(get_gc() BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, a)); } \
    template<class T BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, class A)> \
    gc_ptr<T> new_static_gc(BOOST_PP_ENUM_BINARY_PARAMS(N, const A, & a)) \
    { return new_gc_placeholder<T>(get_static_gc() BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, a)); }
    BOOST_PP_REPEAT_2ND(BOOST_PP_INC(9), NEW_GC, _)
}

#endif
