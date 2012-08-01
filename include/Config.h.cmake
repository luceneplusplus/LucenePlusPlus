/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2011 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef CONFIG_H
#define CONFIG_H

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#pragma warning(disable:4005)
#pragma warning(disable:4996)
#ifndef _WIN64
#pragma warning(disable:4244)
#endif
#endif

#if defined(_WIN32) || defined(_WIN64)
#define LPP_IMPORT __declspec(dllimport)
#define LPP_EXPORT __declspec(dllexport)
#else
#ifdef LPP_HAVE_GXXCLASSVISIBILITY
#define LPP_IMPORT __attribute__ ((visibility("default")))
#define LPP_EXPORT __attribute__ ((visibility("default")))
#else
#define LPP_IMPORT
#define LPP_EXPORT
#endif
#endif

// Define LPPAPI for dll builds
#ifdef LPP_HAVE_DLL
#ifdef LPP_BUILDING_LIB
#define LPPAPI LPP_EXPORT
#define LPPCONTRIBAPI LPP_EXPORT
#else
#define LPPAPI LPP_IMPORT
#define LPPCONTRIBAPI LPP_IMPORT
#endif
#else
#define LPPAPI
#define LPPCONTRIBAPI
#endif

// Check windows
#if defined(_WIN32) || defined(_WIN64)
#define LPP_UNICODE_CHAR_SIZE_2
#if defined(_WIN64)
#define LPP_BUILD_64
#else
#define LPP_BUILD_32
#endif
#endif

// Check GCC
#if defined(__GNUC__)
#define LPP_UNICODE_CHAR_SIZE_4
#if defined(__x86_64__) || defined(__ppc64__)
#define LPP_BUILD_64
#else
#define LPP_BUILD_32
#endif
#endif

// Default to 32-bit platforms
#if !defined(LPP_BUILD_32) && !defined(LPP_BUILD_64)
#define LPP_BUILD_32
#endif

// Default to 4-byte unicode format
#if !defined(LPP_UNICODE_CHAR_SIZE_2) && !defined(LPP_UNICODE_CHAR_SIZE_4)
#define LPP_UNICODE_CHAR_SIZE_4
#endif

// Define to enable cyclic checking in debug builds
#@DEFINE_USE_CYCLIC_CHECK@ LPP_USE_CYCLIC_CHECK

// Define to use custom allocator (useful in Windows builds and when using nedmalloc)
#@DEFINE_USE_ALLOCATOR@ LPP_USE_ALLOCATOR

// Define to use nedmalloc memory allocator
#@DEFINE_USE_NEDMALLOC@ LPP_USE_NEDMALLOC

#ifdef LPP_USE_NEDMALLOC
#define EXTSPEC LPPAPI
#endif

// Make internal bitset storage public
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS

// Force boost file-system version 2 for later boost versions > 1.46
#define BOOST_FILESYSTEM_VERSION 2

#endif
