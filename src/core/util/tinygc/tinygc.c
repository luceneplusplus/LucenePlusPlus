/*
 * @(#) tinygc.c -- TinyGC (Tiny Garbage Collector) source.
 * Copyright (C) 2006-2010 Ivan Maidanski <ivmai@mail.ru> All rights reserved.
 **
 * Version: 2.6
 * See also files: gc.h, gc_gcj.h, gc_mark.h, javaxfc.h
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

/*
 * Control macros: ALL_INTERIOR_POINTERS, DONT_ADD_BYTE_AT_END,
 * FINALIZE_ON_DEMAND, GC_DLL, GC_DONT_EXPAND, GC_GCJ_SUPPORT, GC_GETENV_SKIP,
 * GC_IGNORE_GCJ_INFO, GC_MISC_EXCLUDE, GC_NO_DLINKS, GC_NO_FNLZ,
 * GC_NO_GCBASE, GC_NO_INACTIVE, GC_NO_REGISTER_DLINK,
 * GC_OMIT_REGISTER_KEYWORD, GC_PRINT_MSGS, GC_THREADS, GC_USE_GETTIMEOFDAY,
 * GC_USE_WIN32_SYSTEMTIME, GC_WIN32_THREADS, GC_WIN32_WCE,
 * JAVA_FINALIZATION_NOT_NEEDED.
 **
 * Macros for tuning (also see in gc.h): CONST, GC_ASYNC_PUSHREGS_BEGIN,
 * GC_ASYNC_PUSHREGS_END, GC_CLIBDECL, GC_CORE_API, GC_CORE_CALL,
 * GC_CORE_FREE, GC_CORE_MALLOC, GC_DATASTATIC, GC_DATASTART, GC_DATASTART2,
 * GC_DATAEND, GC_DATAEND2, GC_FATAL_ABORT, GC_FREE_SPACE_DIVISOR,
 * GC_MAX_RETRIES, GC_FASTCALL, GC_INLINE_STATIC, GC_LAZYREFILL_BIGCNT,
 * GC_LAZYREFILL_COUNT, GC_LOG2_OFFIGNORE, GC_NEW_LINE, GC_PUSHREGS_BEGIN,
 * GC_PUSHREGS_END, GC_SIG_SUSPEND, GC_STACKBOTTOM, GC_STACKBOTTOMVAR,
 * GC_STACKLEN, GC_STACKLENVAR, GC_STATIC, GC_THREAD_MUTEX_DEFATTR,
 * GC_THREAD_YIELD, GC_WIN32_CONTEXT_SP_NAME, GC_YIELD_MAX_ATTEMPT, INLINE,
 * MARK_DESCR_OFFSET.
 */

#ifndef _SETJMP_H
#include <setjmp.h>
/* int setjmp(jmp_buf); */
#endif

#ifndef _STDLIB_H
#include <stdlib.h>
/* long atol(const char *); */
/* void exit(int); */
/* void free(void *); */
/* char *getenv(const char *); */
/* void *malloc(size_t); */
#endif

#ifndef _STRING_H
#include <string.h>
/* void *memset(void *, int, size_t); */
#endif

#ifndef _LIMITS_H
#include <limits.h>
#endif

#ifdef GC_WIN32_THREADS

#ifndef _WINDOWS_H
#include <windows.h>
/* BOOL CloseHandle(HANDLE); */
/* HANDLE CreateEvent(SECURITY_ATTRIBUTES *, BOOL, BOOL, LPCTSTR); */
/* BOOL DuplicateHandle(HANDLE, HANDLE, HANDLE, HANDLE *, DWORD, BOOL, DWORD); */
/* HANDLE GetCurrentProcess(void); */
/* HANDLE GetCurrentThread(void); */
/* DWORD GetCurrentThreadId(void); */
/* BOOL GetThreadContext(HANDLE, CONTEXT *); */
/* LONG InterlockedExchange(LONG *, LONG); */
/* DWORD ResumeThread(HANDLE); */
/* BOOL SetEvent(HANDLE); */
/* void Sleep(DWORD); */
/* DWORD SuspendThread(HANDLE); */
/* DWORD WaitForSingleObject(HANDLE, DWORD); */
#endif

#ifndef GC_THREADS
#define GC_THREADS 1
#endif

#else /* GC_WIN32_THREADS */

#ifdef GC_THREADS

#ifndef _ERRNO_H
#include <errno.h>
/* int errno; */
#endif

#ifndef _SIGNAL_H
#include <signal.h>
/* void (*signal(int, void (*)(int)))(int); */
#endif

#ifndef _PTHREAD_H
#include <pthread.h>
/* int pthread_kill(pthread_t, int); */
/* int pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *); */
/* int pthread_mutex_lock(pthread_mutex_t *); */
/* int pthread_mutex_unlock(pthread_mutex_t *); */
/* pthread_t pthread_self(void); */
#endif

#ifndef _SCHED_H
#include <sched.h>
/* int sched_yield(void); */
#endif

#ifdef pthread_usleep_np
/* #include <pthread.h> */
/* unsigned pthread_usleep_np(unsigned); */
#else
#ifndef _UNISTD_H
#include <unistd.h>
/* int usleep(useconds_t); */
#endif
#define pthread_usleep_np usleep
#endif

#endif /* GC_THREADS */

#endif /* ! GC_WIN32_THREADS */

#ifdef GC_PRINT_MSGS

#ifndef _STDIO_H
#include <stdio.h>
/* int fprintf(FILE *, const char *, ...); */
/* FILE * const stderr; */
/* FILE * const stdout; */
#endif

#ifdef GC_USE_WIN32_SYSTEMTIME

#ifndef _WINDOWS_H
#include <windows.h>
/* void GetSystemTime(SYSTEMTIME *); */
#endif

#define GC_CURTIME_T SYSTEMTIME
#define GC_CURTIME_GETMS(pcurt) (GetSystemTime(pcurt), ((((unsigned long)(pcurt)->wDay * 24 + (unsigned long)(pcurt)->wHour) * 60 + (unsigned long)(pcurt)->wMinute) * 60 + (unsigned long)(pcurt)->wSecond) * 1000 + (unsigned long)(pcurt)->wMilliseconds)

#else /* GC_USE_WIN32_SYSTEMTIME */

#ifdef GC_USE_GETTIMEOFDAY

#ifndef _SYS_TIME_H
#include <sys/time.h>
/* int gettimeofday(struct timeval *, void *); */
#endif

#define GC_CURTIME_T struct timeval

#ifdef _SVID_GETTOD
#define GC_CURTIME_GETMS(pcurt) (gettimeofday((void *)(pcurt)), (unsigned long)(pcurt)->tv_sec * 1000 + (unsigned long)(pcurt)->tv_usec / 1000)
#else
#define GC_CURTIME_GETMS(pcurt) (gettimeofday((void *)(pcurt), NULL), (unsigned long)(pcurt)->tv_sec * 1000 + (unsigned long)(pcurt)->tv_usec / 1000)
#endif

#else /* GC_USE_GETTIMEOFDAY */

#ifndef _TIME_H
#include <time.h>
#endif

#ifndef _SYS_TIMEB_H
#include <sys/timeb.h>
/* void ftime(struct timeb *); */
#endif

#define GC_CURTIME_T struct timeb
#define GC_CURTIME_GETMS(pcurt) (ftime(pcurt), (unsigned long)(pcurt)->time * 1000 + (unsigned long)(pcurt)->millitm)

#endif /* ! GC_USE_GETTIMEOFDAY */

#endif /* ! GC_USE_WIN32_SYSTEMTIME */

#define GC_SIZE_TO_ULKB(size) ((unsigned long)((size) >> 10))

#ifndef GC_NEW_LINE
#define GC_NEW_LINE "\n"
#endif

#endif /* GC_PRINT_MSGS */

#ifndef GC_API
#ifdef GC_DLL
#define GC_API __declspec(dllexport)
#endif
#endif

#include "gc.h"

#include "gc_gcj.h"

#include "gc_mark.h"

#include "javaxfc.h"

#ifndef NULL
#define NULL (void *)0
#endif

#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

#ifndef CONST
#define CONST const
#endif

#ifndef GC_FATAL_ABORT
#define GC_FATAL_ABORT exit(-1) /* abort(), DebugBreak() */
#endif

#ifndef GC_DATASTATIC
#define GC_DATASTATIC static
#endif

#ifndef GC_STATIC
#define GC_STATIC static
#endif

#ifndef GC_INLINE_STATIC
#ifdef INLINE
#define GC_INLINE_STATIC GC_STATIC INLINE
#else
#define GC_INLINE_STATIC GC_STATIC __inline
#endif
#endif

#ifndef GC_FASTCALL
#define GC_FASTCALL __fastcall
#endif

#ifndef GC_CORE_API
#define GC_CORE_API extern
#endif

#ifndef GC_CORE_CALL
#define GC_CORE_CALL GC_CALL
#endif

#ifdef GC_PUSHREGS_BEGIN
#ifndef GC_PUSHREGS_END
#define GC_PUSHREGS_END (void)0
#endif
#else
#define GC_PUSHREGS_BEGIN jmp_buf buf; (void)setjmp(buf)
#ifndef GC_PUSHREGS_END
#define GC_PUSHREGS_END GC_noop1((GC_word)(&buf))
#endif
#endif

#define GC_MEM_BZERO(ptr, size) (void)memset(ptr, '\0', (size_t)(size))

#ifdef GC_THREADS

#ifdef GC_WIN32_THREADS

#ifndef GC_WIN32_CONTEXT_SP_NAME
#ifdef _M_AMD64
#define GC_WIN32_CONTEXT_SP_NAME Rsp
#else
#ifdef _M_X64
#define GC_WIN32_CONTEXT_SP_NAME Rsp
#else
#ifdef __x86_64
#define GC_WIN32_CONTEXT_SP_NAME Rsp
#endif
#endif
#endif
#endif

#ifndef GC_WIN32_CONTEXT_SP_NAME
#ifdef _M_ALPHA
#define GC_WIN32_CONTEXT_SP_NAME IntSp
#else
#ifdef _ALPHA_
#define GC_WIN32_CONTEXT_SP_NAME IntSp
#else
#ifdef _M_MRX000
#define GC_WIN32_CONTEXT_SP_NAME IntSp
#else
#ifdef _MIPS_
#define GC_WIN32_CONTEXT_SP_NAME IntSp
#endif
#endif
#endif
#endif
#endif

#ifndef GC_WIN32_CONTEXT_SP_NAME
#ifdef _M_ARM
#define GC_WIN32_CONTEXT_SP_NAME Sp
#else
#ifdef _ARM_
#define GC_WIN32_CONTEXT_SP_NAME Sp
#else
#ifdef _M_PPC
#define GC_WIN32_CONTEXT_SP_NAME Gpr1
#else
#ifdef _PPC_
#define GC_WIN32_CONTEXT_SP_NAME Gpr1
#else
#ifdef _M_SH
#define GC_WIN32_CONTEXT_SP_NAME R15
#else
#ifdef SHx
#define GC_WIN32_CONTEXT_SP_NAME R15
#endif
#endif
#endif
#endif
#endif
#endif
#endif

#ifndef GC_WIN32_CONTEXT_SP_NAME
#define GC_WIN32_CONTEXT_SP_NAME Esp /* x86 */
#endif

#ifndef GC_THREAD_YIELD
#define GC_THREAD_YIELD Sleep(10) /* "long" yield */
#endif

#define GC_THREAD_ID_T DWORD

#ifdef GC_WIN32_WCE
#define GC_THREAD_HANDLE(stkroot) ((HANDLE)(GC_word)(stkroot)->thread_id)
#else
#define GC_THREAD_HANDLE(stkroot) ((stkroot)->thread_handle)
#endif

#else /* GC_WIN32_THREADS */

#ifndef GC_SIG_SUSPEND
#ifdef SIGPWR
#define GC_SIG_SUSPEND SIGPWR
#else
#ifdef SIGUSR1
#define GC_SIG_SUSPEND SIGUSR1
#else
#define GC_SIG_SUSPEND SIGILL
#endif
#endif
#endif

#ifndef GC_CLIBDECL
#ifdef __CLIB
#define GC_CLIBDECL __CLIB
#else
#ifdef _USERENTRY
#define GC_CLIBDECL _USERENTRY
#else
#ifdef _RTL_FUNC
#define GC_CLIBDECL _RTL_FUNC
#else
#define GC_CLIBDECL __cdecl
#endif
#endif
#endif
#endif

#ifdef GC_ASYNC_PUSHREGS_BEGIN
#ifndef GC_ASYNC_PUSHREGS_END
#define GC_ASYNC_PUSHREGS_END (void)0
#endif
#else
#define GC_ASYNC_PUSHREGS_BEGIN GC_PUSHREGS_BEGIN
#ifndef GC_ASYNC_PUSHREGS_END
#define GC_ASYNC_PUSHREGS_END GC_PUSHREGS_END
#endif
#endif

#ifndef GC_THREAD_MUTEX_DEFATTR
#ifdef pthread_mutexattr_default
#define GC_THREAD_MUTEX_DEFATTR pthread_mutexattr_default
#else
#define GC_THREAD_MUTEX_DEFATTR NULL
#endif
#endif

#ifndef GC_THREAD_YIELD
#define GC_THREAD_YIELD (void)sched_yield()
#endif

#ifndef GC_YIELD_MAX_ATTEMPT
#define GC_YIELD_MAX_ATTEMPT 2
#endif

#define GC_ERRNO_SET(value) (void)(errno = (value))

#define GC_THREAD_ID_T pthread_t

#endif /* ! GC_WIN32_THREADS */

#endif /* GC_THREADS */

#ifndef MARK_DESCR_OFFSET
#define MARK_DESCR_OFFSET sizeof(GC_word)
#endif

#ifndef GC_LOG2_OFFIGNORE
#define GC_LOG2_OFFIGNORE 8 /* must be at least 3 */
#endif

#ifndef GC_FREE_SPACE_DIVISOR
#define GC_FREE_SPACE_DIVISOR 3
#endif

#ifndef GC_MAX_RETRIES
#define GC_MAX_RETRIES 2
#endif

#ifndef GC_LAZYREFILL_COUNT
#define GC_LAZYREFILL_COUNT 10 /* must be at least 3 */
#endif

#ifndef GC_LAZYREFILL_BIGCNT
#define GC_LAZYREFILL_BIGCNT 1024
#endif

#ifdef GC_OMIT_REGISTER_KEYWORD
#define GC_REGISTER_KEYWORD /* empty */
#else
#define GC_REGISTER_KEYWORD register
#endif

#define GC_DEFAULT_LOG2_OBJSIZE 8
#define GC_DEFAULT_LOG2_SIZE 3

#define GC_MEM_SIZELIMIT ((GC_word)((~(size_t)0) >> 1) - ((GC_word)1 << (sizeof(int) << 1)))

#define GC_ATOMIC_MASK (((~(GC_word)0) >> 1) + 1)

#ifdef GC_GCJ_SUPPORT
#define GC_HASDSLEN_MASK (((GC_word)GC_ATOMIC_MASK) >> 1)
#else
#define GC_HASDSLEN_MASK 0
#endif

#define GC_NEVER_COLLECT (int)((((unsigned)-1) >> 1) + 1)

#ifndef GC_NO_DLINKS
#define GC_HIDE_POINTER(ptr) (~(GC_word)(ptr))
#endif

#define GC_RANDOM_SEED(gcdata) (((gcdata)->total_heapsize ^ (gcdata)->allocd_before_gc) + ((gcdata)->bytes_allocd ^ (gcdata)->marked_bytes) + ((gcdata)->free_bytes ^ (gcdata)->obj_htable.pending_free_size))
#define GC_HASH_INDEX(word_value, seed, log2_size) ((((word_value) ^ (seed)) * (GC_word)0x9E3779B1L) >> (sizeof(GC_word) * CHAR_BIT - (log2_size)))
#define GC_HASH_RESIZECOND(count, log2_size) (((GC_word)3 << ((log2_size) - 2)) <= (count))

#define GC_LEAVE(gcdata) GC_leave()

#ifdef GC_CORE_MALLOC
GC_CORE_API void *GC_CORE_CALL GC_CORE_MALLOC(size_t size);
#else
#define GC_CORE_MALLOC malloc
#endif

#ifdef GC_CORE_FREE
GC_CORE_API void GC_CORE_CALL GC_CORE_FREE(void *ptr);
#else
#define GC_CORE_FREE free
#endif

#ifdef GC_STACKBOTTOMVAR
extern char *GC_STACKBOTTOMVAR;
#endif

#ifdef GC_STACKLENVAR
extern GC_word GC_STACKLENVAR;
#endif

#ifndef GC_STACKBOTTOM
#ifdef GC_STACKBOTTOMVAR
#define GC_STACKBOTTOM GC_STACKBOTTOMVAR
#else
#define GC_STACKBOTTOM 0
#endif
#endif

#ifndef GC_STACKLEN
#ifdef GC_STACKLENVAR
#define GC_STACKLEN GC_STACKLENVAR
#else
#define GC_STACKLEN 0
#endif
#endif

struct GC_objlink_s
{
 void *obj;
 struct GC_objlink_s *next;
 GC_word atomic_and_size;
};

struct GC_obj_htable_s
{
 struct GC_objlink_s **hroots;
 struct GC_objlink_s *free_list;
 struct GC_objlink_s *marked_list;
 struct GC_objlink_s *follow_list;
 struct GC_objlink_s *unlinked_list;
 GC_word min_obj_addr;
 GC_word max_obj_addr;
 GC_word count;
 GC_word log2_size;
 GC_word pending_free_size;
};

#ifndef GC_NO_DLINKS

struct GC_dlink_s
{
 struct GC_dlink_s *next;
 struct GC_objlink_s *objlink;
 GC_word hidden_link;
};

struct GC_dlink_htable_s
{
 struct GC_dlink_s **hroots;
 struct GC_dlink_s *free_list;
 GC_word count;
 GC_word log2_size;
 GC_word seed;
};

#endif /* ! GC_NO_DLINKS */

#ifndef GC_NO_FNLZ

struct GC_fnlz_s
{
 struct GC_fnlz_s *next;
 struct GC_objlink_s *objlink;
 void *client_data;
 GC_finalization_proc fn;
};

