/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2010 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _WIN32
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#pragma warning(disable:4005)
#pragma warning(disable:4996)
#endif

#ifdef _WIN32
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
#else
#define LPPAPI LPP_IMPORT
#endif
#else
#define LPPAPI
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

// Define to use nedmalloc memory allocator
// #define LPP_USE_NEDMALLOC

#ifdef LPP_USE_NEDMALLOC
#define EXTSPEC LPPAPI
#endif

// Make internal bitset storage public
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
