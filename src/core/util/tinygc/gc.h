/*
 * @(#) gc.h -- TinyGC (Tiny Garbage Collector) header.
 * Copyright (C) 2006-2010 Ivan Maidanski <ivmai@mail.ru> All rights reserved.
 **
 * Version: 2.6
 * See also files: tinygc.c, gc_gcj.h, gc_mark.h, javaxfc.h
 * Required: any ANSI C compiler (assume GC-safe compilation).
 */

/*
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 **
 * This software is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License (GPL) for more details.
 **
 * Linking this library statically or dynamically with other modules is
 * making a combined work based on this library. Thus, the terms and
 * conditions of the GNU General Public License cover the whole
 * combination.
 **
 * As a special exception, the copyright holders of this library give you
 * permission to link this library with independent modules to produce an
 * executable, regardless of the license terms of these independent
 * modules, and to copy and distribute the resulting executable under
 * terms of your choice, provided that you also meet, for each linked
 * independent module, the terms and conditions of the license of that
 * module. An independent module is a module which is not derived from
 * or based on this library. If you modify this library, you may extend
 * this exception to your version of the library, but you are not
 * obligated to do so. If you do not wish to do so, delete this
 * exception statement from your version.
 */

#ifndef GC_H
#define GC_H

#define GC_TINYGC_VER 260 /* TinyGC v2.6 */

/* TinyGC API is a subset of Boehm-Demers-Weiser Conservative GC API v7.2 */

/*
 * Control macros: GC_DLL, GC_DONT_EXPAND, GC_STACKBASE_WITH_REGBASE,
 * GC_THREADS.
 * Macros for tuning: CONST, GC_API, GC_CALL, GC_CALLBACK, GC_DATASTART,
 * GC_DATASTART2, GC_DATASTARTSYM, GC_DATASTARTSYM2, GC_DATAEND, GC_DATAEND2,
 * GC_DATAENDSYM, GC_DATAENDSYM2, GC_FREE_SPACE_DIVISOR, GC_INITIAL_HEAP_SIZE,
 * GC_MAXIMUM_HEAP_SIZE, GC_MAX_RETRIES, GC_NEAR, GC_SIGNEDWORD.
 */

#ifndef _STDDEF_H
#include <stddef.h>
/* typedef size_t; */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef GC_API
#ifdef GC_DLL
#define GC_API __declspec(dllimport)
#else
#define GC_API extern
#endif
#endif

#ifndef GC_CALL
#define GC_CALL /* empty */
#endif

#ifndef GC_CALLBACK
#define GC_CALLBACK GC_CALL
#endif

#ifndef GC_NEAR
/* TinyGC-specific */
#define GC_NEAR /* empty */
#endif

#ifndef GC_SIGNEDWORD
/* TinyGC-specific */
#ifdef _WIN64
#define GC_SIGNEDWORD __int64
#else
#ifdef _LLP64
#define GC_SIGNEDWORD long long
#else
#ifdef _LP64
#define GC_SIGNEDWORD long
#endif
#endif
#endif
#endif

#ifndef GC_STACKBASE_WITH_REGBASE
/* TinyGC-specific */
#ifdef _M_IA64
#define GC_STACKBASE_WITH_REGBASE 1
#else
#ifdef __ia64__
#define GC_STACKBASE_WITH_REGBASE 1
#endif
#endif
#endif

#ifdef GC_SIGNEDWORD
typedef unsigned GC_SIGNEDWORD GC_word;
/* sizeof(GC_word) == sizeof(void GC_NEAR *) */
#else
typedef unsigned GC_word;
#endif

#define GC_MALLOC(size) GC_malloc(size)
#define GC_MALLOC_ATOMIC(size) GC_malloc_atomic(size)

#define GC_NEW(t) ((t GC_NEAR *)GC_MALLOC(sizeof(t)))
#define GC_NEW_ATOMIC(t) ((t GC_NEAR *)GC_MALLOC_ATOMIC(sizeof(t)))

#define GC_GENERAL_REGISTER_DISAPPEARING_LINK(link, obj) GC_general_register_disappearing_link(link, obj)
#define GC_REGISTER_FINALIZER_NO_ORDER(obj, fn, client_data, ofn, odata) GC_register_finalizer_no_order(obj, fn, client_data, ofn, odata)

typedef void (GC_CALLBACK *GC_finalizer_notifier_proc)(void);
typedef int (GC_CALLBACK *GC_stop_func)(void);
typedef void (GC_CALLBACK *GC_finalization_proc)(void GC_NEAR *,
 void GC_NEAR *);