struct GC_fnlz_htable_s
{
 struct GC_fnlz_s **hroots;
 struct GC_fnlz_s *ready_fnlz;
 struct GC_fnlz_s *single_free;
 GC_word count;
 GC_word log2_size;
 GC_word seed;
 int has_client_ptrs;
};

#endif /* ! GC_NO_FNLZ */

#ifndef GC_NO_INACTIVE

struct GC_activation_frame_s
{
 GC_word inactive_sp;
 CONST struct GC_activation_frame_s *prev;
};

#endif /* ! GC_NO_INACTIVE */

struct GC_dataroot_s
{
 struct GC_dataroot_s *next;
 GC_word begin_addr;
 GC_word end_addr;
};

struct GC_stkroot_s
{
 GC_word begin_addr;
 GC_word end_addr;
#ifndef GC_NO_INACTIVE
 CONST struct GC_activation_frame_s *activation_frame;
#endif
#ifdef GC_THREADS
 struct GC_stkroot_s *next;
 GC_THREAD_ID_T thread_id;
#ifdef GC_WIN32_THREADS
#ifndef GC_WIN32_WCE
 HANDLE thread_handle;
#endif
#else
 volatile int suspend_ack;
#endif
#endif
#ifndef GC_NO_INACTIVE
 int inactive;
#endif
#ifndef GC_NO_FNLZ
 int inside_fnlz;
#endif
};

#ifdef GC_THREADS

struct GC_stkroot_htable_s
{
 struct GC_stkroot_s **hroots;
 GC_word count;
 GC_word log2_size;
 GC_word seed;
};

#endif /* GC_THREADS */

struct GC_gcdata_s
{
 struct GC_obj_htable_s obj_htable;
 void *objlinks_block_list;
#ifndef GC_NO_DLINKS
 struct GC_dlink_htable_s dlink_htable;
#endif
#ifndef GC_NO_FNLZ
 struct GC_fnlz_htable_s fnlz_htable;
 GC_word notifier_gc_no;
 GC_word bytes_finalized;
#endif
 struct GC_stkroot_s *cur_stack;
 struct GC_dataroot_s *dataroots;
 GC_word dataroot_size;
 GC_word expanded_heapsize;
 GC_word total_heapsize;
 GC_word allocd_before_gc;
 GC_word bytes_allocd;
 GC_word marked_bytes;
 GC_word free_bytes;
 GC_word followscan_size;
#ifdef GC_THREADS
 struct GC_stkroot_htable_s stkroot_htable;
#endif
 int recycling;
#ifdef GC_GCJ_SUPPORT
#ifndef GC_GETENV_SKIP
#ifndef GC_IGNORE_GCJ_INFO
 int ignore_gcj_info;
#endif
#endif
#endif
};

volatile GC_word GC_noop_sink;

GC_DATASTATIC GC_word GC_gc_no = 0;

GC_DATASTATIC GC_word GC_free_space_divisor = (GC_FREE_SPACE_DIVISOR);
GC_DATASTATIC GC_word GC_max_retries = (GC_MAX_RETRIES);

GC_DATASTATIC GC_finalizer_notifier_proc GC_finalizer_notifier = 0;

GC_DATASTATIC GC_start_callback_proc GC_start_call_back = 0;

#ifdef ALL_INTERIOR_POINTERS
GC_DATASTATIC int GC_all_interior_pointers = 1;
#else
GC_DATASTATIC int GC_all_interior_pointers = 0;
#endif

#ifdef FINALIZE_ON_DEMAND
GC_DATASTATIC int GC_finalize_on_demand = 1;
#else
GC_DATASTATIC int GC_finalize_on_demand = 0;
#endif

GC_DATASTATIC int GC_dont_gc = 0;

#ifdef GC_DONT_EXPAND
GC_DATASTATIC int GC_dont_expand = 1;
#else
GC_DATASTATIC int GC_dont_expand = 0;
#endif

GC_STATIC int GC_CALLBACK GC_never_stop_func(void);
GC_DATASTATIC GC_stop_func GC_default_stop_func = GC_never_stop_func;

#ifndef GC_MISC_EXCLUDE
GC_STATIC void GC_CALLBACK GC_default_warn_proc(char *msg, GC_word arg);
GC_DATASTATIC GC_warn_proc GC_current_warn_proc =
 GC_default_warn_proc; /* ignored */
#endif

GC_DATASTATIC int GC_stack_grows_up = 0;

GC_DATASTATIC struct GC_gcdata_s *GC_gcdata_global = NULL;

GC_DATASTATIC GC_word GC_max_heapsize = ~(GC_word)0;

GC_DATASTATIC CONST struct GC_objlink_s GC_nil_objlink = { NULL, NULL, 0 };

#ifndef GC_NO_DLINKS
GC_DATASTATIC CONST struct GC_dlink_s GC_nil_dlink = { NULL, NULL, 0 };
#endif

#ifndef GC_NO_FNLZ
GC_DATASTATIC CONST struct GC_fnlz_s GC_nil_fnlz = { NULL, NULL, NULL, 0 };
#endif

#ifdef GC_PRINT_MSGS
GC_DATASTATIC int GC_verbose_gc = 0;
#endif

#ifdef GC_THREADS

#ifdef GC_WIN32_THREADS

struct GC_mutex_s
{
 LONG state;
 HANDLE event;
};

GC_DATASTATIC struct GC_mutex_s GC_allocate_ml = { 0, 0 };

#else /* GC_WIN32_THREADS */

volatile int GC_inside_collect = -1;

GC_DATASTATIC pthread_mutex_t GC_allocate_ml;

GC_STATIC void GC_CLIBDECL GC_suspend_handler(int sig);

#endif /* ! GC_WIN32_THREADS */

GC_STATIC void GC_FASTCALL GC_stkroot_add(struct GC_gcdata_s *gcdata,
 GC_THREAD_ID_T thread_id, struct GC_stkroot_s *new_stkroot);
GC_STATIC void GC_FASTCALL GC_stkroot_tblresize(struct GC_gcdata_s *gcdata,
 struct GC_stkroot_s **new_hroots, GC_word new_log2_size);

#else /* GC_THREADS */

GC_DATASTATIC int GC_allocate_ml = 0;

#endif /* ! GC_THREADS */

GC_STATIC void *GC_FASTCALL GC_alloc_hroots(struct GC_gcdata_s *gcdata,
 GC_word new_log2_size, CONST void *nil_ptr);
GC_STATIC void *GC_FASTCALL GC_core_malloc_with_gc(struct GC_gcdata_s *gcdata,
 GC_word size, int *pres);
GC_STATIC int GC_FASTCALL GC_heap_expand(struct GC_gcdata_s *gcdata,
 GC_word incsize);
GC_STATIC void *GC_FASTCALL GC_inner_core_malloc(struct GC_gcdata_s *gcdata,
 GC_word size, int dont_expand);
GC_STATIC int GC_FASTCALL GC_roots_add(struct GC_gcdata_s *gcdata,
 GC_word begin_addr, GC_word end_addr);

void GC_noop1(GC_word value)
{
 GC_noop_sink = value;
}

GC_word GC_approx_sp(void)
{
 volatile GC_word value;
 value = (GC_word)(&value);
 GC_noop1(value);
 return value;
}

GC_STATIC int GC_FASTCALL GC_roots_autodetect(struct GC_gcdata_s *gcdata)
{
 int res = GC_roots_add(gcdata, 0, 0);
#ifdef GC_DATASTART
#ifdef GC_DATAEND
 res |= GC_roots_add(gcdata, (GC_word)GC_DATASTART, (GC_word)GC_DATAEND);
#endif
#endif
#ifdef GC_DATASTART2
#ifdef GC_DATAEND2
 res |= GC_roots_add(gcdata, (GC_word)GC_DATASTART2, (GC_word)GC_DATAEND2);
#endif
#endif
 return res;
}

GC_INLINE_STATIC GC_word GC_FASTCALL GC_stack_detectbase(void)
{
 return (GC_word)(GC_STACKBOTTOM) + (GC_STACKLEN);
}

GC_INLINE_STATIC GC_word GC_FASTCALL GC_stack_approx_size(
 CONST struct GC_gcdata_s *gcdata)
{
 struct GC_stkroot_s *cur_stack = gcdata->cur_stack;
 GC_word totalsize = cur_stack != NULL ? cur_stack->end_addr -
                      cur_stack->begin_addr : sizeof(GC_word);
#ifdef GC_THREADS
 totalsize = gcdata->stkroot_htable.count * totalsize;
#endif
 return totalsize;
}

GC_INLINE_STATIC int GC_FASTCALL GC_guess_collect(
 CONST struct GC_gcdata_s *gcdata, GC_word objsize)
{
 return (((GC_stack_approx_size(gcdata) + gcdata->followscan_size) << 1) +
         gcdata->dataroot_size +
         ((GC_word)sizeof(GC_word) << gcdata->obj_htable.log2_size) +
#ifndef GC_NO_DLINKS
         ((GC_word)sizeof(GC_word) << gcdata->dlink_htable.log2_size) +
#endif
         ((gcdata->marked_bytes + gcdata->bytes_allocd -
         gcdata->followscan_size) >> 2)) / GC_free_space_divisor <=
#ifndef GC_NO_FNLZ
         gcdata->bytes_finalized +
#endif
         gcdata->bytes_allocd + objsize ? 1 : 0;
}

GC_INLINE_STATIC GC_word GC_FASTCALL GC_guess_expand_size(
 CONST struct GC_gcdata_s *gcdata, GC_word objsize)
{
 GC_word space_divisor = GC_free_space_divisor + 1;
 return (gcdata->marked_bytes + gcdata->bytes_allocd) / space_divisor >=
         gcdata->free_bytes ? gcdata->free_bytes * space_divisor +
         (gcdata->bytes_allocd >> 3) + (objsize << 2) +
         gcdata->dataroot_size : 0;
}

GC_STATIC void GC_FASTCALL GC_abort_badptr(CONST void *ptr)
{
#ifdef GC_PRINT_MSGS
 fprintf(stderr, " GC: Illegal pointer specified: 0x%lX." GC_NEW_LINE,
  (unsigned long)((GC_word)ptr));
#else
 GC_noop1((GC_word)ptr);
#endif
 GC_FATAL_ABORT;
}

GC_INLINE_STATIC int GC_FASTCALL GC_config_set(struct GC_gcdata_s *gcdata)
{
 int res = 0;
#ifdef GC_GETENV_SKIP
#ifdef GC_PRINT_MSGS
 GC_verbose_gc = 1;
#endif
 GC_noop1((GC_word)gcdata);
#else
 char *str;
 GC_word value;
 if ((str = getenv("GC_ALL_INTERIOR_POINTERS")) != NULL && *str)
  GC_all_interior_pointers = *str != '0' || *(str + 1) ? 1 : 0;
 if ((str = getenv("GC_DONT_GC")) != NULL && *str)
  GC_dont_gc = GC_NEVER_COLLECT;
#ifdef GC_GCJ_SUPPORT
#ifndef GC_IGNORE_GCJ_INFO
 if ((str = getenv("GC_IGNORE_GCJ_INFO")) != NULL && *str)
  gcdata->ignore_gcj_info = 1;
#endif
#endif
#ifdef GC_PRINT_MSGS
 if ((str = getenv("GC_PRINT_STATS")) != NULL && *str)
  GC_verbose_gc = 1;
#endif
 if (((str = getenv("GC_FREE_SPACE_DIVISOR")) != NULL && *str &&
     ((GC_free_space_divisor = (GC_word)atol(str)) == 0 ||
     GC_free_space_divisor == ~(GC_word)0)) ||
     ((str = getenv("GC_MAXIMUM_HEAP_SIZE")) != NULL && *str &&
     (GC_max_heapsize = (GC_word)atol(str)) == 0) ||
     ((str = getenv("GC_INITIAL_HEAP_SIZE")) != NULL && *str &&
     ((value = (GC_word)atol(str)) - (GC_word)1 >= GC_max_heapsize ||
     (gcdata->total_heapsize < value && GC_heap_expand(gcdata,
     value - gcdata->total_heapsize) < 0))))
  res = -1;
#endif
 return res;
}

GC_STATIC int GC_CALLBACK GC_never_stop_func(void)
{
 return 0;
}

GC_API GC_word GC_CALL GC_get_gc_no(void)
{
 return GC_gc_no;
}

GC_API void GC_CALL GC_set_finalize_on_demand(int value)
{
 GC_finalize_on_demand = value;
}

GC_API void GC_CALL GC_set_java_finalization(int value)
{
 if (!value)
  GC_abort_badptr(NULL);
}

GC_API void GC_CALL GC_set_max_heap_size(GC_word size)
{
 GC_max_heapsize = size ? size : ~(GC_word)0;
}

#ifndef GC_MISC_EXCLUDE

GC_API void GC_CALL GC_set_free_space_divisor(GC_word value)
{
 if (!value || value == ~(GC_word)0)
  GC_abort_badptr(NULL);
 GC_free_space_divisor = value;
}

GC_API void GC_CALL GC_set_all_interior_pointers(int value)
{
 GC_all_interior_pointers = value;
}

GC_API void GC_CALL GC_set_dont_expand(int value)
{
 GC_dont_expand = value;
}

GC_API void GC_CALL GC_set_no_dls(int value)
{
 /* dummy */
 GC_noop1((GC_word)value);
}

GC_API void GC_CALL GC_set_dont_precollect(int value)
{
 /* dummy */
 GC_noop1((GC_word)value);
}

GC_API void GC_CALL GC_set_force_unmap_on_gcollect(int value)
{
 /* dummy */
 GC_noop1((GC_word)value);
}

GC_API void GC_CALL GC_set_max_retries(GC_word value)
{
 GC_max_retries = value;
}

GC_STATIC void GC_CALLBACK GC_default_warn_proc(char *msg, GC_word arg)
{
 /* dummy */
 GC_noop1((GC_word)msg ^ arg);
}

GC_API void GC_CALLBACK GC_ignore_warn_proc(char *msg, GC_word arg)
{
 GC_default_warn_proc(msg, arg);
}

#endif /* ! GC_MISC_EXCLUDE */

GC_API void *GC_CALL GC_call_with_stack_base(GC_stack_base_func fn,
 void *client_data)
{
 GC_word stack_data;
 struct GC_stack_base sb;
 sb.mem_base = (void *)&stack_data;
 return (*fn)(&sb, client_data);
}

GC_API int GC_CALL GC_get_stack_base(struct GC_stack_base *sb)
{
 if (sb == NULL)
  GC_abort_badptr(NULL);
 return GC_UNIMPLEMENTED;
}

GC_INLINE_STATIC struct GC_gcdata_s *GC_FASTCALL GC_gcdata_alloc(void)
{
 struct GC_gcdata_s *gcdata;
 if ((gcdata = GC_CORE_MALLOC(sizeof(struct GC_gcdata_s))) != NULL)
 {
  GC_MEM_BZERO(gcdata, sizeof(struct GC_gcdata_s));
  if ((gcdata->obj_htable.hroots = GC_alloc_hroots(gcdata,
      GC_DEFAULT_LOG2_OBJSIZE, &GC_nil_objlink)) != NULL
#ifndef GC_NO_DLINKS
      && (gcdata->dlink_htable.hroots = GC_alloc_hroots(gcdata,
      GC_DEFAULT_LOG2_SIZE, &GC_nil_dlink)) != NULL
#endif
#ifndef GC_NO_FNLZ
      && (gcdata->fnlz_htable.hroots = GC_alloc_hroots(gcdata,
      GC_DEFAULT_LOG2_SIZE, &GC_nil_fnlz)) != NULL
#endif
#ifdef GC_THREADS
      && (gcdata->stkroot_htable.hroots = GC_alloc_hroots(gcdata,
      GC_DEFAULT_LOG2_SIZE, NULL)) != NULL
#endif
      )
  {
   gcdata->obj_htable.min_obj_addr = ~(GC_word)0;
   gcdata->obj_htable.max_obj_addr = (GC_word)1 << GC_LOG2_OFFIGNORE;
   gcdata->obj_htable.log2_size = GC_DEFAULT_LOG2_OBJSIZE;
#ifndef GC_NO_DLINKS
   gcdata->dlink_htable.log2_size = GC_DEFAULT_LOG2_SIZE;
#endif
#ifndef GC_NO_FNLZ
   gcdata->fnlz_htable.log2_size = GC_DEFAULT_LOG2_SIZE;
   gcdata->notifier_gc_no = GC_gc_no;
#endif
#ifdef GC_THREADS
   gcdata->stkroot_htable.log2_size = GC_DEFAULT_LOG2_SIZE;
#endif
  }
   else gcdata = NULL;
 }
 return gcdata;
}

GC_STATIC int GC_FASTCALL GC_heap_expand(struct GC_gcdata_s *gcdata,
 GC_word incsize)
{
 void *ptr;
 GC_word free_bytes = gcdata->free_bytes;
 GC_word total_heapsize = gcdata->total_heapsize;
 GC_word max_heapsize;
 int res = -1;
 incsize = incsize > ((GC_word)sizeof(GC_word) << GC_LOG2_OFFIGNORE) ?
            (incsize + (sizeof(GC_word) - 1)) & ~(sizeof(GC_word) - 1) :
            (GC_word)sizeof(GC_word) << GC_LOG2_OFFIGNORE;
 if (free_bytes + incsize <= GC_MEM_SIZELIMIT)
 {
  gcdata->expanded_heapsize = total_heapsize + incsize;
  if ((max_heapsize = GC_max_heapsize) > total_heapsize)
  {
   if (max_heapsize - total_heapsize < incsize)
    incsize = max_heapsize - total_heapsize;
#ifdef GC_PRINT_MSGS
   if (GC_verbose_gc)
    fprintf(stdout,
     "[GC: Expand by %lu KiB after %lu KiB allocd, %lu KiB free of %lu KiB]"
     GC_NEW_LINE, GC_SIZE_TO_ULKB(incsize),
     GC_SIZE_TO_ULKB(gcdata->bytes_allocd), GC_SIZE_TO_ULKB(free_bytes),
     GC_SIZE_TO_ULKB(total_heapsize));
#endif
   while ((ptr = GC_CORE_MALLOC((size_t)free_bytes +
          (size_t)incsize)) == NULL)
    if ((incsize = incsize >> 1) == 0)
     break;
   if (ptr != NULL)
   {
    total_heapsize += incsize;
    GC_CORE_FREE(ptr);
    gcdata->expanded_heapsize = total_heapsize;
    gcdata->total_heapsize = total_heapsize;
    gcdata->free_bytes = free_bytes + incsize;
    res = 0;
   }
  }
 }
 return res;
}

