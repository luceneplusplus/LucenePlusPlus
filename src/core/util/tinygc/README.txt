                  TinyGC v2.6 (Tiny Garbage Collector)
                  ------------------------------------

Copyright (C) 2006-2010 Ivan Maidanski <ivmai@mail.ru>
All rights reserved.

Project home page
-----------------

http://tinygc.sourceforge.net


Preface
-------

TinyGC is an independent implementation of the API of the well-known
Boehm-Demers-Weiser Conservative GC ("BDWGC" or "BoehmGC" for short).

TinyGC has been initially developed as a part of the JCGO project to be
used as a BoehmGC replacement. At present, TinyGC is a standalone
project.

Disclaimer/License
------------------

This is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This software is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License (GPL) for more details.

You should have received a copy of the GNU General Public License
along with this software. If not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA 02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library. Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module. An independent module is a module which is not derived from
or based on this library. If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so. If you do not wish to do so, delete this
exception statement from your version.

Target environments
-------------------

TinyGC is designed to be used primary in projects requiring Java-like
memory garbage collection functionality for:
- memory constrained environments;
- 8/16-bit systems;
- BoehmGC temporal replacement (for application debugging, testing and
benchmarking purposes);
- targets where BoehmGC is still not ported to.

TinyGC is NOT designed for speed.

Design principles
-----------------

The major principles are:
- source and binary compatibility with BoehmGC;
- implements only the minimal subset of the BoehmGC API (v7.2)
sufficient for the Java/GCJ-like functionality;
- highly portable (contains no assembler, no machine/OS-specific code
portions) and tunable;
- supports 16/32/64-bit architectures ("flat" data models only);
- compact code and small initial internal data size;
- simple collection World-stopped Mark-and-Sweep algorithm
implementation;
- malloc-based allocation (i.e. every object is allocated using
malloc());
- objects finalization and memory recycling (reclaiming to the
underlaying malloc implementation) is done lazily (between
collections).

Advantages and drawbacks
------------------------

The major TinyGC advantages over BoehmGC are:
- small code size (it could be as small as 3800 bytes);
- 16-bit architectures are supported;
- small initial internal data size;
- less source code (to verify);
- minimal set of the underlaying clib/pthread/Win32 functions used.

The drawbacks of TinyGC are:
- lower allocation speed and larger world-stopped collection delays
(typically);
- only a small subset of the BoehmGC API is implemented;
- no support for C++;
- no support for architectures with a separate registers stack (like
IA-64);
- no find-leak and pointer back-trace modes;
- no automatic registration of static data roots, stack bottom and
threads;
- no "advanced" allocation and collection technologies (no
blacklisting, memory unmapping, thread-local allocation, parallel
marking, generation and incremental collections);
- relies on the underlaying malloc/free() implementation (which may be
broken for large heaps, like, e.g., in some versions of msvcrt);
- "all-interior-pointers" mode is limited by the offset of 256
(1 << GC_LOG2_OFFIGNORE) bytes (this also means that disappearing links
must not be placed at this or higher offsets of an allocated object);
- only the length-based descriptor for the GCJ-style allocation is
supported;
- only Java-like "no-order" finalization policy is implemented.

Notes
-----

Release notes:
- no binary distribution is offered on the official site;
- no make file (or other building script) is provided.

Implementation notes:
- the same environment variables are recognized as in BoehmGC;
- the finalization and disappearing links implementations are generally
the same as in BoehmGC;
- full TinyGC implementation resides in a single file (all the internal
symbols are not visible outside);
- both pthreads and Win32 threads are supported;
- no thread-safety of the underlaying malloc/free is required;
- the stack direction is detected at TinyGC initialization;
- no warnings are printed;
- the thread "suspend" handler does not use pthread synchronization
primitives (yielding and sleeping are used instead for better
portability);
- CPU state is saved by setjmp();
- there is no object "header" (i.e. the original object size is passed
to the underlaying malloc()).

Usage notes:
- all pointers must be word-aligned;
- it is assumed that the compiler performs only GC-safe pointer
transformations;
- static data roots must be manually registered;
- application threads must be manually registered and unregistered;
- it would be good to use GC_call_with_gc_active() to record the
correct main stack base (after GC_INIT()).

Tuning macros
-------------

Useful macros for tuning (same as in BoehmGC):
- GC_DLL - compile to produce a DLL (gc.dll);
- ALL_INTERIOR_POINTERS - turn on "all-interior-pointers" mode by
default;
- GC_GCJ_SUPPORT - compile with GCJ-style allocation support;
- GC_THREADS - compile with thread support (pthread-based by default);
- GC_WIN32_THREADS - compile with Win32-based thread support;
- JAVA_FINALIZATION_NOT_NEEDED - exclude GC_finalize_all() from the
API;
- DONT_ADD_BYTE_AT_END - do not pad objects even if
"all-interior-pointers" mode is on;
- FINALIZE_ON_DEMAND - causes finalizers to be run only in response to
explicit GC_invoke_finalizers() calls by default (unless overridden at
run-time);
- GC_IGNORE_GCJ_INFO - disable GCJ-style type information (useful for
debugging);
- GC_DONT_EXPAND - do not implicitly expand the heap by default (unless
overridden at run-time);
- GC_INITIAL_HEAP_SIZE=<value> - set the desired default initial heap
size (in bytes);
- GC_FREE_SPACE_DIVISOR=<value> - the default trade-off between garbage
collection and heap growth;
- GC_MAX_RETRIES=<value> - the default maximum number of garbage
collections attempted before reporting out of memory after a heap
expansion failure.

