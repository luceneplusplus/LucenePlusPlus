/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
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


// Define to enable boost integer types
#@USE_BOOST_INTEGER@ LPP_USE_BOOST_INTEGER


// Generic helper definitions for shared library support
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
  #define LPP_IMPORT __declspec(dllimport)
  #define LPP_EXPORT __declspec(dllexport)
  #define LPP_LOCAL
#else
  #if __GNUC__ >= 4
    #define LPP_IMPORT __attribute__ ((visibility ("default")))
    #define LPP_EXPORT __attribute__ ((visibility ("default")))
    #define LPP_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define LPP_IMPORT
    #define LPP_EXPORT
    #define LPP_LOCAL
  #endif
#endif

// bulding shared?
#@LPP_SHARED_DLL@ LPP_SHARED_LIB


// setup library binds
#ifdef LPP_SHARED_LIB
  #ifdef LPP_BUILDING_LIB
    #define LPP_API LPP_EXPORT
    #define LPP_CONTRIB_API LPP_EXPORT
  #else
    #define LPP_API LPP_IMPORT
    #define LPP_CONTRIB_API LPP_IMPORT
  #endif
#else
  #define LPP_API
  #define LPP_CONTRIB_API
  #define LPP_LOCAL
#endif // LPP_LOCAL


// legacy binds
#define LPPAPI LPP_API
#define LPPCONTRIBAPI LPP_CONTRIB_API
#define LPPLOCAL LPP_LOCAL


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
#@USE_CYCLIC_CHECK@ LPP_USE_CYCLIC_CHECK


// Make internal bitset storage public
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
#define BOOST_FILESYSTEM_VERSION 3


// Use windows definitions
#if defined(_WIN32) || defined(_WIN64)
  #define BOOST_USE_WINDOWS_H
#endif

// Disable deprication warnings in windows
#if defined(_WIN32) || defined(_WIN64)
  #define _CRT_SECURE_NO_WARNINGS
#endif


#endif //CONFIG_H