#ifndef GC_MISC_EXCLUDE

GC_STATIC void GC_FASTCALL GC_roots_del_inside(struct GC_gcdata_s *gcdata,
 GC_word begin_addr, GC_word end_addr)
{
 GC_REGISTER_KEYWORD struct GC_dataroot_s *dataroot;
 GC_REGISTER_KEYWORD struct GC_dataroot_s **pnext = &gcdata->dataroots;
 struct GC_dataroot_s *pred;
 GC_word count;
 while ((dataroot = *pnext) != NULL && dataroot->begin_addr < begin_addr)
  pnext = &dataroot->next;
 if (dataroot != NULL)
 {
  count = 0;
  do
  {
   if (end_addr < dataroot->end_addr)
    break;
   gcdata->dataroot_size -= dataroot->end_addr - dataroot->begin_addr;
   dataroot = (pred = dataroot)->next;
   GC_CORE_FREE(pred);
   count++;
  } while (dataroot != NULL);
  *pnext = dataroot;
  gcdata->free_bytes += count * sizeof(struct GC_dataroot_s);
 }
}

GC_INLINE_STATIC int GC_FASTCALL GC_roots_exclude(struct GC_gcdata_s *gcdata,
 GC_word begin_addr, GC_word end_addr)
{
 struct GC_dataroot_s *dataroot = gcdata->dataroots;
 struct GC_dataroot_s *new_dataroot;
 int res = 0;
 while (dataroot != NULL && begin_addr >= dataroot->end_addr)
  dataroot = dataroot->next;
 if (dataroot != NULL && dataroot->begin_addr < end_addr)
 {
  if (dataroot->begin_addr < begin_addr)
  {
   if (end_addr < dataroot->end_addr)
   {
    new_dataroot = GC_core_malloc_with_gc(gcdata,
                    sizeof(struct GC_dataroot_s), &res);
    res = -1;
    if (new_dataroot != NULL)
    {
     gcdata->dataroot_size -= end_addr - begin_addr;
     new_dataroot->begin_addr = end_addr;
     new_dataroot->end_addr = dataroot->end_addr;
     new_dataroot->next = dataroot->next;
     dataroot->end_addr = begin_addr;
     dataroot->next = new_dataroot;
     res = 0;
    }
   }
    else
    {
     gcdata->dataroot_size -= dataroot->end_addr - begin_addr;
     dataroot->end_addr = begin_addr;
     if ((dataroot = dataroot->next) != NULL &&
         dataroot->begin_addr < end_addr && end_addr < dataroot->end_addr)
     {
      gcdata->dataroot_size -= end_addr - dataroot->begin_addr;
      dataroot->begin_addr = end_addr;
     }
    }
  }
   else
   {
    gcdata->dataroot_size -= end_addr - dataroot->begin_addr;
    dataroot->begin_addr = end_addr;
   }
 }
 return res;
}

#endif /* ! GC_MISC_EXCLUDE */

GC_STATIC int GC_FASTCALL GC_roots_add(struct GC_gcdata_s *gcdata,
 GC_word begin_addr, GC_word end_addr)
{
 GC_REGISTER_KEYWORD struct GC_dataroot_s *dataroot;
 struct GC_dataroot_s *new_dataroot;
 struct GC_dataroot_s **pnext;
 int res = 0;
 if (begin_addr)
 {
  begin_addr = (begin_addr + (sizeof(GC_word) - 1)) & ~(sizeof(GC_word) - 1);
  if ((end_addr = end_addr & ~(sizeof(GC_word) - 1)) > begin_addr)
  {
   pnext = &gcdata->dataroots;
   while ((dataroot = *pnext) != NULL && dataroot->end_addr < begin_addr)
    pnext = &dataroot->next;
   if (dataroot == NULL || end_addr < dataroot->begin_addr)
   {
    new_dataroot = GC_core_malloc_with_gc(gcdata,
                    sizeof(struct GC_dataroot_s), &res);
    res = -1;
    if (new_dataroot != NULL)
    {
     gcdata->dataroot_size += end_addr - begin_addr;
     new_dataroot->begin_addr = begin_addr;
     new_dataroot->end_addr = end_addr;
     new_dataroot->next = dataroot;
     *pnext = new_dataroot;
     res = 0;
    }
   }
    else
    {
     if (begin_addr < dataroot->begin_addr)
     {
      gcdata->dataroot_size += dataroot->begin_addr - begin_addr;
      dataroot->begin_addr = begin_addr;
     }
     if (dataroot->end_addr < end_addr)
     {
      dataroot = dataroot->next;
      while (dataroot != NULL && end_addr >= dataroot->begin_addr)
      {
       if (dataroot->end_addr >= end_addr)
        end_addr = dataroot->end_addr;
       gcdata->dataroot_size -= dataroot->end_addr - dataroot->begin_addr;
       dataroot = (new_dataroot = dataroot)->next;
       GC_CORE_FREE(new_dataroot);
       gcdata->free_bytes += sizeof(struct GC_dataroot_s);
      }
      (new_dataroot = *pnext)->next = dataroot;
      gcdata->dataroot_size += end_addr - new_dataroot->end_addr;
      new_dataroot->end_addr = end_addr;
     }
    }
  }
 }
 return res;
}

#ifdef GC_WIN32_THREADS

GC_INLINE_STATIC int GC_FASTCALL GC_win32_block_on_mutex(
 struct GC_mutex_s *pmutex)
{
 while (InterlockedExchange(&pmutex->state, -1))
  if (WaitForSingleObject(pmutex->event, INFINITE) == WAIT_FAILED)
   return -1;
 return 0;
}

#endif /* GC_WIN32_THREADS */

GC_STATIC int GC_FASTCALL GC_enter(struct GC_gcdata_s **pgcdata)
{
 GC_REGISTER_KEYWORD struct GC_gcdata_s *gcdata;
 GC_REGISTER_KEYWORD struct GC_stkroot_s *cur_stack;
 int res;
#ifdef GC_THREADS
 struct GC_stkroot_s **new_hroots;
 GC_THREAD_ID_T thread_id;
 GC_word new_log2_size;
 if (
#ifdef GC_WIN32_THREADS
     (!GC_allocate_ml.event && (GC_allocate_ml.event =
     CreateEvent(NULL, (BOOL)0, (BOOL)0, NULL)) == 0) ||
     (InterlockedExchange(&GC_allocate_ml.state, 1) &&
     GC_win32_block_on_mutex(&GC_allocate_ml) < 0) ||
     (thread_id = GetCurrentThreadId()) == (GC_THREAD_ID_T)-1L
#else
     (GC_inside_collect == -1 && (pthread_mutex_init(&GC_allocate_ml,
     GC_THREAD_MUTEX_DEFATTR) ? 1 : (GC_inside_collect = 0))) ||
     pthread_mutex_lock(&GC_allocate_ml) ||
     (thread_id = pthread_self()) == (pthread_t)(~(GC_word)0)
#endif
     )
 {
  *(GC_THREAD_ID_T volatile *)&thread_id = 0;
#ifdef GC_PRINT_MSGS
  fprintf(stderr, " GC: Cannot initialize or lock mutex!" GC_NEW_LINE);
#endif
  GC_FATAL_ABORT;
 }
#else
 if (++GC_allocate_ml != 1)
 {
#ifdef GC_PRINT_MSGS
  fprintf(stderr, " GC: Not re-entrant!" GC_NEW_LINE);
#endif
  GC_FATAL_ABORT;
 }
 res = GC_UNIMPLEMENTED;
#endif
 if ((gcdata = GC_gcdata_global) == NULL)
 {
  if ((gcdata = GC_gcdata_alloc()) == NULL || GC_config_set(gcdata) < 0 ||
      GC_roots_autodetect(gcdata) < 0 || (gcdata->cur_stack =
      GC_inner_core_malloc(gcdata, sizeof(struct GC_stkroot_s), 0)) == NULL)
  {
#ifdef GC_PRINT_MSGS
   fprintf(stderr,
    " GC: Cannot startup - bad config params or no memory!" GC_NEW_LINE);
#endif
   GC_FATAL_ABORT;
  }
  if (GC_approx_sp() > (GC_word)pgcdata)
   GC_stack_grows_up = 1;
  cur_stack = gcdata->cur_stack;
#ifndef GC_NO_INACTIVE
  cur_stack->activation_frame = NULL;
  cur_stack->inactive = 0;
#endif
#ifndef GC_NO_FNLZ
  cur_stack->inside_fnlz = 0;
#endif
  cur_stack->begin_addr = (cur_stack->end_addr = GC_stack_detectbase()) != 0 ?
                           cur_stack->end_addr : ~(GC_word)0;
#ifdef GC_THREADS
  GC_stkroot_add(gcdata, thread_id, cur_stack);
  res = GC_SUCCESS;
#endif
  GC_gcdata_global = gcdata;
 }
  else
  {
   cur_stack = gcdata->cur_stack;
#ifdef GC_THREADS
   res = GC_DUPLICATE;
   if (cur_stack == NULL || cur_stack->thread_id != thread_id)
   {
    cur_stack =
     gcdata->stkroot_htable.hroots[GC_HASH_INDEX((GC_word)thread_id,
     gcdata->stkroot_htable.seed, gcdata->stkroot_htable.log2_size)];
    while (cur_stack != NULL && cur_stack->thread_id != thread_id)
     cur_stack = cur_stack->next;
    if ((gcdata->cur_stack = cur_stack) == NULL)
    {
     if (GC_HASH_RESIZECOND(gcdata->stkroot_htable.count,
         gcdata->stkroot_htable.log2_size) &&
         (new_hroots = GC_alloc_hroots(gcdata, new_log2_size =
         gcdata->stkroot_htable.log2_size + 1, NULL)) != NULL)
      GC_stkroot_tblresize(gcdata, new_hroots, new_log2_size);
     res = 0;
     if ((cur_stack = GC_core_malloc_with_gc(gcdata,
         sizeof(struct GC_stkroot_s), &res)) == NULL)
     {
#ifdef GC_PRINT_MSGS
      fprintf(stderr, " GC: Cannot register new thread!" GC_NEW_LINE);
#endif
      GC_FATAL_ABORT;
     }
#ifndef GC_NO_INACTIVE
     cur_stack->activation_frame = NULL;
     cur_stack->inactive = 0;
#endif
#ifndef GC_NO_FNLZ
     cur_stack->inside_fnlz = 0;
#endif
     cur_stack->begin_addr = ~(GC_word)0;
     cur_stack->end_addr = 0;
     GC_stkroot_add(gcdata, thread_id, cur_stack);
     gcdata->cur_stack = cur_stack;
     res = GC_SUCCESS;
    }
   }
#endif
  }
 if (cur_stack->begin_addr >= (GC_word)pgcdata)
  cur_stack->begin_addr = (GC_word)pgcdata - sizeof(GC_word);
 if ((GC_word)pgcdata >= cur_stack->end_addr)
  cur_stack->end_addr = (GC_word)pgcdata + (sizeof(GC_word) << 1);
 *pgcdata = gcdata;
 return res;
}

GC_INLINE_STATIC void GC_FASTCALL GC_leave(void)
{
#ifdef GC_THREADS
#ifdef GC_WIN32_THREADS
 if (InterlockedExchange(&GC_allocate_ml.state, 0) < 0 &&
     !SetEvent(GC_allocate_ml.event))
  GC_FATAL_ABORT;
#else
 if (pthread_mutex_unlock(&GC_allocate_ml))
  GC_FATAL_ABORT;
#endif
#else
 GC_allocate_ml = 0;
#endif
}

#ifndef GC_NO_INACTIVE

GC_INLINE_STATIC int GC_FASTCALL GC_set_inactive_sp(
 struct GC_gcdata_s **pgcdata)
{
 GC_REGISTER_KEYWORD struct GC_stkroot_s *cur_stack;
 if ((cur_stack = (*pgcdata)->cur_stack) == NULL || cur_stack->inactive)
  return 0;
 if (GC_stack_grows_up)
  cur_stack->end_addr = (GC_word)pgcdata - sizeof(GC_word);
  else cur_stack->begin_addr = (GC_word)pgcdata + (sizeof(GC_word) << 1);
 cur_stack->inactive = 1;
 return 1;
}

GC_INLINE_STATIC int GC_FASTCALL GC_set_activation_frame(
 struct GC_activation_frame_s *activation_frame,
 struct GC_stkroot_s *cur_stack)
{
 if (cur_stack == NULL || !cur_stack->inactive)
  return 0;
 activation_frame->inactive_sp = GC_stack_grows_up ? cur_stack->end_addr :
                                  cur_stack->begin_addr;
 activation_frame->prev = cur_stack->activation_frame;
 cur_stack->inactive = 0;
 cur_stack->activation_frame = activation_frame;
 return 1;
}

GC_INLINE_STATIC void GC_FASTCALL GC_restore_inactive_sp(
 struct GC_stkroot_s *cur_stack,
 CONST struct GC_activation_frame_s *activation_frame)
{
 cur_stack->activation_frame = activation_frame->prev;
 *(GC_stack_grows_up ? &cur_stack->end_addr :
  &cur_stack->begin_addr) = activation_frame->inactive_sp;
 cur_stack->inactive = 1;
}

#endif /* ! GC_NO_INACTIVE */

#ifdef GC_THREADS

#ifndef GC_WIN32_THREADS

GC_STATIC void GC_FASTCALL GC_thread_yield(int attempt)
{
 if (attempt >= GC_YIELD_MAX_ATTEMPT)
  (void)pthread_usleep_np((unsigned)(attempt - GC_YIELD_MAX_ATTEMPT) * 1000);
  else GC_THREAD_YIELD;
}

#endif /* ! GC_WIN32_THREADS */

#endif /* GC_THREADS */

GC_STATIC void GC_FASTCALL GC_mutator_suspend(struct GC_gcdata_s *gcdata)
{
#ifdef GC_THREADS
 GC_REGISTER_KEYWORD GC_word addr;
 GC_REGISTER_KEYWORD struct GC_stkroot_s *stkroot;
#ifndef GC_WIN32_THREADS
 struct GC_stkroot_s **pnext;
#endif
 struct GC_stkroot_s *cur_stack;
#ifndef GC_WIN32_THREADS
 int attempt;
 GC_inside_collect = 1;
#endif
 if ((GC_word)((cur_stack = gcdata->cur_stack) != NULL ? 1 : 0) <
     gcdata->stkroot_htable.count)
 {
  addr = (GC_word)gcdata->stkroot_htable.hroots - sizeof(GC_word);
  for (;;)
  {
   for (;;)
   {
    if (*(void **)(addr += sizeof(GC_word)) != NULL)
     break;
   }
   if ((GC_word)(stkroot = *(struct GC_stkroot_s **)addr) == ~(GC_word)0)
    break;
#ifdef GC_WIN32_THREADS
   do
   {
    if (stkroot != cur_stack
#ifndef GC_NO_INACTIVE
        && !stkroot->inactive
#endif
        )
    {
#ifdef GC_WIN32_WCE
     while (SuspendThread(GC_THREAD_HANDLE(stkroot)) == ~(DWORD)0)
      GC_THREAD_YIELD;
#else
     if (SuspendThread(stkroot->thread_handle) == ~(DWORD)0)
     {
#ifdef GC_PRINT_MSGS
      fprintf(stderr, " GC: Cannot suspend thread!" GC_NEW_LINE);
#endif
      GC_FATAL_ABORT;
     }
#endif
    }
   } while ((stkroot = stkroot->next) != NULL);
#else
   pnext = (struct GC_stkroot_s **)addr;
   do
   {
    if (stkroot != cur_stack
#ifndef GC_NO_INACTIVE
        && !stkroot->inactive
#endif
        )
    {
     stkroot->suspend_ack = 1;
     (void)signal(GC_SIG_SUSPEND, GC_suspend_handler);
     if (pthread_kill(stkroot->thread_id, GC_SIG_SUSPEND))
     {
      *pnext = stkroot->next;
#ifdef GC_PRINT_MSGS
      fprintf(stderr,
       " GC: Cannot send signal to thread: 0x%lX." GC_NEW_LINE,
       (unsigned long)((GC_word)stkroot->thread_id));
#endif
      gcdata->stkroot_htable.count--;
      GC_CORE_FREE(stkroot);
      gcdata->free_bytes += sizeof(struct GC_stkroot_s);
     }
      else pnext = &stkroot->next;
    }
     else pnext = &stkroot->next;
   } while ((stkroot = *pnext) != NULL);
#endif
  }
#ifndef GC_WIN32_THREADS
  addr = (GC_word)gcdata->stkroot_htable.hroots - sizeof(GC_word);
  for (;;)
  {
   for (;;)
   {
    if (*(void **)(addr += sizeof(GC_word)) != NULL)
     break;
   }
   if ((GC_word)(stkroot = *(struct GC_stkroot_s **)addr) == ~(GC_word)0)
    break;
   do
   {
    attempt = 0;
    while (stkroot->suspend_ack)
     GC_thread_yield(attempt++);
   } while ((stkroot = stkroot->next) != NULL);
  }
#endif
 }
#else
 GC_noop1((GC_word)gcdata);
#endif
}