typedef void (GC_CALLBACK *GC_warn_proc)(char GC_NEAR *, GC_word);
typedef void GC_NEAR *(GC_CALLBACK *GC_fn_type)(void GC_NEAR *);

GC_API GC_word GC_CALL GC_get_gc_no(void);

GC_API void GC_CALL GC_set_all_interior_pointers(int);
GC_API void GC_CALL GC_set_finalize_on_demand(int);
GC_API void GC_CALL GC_set_java_finalization(int);
GC_API void GC_CALL GC_set_dont_expand(int);
GC_API void GC_CALL GC_set_no_dls(int);
GC_API void GC_CALL GC_set_dont_precollect(int);

GC_API void GC_CALL GC_set_free_space_divisor(GC_word);
GC_API void GC_CALL GC_set_max_retries(GC_word);

GC_API void GC_CALL GC_set_finalizer_notifier(GC_finalizer_notifier_proc);
GC_API GC_finalizer_notifier_proc GC_CALL GC_get_finalizer_notifier(void);

GC_API void GC_CALL GC_init(void);

GC_API void GC_NEAR *GC_CALL GC_malloc(size_t);
GC_API void GC_NEAR *GC_CALL GC_malloc_atomic(size_t);

GC_API void GC_NEAR *GC_CALL GC_base(void GC_NEAR *);

GC_API int GC_CALL GC_expand_hp(size_t);
GC_API void GC_CALL GC_set_max_heap_size(GC_word);

GC_API void GC_CALL GC_exclude_static_roots(void GC_NEAR *, void GC_NEAR *);
GC_API void GC_CALL GC_clear_roots(void);
GC_API void GC_CALL GC_add_roots(void GC_NEAR *, void GC_NEAR *);
GC_API void GC_CALL GC_remove_roots(void GC_NEAR *, void GC_NEAR *);

GC_API void GC_CALL GC_gcollect(void);
GC_API void GC_CALL GC_gcollect_and_unmap(void);
GC_API int GC_CALL GC_try_to_collect(GC_stop_func);

GC_API void GC_CALL GC_set_stop_func(GC_stop_func);
GC_API GC_stop_func GC_CALL GC_get_stop_func(void);

GC_API size_t GC_CALL GC_get_heap_size(void);
GC_API size_t GC_CALL GC_get_free_bytes(void);
GC_API size_t GC_CALL GC_get_bytes_since_gc(void);
GC_API size_t GC_CALL GC_get_total_bytes(void);

GC_API void GC_CALL GC_disable(void);
GC_API void GC_CALL GC_enable(void);

GC_API void GC_CALL GC_enable_incremental(void);

GC_API void GC_CALL GC_register_finalizer_no_order(void GC_NEAR *,
 GC_finalization_proc, void GC_NEAR *, GC_finalization_proc GC_NEAR *,
 void GC_NEAR *GC_NEAR *);

#define GC_NO_MEMORY 2

GC_API int GC_CALL GC_general_register_disappearing_link(
 void GC_NEAR *GC_NEAR *, void GC_NEAR *);
GC_API int GC_CALL GC_unregister_disappearing_link(void GC_NEAR *GC_NEAR *);

GC_API int GC_CALL GC_should_invoke_finalizers(void);
GC_API int GC_CALL GC_invoke_finalizers(void);

GC_API void GC_CALL GC_set_warn_proc(GC_warn_proc);
GC_API GC_warn_proc GC_CALL GC_get_warn_proc(void);
GC_API void GC_CALLBACK GC_ignore_warn_proc(char GC_NEAR *, GC_word);

GC_API void GC_NEAR *GC_CALL GC_call_with_alloc_lock(GC_fn_type,
 void GC_NEAR *);

#define GC_SUCCESS 0
#define GC_DUPLICATE 1
#define GC_UNIMPLEMENTED 3

struct GC_stack_base
{
 void GC_NEAR *mem_base;
#ifdef GC_STACKBASE_WITH_REGBASE
 void GC_NEAR *reg_base; /* not used by TinyGC */
#endif
};

typedef void GC_NEAR *(GC_CALLBACK *GC_stack_base_func)(
 struct GC_stack_base GC_NEAR *, void GC_NEAR *);

GC_API void GC_NEAR *GC_CALL GC_call_with_stack_base(GC_stack_base_func,
 void GC_NEAR *);
GC_API int GC_CALL GC_get_stack_base(struct GC_stack_base GC_NEAR *);

