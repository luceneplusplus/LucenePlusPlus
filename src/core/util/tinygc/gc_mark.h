/*
 * @(#) gc_mark.h -- TinyGC additional header (explicit GC marker control).
 * Copyright (C) 2006-2010 Ivan Maidanski <ivmai@mail.ru> All rights reserved.
 **
 * See also files: tinygc.c, gc.h, gc_gcj.h, javaxfc.h
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

#ifndef GC_MARK_H
#define GC_MARK_H

/* TinyGC API is a subset of Boehm-Demers-Weiser Conservative GC API */

#ifndef GC_H
#include "gc.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define GC_DS_TAG_BITS 2
#define GC_DS_TAGS ((1 << GC_DS_TAG_BITS) - 1)

#define GC_DS_LENGTH 0

typedef void (GC_CALLBACK *GC_start_callback_proc)(void);
GC_API void GC_CALL GC_set_start_callback(GC_start_callback_proc);
GC_API GC_start_callback_proc GC_CALL GC_get_start_callback(void);

#ifdef __cplusplus
}
#endif

#endif