GC_STATIC void GC_FASTCALL GC_mutator_resume(struct GC_gcdata_s *gcdata)
{
#ifdef GC_THREADS
#ifdef GC_WIN32_THREADS
 GC_REGISTER_KEYWORD GC_word addr;
 GC_REGISTER_KEYWORD struct GC_stkroot_s *stkroot;
 struct GC_stkroot_s *cur_stack;
 DWORD res;
 if ((GC_word)((cur_stack = gcdata->cur_stack) != NULL ? 1 : 0) <
     gcdata->stkroot_htable.count)
 {
  addr = (GC_word)gcdata->stkroot_htable.hroots - sizeof(GC_word);
  for (;;)
  {
   for (;;)
   {
    if (*(void **)(addr += sizeof(GC_word)) != NULL)
     break;
   }
   if ((GC_word)(stkroot = *(struct GC_stkroot_s **)addr) == ~(GC_word)0)
    break;
   do
   {
    if (stkroot != cur_stack &&
#ifndef GC_NO_INACTIVE
        !stkroot->inactive &&
#endif
        ((res = ResumeThread(GC_THREAD_HANDLE(stkroot))) == ~(DWORD)0 ||
        !res))
    {
#ifdef GC_PRINT_MSGS
     fprintf(stderr, " GC: Cannot resume thread!" GC_NEW_LINE);
#endif
     GC_FATAL_ABORT;
    }
   } while ((stkroot = stkroot->next) != NULL);
  }
 }
#else
 GC_inside_collect = 0;
 GC_noop1((GC_word)gcdata);
#endif
#else
 GC_noop1((GC_word)gcdata);
#endif
}

GC_STATIC void GC_FASTCALL GC_scan_region(struct GC_gcdata_s *gcdata,
 GC_word begin_addr, GC_word end_addr, int interior_pointers)
{
 GC_REGISTER_KEYWORD GC_word *region;
 GC_REGISTER_KEYWORD GC_word *end_of_region;
 GC_REGISTER_KEYWORD GC_word min_obj_addr;
 GC_REGISTER_KEYWORD GC_word ignore_off;
 GC_REGISTER_KEYWORD GC_word addr;
 struct GC_objlink_s **hroots;
 struct GC_objlink_s *marked_list;
 struct GC_objlink_s *follow_list;
 GC_word log2_size;
 GC_word hmask;
 GC_word count;
 GC_word align_mask;
 if (begin_addr < end_addr)
 {
  hroots = gcdata->obj_htable.hroots;
  marked_list = gcdata->obj_htable.marked_list;
  follow_list = gcdata->obj_htable.follow_list;
  hmask = ((GC_word)1 << (log2_size = gcdata->obj_htable.log2_size)) - 1;
  count = 0;
  align_mask = 0;
  ignore_off = gcdata->obj_htable.max_obj_addr -
                (min_obj_addr = gcdata->obj_htable.min_obj_addr);
  region = (GC_word *)begin_addr;
  end_of_region = (GC_word *)end_addr;
  if (!interior_pointers)
   align_mask = sizeof(GC_word) - 1;
  do
  {
   if (*region - min_obj_addr >= ignore_off)
    do
    {
     if (++region >= end_of_region)
      goto out;
    } while (*region - min_obj_addr >= ignore_off);
   if (((addr = *region) & align_mask) == 0)
   {
    GC_REGISTER_KEYWORD struct GC_objlink_s *objlink;
    GC_REGISTER_KEYWORD struct GC_objlink_s **pnext;
    if ((GC_word)(objlink = *(pnext = &hroots[(((addr >> log2_size) ^ addr) >>
        GC_LOG2_OFFIGNORE) & hmask]))->obj > addr)
     for (;;)
     {
      if ((GC_word)(objlink = *(pnext = &objlink->next))->obj <= addr)
       break;
     }
    if (interior_pointers)
    {
     if ((objlink->atomic_and_size & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK)) <=
         addr - (GC_word)objlink->obj)
     {
      if ((addr | ~(((GC_word)1 << GC_LOG2_OFFIGNORE) - (GC_word)1)) ==
          ~(GC_word)0)
       continue;
      pnext = &hroots[((((addr - ((GC_word)1 << GC_LOG2_OFFIGNORE)) >>
               log2_size) ^ (addr - ((GC_word)1 << GC_LOG2_OFFIGNORE))) >>
               GC_LOG2_OFFIGNORE) & hmask];
      while ((GC_word)(objlink = *pnext)->obj > addr)
       pnext = &objlink->next;
      if ((objlink->atomic_and_size & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK)) <=
          addr - (GC_word)objlink->obj)
       continue;
     }
    }
     else
     {
      if ((GC_word)objlink->obj != addr)
       continue;
     }
    *pnext = objlink->next;
    count++;
    if ((objlink->atomic_and_size & GC_ATOMIC_MASK) != 0)
    {
     objlink->next = marked_list;
     marked_list = objlink;
    }
     else
     {
      objlink->next = follow_list;
      follow_list = objlink;
     }
   }
  } while (++region < end_of_region);
out:
  gcdata->obj_htable.count -= count;
  gcdata->obj_htable.marked_list = marked_list;
  gcdata->obj_htable.follow_list = follow_list;
 }
}

GC_INLINE_STATIC void *GC_FASTCALL GC_roots_scan(struct GC_gcdata_s *gcdata,
 GC_stop_func stop_func)
{
 struct GC_dataroot_s *dataroot;
 if ((dataroot = gcdata->dataroots) != NULL)
 {
  do
  {
   if ((*stop_func)())
    break;
   GC_scan_region(gcdata, dataroot->begin_addr, dataroot->end_addr,
    GC_all_interior_pointers);
  } while ((dataroot = dataroot->next) != NULL);
 }
 return dataroot;
}

#ifndef GC_NO_INACTIVE

#ifdef GC_THREADS
GC_STATIC
#else
GC_INLINE_STATIC
#endif
void GC_FASTCALL GC_stack_scan_frames(struct GC_gcdata_s *gcdata,
 GC_word begin_addr, GC_word end_addr,
 CONST struct GC_activation_frame_s *activation_frame)
{
 if (activation_frame != NULL)
 {
  if (GC_stack_grows_up)
  {
   do
   {
    GC_scan_region(gcdata, ((GC_word)activation_frame +
     (sizeof(struct GC_activation_frame_s) + (sizeof(GC_word) << 1) - 1)) &
     ~(sizeof(GC_word) - 1), end_addr & ~(sizeof(GC_word) - 1), 1);
    end_addr = activation_frame->inactive_sp;
   } while ((activation_frame = activation_frame->prev) != NULL);
  }
   else
   {
    do
    {
     GC_scan_region(gcdata, (begin_addr + (sizeof(GC_word) - 1)) &
      ~(sizeof(GC_word) - 1), ((GC_word)activation_frame - sizeof(GC_word)) &
      ~(sizeof(GC_word) - 1), 1);
     begin_addr = activation_frame->inactive_sp;
    } while ((activation_frame = activation_frame->prev) != NULL);
   }
 }
 GC_scan_region(gcdata, (begin_addr + (sizeof(GC_word) - 1)) &
  ~(sizeof(GC_word) - 1), end_addr & ~(sizeof(GC_word) - 1), 1);
}

#endif /* ! GC_NO_INACTIVE */

GC_STATIC void GC_FASTCALL GC_stack_scan_cur(struct GC_gcdata_s *gcdata)
{
 GC_word begin_addr;
 GC_word end_addr;
 struct GC_stkroot_s *cur_stack;
 GC_PUSHREGS_BEGIN;
 if ((cur_stack = gcdata->cur_stack) != NULL)
 {
  begin_addr = cur_stack->begin_addr;
  end_addr = GC_approx_sp();
  if (!GC_stack_grows_up)
  {
   begin_addr = end_addr;
   end_addr = cur_stack->end_addr;
  }
#ifdef GC_NO_INACTIVE
  GC_scan_region(gcdata, (begin_addr + (sizeof(GC_word) - 1)) &
   ~(sizeof(GC_word) - 1), end_addr & ~(sizeof(GC_word) - 1), 1);
#else
  GC_stack_scan_frames(gcdata, begin_addr, end_addr,
   cur_stack->activation_frame);
#endif
 }
 GC_PUSHREGS_END;
}

GC_STATIC void GC_FASTCALL GC_scan_followable(struct GC_gcdata_s *gcdata,
 GC_stop_func stop_func)
{
 GC_REGISTER_KEYWORD struct GC_objlink_s *objlink;
 GC_word begin_addr;
 while ((objlink = gcdata->obj_htable.follow_list) != NULL && !(*stop_func)())
 {
  gcdata->obj_htable.follow_list = objlink->next;
  objlink->next = gcdata->obj_htable.marked_list;
  begin_addr = (GC_word)(gcdata->obj_htable.marked_list = objlink)->obj;
#ifdef GC_GCJ_SUPPORT
#ifdef GC_IGNORE_GCJ_INFO
  GC_scan_region(gcdata, begin_addr, begin_addr + (objlink->atomic_and_size &
   ~(GC_ATOMIC_MASK | (sizeof(GC_word) - 1))), GC_all_interior_pointers);
#else
  GC_scan_region(gcdata, begin_addr,
   (((objlink->atomic_and_size & GC_HASDSLEN_MASK) != 0 ?
   *(GC_word *)(*(GC_word *)begin_addr + MARK_DESCR_OFFSET) :
   objlink->atomic_and_size) & ~(GC_ATOMIC_MASK | (sizeof(GC_word) - 1))) +
   begin_addr, GC_all_interior_pointers);
#endif
#else
  GC_scan_region(gcdata, begin_addr, begin_addr + (objlink->atomic_and_size &
   ~(GC_ATOMIC_MASK | (sizeof(GC_word) - 1))), GC_all_interior_pointers);
#endif
 }
}

#ifdef GC_THREADS

#ifndef GC_WIN32_THREADS

GC_STATIC void GC_CLIBDECL GC_suspend_handler(int sig)
{
 GC_REGISTER_KEYWORD struct GC_gcdata_s *gcdata;
 struct GC_stkroot_s *volatile stkroot;
 pthread_t thread_id;
 int old_errno;
 int attempt;
 if ((gcdata = GC_gcdata_global) != NULL)
 {
  old_errno = errno;
#ifdef SIG_ACK
  if (signal(sig, GC_suspend_handler) != SIG_DFL)
   (void)signal(sig, SIG_ACK);
#else
  (void)signal(sig, GC_suspend_handler);
#endif
  thread_id = pthread_self();
  stkroot =
   gcdata->stkroot_htable.hroots[GC_HASH_INDEX((GC_word)thread_id,
   gcdata->stkroot_htable.seed, gcdata->stkroot_htable.log2_size)];
  while (stkroot != NULL && stkroot->thread_id != thread_id)
   stkroot = stkroot->next;
  if (gcdata->cur_stack != stkroot && stkroot != NULL
#ifndef GC_NO_INACTIVE
      && !stkroot->inactive
#endif
      )
  {
   GC_ASYNC_PUSHREGS_BEGIN;
   *(volatile GC_word *)(GC_stack_grows_up ? &stkroot->end_addr :
    &stkroot->begin_addr) = GC_approx_sp();
   attempt = 0;
   do
   {
    stkroot->suspend_ack = 0;
    GC_thread_yield(attempt++);
   } while (GC_inside_collect);
   GC_ASYNC_PUSHREGS_END;
  }
  GC_ERRNO_SET(old_errno);
 }
}

#endif /* ! GC_WIN32_THREADS */

GC_STATIC GC_word GC_FASTCALL GC_stkroot_scan_other(
 struct GC_gcdata_s *gcdata, GC_stop_func stop_func)
{
#ifdef GC_WIN32_THREADS
 CONTEXT context;
#endif
 GC_REGISTER_KEYWORD GC_word addr;
 struct GC_stkroot_s *stkroot = (void *)(~(GC_word)0);
 struct GC_stkroot_s *cur_stack;
 if ((cur_stack = gcdata->cur_stack) == NULL ||
     gcdata->stkroot_htable.count > (GC_word)1)
 {
  addr = (GC_word)gcdata->stkroot_htable.hroots - sizeof(GC_word);
  do
  {
   for (;;)
   {
    if (*(void **)(addr += sizeof(GC_word)) != NULL)
     break;
   }
   if ((GC_word)(stkroot = *(struct GC_stkroot_s **)addr) == ~(GC_word)0)
    break;
   do
   {
    if (stkroot != cur_stack)
    {
     if ((*stop_func)())
      break;
#ifdef GC_WIN32_THREADS
#ifndef GC_NO_INACTIVE
     if (!stkroot->inactive)
#endif
     {
      context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
      if (GetThreadContext(GC_THREAD_HANDLE(stkroot), &context))
      {
       GC_scan_region(gcdata, (GC_word)(&context),
        (GC_word)(&context) + (sizeof(context) & ~(sizeof(GC_word) - 1)), 1);
       if ((*stop_func)())
        break;
       *(GC_stack_grows_up ? &stkroot->end_addr : &stkroot->begin_addr) =
        context.GC_WIN32_CONTEXT_SP_NAME;
      }
#ifdef GC_PRINT_MSGS
       else fprintf(stderr,
             " GC: Cannot get context of thread: 0x%lX." GC_NEW_LINE,
             (unsigned long)stkroot->thread_id);
#endif
     }
#endif
#ifdef GC_NO_INACTIVE
     GC_scan_region(gcdata, (stkroot->begin_addr + (sizeof(GC_word) - 1)) &
      ~(sizeof(GC_word) - 1), stkroot->end_addr & ~(sizeof(GC_word) - 1), 1);
#else
     GC_stack_scan_frames(gcdata, stkroot->begin_addr, stkroot->end_addr,
      stkroot->activation_frame);
#endif
    }
   } while ((stkroot = stkroot->next) != NULL);
  } while (stkroot == NULL);
 }
 return ~(GC_word)stkroot;
}

GC_STATIC void GC_FASTCALL GC_stkroot_tblresize(struct GC_gcdata_s *gcdata,
 struct GC_stkroot_s **new_hroots, GC_word new_log2_size)
{
 GC_REGISTER_KEYWORD GC_word addr =
  (GC_word)gcdata->stkroot_htable.hroots - sizeof(GC_word);
 struct GC_stkroot_s **pnext;
 struct GC_stkroot_s *stkroot;
 struct GC_stkroot_s *new_stkroot;
 GC_word seed = addr + GC_RANDOM_SEED(gcdata);
 for (;;)
 {
  for (;;)
  {
   if (*(void **)(addr += sizeof(GC_word)) != NULL)
    break;
  }
  if ((GC_word)(stkroot = *(struct GC_stkroot_s **)addr) == ~(GC_word)0)
   break;
  do
  {
   pnext = &new_hroots[GC_HASH_INDEX((GC_word)stkroot->thread_id, seed,
            new_log2_size)];
   stkroot = (new_stkroot = stkroot)->next;
   new_stkroot->next = *pnext;
   *pnext = new_stkroot;
  } while (stkroot != NULL);
 }
 gcdata->stkroot_htable.seed = seed;
 GC_CORE_FREE(gcdata->stkroot_htable.hroots);
 gcdata->free_bytes += ((GC_word)sizeof(GC_word) <<
                        gcdata->stkroot_htable.log2_size) + sizeof(GC_word);
 gcdata->stkroot_htable.hroots = new_hroots;
 gcdata->stkroot_htable.log2_size = new_log2_size;
}

GC_STATIC void GC_FASTCALL GC_stkroot_add(struct GC_gcdata_s *gcdata,
 GC_THREAD_ID_T thread_id, struct GC_stkroot_s *new_stkroot)
{
 struct GC_stkroot_s **pnext =
  &gcdata->stkroot_htable.hroots[GC_HASH_INDEX((GC_word)thread_id,
  gcdata->stkroot_htable.seed, gcdata->stkroot_htable.log2_size)];
#ifdef GC_WIN32_THREADS
#ifndef GC_WIN32_WCE
 HANDLE process_handle = GetCurrentProcess();
 if (!DuplicateHandle(process_handle, GetCurrentThread(), process_handle,
     &new_stkroot->thread_handle, 0, (BOOL)0, DUPLICATE_SAME_ACCESS))
 {
#ifdef GC_PRINT_MSGS
  fprintf(stderr, " GC: Cannot duplicate thread handle!" GC_NEW_LINE);
#endif
  GC_FATAL_ABORT;
 }
#endif
#else
 new_stkroot->suspend_ack = 0;
#endif
 new_stkroot->next = *pnext;
 new_stkroot->thread_id = thread_id;
 *pnext = new_stkroot;
 gcdata->stkroot_htable.count++;
#ifndef GC_WIN32_THREADS
 (void)signal(GC_SIG_SUSPEND, GC_suspend_handler);
#endif
}

GC_INLINE_STATIC void GC_FASTCALL GC_stkroot_delete_cur(
 struct GC_gcdata_s *gcdata)
{
 struct GC_stkroot_s *cur_stack = gcdata->cur_stack;
 GC_REGISTER_KEYWORD struct GC_stkroot_s **pnext =
  &gcdata->stkroot_htable.hroots[GC_HASH_INDEX((GC_word)cur_stack->thread_id,
  gcdata->stkroot_htable.seed, gcdata->stkroot_htable.log2_size)];
 while (*pnext != cur_stack)
  pnext = &(*pnext)->next;
 *pnext = cur_stack->next;
 gcdata->stkroot_htable.count--;
#ifdef GC_WIN32_THREADS
#ifndef GC_WIN32_WCE
 (void)CloseHandle(cur_stack->thread_handle);
#endif
#endif
 gcdata->cur_stack = NULL;
 GC_CORE_FREE(cur_stack);
 gcdata->free_bytes += sizeof(struct GC_stkroot_s);
}

#endif /* GC_THREADS */

#ifndef GC_NO_DLINKS

GC_STATIC void GC_FASTCALL GC_dlink_scan_clear(struct GC_gcdata_s *gcdata)
{
 GC_REGISTER_KEYWORD GC_word addr =
  (GC_word)gcdata->dlink_htable.hroots - sizeof(GC_word);
 GC_REGISTER_KEYWORD struct GC_dlink_s *dlink;
 struct GC_dlink_s **pnext;
 struct GC_dlink_s *free_list = gcdata->dlink_htable.free_list;
 struct GC_objlink_s **obj_hroots = gcdata->obj_htable.hroots;
 GC_word obj_log2_size;
 GC_word obj_hmask;
 GC_word saved_addr;
 GC_word count = 0;
 obj_hmask = ((GC_word)1 << (obj_log2_size =
              gcdata->obj_htable.log2_size)) - 1;
 for (;;)
 {
  for (;;)
  {
   if (*(struct GC_dlink_s **)(addr += sizeof(GC_word)) != &GC_nil_dlink)
    break;
  }
  if ((GC_word)(dlink = *(struct GC_dlink_s **)addr) == ~(GC_word)0)
   break;
  pnext = (struct GC_dlink_s **)(saved_addr = addr);
  do
  {
   GC_REGISTER_KEYWORD struct GC_objlink_s *objlink;
   addr = (GC_word)dlink->objlink->obj;
   if ((GC_word)(objlink = obj_hroots[(((addr >> obj_log2_size) ^ addr) >>
       GC_LOG2_OFFIGNORE) & obj_hmask])->obj > addr)
    for (;;)
    {
     if ((GC_word)(objlink = objlink->next)->obj <= addr)
      break;
    }
   if ((GC_word)objlink->obj == addr)
   {
    *pnext = dlink->next;
    *(GC_word *)(~dlink->hidden_link) = 0;
    dlink->next = free_list;
    count++;
    free_list = dlink;
   }
    else pnext = &dlink->next;
  } while ((dlink = *pnext) != &GC_nil_dlink);
  addr = saved_addr;
 }
 gcdata->dlink_htable.free_list = free_list;
 gcdata->dlink_htable.count -= count;
 gcdata->free_bytes += count * sizeof(struct GC_dlink_s);
}