GC_API void GC_NEAR *GC_CALL GC_do_blocking(GC_fn_type, void GC_NEAR *);
GC_API void GC_NEAR *GC_CALL GC_call_with_gc_active(GC_fn_type,
 void GC_NEAR *);

#ifdef GC_THREADS

#ifndef GC_NO_THREAD_REDIRECTS
/* No "implicit thread registration" mode */
#define GC_NO_THREAD_REDIRECTS
#endif

GC_API void GC_CALL GC_allow_register_threads(void);
GC_API int GC_CALL GC_register_my_thread(
 CONST struct GC_stack_base GC_NEAR *);
GC_API int GC_CALL GC_unregister_my_thread(void);

#endif

GC_API void GC_CALL GC_set_force_unmap_on_gcollect(int);

#ifdef GC_DATASTARTSYM
extern char GC_DATASTARTSYM;
#ifndef GC_DATASTART
#define GC_DATASTART ((void GC_NEAR *)&GC_DATASTARTSYM)
#endif
#endif

#ifdef GC_DATAENDSYM
extern char GC_DATAENDSYM;
#ifndef GC_DATAEND
#define GC_DATAEND ((void GC_NEAR *)&GC_DATAENDSYM)
#endif
#endif

#ifdef GC_DATASTARTSYM2
extern char GC_DATASTARTSYM2;
#ifndef GC_DATASTART2
#define GC_DATASTART2 ((void GC_NEAR *)&GC_DATASTARTSYM2)
#endif
#endif

#ifdef GC_DATAENDSYM2
extern char GC_DATAENDSYM2;
#ifndef GC_DATAEND2
#define GC_DATAEND2 ((void GC_NEAR *)&GC_DATAENDSYM2)
#endif
#endif

#ifdef GC_DATASTART
#ifndef GC_DATAEND
#define GC_DATAEND GC_DATASTART
#endif
#ifdef GC_DATASTART2
#ifndef GC_DATAEND2
#define GC_DATAEND2 GC_DATASTART2
#endif
#define GC_INIT_CONF_ROOTS (GC_add_roots(GC_DATASTART, GC_DATAEND), GC_add_roots(GC_DATASTART2, GC_DATAEND2))
#else
#define GC_INIT_CONF_ROOTS GC_add_roots(GC_DATASTART, GC_DATAEND)
#endif
#else
#define GC_INIT_CONF_ROOTS /* empty */
#endif

#ifdef GC_DONT_EXPAND
#define GC_INIT_CONF_DONT_EXPAND GC_set_dont_expand(1)
#else
#define GC_INIT_CONF_DONT_EXPAND /* empty */
#endif

#ifdef GC_MAX_RETRIES
#define GC_INIT_CONF_MAX_RETRIES GC_set_max_retries(GC_MAX_RETRIES)
#else
#define GC_INIT_CONF_MAX_RETRIES /* empty */
#endif

#ifdef GC_FREE_SPACE_DIVISOR
#define GC_INIT_CONF_FREE_SPACE_DIVISOR GC_set_free_space_divisor(GC_FREE_SPACE_DIVISOR)
#else
#define GC_INIT_CONF_FREE_SPACE_DIVISOR /* empty */
#endif

#ifdef GC_MAXIMUM_HEAP_SIZE
#define GC_INIT_CONF_MAXIMUM_HEAP_SIZE GC_set_max_heap_size(GC_MAXIMUM_HEAP_SIZE)
#else
#define GC_INIT_CONF_MAXIMUM_HEAP_SIZE /* empty */
#endif

#ifdef GC_INITIAL_HEAP_SIZE
#define GC_INIT_CONF_INITIAL_HEAP_SIZE { size_t GC_heap_size; (void)((GC_heap_size = GC_get_heap_size()) < (size_t)(GC_INITIAL_HEAP_SIZE) ? GC_expand_hp((size_t)(GC_INITIAL_HEAP_SIZE) - GC_heap_size) : 0); }
#else
#define GC_INIT_CONF_INITIAL_HEAP_SIZE /* empty */
#endif

#define GC_INIT() { GC_INIT_CONF_DONT_EXPAND; GC_INIT_CONF_MAX_RETRIES; GC_INIT_CONF_FREE_SPACE_DIVISOR; GC_INIT_CONF_MAXIMUM_HEAP_SIZE; GC_init(); GC_INIT_CONF_ROOTS; GC_INIT_CONF_INITIAL_HEAP_SIZE; }

#ifdef __cplusplus
}
#endif

#endif