Major TinyGC-specific macros:
- GC_PRINT_MSGS - compile with statistic and error printing
capabilities;
- GC_GETENV_SKIP - do not recognize any environment variable (for
smaller code size or for WinCE targets);
- GC_WIN32_WCE - compile for WinCE (use thread Id instead of thread
handle, and retry on SuspendThread() failures);
- GC_NO_INACTIVE, GC_MISC_EXCLUDE - exclude the corresponding parts of
the TinyGC API (for smaller code size);
- GC_NO_GCBASE, GC_NO_FNLZ, GC_NO_DLINKS, GC_NO_REGISTER_DLINK -
exclude the support (i.e. expose dummy support) for the corresponding
TinyGC capabilities (for smaller code size);
- GC_USE_WIN32_SYSTEMTIME - use Win32 GetSystemTime() API call instead
of ftime() or gettimeofday() ones (useful for WinCE);
- GC_USE_GETTIMEOFDAY - use Unix gettimeofday() API call instead of
ftime() one;
- GC_OMIT_REGISTER_KEYWORD - ignore C "register" keyword;
- CONST=/**/ - ignore C "const" keyword;
- INLINE=/**/ - ignore C "__inline" keyword;
- GC_FASTCALL=/**/ - ignore C x86-specific "__fastcall" keyword;
- GC_CLIBDECL=/**/ - ignore C x86-specific "__cdecl" keyword;
- GC_DATASTATIC=/**/ - do not use C static storage class for global
data;
- GC_STATIC=/**/ - do not use C static linkage for GC internal
functions;
- GC_DATASTARTSYM=<id> - specify the external symbol which is the first
one in the program data section;
- GC_DATAENDSYM=<id> - specify the external symbol which is the last
one in the program data section;
- GC_DATASTARTSYM2=<id> - specify the external symbol which is the
first one in the program ".bss" section;
- GC_DATAENDSYM2=<id> - specify the external symbol which is the last
one in the program ".bss" section;
- GC_STACKBOTTOMVAR=<id> - specify the external symbol pointing to the
program main thread stack bottom (or top if GC_STACKLEN is 0);
- GC_STACKLENVAR=<id> - specify the external symbol pointing to the
program main thread stack size;
- GC_SIG_SUSPEND=<sig_id> - use specific signal to suspend Posix
threads;
- GC_WIN32_CONTEXT_SP_NAME=<sp_id> - use specific stack pointer
register name (defined in Win32 "winnt.h");
- GC_LOG2_OFFIGNORE=<value> - explicitly specify the number of address
lowest bits ignored for object address hash computations.

Useful macros for client application tuning (same as in BoehmGC):
- GC_DONT_EXPAND - do not implicitly expand the heap (unless overridden
at run-time);
- GC_DLL - use TinyGC residing in a DLL;
- GC_THREADS - declare the prototypes for the collector multi-threading
support;
- GC_CALL=<calling_conv> - explicitly specify calling convention for
the GC API functions;
- GC_CALLBACK=<calling_conv> - explicitly specify an alternate calling
convention for the GC API user callbacks;
- GC_INITIAL_HEAP_SIZE=<value> - set the desired initial heap size (in
bytes);
- GC_MAXIMUM_HEAP_SIZE=<value> - set the desired maximum heap size (in
bytes);
- GC_FREE_SPACE_DIVISOR=<value> - set the desired trade-off between
garbage collection and heap growth;
- GC_MAX_RETRIES=<value> - set the desired maximum number of garbage
collections attempted before reporting out of memory after a heap
expansion failure.

Note 1: if GC_NO_DLINKS is used without GC_NO_REGISTER_DLINK then all
the disappearing links are treated as normal pointers.

Note 2: for Unix use the command-line options:
 -DGC_FASTCALL= -DGC_CLIBDECL=

Note 3: for Solaris SunOS "cc" use the command-line options:
 -DGC_FASTCALL= -DGC_CLIBDECL= -DINLINE=inline
 -erroff=E_WHITE_SPACE_IN_DIRECTIVE

Environment variables
---------------------

Environment variables recognized (if supported, same as in BoehmGC):
GC_DONT_GC - turn off garbage collection;
GC_PRINT_STATS - turn on statistic printing on every garbage collection
(if supported);
GC_ALL_INTERIOR_POINTERS - turn on "all-interior-pointers" collector
mode;
GC_IGNORE_GCJ_INFO - ignore the GCJ-style type descriptors (if
supported);
GC_INITIAL_HEAP_SIZE=<value> - set the initial heap size (in bytes);
GC_MAXIMUM_HEAP_SIZE=<value> - set the maximum heap size (in bytes);
GC_FREE_SPACE_DIVISOR=<value> - change the default trade-off between
garbage collection and heap growth.

Note: the values assigned to the specified environment variables
override the similar default (or the preset at the compilation time)
values.

Files list
----------

The TinyGC distribution consists of:
- ChangeLog - the standard changes log file;
- GNU_GPL.txt - the GNU GPLv2 license;
- README.txt - this file;
- gc.h - the main include file (the subset of that in BoehmGC);
- gc_gcj.h - GCJ-style allocation API (the subset of that in BoehmGC);
- gc_mark.h - contains only the constants for the GCJ-style
length-based descriptors and the collections notifier prototypes;
- javaxfc.h - same as in BoehmGC;
- tinygc.c - the TinyGC implementation itself.

User's feedback
---------------

Any questions, suggestions, bug reports and patches are welcomed at
the TinyGC site tracker (hosted at SourceForge.net).

                          --- [ End of File ] ---