GC_STATIC GC_word GC_FASTCALL GC_dlink_free_pending(
 struct GC_gcdata_s *gcdata, GC_word min_free_count)
{
 GC_REGISTER_KEYWORD struct GC_dlink_s *dlink;
 GC_word count = 0;
 do
 {
  if ((dlink = gcdata->dlink_htable.free_list) == NULL)
   break;
  gcdata->dlink_htable.free_list = dlink->next;
  GC_CORE_FREE(dlink);
 } while (++count < min_free_count);
 return count;
}

GC_INLINE_STATIC void GC_FASTCALL GC_dlink_tblresize(
 struct GC_gcdata_s *gcdata, struct GC_dlink_s **new_hroots,
 GC_word new_log2_size)
{
 GC_REGISTER_KEYWORD GC_word addr =
  (GC_word)gcdata->dlink_htable.hroots - sizeof(GC_word);
 struct GC_dlink_s **pnext;
 struct GC_dlink_s *dlink;
 GC_word hidden_link;
 GC_word saved_addr;
 GC_word seed = addr + GC_RANDOM_SEED(gcdata);
 for (;;)
 {
  for (;;)
  {
   if (*(struct GC_dlink_s **)(addr += sizeof(GC_word)) != &GC_nil_dlink)
    break;
  }
  if ((GC_word)(dlink = *(struct GC_dlink_s **)addr) == ~(GC_word)0)
   break;
  saved_addr = addr;
  do
  {
   hidden_link = dlink->hidden_link;
   pnext = &new_hroots[GC_HASH_INDEX(hidden_link >> GC_LOG2_OFFIGNORE,
            seed, new_log2_size)];
   while (((struct GC_dlink_s *)(addr = (GC_word)(*pnext)))->hidden_link >
          hidden_link)
    pnext = &((struct GC_dlink_s *)addr)->next;
   dlink = (*pnext = dlink)->next;
   (*pnext)->next = (struct GC_dlink_s *)addr;
  } while (dlink != &GC_nil_dlink);
  addr = saved_addr;
 }
 gcdata->dlink_htable.seed = seed;
 GC_CORE_FREE(gcdata->dlink_htable.hroots);
 gcdata->free_bytes += ((GC_word)sizeof(GC_word) <<
                        gcdata->dlink_htable.log2_size) + sizeof(GC_word);
 gcdata->dlink_htable.hroots = new_hroots;
 gcdata->dlink_htable.log2_size = new_log2_size;
}

GC_INLINE_STATIC int GC_FASTCALL GC_dlink_add(struct GC_gcdata_s *gcdata,
 GC_word hidden_link, struct GC_objlink_s *objlink,
 struct GC_dlink_s *new_dlink)
{
 GC_REGISTER_KEYWORD struct GC_dlink_s *dlink;
 struct GC_dlink_s **pnext;
 if (!gcdata->dlink_htable.count)
  gcdata->dlink_htable.seed += GC_RANDOM_SEED(gcdata);
 pnext = &gcdata->dlink_htable.hroots[GC_HASH_INDEX(hidden_link >>
          GC_LOG2_OFFIGNORE, gcdata->dlink_htable.seed,
          gcdata->dlink_htable.log2_size)];
 while ((dlink = *pnext)->hidden_link > hidden_link)
  pnext = &dlink->next;
 if (dlink->hidden_link == hidden_link)
 {
  dlink->objlink = objlink;
  if (new_dlink != NULL &&
      (dlink = gcdata->dlink_htable.free_list) != new_dlink)
  {
   new_dlink->next = dlink;
   gcdata->dlink_htable.free_list = new_dlink;
   gcdata->free_bytes += sizeof(struct GC_dlink_s);
  }
  return GC_DUPLICATE;
 }
 if (new_dlink == NULL)
  return GC_NO_MEMORY;
 if (gcdata->dlink_htable.free_list == new_dlink)
 {
  gcdata->free_bytes -= sizeof(struct GC_dlink_s);
  gcdata->dlink_htable.free_list = new_dlink->next;
 }
 new_dlink->objlink = objlink;
 new_dlink->next = dlink;
 new_dlink->hidden_link = hidden_link;
 *pnext = new_dlink;
 gcdata->dlink_htable.count++;
 return GC_SUCCESS;
}

GC_STATIC int GC_FASTCALL GC_dlink_delete(struct GC_gcdata_s *gcdata,
 GC_word hidden_link, GC_word min_hidden, GC_word max_hidden)
{
 GC_REGISTER_KEYWORD struct GC_dlink_s *dlink;
 struct GC_dlink_s **pnext =
  &gcdata->dlink_htable.hroots[GC_HASH_INDEX(hidden_link >> GC_LOG2_OFFIGNORE,
  gcdata->dlink_htable.seed, gcdata->dlink_htable.log2_size)];
 while ((dlink = *pnext)->hidden_link > max_hidden)
  pnext = &dlink->next;
 if (dlink->hidden_link < min_hidden)
  return 0;
 do
 {
  *pnext = dlink->next;
  gcdata->dlink_htable.count--;
  GC_CORE_FREE(dlink);
  dlink = *pnext;
  gcdata->free_bytes += sizeof(struct GC_dlink_s);
 } while (dlink->hidden_link >= min_hidden);
 return 1;
}

#endif /* ! GC_NO_DLINKS */

#ifndef GC_NO_FNLZ

GC_STATIC int GC_FASTCALL GC_objlink_mark(struct GC_gcdata_s *gcdata,
 GC_word addr, int interior_pointers)
{
 GC_REGISTER_KEYWORD struct GC_objlink_s *objlink;
 GC_REGISTER_KEYWORD struct GC_objlink_s **pnext;
 struct GC_objlink_s **hroots = gcdata->obj_htable.hroots;
 GC_word log2_size = gcdata->obj_htable.log2_size;
 pnext = &hroots[(((addr >> log2_size) ^ addr) >> GC_LOG2_OFFIGNORE) &
          (((GC_word)1 << log2_size) - 1)];
 while ((GC_word)(objlink = *pnext)->obj > addr)
  pnext = &objlink->next;
 if (interior_pointers)
 {
  if ((objlink->atomic_and_size & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK)) <=
      addr - (GC_word)objlink->obj)
  {
   if ((addr | ~(((GC_word)1 << GC_LOG2_OFFIGNORE) - (GC_word)1)) ==
       ~(GC_word)0)
    return 0;
   pnext = &hroots[((((addr - ((GC_word)1 << GC_LOG2_OFFIGNORE)) >>
            log2_size) ^ (addr - ((GC_word)1 << GC_LOG2_OFFIGNORE))) >>
            GC_LOG2_OFFIGNORE) & (((GC_word)1 << log2_size) - 1)];
   while ((GC_word)(objlink = *pnext)->obj > addr)
    pnext = &objlink->next;
   if ((objlink->atomic_and_size & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK)) <=
       addr - (GC_word)objlink->obj)
    return 0;
  }
 }
  else
  {
   if ((GC_word)objlink->obj != addr)
    return 0;
  }
 *pnext = objlink->next;
 gcdata->obj_htable.count--;
 if ((objlink->atomic_and_size & GC_ATOMIC_MASK) != 0)
 {
  objlink->next = gcdata->obj_htable.marked_list;
  gcdata->obj_htable.marked_list = objlink;
 }
  else
  {
   objlink->next = gcdata->obj_htable.follow_list;
   gcdata->obj_htable.follow_list = objlink;
  }
 return 1;
}

GC_INLINE_STATIC GC_word GC_FASTCALL GC_fnlz_precollect(
 struct GC_gcdata_s *gcdata, GC_word *pcount)
{
 GC_REGISTER_KEYWORD GC_word addr;
 GC_REGISTER_KEYWORD struct GC_fnlz_s *fnlz;
 void *client_data;
 struct GC_objlink_s *objlink;
 GC_word bytes_finalized = 0;
 int interior_pointers = GC_all_interior_pointers;
 if ((fnlz = gcdata->fnlz_htable.ready_fnlz) != NULL)
 {
  addr = 0;
  do
  {
   bytes_finalized += (objlink = fnlz->objlink)->atomic_and_size;
   (void)GC_objlink_mark(gcdata, (GC_word)objlink->obj, 0);
   addr++;
   if ((client_data = fnlz->client_data) != NULL)
    (void)GC_objlink_mark(gcdata, (GC_word)client_data, interior_pointers);
  } while ((fnlz = fnlz->next) != NULL);
  *pcount += addr;
 }
 if (gcdata->fnlz_htable.has_client_ptrs)
 {
  addr = (GC_word)gcdata->fnlz_htable.hroots - sizeof(GC_word);
  for (;;)
  {
   for (;;)
   {
    if (*(struct GC_fnlz_s **)(addr += sizeof(GC_word)) != &GC_nil_fnlz)
     break;
   }
   if ((GC_word)(fnlz = *(struct GC_fnlz_s **)addr) == ~(GC_word)0)
    break;
   do
   {
    if ((client_data = fnlz->client_data) != NULL)
     (void)GC_objlink_mark(gcdata, (GC_word)client_data, interior_pointers);
   } while ((fnlz = fnlz->next) != &GC_nil_fnlz);
  }
 }
 return bytes_finalized & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK);
}

GC_STATIC GC_word GC_FASTCALL GC_fnlz_after_collect(
 struct GC_gcdata_s *gcdata, GC_word *pcount)
{
 GC_REGISTER_KEYWORD GC_word addr =
  (GC_word)gcdata->fnlz_htable.hroots - sizeof(GC_word);
 GC_REGISTER_KEYWORD struct GC_fnlz_s *fnlz;
 struct GC_fnlz_s **pnext;
 struct GC_fnlz_s *ready_fnlz = gcdata->fnlz_htable.ready_fnlz;
 struct GC_objlink_s *objlink;
 GC_word bytes_finalized = 0;
 GC_word count = 0;
 for (;;)
 {
  for (;;)
  {
   if (*(struct GC_fnlz_s **)(addr += sizeof(GC_word)) != &GC_nil_fnlz)
    break;
  }
  if ((GC_word)(fnlz = *(struct GC_fnlz_s **)addr) == ~(GC_word)0)
   break;
  pnext = (struct GC_fnlz_s **)addr;
  do
  {
   if (GC_objlink_mark(gcdata, (GC_word)(objlink = fnlz->objlink)->obj, 0))
   {
    *pnext = fnlz->next;
    bytes_finalized += objlink->atomic_and_size;
    fnlz->next = ready_fnlz;
    count++;
    ready_fnlz = fnlz;
   }
    else pnext = &fnlz->next;
  } while ((fnlz = *pnext) != &GC_nil_fnlz);
 }
 gcdata->fnlz_htable.ready_fnlz = ready_fnlz;
 if ((gcdata->fnlz_htable.count -= count) == 0)
  gcdata->fnlz_htable.has_client_ptrs = 0;
 gcdata->free_bytes += count * sizeof(struct GC_fnlz_s);
 *pcount += count;
 return bytes_finalized & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK);
}

GC_INLINE_STATIC void GC_FASTCALL GC_fnlz_tblresize(
 struct GC_gcdata_s *gcdata, struct GC_fnlz_s **new_hroots,
 GC_word new_log2_size)
{
 GC_REGISTER_KEYWORD GC_word addr =
  (GC_word)gcdata->fnlz_htable.hroots - sizeof(GC_word);
 struct GC_fnlz_s **pnext;
 struct GC_fnlz_s *fnlz;
 struct GC_objlink_s *objlink;
 GC_word saved_addr;
 GC_word seed = addr + GC_RANDOM_SEED(gcdata);
 for (;;)
 {
  for (;;)
  {
   if (*(struct GC_fnlz_s **)(addr += sizeof(GC_word)) != &GC_nil_fnlz)
    break;
  }
  if ((GC_word)(fnlz = *(struct GC_fnlz_s **)addr) == ~(GC_word)0)
   break;
  saved_addr = addr;
  do
  {
   objlink = fnlz->objlink;
   pnext = &new_hroots[GC_HASH_INDEX((GC_word)objlink /
            (sizeof(GC_word) << 1), seed, new_log2_size)];
   while ((GC_word)((struct GC_fnlz_s *)(addr = (GC_word)(*pnext)))->objlink >
          (GC_word)objlink)
    pnext = &((struct GC_fnlz_s *)addr)->next;
   fnlz = (*pnext = fnlz)->next;
   (*pnext)->next = (struct GC_fnlz_s *)addr;
  } while (fnlz != &GC_nil_fnlz);
  addr = saved_addr;
 }
 gcdata->fnlz_htable.seed = seed;
 GC_CORE_FREE(gcdata->fnlz_htable.hroots);
 gcdata->free_bytes += ((GC_word)sizeof(GC_word) <<
                        gcdata->fnlz_htable.log2_size) + sizeof(GC_word);
 gcdata->fnlz_htable.hroots = new_hroots;
 gcdata->fnlz_htable.log2_size = new_log2_size;
}

GC_INLINE_STATIC GC_finalization_proc GC_FASTCALL GC_fnlz_add_del(
 struct GC_gcdata_s *gcdata, struct GC_objlink_s *objlink,
 GC_finalization_proc fn, void *client_data, void **odata)
{
 GC_REGISTER_KEYWORD struct GC_fnlz_s *fnlz;
 struct GC_fnlz_s **pnext;
 struct GC_fnlz_s *new_fnlz;
 GC_finalization_proc old_fn;
 if (!gcdata->fnlz_htable.count)
  gcdata->fnlz_htable.seed += GC_RANDOM_SEED(gcdata);
 pnext = &gcdata->fnlz_htable.hroots[GC_HASH_INDEX((GC_word)objlink /
          (sizeof(GC_word) << 1), gcdata->fnlz_htable.seed,
          gcdata->fnlz_htable.log2_size)];
 while ((GC_word)(fnlz = *pnext)->objlink > (GC_word)objlink)
  pnext = &fnlz->next;
 if (fnlz->objlink == objlink)
 {
  *odata = fnlz->client_data;
  old_fn = fnlz->fn;
  if (fn != 0)
  {
   fnlz->client_data = client_data;
   fnlz->fn = fn;
   if (!gcdata->fnlz_htable.has_client_ptrs &&
       (GC_word)client_data >= gcdata->obj_htable.min_obj_addr &&
       (GC_word)client_data < gcdata->obj_htable.max_obj_addr)
    gcdata->fnlz_htable.has_client_ptrs = 1;
  }
   else
   {
    *pnext = fnlz->next;
    if (gcdata->fnlz_htable.single_free != NULL)
    {
     GC_CORE_FREE(fnlz);
     gcdata->free_bytes += sizeof(struct GC_fnlz_s);
    }
     else gcdata->fnlz_htable.single_free = fnlz;
    if (!(--gcdata->fnlz_htable.count))
     gcdata->fnlz_htable.has_client_ptrs = 0;
   }
 }
  else
  {
   if (fn != 0)
   {
    if ((new_fnlz = gcdata->fnlz_htable.single_free) != NULL)
    {
     gcdata->fnlz_htable.single_free = NULL;
     new_fnlz->next = fnlz;
     new_fnlz->objlink = objlink;
     new_fnlz->client_data = client_data;
     new_fnlz->fn = fn;
     *pnext = new_fnlz;
     gcdata->fnlz_htable.count++;
     *odata = NULL;
     if (!gcdata->fnlz_htable.has_client_ptrs &&
         (GC_word)client_data >= gcdata->obj_htable.min_obj_addr &&
         (GC_word)client_data < gcdata->obj_htable.max_obj_addr)
      gcdata->fnlz_htable.has_client_ptrs = 1;
    }
   }
    else *odata = NULL;
   old_fn = 0;
  }
 return old_fn;
}

GC_STATIC GC_finalization_proc GC_FASTCALL GC_fnlz_del_ready(
 struct GC_gcdata_s *gcdata, struct GC_objlink_s **pobjlink, void **odata)
{
 GC_REGISTER_KEYWORD struct GC_fnlz_s *fnlz;
 GC_finalization_proc fn = 0;
 if ((fnlz = gcdata->fnlz_htable.ready_fnlz) != NULL)
 {
  *pobjlink = fnlz->objlink;
  gcdata->fnlz_htable.ready_fnlz = fnlz->next;
  *odata = fnlz->client_data;
  fn = fnlz->fn;
  GC_CORE_FREE(fnlz);
 }
 return fn;
}

#ifndef JAVA_FINALIZATION_NOT_NEEDED

GC_INLINE_STATIC void GC_FASTCALL GC_fnlz_ready_all(
 struct GC_gcdata_s *gcdata)
{
 GC_REGISTER_KEYWORD GC_word addr =
  (GC_word)gcdata->fnlz_htable.hroots - sizeof(GC_word);
 GC_REGISTER_KEYWORD struct GC_fnlz_s *fnlz;
 struct GC_fnlz_s *ready_fnlz = gcdata->fnlz_htable.ready_fnlz;
 for (;;)
 {
  for (;;)
  {
   if (*(struct GC_fnlz_s **)(addr += sizeof(GC_word)) != &GC_nil_fnlz)
    break;
  }
  if ((GC_word)(fnlz = *(struct GC_fnlz_s **)addr) == ~(GC_word)0)
   break;
  while (fnlz->next != &GC_nil_fnlz)
   fnlz = fnlz->next;
  fnlz->next = ready_fnlz;
  ready_fnlz = *(struct GC_fnlz_s **)addr;
  *(CONST struct GC_fnlz_s **)addr = &GC_nil_fnlz;
 }
 gcdata->free_bytes += gcdata->fnlz_htable.count * sizeof(struct GC_fnlz_s);
 gcdata->fnlz_htable.ready_fnlz = ready_fnlz;
 gcdata->fnlz_htable.count = 0;
 gcdata->fnlz_htable.has_client_ptrs = 0;
}

