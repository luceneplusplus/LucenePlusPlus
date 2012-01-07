/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2012 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef _LUTZE_GC
#define _LUTZE_GC

#include <set>
#include <string>
#include <boost/cstdint.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/preprocessor/punctuation.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic.hpp>
#include "gc_object.h"
#include "gc_ptr.h"

namespace lutze
{
    using boost::int32_t;
    using boost::uint32_t;
    using boost::int64_t;
    using boost::uint64_t;
    using boost::uint8_t;

    class gc;
    typedef std::set<gc*> gc_set;

    class gc
    {
    public:
        gc(bool alloc_only = false);
        ~gc();

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

        static boost::shared_ptr<boost::mutex> gc_registry_mutex;
        static boost::shared_ptr<gc_set> gc_registry;

        bool alloc_only;
        uint32_t mark_token;
        uint32_t register_count;

    public:
        // return current lutze gc version
        static std::string gc_version();

        // register gc instance at thread launch
        static void register_gc(gc* pgc)
        {
            if (!gc_registry_mutex)
                gc_registry_mutex = boost::make_shared<boost::mutex>();
            if (!gc_registry)
                gc_registry = boost::make_shared<gc_set>();
            boost::mutex::scoped_lock lock(*gc_registry_mutex);
            gc_registry->insert(pgc);
        }

        // unregister gc instance after thread termination
        static void unregister_gc(gc* pgc)
        {
            {
                boost::mutex::scoped_lock lock(*gc_registry_mutex);
                gc_registry->erase(pgc);
            }
            delete pgc;
        }

        // register new object in this gc instance
        inline void register_object(const gc_object* pobj)
        {
            ++register_count;
            object_registry.insert(std::make_pair(normalize_ptr(pobj), gc_node(pobj)));
        }

        // unregister destroyed object from this gc instance
        inline void unregister_object(const gc_object* pobj)
        {
            object_registry.erase(normalize_ptr(pobj));
        }

        // a default mark function called for pod types
        template <class OBJ>
        void mark(OBJ obj)
        {
            // do nothing
        }

        // mark object pointer as reachable
        template <class OBJ>
        void mark(OBJ* obj)
        {
            mark_object(reinterpret_cast<gc_object*>(obj));
        }

        // mark object pointer as reachable
        template <class OBJ>
        void mark(const gc_ptr<OBJ>& obj)
        {
            mark_object(obj.get());
        }

        // a default unmark function called for pod types
        template <class OBJ>
        void unmark(OBJ obj)
        {
            // do nothing
        }

        // mark object pointer as unreachable (used to force out of scope)
        template <class OBJ>
        void unmark(OBJ* obj)
        {
            unmark_object(reinterpret_cast<gc_object*>(obj));
        }

        // mark object pointer as unreachable (used to force out of scope)
        template <class OBJ>
        void unmark(const gc_ptr<OBJ>& obj)
        {
            unmark_object(obj.get());
        }

        // perform garbage collection (can force collection which ignores thresholds)
        void collect(bool force = false);

        // perform final collection when gc instance terminates
        void final_collect();

    private:
        // normalize given pointer to compensate for alignment
        inline void* normalize_ptr(const gc_object* pobj)
        {
            return (void*)((uintptr_t)pobj & ~0xf);
        }

        // retrieve current thread stack top address
        uintptr_t stack_top() const;

        // check thresholds and return true if collection should be performed
        bool check_threshold();

        // prepare mark token and transfer queue for collection
        void init_collect();

        // scan stack address space for object roots
        void scan_stack(node_map& roots);

        // recursively mark root objects
        void mark_objects(const node_map& roots);

        // mark given object pointer as reachable
        void mark_object(const gc_object* pobj);

        // mark given object pointer as unreachable
        void unmark_object(const gc_object* pobj);

        // sweep all unreachable objects to release queue
        void sweep_objects();

        // clean up release queue by transferring ownership or destroying objects
        void dispose_objects();

        // called when transferring objects from other gc instances
        void transfer(const node_map& transfer_nodes);
    };

    // function for retrieving the gc instance for current thread
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

    // function for retrieving the static gc instance for current thread
    static gc& get_static_gc()
    {
        static gc* static_gc = NULL;
        if (static_gc == NULL)
            static_gc = new gc(true);
        return *static_gc;
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
