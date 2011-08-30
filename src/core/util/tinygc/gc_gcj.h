/*
 * @(#) gc_gcj.h -- TinyGC additional header (GCJ-style API).
 * Copyright (C) 2006-2009 Ivan Maidanski <ivmai@mail.ru> All rights reserved.
 **
 * See also files: tinygc.c, gc.h, gc_mark.h, javaxfc.h
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

#ifndef GC_GCJ_H
#define GC_GCJ_H

/* TinyGC API is a subset of Boehm-Demers-Weiser Conservative GC API */

#ifndef GC_H
#include "gc.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef GC_NEAR
/* TinyGC-specific */
#define GC_NEAR /* empty */
#endif

#define GC_GCJ_MALLOC(size, vtable) GC_gcj_malloc(size, vtable)

GC_API void GC_CALL GC_init_gcj_malloc(int, void * /* GC_mark_proc */);

GC_API void GC_NEAR *GC_CALL GC_gcj_malloc(size_t, void GC_NEAR *);

#ifdef __cplusplus
}
#endif

#endif