#endif /* ! JAVA_FINALIZATION_NOT_NEEDED */

#endif /* ! GC_NO_FNLZ */

GC_INLINE_STATIC void GC_FASTCALL GC_objlink_add(struct GC_gcdata_s *gcdata,
 void *obj, GC_word objsize, GC_word vtable)
{
 GC_REGISTER_KEYWORD struct GC_objlink_s *objlink;
 struct GC_objlink_s **pnext;
 struct GC_objlink_s *new_objlink;
 gcdata->obj_htable.unlinked_list =
  (new_objlink = gcdata->obj_htable.unlinked_list)->next;
 if (vtable)
 {
  GC_MEM_BZERO(obj, objsize);
  gcdata->followscan_size += objsize & ~(sizeof(GC_word) - 1);
#ifdef GC_GCJ_SUPPORT
#ifdef GC_IGNORE_GCJ_INFO
  if (vtable != ~(GC_word)0)
   *(GC_word *)obj = vtable;
  new_objlink->atomic_and_size = objsize;
#else
  new_objlink->atomic_and_size =
   vtable != ~(GC_word)0 && (*(GC_word *)obj = vtable,
#ifndef GC_GETENV_SKIP
   !gcdata->ignore_gcj_info &&
#endif
   *(GC_word *)(vtable + MARK_DESCR_OFFSET) != GC_DS_LENGTH) ?
   objsize | GC_HASDSLEN_MASK : objsize;
#endif
#else
  new_objlink->atomic_and_size = objsize;
#endif
 }
  else new_objlink->atomic_and_size = objsize | GC_ATOMIC_MASK;
 new_objlink->obj = obj;
 if ((GC_word)(objlink = *(pnext =
     &gcdata->obj_htable.hroots[(((((GC_word)obj) >>
     gcdata->obj_htable.log2_size) ^ (GC_word)obj) >> GC_LOG2_OFFIGNORE) &
     (((GC_word)1 << gcdata->obj_htable.log2_size) - 1)]))->obj >
     (GC_word)obj)
  for (;;)
  {
   if ((GC_word)(objlink = *(pnext = &objlink->next))->obj <= (GC_word)obj)
    break;
  }
 new_objlink->next = objlink;
 *pnext = new_objlink;
 if (gcdata->obj_htable.min_obj_addr >= (GC_word)obj)
  gcdata->obj_htable.min_obj_addr = (GC_word)obj;
 if ((GC_word)obj + ((GC_word)1 << GC_LOG2_OFFIGNORE) >
     gcdata->obj_htable.max_obj_addr)
  gcdata->obj_htable.max_obj_addr =
   (GC_word)obj + ((GC_word)1 << GC_LOG2_OFFIGNORE);
 gcdata->obj_htable.count++;
}

GC_STATIC GC_word GC_FASTCALL GC_objlink_remove_all(
 struct GC_gcdata_s *gcdata)
{
 GC_REGISTER_KEYWORD GC_word addr =
  (GC_word)gcdata->obj_htable.hroots - sizeof(GC_word);
 GC_REGISTER_KEYWORD struct GC_objlink_s *objlink;
 GC_word removed_bytes = 0;
 GC_word count = 0;
 struct GC_objlink_s *free_list = gcdata->obj_htable.free_list;
 for (;;)
 {
  for (;;)
  {
   if (*(struct GC_objlink_s **)(addr += sizeof(GC_word)) != &GC_nil_objlink)
    break;
  }
  if ((GC_word)(objlink = *(struct GC_objlink_s **)addr) == ~(GC_word)0)
   break;
  removed_bytes += objlink->atomic_and_size;
  while (objlink->next != &GC_nil_objlink)
  {
   removed_bytes += (objlink = objlink->next)->atomic_and_size;
   count++;
  }
  objlink->next = free_list;
  count++;
  free_list = *(struct GC_objlink_s **)addr;
  *(CONST struct GC_objlink_s **)addr = &GC_nil_objlink;
 }
 gcdata->marked_bytes -= removed_bytes & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK);
 gcdata->obj_htable.free_list = free_list;
 gcdata->obj_htable.count = 0;
 gcdata->obj_htable.min_obj_addr = ~(GC_word)0;
 gcdata->obj_htable.max_obj_addr = (GC_word)1 << GC_LOG2_OFFIGNORE;
 return count;
}

#ifdef GC_NO_FNLZ
#ifdef GC_NO_GCBASE
GC_INLINE_STATIC
#else
GC_STATIC
#endif
#else
GC_STATIC
#endif
struct GC_objlink_s *GC_FASTCALL GC_objlink_get(struct GC_gcdata_s *gcdata,
 void *displaced_pointer)
{
 GC_REGISTER_KEYWORD struct GC_objlink_s *objlink;
 struct GC_objlink_s **hroots = gcdata->obj_htable.hroots;
 GC_word log2_size = gcdata->obj_htable.log2_size;
 objlink = hroots[(((((GC_word)displaced_pointer) >> log2_size) ^
            (GC_word)displaced_pointer) >> GC_LOG2_OFFIGNORE) &
            (((GC_word)1 << log2_size) - 1)];
 while ((GC_word)objlink->obj > (GC_word)displaced_pointer)
  objlink = objlink->next;
 if ((objlink->atomic_and_size & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK)) <=
     (GC_word)displaced_pointer - (GC_word)objlink->obj)
 {
  objlink = hroots[(((((GC_word)displaced_pointer -
             (((GC_word)1 << GC_LOG2_OFFIGNORE) - (GC_word)1)) >>
             log2_size) ^ ((GC_word)displaced_pointer -
             (((GC_word)1 << GC_LOG2_OFFIGNORE) - (GC_word)1))) >>
             GC_LOG2_OFFIGNORE) & (((GC_word)1 << log2_size) - 1)];
  while ((GC_word)objlink->obj > (GC_word)displaced_pointer)
   objlink = objlink->next;
  if ((objlink->atomic_and_size & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK)) <=
      (GC_word)displaced_pointer - (GC_word)objlink->obj)
   objlink = NULL;
 }
 return objlink;
}

#ifdef GC_NO_FNLZ
#ifdef GC_NO_GCBASE
GC_INLINE_STATIC
#else
GC_STATIC
#endif
#else
GC_STATIC
#endif
struct GC_objlink_s *GC_FASTCALL GC_objlink_refill_find(
 struct GC_gcdata_s *gcdata, void *displaced_pointer)
{
 GC_REGISTER_KEYWORD struct GC_objlink_s *marked_list;
 GC_REGISTER_KEYWORD struct GC_objlink_s **pnext;
 struct GC_objlink_s *objlink;
 struct GC_objlink_s **hroots;
 GC_word min_obj_addr;
 GC_word max_obj_addr;
 GC_word log2_size;
 GC_word hmask;
 GC_word count;
 if ((void *)(marked_list = gcdata->obj_htable.follow_list) !=
     (void *)gcdata->obj_htable.marked_list)
 {
  hroots = gcdata->obj_htable.hroots;
  min_obj_addr = gcdata->obj_htable.min_obj_addr;
  max_obj_addr = gcdata->obj_htable.max_obj_addr -
                  ((GC_word)1 << GC_LOG2_OFFIGNORE);
  if (marked_list == NULL)
   marked_list = gcdata->obj_htable.marked_list;
  hmask = ((GC_word)1 << (log2_size = gcdata->obj_htable.log2_size)) - 1;
  count = 0;
  do
  {
   do
   {
    GC_REGISTER_KEYWORD GC_word addr = (GC_word)marked_list->obj;
    if (min_obj_addr >= addr)
     min_obj_addr = addr;
    if (max_obj_addr <= addr)
     max_obj_addr = addr;
    pnext = &hroots[(((addr >> log2_size) ^ addr) >>
             GC_LOG2_OFFIGNORE) & hmask];
    while ((GC_word)(objlink = *pnext)->obj > addr)
     pnext = &objlink->next;
    count++;
    if ((GC_word)displaced_pointer - addr <
        (marked_list->atomic_and_size & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK)))
     break;
    marked_list = (*pnext = marked_list)->next;
    (*pnext)->next = objlink;
   } while (marked_list != NULL);
   if (marked_list != NULL)
    break;
   if (gcdata->obj_htable.follow_list == NULL)
   {
    gcdata->obj_htable.marked_list = NULL;
    break;
   }
   gcdata->obj_htable.follow_list = NULL;
  } while ((marked_list = gcdata->obj_htable.marked_list) != NULL);
  gcdata->obj_htable.min_obj_addr = min_obj_addr;
  gcdata->obj_htable.max_obj_addr =
   max_obj_addr + ((GC_word)1 << GC_LOG2_OFFIGNORE);
  gcdata->obj_htable.count += count;
  if (marked_list != NULL)
  {
   if (gcdata->obj_htable.follow_list != NULL)
    gcdata->obj_htable.follow_list = marked_list->next;
    else gcdata->obj_htable.marked_list = marked_list->next;
   *pnext = marked_list;
   marked_list->next = objlink;
  }
 }
 return marked_list;
}

GC_STATIC GC_word GC_FASTCALL GC_objlink_some_refill(
 struct GC_gcdata_s *gcdata, GC_word max_count)
{
 GC_REGISTER_KEYWORD struct GC_objlink_s *marked_list;
 GC_REGISTER_KEYWORD struct GC_objlink_s **pnext;
 struct GC_objlink_s *objlink;
 struct GC_objlink_s **hroots;
 GC_word min_obj_addr;
 GC_word max_obj_addr;
 GC_word log2_size;
 GC_word hmask;
 GC_word count = 0;
 if ((void *)(marked_list = gcdata->obj_htable.follow_list) !=
     (void *)gcdata->obj_htable.marked_list)
 {
  hroots = gcdata->obj_htable.hroots;
  min_obj_addr = gcdata->obj_htable.min_obj_addr;
  max_obj_addr = gcdata->obj_htable.max_obj_addr -
                  ((GC_word)1 << GC_LOG2_OFFIGNORE);
  if (marked_list == NULL)
   marked_list = gcdata->obj_htable.marked_list;
  hmask = ((GC_word)1 << (log2_size = gcdata->obj_htable.log2_size)) - 1;
  count = max_count;
  do
  {
   GC_REGISTER_KEYWORD GC_word addr = (GC_word)marked_list->obj;
   if (min_obj_addr >= addr)
    min_obj_addr = addr;
   if (max_obj_addr <= addr)
    max_obj_addr = addr;
   if ((GC_word)(objlink = *(pnext = &hroots[(((addr >> log2_size) ^ addr) >>
       GC_LOG2_OFFIGNORE) & hmask]))->obj > addr)
    for (;;)
    {
     if ((GC_word)(objlink = *(pnext = &objlink->next))->obj <= addr)
      break;
    }
   marked_list = (*pnext = marked_list)->next;
   (*pnext)->next = objlink;
  } while (--count && marked_list != NULL);
  gcdata->obj_htable.min_obj_addr = min_obj_addr;
  count = max_count - count;
  gcdata->obj_htable.max_obj_addr =
   max_obj_addr + ((GC_word)1 << GC_LOG2_OFFIGNORE);
  gcdata->obj_htable.count += count;
  if (gcdata->obj_htable.follow_list != NULL)
   gcdata->obj_htable.follow_list = marked_list;
   else gcdata->obj_htable.marked_list = marked_list;
 }
 return count;
}

GC_STATIC GC_word GC_FASTCALL GC_objlink_free_pending(
 struct GC_gcdata_s *gcdata, GC_word min_free_bytes)
{
 GC_REGISTER_KEYWORD struct GC_objlink_s *objlink;
#ifndef GC_NO_DLINKS
 GC_word max_hidden;
 GC_word min_hidden;
#endif
 GC_word objsize;
 GC_word totalsize = 0;
 GC_word count = 0;
 while ((objlink = gcdata->obj_htable.free_list) != NULL)
 {
  objsize = objlink->atomic_and_size;
#ifndef GC_NO_DLINKS
  if (gcdata->dlink_htable.count)
  {
   min_hidden = GC_HIDE_POINTER((char *)objlink->obj +
                 (objsize & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK |
                 (sizeof(GC_word) - 1))) - (GC_word)sizeof(GC_word));
   if ((max_hidden = GC_HIDE_POINTER(objlink->obj)) >= min_hidden)
   {
    (void)GC_dlink_delete(gcdata, max_hidden, min_hidden, max_hidden);
    if (((min_hidden ^ max_hidden) &
        ~(((GC_word)1 << GC_LOG2_OFFIGNORE) - 1)) != 0)
     (void)GC_dlink_delete(gcdata, max_hidden -
      ((GC_word)1 << GC_LOG2_OFFIGNORE), min_hidden, max_hidden);
   }
  }
#endif
  gcdata->obj_htable.free_list = objlink->next;
  GC_CORE_FREE(objlink->obj);
  if ((objsize & GC_ATOMIC_MASK) == 0)
   gcdata->followscan_size -= objsize & ~(GC_HASDSLEN_MASK |
                               (sizeof(GC_word) - 1));
  objlink->obj = NULL;
  objlink->next = gcdata->obj_htable.unlinked_list;
  gcdata->obj_htable.unlinked_list = objlink;
  totalsize += objsize;
  count++;
  if ((gcdata->free_bytes +=
      objsize & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK)) >= min_free_bytes)
   break;
 }
 gcdata->obj_htable.pending_free_size -=
  totalsize & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK);
 return count;
}

GC_STATIC void GC_FASTCALL GC_collect_unreachable(struct GC_gcdata_s *gcdata,
 GC_stop_func stop_func)
{
 GC_word oldcnt;
 GC_word obj_count;
 GC_start_callback_proc start_fn;
#ifndef GC_NO_FNLZ
 GC_word ready_count = 0;
#endif
 struct GC_objlink_s **new_hroots;
 int stopped;
#ifdef GC_PRINT_MSGS
#ifndef GC_NO_DLINKS
 GC_word dlinks_count = gcdata->dlink_htable.count;
#endif
 GC_CURTIME_T curt;
 unsigned long time_ms = 0;
#endif
 if (gcdata->dataroots != NULL)
 {
  if ((start_fn = GC_start_call_back) != 0)
   (*start_fn)();
#ifdef GC_PRINT_MSGS
  if (GC_verbose_gc)
  {
   fprintf(stdout,
    "[GC: #%lu Scan %lu KiB after %lu KiB allocd, %lu KiB free of %lu KiB]"
    GC_NEW_LINE, (unsigned long)(GC_gc_no + 1),
    GC_SIZE_TO_ULKB(GC_stack_approx_size(gcdata) + gcdata->dataroot_size +
    gcdata->followscan_size), GC_SIZE_TO_ULKB(gcdata->bytes_allocd),
    GC_SIZE_TO_ULKB(gcdata->free_bytes),
    GC_SIZE_TO_ULKB(gcdata->total_heapsize));
   time_ms = GC_CURTIME_GETMS(&curt);
  }
  obj_count = gcdata->obj_htable.count;
#endif
  if (!(*stop_func)())
  {
   stopped = 0;
   while (GC_objlink_some_refill(gcdata, GC_LAZYREFILL_BIGCNT))
    if ((stopped = (*stop_func)()) != 0)
     break;
   obj_count = gcdata->obj_htable.count;
   if (!stopped)
   {
#ifndef GC_NO_FNLZ
    oldcnt = GC_fnlz_precollect(gcdata, &ready_count);
#endif
    if (
#ifdef GC_NO_FNLZ
        obj_count
#else
        gcdata->obj_htable.count &&
        ((!oldcnt && !gcdata->fnlz_htable.count) ||
        (stopped = (*stop_func)()) == 0)
#endif
        )
    {
     GC_mutator_suspend(gcdata);
     GC_stack_scan_cur(gcdata);
     stopped = 1;
     if (GC_roots_scan(gcdata, stop_func) == NULL
#ifdef GC_THREADS
         && !GC_stkroot_scan_other(gcdata, stop_func)
#endif
         )
     {
      GC_scan_followable(gcdata, stop_func);
      if (gcdata->obj_htable.follow_list == NULL)
       stopped = 0;
     }
     GC_mutator_resume(gcdata);
#ifdef GC_NO_FNLZ
#ifndef GC_NO_DLINKS
     if (!stopped && gcdata->dlink_htable.count)
      GC_dlink_scan_clear(gcdata);
#endif
#else
     if (!stopped)
     {
#ifndef GC_NO_DLINKS
      if (gcdata->dlink_htable.count)
       GC_dlink_scan_clear(gcdata);
#endif
      if (gcdata->fnlz_htable.count)
       oldcnt += GC_fnlz_after_collect(gcdata, &ready_count);
      gcdata->bytes_finalized = oldcnt;
     }
#endif
    }
    if (!stopped)
    {
     GC_gc_no++;
#ifndef GC_NO_FNLZ
     GC_scan_followable(gcdata, GC_never_stop_func);
#endif
     oldcnt = obj_count;
     gcdata->marked_bytes += gcdata->bytes_allocd;
     if (gcdata->obj_htable.count)
     {
      gcdata->obj_htable.pending_free_size += gcdata->marked_bytes;
      obj_count -= GC_objlink_remove_all(gcdata);
      gcdata->obj_htable.pending_free_size -= gcdata->marked_bytes;
     }
     if ((oldcnt >> 1) < obj_count &&
         GC_HASH_RESIZECOND(oldcnt, gcdata->obj_htable.log2_size) &&
         (new_hroots = GC_alloc_hroots(gcdata,
         gcdata->obj_htable.log2_size + 1, &GC_nil_objlink)) != NULL)
     {
      GC_CORE_FREE(gcdata->obj_htable.hroots);
      gcdata->free_bytes += ((GC_word)sizeof(GC_word) <<
                             gcdata->obj_htable.log2_size) + sizeof(GC_word);
      gcdata->obj_htable.hroots = new_hroots;
      gcdata->obj_htable.log2_size++;
     }
     gcdata->allocd_before_gc += gcdata->bytes_allocd;
     gcdata->bytes_allocd = 0;
    }
   }
  }
  if (!gcdata->bytes_allocd)
  {
   gcdata->recycling = 1;
#ifdef GC_PRINT_MSGS
   if (GC_verbose_gc)
   {
    fprintf(stdout,
     "[GC: Done in %lu ms, %lu KiB used by %lu objs, %lu KiB used by GC]"
     GC_NEW_LINE, GC_CURTIME_GETMS(&curt) - time_ms,
     GC_SIZE_TO_ULKB(gcdata->marked_bytes), (unsigned long)obj_count,
     GC_SIZE_TO_ULKB(gcdata->total_heapsize - gcdata->marked_bytes -
     gcdata->free_bytes));
#ifndef GC_NO_DLINKS
    if (dlinks_count)
     fprintf(stdout,
      "[GC: %lu disappearing links cleared of %lu registered]" GC_NEW_LINE,
      (unsigned long)(dlinks_count - gcdata->dlink_htable.count),
      (unsigned long)dlinks_count);
#endif
#ifndef GC_NO_FNLZ
    if ((oldcnt = gcdata->fnlz_htable.count + ready_count) != 0)
     fprintf(stdout,
      "[GC: %lu finalizers ready of %lu registered]" GC_NEW_LINE,
      (unsigned long)ready_count, (unsigned long)oldcnt);
#endif
   }
#endif
  }
 }
}

