/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef LUCENEPTR_H
#define LUCENEPTR_H

#include "Config.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#ifdef LPP_USE_GC
#include "gc.hpp"
#endif

#ifdef LPP_USE_GC
#define LucenePtr gc::gc_ptr
#define LuceneWeakPtr gc::wk_ptr

#define LuceneDynamicCast gc::dynamic_cast_gc_ptr
#define LuceneStaticCast gc::static_cast_gc_ptr
#define LuceneConstCast gc::const_cast_gc_ptr

#else
#define LucenePtr boost::shared_ptr
#define LuceneWeakPtr boost::weak_ptr

#define LuceneDynamicCast boost::dynamic_pointer_cast
#define LuceneStaticCast boost::static_pointer_cast
#define LuceneConstCast boost::const_pointer_cast

#endif

#endif