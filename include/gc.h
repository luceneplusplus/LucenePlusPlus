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
#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/unordered_map.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>
#include <boost/preprocessor/punctuation.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/arithmetic.hpp>
#include "gc_ptr.h"

namespace lutze
{
    class gc;

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

        friend class gc;
    };

    // all garbage collected container classes must be derived from this class
    class gc_container
    {
    public:
        virtual gc_object* get_object() const
        {
            return NULL;
        }
    };

    class gc
    {
    public:
        gc(bool static_gc = false);
        ~gc();

        typedef std::set<gc*> gc_set;

    private:
        struct gc_node
        {
            gc_node(const gc_object* obj = NULL) : mark_token(0), object(obj) {}
            const gc_object* object;
            uint32_t mark_token;
            gc_set history;
        };

        typedef boost::unordered_map<const void*, gc_node> node_map;

        struct scoped_lock_if
        {
            scoped_lock_if(boost::mutex& mutex, bool lock_cond) : mutex(mutex), lock_cond(lock_cond)
            {
                if (lock_cond)
                    mutex.lock();
            }
            ~scoped_lock_if()
            {
                if (lock_cond)
                    mutex.unlock();
            }
            boost::mutex& mutex;
            bool lock_cond;
        };

        node_map object_registry;
        node_map unmark_objects;
        node_map release_queue;

        boost::mutex static_mutex;
        boost::mutex transfer_mutex;
        node_map transfer_queue;

        bool static_gc;
        uint32_t mark_token;
        uint32_t register_count;

    public:
        // return current lutze gc version
        static std::string gc_version();

        // initialize lutze garbage collector
        static bool gc_init();

        // terminate and clean up lutze garbage collector
        static void gc_term();

        // register gc instance at thread launch
        static void register_gc(gc* pgc);

        // unregister gc instance after thread termination
        static void unregister_gc(gc* pgc);

        // retrieve the gc instance for current thread
        static gc& get_gc();

        // retrieve the static gc instance for current thread
        static gc& get_static_gc();

        // register new object in this gc instance
        inline void register_object(const gc_object* pobj)
        {
            scoped_lock_if lock(static_mutex, static_gc);
            ++register_count;
            object_registry.insert(std::make_pair(normalize_ptr(pobj), gc_node(pobj)));
        }

        // unregister destroyed object from this gc instance
        inline void unregister_object(const gc_object* pobj)
        {
            scoped_lock_if lock(static_mutex, static_gc);
            object_registry.erase(normalize_ptr(pobj));
        }

        // a default mark function called for pod types
        template <class OBJ>
        void mark(OBJ obj, typename boost::disable_if< boost::is_convertible<OBJ, gc_container> >::type* dummy = 0)
        {
            // do nothing
        }

        // mark container as reachable
        void mark(const gc_container& obj)
        {
            mark_object(obj.get_object());
        }

        // mark object pointer as reachable
        template <class OBJ>
        void mark(const gc_ptr<OBJ>& obj)
        {
            mark_object(reinterpret_cast<gc_object*>(obj.get()));
        }

        // a default unmark function called for pod types
        template <class OBJ>
        void unmark(OBJ obj, typename boost::disable_if< boost::is_convertible<OBJ, gc_container> >::type* dummy = 0)
        {
            // do nothing
        }

        // mark container as unreachable
        void unmark(const gc_container& obj)
        {
            mark_object(obj.get_object());
        }

        // mark object pointer as unreachable (used to force out of scope)
        template <class OBJ>
        void unmark(const gc_ptr<OBJ>& obj)
        {
            unmark_object(reinterpret_cast<gc_object*>(obj.get()));
        }

        // check threshold before performing collection
        void collect(bool force = false);

        // perform final collection when gc instance terminates
        void final_collect();

    private:
        static boost::mutex& gc_registry_mutex(); // todo
        static gc_set& gc_registry(); // todo

        // normalize given pointer to compensate for alignment
        inline void* normalize_ptr(const gc_object* pobj)
        {
            return (void*)((uintptr_t)pobj & ~0xf);
        }

        void static_collect(bool force);

        // retrieve current thread stack top address
        uintptr_t stack_top() const;

        // check thresholds and return true if collection should be performed
        bool check_threshold();

        // prepare mark token and transfer queue for collection
        void init_collect();

        // scan stack address space for object roots
        void find_roots(node_map& roots);

        // recursively mark root objects
        void mark_objects(const node_map& roots);

        // mark given object pointer as reachable
        void mark_object(const gc_object* pobj);

        // mark given object pointer as unreachable
        void unmark_object(const gc_object* pobj);

        // sweep all unreachable objects to release queue
        void sweep_objects();

        // clean up release queue by transferring ownership or destroying objects
        void dispose_objects(bool destroy = false);

        // called when transferring objects from other gc instances
        void transfer(const node_map& transfer_nodes);
    };

    // The following expands to...
    // template <class T, class A1, ... class A9>
    // gc_ptr<T> new_gc(const A1& a1, ... const A9& a9)
    // ...
    // template <class T, class A1, ... class A9>
    // gc_ptr<T> new_static_gc(const A1& a1, ... const A9& a9)
    // ...

    #define NEW_GC(Z, N, _) \
    template<class T BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, class A)> \
    gc_ptr<T> new_gc(BOOST_PP_ENUM_BINARY_PARAMS(N, const A, & a)) \
    { \
        gc& gc = gc::get_gc(); \
        T* pobj = new T(BOOST_PP_ENUM_PARAMS(N, a)); \
        gc.register_object(static_cast<gc_object*>(pobj)); \
        gc.collect(); \
        return gc_ptr<T>(pobj); \
    } \
    template<class T BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, class A)> \
    gc_ptr<T> new_static_gc(BOOST_PP_ENUM_BINARY_PARAMS(N, const A, & a)) \
    { \
        gc& gc = gc::get_static_gc(); \
        T* pobj = new T(BOOST_PP_ENUM_PARAMS(N, a)); \
        gc.register_object(static_cast<gc_object*>(pobj)); \
        return gc_ptr<T>(pobj); \
    }
    BOOST_PP_REPEAT_2ND(BOOST_PP_INC(9), NEW_GC, _)
}

#endif