GC_STATIC void *GC_FASTCALL GC_inner_core_malloc(struct GC_gcdata_s *gcdata,
 GC_word size, int dont_expand)
{
 GC_word free_bytes;
 void *ptr = NULL;
 if (size <= (~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK) < GC_MEM_SIZELIMIT ?
     ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK) : GC_MEM_SIZELIMIT) &&
     ((free_bytes = gcdata->free_bytes) > size || (!dont_expand &&
     gcdata->total_heapsize - (free_bytes - size) <= GC_max_heapsize)))
 {
  do
  {
   ptr = GC_CORE_MALLOC((size_t)size);
  } while ((GC_word)ptr > ~((GC_word)1 << GC_LOG2_OFFIGNORE) ||
           ((GC_word)ptr & (sizeof(GC_word) - 1)) != 0);
  if (ptr != NULL)
  {
   if (free_bytes < size)
   {
    gcdata->total_heapsize += size - free_bytes;
    gcdata->free_bytes = 0;
   }
    else gcdata->free_bytes = free_bytes - size;
  }
 }
 return ptr;
}

GC_STATIC void *GC_FASTCALL GC_alloc_hroots(struct GC_gcdata_s *gcdata,
 GC_word new_log2_size, CONST void *nil_ptr)
{
 GC_REGISTER_KEYWORD GC_word size = (GC_word)sizeof(GC_word) << new_log2_size;
 void *ptr;
 if ((ptr = GC_inner_core_malloc(gcdata, size + sizeof(GC_word),
     GC_dont_expand)) != NULL || ((
#ifndef GC_NO_DLINKS
     GC_dlink_free_pending(gcdata, ~(GC_word)0) +
#endif
     GC_objlink_free_pending(gcdata, ~(GC_word)0) != 0 || GC_dont_expand) &&
     (ptr = GC_inner_core_malloc(gcdata, size + sizeof(GC_word), 0)) != NULL))
 {
  *(GC_word *)((char *)ptr + size) = ~(GC_word)0;
  if (nil_ptr != NULL)
  {
   while ((size -= sizeof(GC_word)) != 0)
    *(CONST void **)((char *)ptr + size) = nil_ptr;
   *(CONST void **)ptr = nil_ptr;
  }
   else GC_MEM_BZERO(ptr, size);
 }
 return ptr;
}

GC_STATIC void *GC_FASTCALL GC_core_malloc_with_gc(struct GC_gcdata_s *gcdata,
 GC_word size, int *pres)
{
 GC_REGISTER_KEYWORD void *ptr;
#ifndef GC_NO_FNLZ
 struct GC_stkroot_s *cur_stack;
#endif
 GC_word count;
 if ((ptr = GC_inner_core_malloc(gcdata, size, GC_dont_expand)) == NULL)
 {
  if (*pres >= 0)
  {
   *pres = 0;
   if ((!GC_objlink_free_pending(gcdata, ~(GC_word)0) ||
       (ptr = GC_inner_core_malloc(gcdata, size, GC_dont_expand)) == NULL) &&
       !GC_dont_gc)
   {
    GC_collect_unreachable(gcdata, GC_never_stop_func);
    *pres = -1;
#ifndef GC_NO_FNLZ
    if ((cur_stack = gcdata->cur_stack) != NULL)
     cur_stack->inside_fnlz = 0;
#endif
   }
  }
  if (ptr == NULL && ((count = GC_objlink_free_pending(gcdata, size)) == 0 ||
      (ptr = GC_inner_core_malloc(gcdata, size, GC_dont_expand)) == NULL))
  {
#ifndef GC_NO_DLINKS
   if (GC_dlink_free_pending(gcdata, ~(GC_word)0))
   {
    *pres = 1;
    if ((ptr = GC_inner_core_malloc(gcdata, size, 0)) != NULL)
     *pres = 0;
   }
    else
#endif
    {
     if (!count || GC_dont_expand)
      ptr = GC_inner_core_malloc(gcdata, size, 0);
    }
#ifdef GC_PRINT_MSGS
   if (ptr == NULL && GC_verbose_gc)
    fprintf(stderr,
     " GC: Out of memory! Cannot allocate %lu bytes." GC_NEW_LINE,
     (unsigned long)size);
#endif
  }
 }
 return ptr;
}

GC_STATIC int GC_FASTCALL GC_alloc_objlinks(struct GC_gcdata_s *gcdata,
 int *pres)
{
 GC_REGISTER_KEYWORD void *objlinks_block_list;
 GC_word count = (GC_word)1 << (gcdata->obj_htable.log2_size - 2);
 if ((objlinks_block_list = GC_core_malloc_with_gc(gcdata,
     count * sizeof(struct GC_objlink_s) + sizeof(GC_word), pres)) == NULL)
  return 0;
 *(void **)objlinks_block_list = gcdata->objlinks_block_list;
 gcdata->objlinks_block_list = objlinks_block_list;
 if (*pres >= 0)
  *pres = 0;
 ((struct GC_objlink_s *)(objlinks_block_list =
  (char *)objlinks_block_list + sizeof(GC_word)))->obj = NULL;
 ((struct GC_objlink_s *)objlinks_block_list)->next =
  gcdata->obj_htable.unlinked_list;
 while (--count)
 {
  ((struct GC_objlink_s *)((GC_word)objlinks_block_list +
   sizeof(struct GC_objlink_s)))->next = objlinks_block_list;
  ((struct GC_objlink_s *)(objlinks_block_list =
   (char *)objlinks_block_list + sizeof(struct GC_objlink_s)))->obj = NULL;
 }
 gcdata->obj_htable.unlinked_list = objlinks_block_list;
 return 1;
}

GC_STATIC void *GC_FASTCALL GC_general_malloc(struct GC_gcdata_s **pgcdata,
 GC_word objsize, GC_word vtable)
{
 struct GC_gcdata_s *gcdata;
 void *obj = NULL;
 struct GC_objlink_s **new_hroots;
 GC_word retry;
 int res;
#ifndef GC_NO_FNLZ
 struct GC_objlink_s *objlink;
 void *client_data;
 GC_finalization_proc fn;
 GC_finalizer_notifier_proc notifier_fn;
#endif
 if (objsize)
 {
  retry = ~(GC_word)0;
#ifndef GC_NO_FNLZ
  objlink = NULL;
  client_data = NULL;
  fn = 0;
#endif
#ifndef DONT_ADD_BYTE_AT_END
  if (objsize < ((GC_word)1 << GC_LOG2_OFFIGNORE) && GC_all_interior_pointers)
   objsize++;
#endif
  do
  {
   res = 1;
   GC_enter(pgcdata);
   gcdata = *pgcdata;
   if (retry == ~(GC_word)0)
   {
    if (
#ifdef GC_NO_FNLZ
        GC_HASH_RESIZECOND(gcdata->obj_htable.count,
        gcdata->obj_htable.log2_size) &&
#else
        GC_HASH_RESIZECOND(gcdata->obj_htable.count,
        gcdata->obj_htable.log2_size +
        (GC_word)gcdata->cur_stack->inside_fnlz) &&
#endif
        gcdata->bytes_allocd)
    {
     if (GC_dont_gc || gcdata->dataroots == NULL)
     {
      if ((GC_dont_gc == GC_NEVER_COLLECT || gcdata->dataroots == NULL) &&
          (new_hroots = GC_alloc_hroots(gcdata,
          gcdata->obj_htable.log2_size + 1, &GC_nil_objlink)) != NULL)
      {
       (void)GC_objlink_free_pending(gcdata, ~(GC_word)0);
       gcdata->obj_htable.free_list = gcdata->obj_htable.marked_list;
       retry = gcdata->marked_bytes;
       (void)GC_objlink_remove_all(gcdata);
       gcdata->marked_bytes = retry;
       gcdata->obj_htable.marked_list = gcdata->obj_htable.free_list;
       retry = ~(GC_word)0;
       gcdata->obj_htable.free_list = NULL;
       GC_CORE_FREE(gcdata->obj_htable.hroots);
       gcdata->free_bytes += ((GC_word)sizeof(GC_word) <<
                              gcdata->obj_htable.log2_size) + sizeof(GC_word);
       gcdata->obj_htable.hroots = new_hroots;
       gcdata->obj_htable.log2_size++;
       res = -1;
      }
     }
      else
      {
       GC_collect_unreachable(gcdata, GC_default_stop_func);
       res = -1;
      }
    }
     else
     {
      if (!gcdata->recycling &&
#ifndef GC_NO_FNLZ
          !gcdata->cur_stack->inside_fnlz &&
#endif
          !GC_dont_gc)
      {
       if (GC_guess_collect(gcdata, objsize))
       {
        GC_collect_unreachable(gcdata, GC_default_stop_func);
        res = -1;
       }
        else retry = 0;
      }
      if (gcdata->bytes_allocd &&
          (retry = GC_guess_expand_size(gcdata, objsize)) != 0)
      {
       if (!GC_dont_expand &&
           gcdata->expanded_heapsize <= gcdata->total_heapsize)
        (void)GC_heap_expand(gcdata, retry);
       retry = 0;
      }
     }
   }
   if (gcdata->obj_htable.unlinked_list != NULL ||
       GC_alloc_objlinks(gcdata, &res))
    obj = GC_core_malloc_with_gc(gcdata, objsize, &res);
#ifndef GC_NO_FNLZ
   notifier_fn = 0;
   if (gcdata->fnlz_htable.ready_fnlz != NULL && GC_finalize_on_demand &&
       gcdata->notifier_gc_no != GC_gc_no)
   {
    gcdata->notifier_gc_no = GC_gc_no;
    notifier_fn = GC_finalizer_notifier;
   }
#endif
   if (obj != NULL)
   {
    gcdata->bytes_allocd += objsize;
    GC_objlink_add(gcdata, obj, objsize, vtable);
    if (!retry && gcdata->recycling && res > 0)
    {
     res = (int)GC_objlink_some_refill(gcdata, GC_LAZYREFILL_COUNT);
     if (gcdata->obj_htable.free_list != NULL &&
         (res += (int)GC_objlink_free_pending(gcdata, 0) << 1) > 1 &&
         (res > 2 || GC_objlink_free_pending(gcdata,
         gcdata->free_bytes + ((GC_word)1 << GC_LOG2_OFFIGNORE)) == 1))
      (void)GC_objlink_free_pending(gcdata, 0);
#ifndef GC_NO_FNLZ
     if (notifier_fn == 0 && !GC_dont_gc && !GC_finalize_on_demand &&
         ++gcdata->cur_stack->inside_fnlz == 1)
     {
      if ((fn = GC_fnlz_del_ready(gcdata, &objlink, &client_data)) != 0)
       res = 1;
       else gcdata->cur_stack->inside_fnlz = 0;
     }
#endif
     if (!res
#ifndef GC_NO_DLINKS
         && (gcdata->dlink_htable.free_list == NULL ||
         !GC_dlink_free_pending(gcdata,
         objsize / (sizeof(struct GC_dlink_s) - sizeof(GC_word)) + 1))
#endif
        )
     {
      gcdata->recycling = 0;
#ifdef GC_PRINT_MSGS
      if (GC_verbose_gc)
       fprintf(stdout,
        "[GC: Recycled, %lu + %lu /A/ KiB in use, %lu KiB free of %lu KiB]"
        GC_NEW_LINE, GC_SIZE_TO_ULKB(gcdata->followscan_size),
        GC_SIZE_TO_ULKB(gcdata->marked_bytes + gcdata->bytes_allocd -
        gcdata->followscan_size), GC_SIZE_TO_ULKB(gcdata->free_bytes),
        GC_SIZE_TO_ULKB(gcdata->total_heapsize));
#endif
     }
    }
   }
#ifndef GC_NO_FNLZ
    else
    {
     if (notifier_fn == 0 && !GC_finalize_on_demand)
      fn = GC_fnlz_del_ready(gcdata, &objlink, &client_data);
    }
#endif
   *pgcdata = NULL;
   GC_LEAVE(gcdata);
#ifdef GC_NO_FNLZ
   if (obj != NULL || res <= 0)
    break;
#else
   if (fn != 0)
   {
    (*fn)(objlink->obj, client_data);
    if (obj != NULL)
    {
     GC_enter(pgcdata);
     gcdata = *pgcdata;
     gcdata->cur_stack->inside_fnlz = 0;
     *pgcdata = NULL;
     GC_LEAVE(gcdata);
     break;
    }
    fn = 0;
    if (!GC_finalize_on_demand)
     for (;;)
     {
      GC_enter(pgcdata);
      gcdata = *pgcdata;
      (void)GC_objlink_some_refill(gcdata, GC_LAZYREFILL_BIGCNT);
      fn = GC_fnlz_del_ready(gcdata, &objlink, &client_data);
      *pgcdata = NULL;
      GC_LEAVE(gcdata);
      if (fn == 0)
       break;
      (*fn)(objlink->obj, client_data);
     }
   }
    else
    {
     if (notifier_fn != 0)
      (*notifier_fn)();
     if (obj != NULL || (notifier_fn == 0 && res <= 0))
      break;
    }
#endif
   if (retry == ~(GC_word)0)
    retry = 0;
  } while (++retry <= GC_max_retries);
 }
 return obj;
}

GC_API void *GC_CALL GC_malloc(size_t size)
{
 struct GC_gcdata_s *gcdata;
 return GC_general_malloc(&gcdata, (GC_word)size, ~(GC_word)0);
}

GC_API void *GC_CALL GC_malloc_atomic(size_t size)
{
 struct GC_gcdata_s *gcdata;
 return GC_general_malloc(&gcdata, (GC_word)size, 0);
}

GC_API void GC_CALL GC_init(void)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 GC_LEAVE(gcdata);
}

GC_API void GC_CALL GC_set_finalizer_notifier(GC_finalizer_notifier_proc fn)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 GC_finalizer_notifier = fn;
 GC_LEAVE(gcdata);
}

GC_API void GC_CALL GC_set_start_callback(GC_start_callback_proc fn)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 GC_start_call_back = fn;
 GC_LEAVE(gcdata);
}

GC_API void GC_CALL GC_set_stop_func(GC_stop_func fn)
{
 struct GC_gcdata_s *gcdata;
 if (fn == 0)
  GC_abort_badptr(NULL);
 GC_enter(&gcdata);
 GC_default_stop_func = fn;
 GC_LEAVE(gcdata);
}

GC_API void *GC_CALL GC_do_blocking(GC_fn_type fn, void *client_data)
{
#ifndef GC_NO_INACTIVE
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 if (!GC_set_inactive_sp(&gcdata))
  GC_abort_badptr(NULL);
 GC_LEAVE(gcdata);
#endif
 client_data = (*fn)(client_data);
#ifndef GC_NO_INACTIVE
 GC_enter(&gcdata);
 gcdata->cur_stack->inactive = 0;
 GC_LEAVE(gcdata);
#endif
 return client_data;
}

GC_API void *GC_CALL GC_call_with_gc_active(GC_fn_type fn, void *client_data)
{
 struct GC_gcdata_s *gcdata;
#ifdef GC_NO_INACTIVE
 GC_enter(&gcdata);
 GC_LEAVE(gcdata);
 return (*fn)(client_data);
#else
 struct GC_activation_frame_s frame;
 GC_enter(&gcdata);
 if (!GC_set_activation_frame(&frame, gcdata->cur_stack))
 {
  GC_LEAVE(gcdata);
  return (*fn)(client_data);
 }
 GC_LEAVE(gcdata);
 client_data = (*fn)(client_data);
 GC_enter(&gcdata);
 GC_restore_inactive_sp(gcdata->cur_stack, &frame);
 GC_LEAVE(gcdata);
 return client_data;
#endif
}

#ifdef GC_THREADS

GC_API void GC_CALL GC_allow_register_threads(void)
{
 /* dummy */
}

GC_API int GC_CALL GC_register_my_thread(CONST struct GC_stack_base *sb)
{
 struct GC_gcdata_s *gcdata;
 struct GC_stkroot_s *cur_stack;
 GC_word stack_addr;
 int res;
 res = GC_enter(&gcdata);
 if ((stack_addr = (GC_word)sb->mem_base) != 0)
 {
  if ((cur_stack = gcdata->cur_stack)->begin_addr > stack_addr)
   cur_stack->begin_addr = stack_addr;
  if (cur_stack->end_addr < stack_addr)
   cur_stack->end_addr = stack_addr;
 }
 GC_LEAVE(gcdata);
 return res;
}

GC_API int GC_CALL GC_unregister_my_thread(void)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 GC_stkroot_delete_cur(gcdata);
 GC_LEAVE(gcdata);
 return GC_SUCCESS;
}

#endif /* GC_THREADS */

GC_API void *GC_CALL GC_call_with_alloc_lock(GC_fn_type fn, void *client_data)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 client_data = (*fn)(client_data);
 GC_LEAVE(gcdata);
 return client_data;
}

#ifndef GC_MISC_EXCLUDE

GC_API GC_finalizer_notifier_proc GC_CALL GC_get_finalizer_notifier(void)
{
 struct GC_gcdata_s *gcdata;
 GC_finalizer_notifier_proc fn;
 GC_enter(&gcdata);
 fn = GC_finalizer_notifier;
 GC_LEAVE(gcdata);
 return fn;
}

GC_API GC_start_callback_proc GC_CALL GC_get_start_callback(void)
{
 struct GC_gcdata_s *gcdata;
 GC_start_callback_proc fn;
 GC_enter(&gcdata);
 fn = GC_start_call_back;
 GC_LEAVE(gcdata);
 return fn;
}

GC_API GC_stop_func GC_CALL GC_get_stop_func(void)
{
 struct GC_gcdata_s *gcdata;
 GC_stop_func fn;
 GC_enter(&gcdata);
 fn = GC_default_stop_func;
 GC_LEAVE(gcdata);
 return fn;
}

GC_API void GC_CALL GC_set_warn_proc(GC_warn_proc fn)
{
 struct GC_gcdata_s *gcdata;
 if (fn == 0)
  GC_abort_badptr(NULL);
 GC_enter(&gcdata);
 GC_current_warn_proc = fn;
 GC_LEAVE(gcdata);
}

GC_API GC_warn_proc GC_CALL GC_get_warn_proc(void)
{
 struct GC_gcdata_s *gcdata;
 GC_warn_proc fn;
 GC_enter(&gcdata);
 fn = GC_current_warn_proc;
 GC_LEAVE(gcdata);
 return fn;
}

GC_API void GC_CALL GC_enable_incremental(void)
{
 /* dummy */
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 GC_LEAVE(gcdata);
}

GC_API int GC_CALL GC_expand_hp(size_t incsize)
{
 struct GC_gcdata_s *gcdata;
 int res;
 GC_enter(&gcdata);
 res = GC_heap_expand(gcdata, (GC_word)incsize);
 GC_LEAVE(gcdata);
 return res >= 0 ? 1 : 0;
}

GC_API void GC_CALL GC_disable(void)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 GC_dont_gc++;
 GC_LEAVE(gcdata);
}

GC_API void GC_CALL GC_enable(void)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 GC_dont_gc--;
 GC_LEAVE(gcdata);
}

GC_API int GC_CALL GC_should_invoke_finalizers(void)
{
 int res = 0;
#ifndef GC_NO_FNLZ
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 if (gcdata->fnlz_htable.ready_fnlz != NULL)
  res = 1;
 GC_LEAVE(gcdata);
#endif
 return res;
}

GC_API size_t GC_CALL GC_get_bytes_since_gc(void)
{
 struct GC_gcdata_s *gcdata;
 GC_word size;
 GC_enter(&gcdata);
 size = gcdata->bytes_allocd;
 GC_LEAVE(gcdata);
 return (size_t)size;
}

GC_API size_t GC_CALL GC_get_total_bytes(void)
{
 struct GC_gcdata_s *gcdata;
 GC_word size;
 GC_enter(&gcdata);
 size = gcdata->allocd_before_gc + gcdata->bytes_allocd;
 GC_LEAVE(gcdata);
 return (size_t)size;
}

GC_API void GC_CALL GC_remove_roots(void *low_addr, void *high_addr_plus_1)
{
 struct GC_gcdata_s *gcdata;
 if (low_addr != NULL)
 {
  low_addr = (void *)(((GC_word)low_addr + (sizeof(GC_word) - 1)) &
              ~(sizeof(GC_word) - 1));
  high_addr_plus_1 =
   (void *)((GC_word)high_addr_plus_1 & ~(sizeof(GC_word) - 1));
  GC_enter(&gcdata);
  if ((GC_word)low_addr < (GC_word)high_addr_plus_1)
   GC_roots_del_inside(gcdata, (GC_word)low_addr, (GC_word)high_addr_plus_1);
  GC_LEAVE(gcdata);
 }
}

GC_API void GC_CALL GC_clear_roots(void)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 GC_roots_del_inside(gcdata, sizeof(GC_word),
  ~(GC_word)(sizeof(GC_word) - 1));
 GC_LEAVE(gcdata);
}

GC_API void GC_CALL GC_exclude_static_roots(void *low_addr,
 void *high_addr_plus_1)
{
 struct GC_gcdata_s *gcdata;
 if (low_addr != NULL)
 {
  low_addr = (void *)(((GC_word)low_addr + (sizeof(GC_word) - 1)) &
              ~(sizeof(GC_word) - 1));
  high_addr_plus_1 =
   (void *)((GC_word)high_addr_plus_1 & ~(sizeof(GC_word) - 1));
  GC_enter(&gcdata);
  if ((GC_word)low_addr < (GC_word)high_addr_plus_1)
  {
   GC_roots_del_inside(gcdata, (GC_word)low_addr, (GC_word)high_addr_plus_1);
   (void)GC_roots_exclude(gcdata, (GC_word)low_addr,
    (GC_word)high_addr_plus_1);
  }
  GC_LEAVE(gcdata);
 }
}

#endif /* ! GC_MISC_EXCLUDE */

GC_API void GC_CALL GC_add_roots(void *low_addr, void *high_addr_plus_1)
{
 struct GC_gcdata_s *gcdata;
 GC_enter(&gcdata);
 (void)GC_roots_add(gcdata, (GC_word)low_addr, (GC_word)high_addr_plus_1);
 GC_LEAVE(gcdata);
}

GC_API size_t GC_CALL GC_get_heap_size(void)
{
 struct GC_gcdata_s *gcdata;
 GC_word size;
 GC_enter(&gcdata);
 size = gcdata->marked_bytes + gcdata->bytes_allocd +
#ifndef GC_NO_DLINKS
         gcdata->dlink_htable.count * sizeof(struct GC_dlink_s) +
#endif
#ifndef GC_NO_FNLZ
         gcdata->fnlz_htable.count * sizeof(struct GC_fnlz_s) +
#endif
         gcdata->obj_htable.pending_free_size + gcdata->free_bytes;
 GC_LEAVE(gcdata);
 return (size_t)size;
}

GC_API size_t GC_CALL GC_get_free_bytes(void)
{
 struct GC_gcdata_s *gcdata;
 GC_word size;
 GC_enter(&gcdata);
 size = gcdata->free_bytes;
 GC_LEAVE(gcdata);
 return (size_t)size;
}

GC_API int GC_CALL GC_try_to_collect(GC_stop_func stop_func)
{
 struct GC_gcdata_s *gcdata;
 int res = 1;
#ifndef GC_NO_FNLZ
 GC_finalizer_notifier_proc notifier_fn = 0;
#endif
 if (stop_func == 0)
  GC_abort_badptr(NULL);
 GC_enter(&gcdata);
 if (!GC_dont_gc && gcdata->bytes_allocd)
 {
  GC_collect_unreachable(gcdata, stop_func);
  res = 0;
  if (!gcdata->bytes_allocd)
  {
   res = 1;
#ifndef GC_NO_FNLZ
   gcdata->cur_stack->inside_fnlz = 0;
   if (gcdata->fnlz_htable.ready_fnlz != NULL && GC_finalize_on_demand)
   {
    gcdata->notifier_gc_no = GC_gc_no;
    notifier_fn = GC_finalizer_notifier;
   }
#endif
  }
 }
 GC_LEAVE(gcdata);
#ifndef GC_NO_FNLZ
 if (notifier_fn != 0)
  (*notifier_fn)();
#endif
 return res;
}

GC_API void GC_CALL GC_gcollect_and_unmap(void)
{
 (void)GC_try_to_collect(GC_never_stop_func);
}

GC_API void GC_CALL GC_gcollect(void)
{
 struct GC_gcdata_s *gcdata;
#ifndef GC_NO_FNLZ
 GC_finalizer_notifier_proc notifier_fn = 0;
#endif
 GC_enter(&gcdata);
 if (!GC_dont_gc && gcdata->bytes_allocd)
 {
  GC_collect_unreachable(gcdata, GC_default_stop_func);
#ifndef GC_NO_FNLZ
  if (!gcdata->bytes_allocd)
  {
   gcdata->cur_stack->inside_fnlz = 0;
   if (gcdata->fnlz_htable.ready_fnlz != NULL && GC_finalize_on_demand)
   {
    gcdata->notifier_gc_no = GC_gc_no;
    notifier_fn = GC_finalizer_notifier;
   }
  }
#endif
 }
 GC_LEAVE(gcdata);
#ifndef GC_NO_FNLZ
 if (notifier_fn != 0)
  (*notifier_fn)();
#endif
}

GC_API void *GC_CALL GC_base(void *displaced_pointer)
{
#ifdef GC_NO_GCBASE
 if (displaced_pointer != NULL)
  GC_abort_badptr(displaced_pointer);
#else
 struct GC_gcdata_s *gcdata;
 struct GC_objlink_s *objlink;
 if (displaced_pointer != NULL)
 {
  GC_enter(&gcdata);
  displaced_pointer =
   (objlink = GC_objlink_get(gcdata, displaced_pointer)) != NULL ||
   (objlink = GC_objlink_refill_find(gcdata, displaced_pointer)) != NULL ?
   objlink->obj : NULL;
  GC_LEAVE(gcdata);
 }
#endif
 return displaced_pointer;
}

GC_API int GC_CALL GC_general_register_disappearing_link(void **link,
 void *obj)
{
#ifndef GC_NO_REGISTER_DLINK
 struct GC_gcdata_s *gcdata;
 struct GC_objlink_s *objlink;
#ifdef GC_NO_DLINKS
 GC_word objsize;
#else
 struct GC_dlink_s **new_hroots;
 struct GC_dlink_s *new_dlink;
 GC_word new_log2_size;
#endif
#endif
 int res = 0;
 if (((GC_word)link & (sizeof(GC_word) - 1)) != 0 || link == NULL)
  GC_abort_badptr(link);
#ifndef GC_NO_REGISTER_DLINK
 GC_enter(&gcdata);
 if (obj != NULL)
 {
#ifdef GC_NO_DLINKS
  if ((objlink = GC_objlink_get(gcdata, (void *)link)) != NULL ||
      (objlink = GC_objlink_refill_find(gcdata, (void *)link)) != NULL)
  {
   obj = NULL;
   if (((objsize = objlink->atomic_and_size) & GC_ATOMIC_MASK) != 0)
   {
    gcdata->followscan_size += objsize & ~(GC_ATOMIC_MASK |
                                (sizeof(GC_word) - 1));
#ifndef GC_GCJ_SUPPORT
    objlink->atomic_and_size = objsize & ~GC_ATOMIC_MASK;
#endif
   }
#ifdef GC_GCJ_SUPPORT
   objlink->atomic_and_size = objsize & ~(GC_ATOMIC_MASK | GC_HASDSLEN_MASK);
#endif
  }
#else
  if ((objlink = GC_objlink_get(gcdata, obj)) != NULL ||
      (objlink = GC_objlink_refill_find(gcdata, obj)) != NULL)
  {
   if (objlink->obj == obj)
   {
    res = GC_dlink_add(gcdata, GC_HIDE_POINTER(link), objlink,
           (new_dlink = gcdata->dlink_htable.free_list) != NULL &&
           gcdata->free_bytes >= sizeof(struct GC_dlink_s) ? new_dlink :
           ((GC_HASH_RESIZECOND(gcdata->dlink_htable.count,
           gcdata->dlink_htable.log2_size) &&
           (new_hroots = GC_alloc_hroots(gcdata, new_log2_size =
           gcdata->dlink_htable.log2_size + 1, &GC_nil_dlink)) != NULL ?
           (GC_dlink_tblresize(gcdata, new_hroots, new_log2_size), 0) : 0),
           GC_core_malloc_with_gc(gcdata, sizeof(struct GC_dlink_s), &res)));
    obj = NULL;
   }
  }
   else obj = NULL;
#endif
 }
 GC_LEAVE(gcdata);
#endif
 if (obj != NULL)
  GC_abort_badptr(obj);
 return res;
}

#ifndef GC_MISC_EXCLUDE

GC_API int GC_CALL GC_unregister_disappearing_link(void **link)
{
#ifndef GC_NO_DLINKS
 struct GC_gcdata_s *gcdata;
 GC_word hidden_link;
#endif
 int res = 0;
#ifdef GC_NO_DLINKS
 GC_noop1((GC_word)link);
#else
 GC_enter(&gcdata);
 if (gcdata->dlink_htable.count && (hidden_link = GC_HIDE_POINTER(link)) != 0)
  res = GC_dlink_delete(gcdata, hidden_link, hidden_link, hidden_link);
 GC_LEAVE(gcdata);
#endif
 return res;
}

#endif /* ! GC_MISC_EXCLUDE */

GC_API void GC_CALL GC_register_finalizer_no_order(void *obj,
 GC_finalization_proc fn, void *client_data, GC_finalization_proc *ofn,
 void **odata)
{
#ifdef GC_NO_FNLZ
 if (obj != NULL && fn != 0)
  GC_noop1((GC_word)client_data);
 if (ofn != NULL)
  *ofn = 0;
 if (odata != NULL)
  *odata = NULL;
#else
 struct GC_gcdata_s *gcdata;
 struct GC_objlink_s *objlink;
 struct GC_fnlz_s **new_hroots;
 void *old_data;
 GC_word new_log2_size;
 int res = 0;
 if (odata == NULL)
  odata = &old_data;
 GC_enter(&gcdata);
 if (obj != NULL && ((objlink = GC_objlink_get(gcdata, obj)) != NULL ||
     (objlink = GC_objlink_refill_find(gcdata, obj)) != NULL))
 {
  if (objlink->obj == obj)
  {
   if (fn != 0)
   {
    if (GC_HASH_RESIZECOND(gcdata->fnlz_htable.count,
        gcdata->fnlz_htable.log2_size) &&
        (new_hroots = GC_alloc_hroots(gcdata, new_log2_size =
        gcdata->fnlz_htable.log2_size + 1, &GC_nil_fnlz)) != NULL)
     GC_fnlz_tblresize(gcdata, new_hroots, new_log2_size);
    if (gcdata->fnlz_htable.single_free == NULL)
     res = (gcdata->fnlz_htable.single_free = GC_core_malloc_with_gc(gcdata,
            sizeof(struct GC_fnlz_s), &res)) != NULL ? 1 : -1;
   }
   obj = NULL;
   if ((fn = GC_fnlz_add_del(gcdata, objlink, fn, client_data, odata)) != 0)
   {
    if (ofn != NULL)
     *ofn = fn;
    fn = 0;
   }
    else
    {
     if ((res >> 1) == 0)
     {
      if (ofn != NULL)
       *ofn = 0;
      if (res > 0 && !GC_finalize_on_demand &&
          ++gcdata->cur_stack->inside_fnlz == 1 &&
          (fn = GC_fnlz_del_ready(gcdata, &objlink, &client_data)) == 0)
       gcdata->cur_stack->inside_fnlz = 0;
     }
    }
  }
 }
  else
  {
   obj = NULL;
   if (ofn != NULL)
    *ofn = 0;
   fn = 0;
   *odata = NULL;
   objlink = NULL;
  }
 GC_LEAVE(gcdata);
 if (obj != NULL)
  GC_abort_badptr(obj);
 if (fn != 0)
 {
  (*fn)(objlink->obj, client_data);
  GC_enter(&gcdata);
  gcdata->cur_stack->inside_fnlz = 0;
  GC_LEAVE(gcdata);
 }
#endif
}

GC_API int GC_CALL GC_invoke_finalizers(void)
{
 GC_word count = 0;
#ifndef GC_NO_FNLZ
 struct GC_gcdata_s *gcdata;
 struct GC_objlink_s *objlink = NULL;
 void *client_data = NULL;
 GC_finalization_proc fn;
 for (;;)
 {
  GC_enter(&gcdata);
  if ((fn = GC_fnlz_del_ready(gcdata, &objlink, &client_data)) != 0)
  {
   if (!count)
    gcdata->cur_stack->inside_fnlz++;
  }
   else
   {
    if (count)
     gcdata->cur_stack->inside_fnlz = 0;
   }
  GC_LEAVE(gcdata);
  if (fn == 0)
   break;
  count++;
  (*fn)(objlink->obj, client_data);
 }
#endif
 return (int)count;
}

#ifdef GC_GCJ_SUPPORT

GC_API void GC_CALL GC_init_gcj_malloc(int mp_index, void *mp)
{
 /* dummy */
 struct GC_gcdata_s *gcdata;
 if (mp != 0)
  GC_noop1((GC_word)mp_index);
 GC_enter(&gcdata);
 GC_LEAVE(gcdata);
}

GC_API void *GC_CALL GC_gcj_malloc(size_t size, void *vtable)
{
 struct GC_gcdata_s *gcdata;
 if ((*(GC_word *)((char *)vtable + MARK_DESCR_OFFSET) & GC_DS_TAGS) !=
     GC_DS_LENGTH || size < sizeof(GC_word))
  GC_abort_badptr(vtable);
 return GC_general_malloc(&gcdata, (GC_word)size, (GC_word)vtable);
}

#endif /* GC_GCJ_SUPPORT */

#ifndef JAVA_FINALIZATION_NOT_NEEDED

GC_API void GC_CALL GC_finalize_all(void)
{
#ifndef GC_NO_FNLZ
 struct GC_gcdata_s *gcdata;
 struct GC_objlink_s *objlink = NULL;
 void *client_data = NULL;
 GC_word count = 0;
 GC_finalization_proc fn;
 for (;;)
 {
  GC_enter(&gcdata);
  if (!count && gcdata->fnlz_htable.count)
   GC_fnlz_ready_all(gcdata);
  if ((fn = GC_fnlz_del_ready(gcdata, &objlink, &client_data)) != 0)
  {
   if (!count)
    gcdata->cur_stack->inside_fnlz++;
  }
   else
   {
    if (count)
     gcdata->cur_stack->inside_fnlz = 0;
   }
  GC_LEAVE(gcdata);
  if (fn != 0)
  {
   count++;
   (*fn)(objlink->obj, client_data);
  }
   else
   {
    if (!count)
     break;
    count = 0;
   }
 }
#endif
}

#endif /* ! JAVA_FINALIZATION_NOT_NEEDED */
