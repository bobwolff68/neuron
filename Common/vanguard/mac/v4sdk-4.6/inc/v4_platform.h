
/**
 * @file v4_platform.h
 * 
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */


#ifndef __V4PLATFORM_H__
#define __V4PLATFORM_H__

#ifndef __GNUC__
/* switch of windows '_get_purecall_handler' : no function prototype given: converting '()' to '(void)'*/
#pragma warning(disable:4255)
#endif
#include <stdlib.h>
#include <stdio.h>
#ifndef __GNUC__
#pragma warning(default:4255)
#endif

#include <assert.h>
#include <string.h>
#include "v4_pragmas.h"
#include "v4_stdint.h"


// determine special prefix for modern MSC versions
#if !defined(_W64)
	#if _MSC_VER >= 1300
		#define _W64 __w64
	#else
		#define _W64
	#endif
#endif


// CPU architecture auto detect
#if defined(_M_IX86) || defined(__i386__)
	//#define V4_IA32
#elif defined(_M_IA64) || defined(_IA64)
	//#define V4_IA64
#endif


// OS auto detect using predefined compiler symbols

// Windows:
#if  !defined(WIN32) && !defined(WIN64)
	#if defined(_WIN64)
		#define WIN64
	#elif defined(_WIN32)
		#define WIN32
	#endif
#endif

// Linux:
#if  !defined(LIN32) && !defined(LIN64)
	#if defined(__linux__)
		#define LIN32
	#endif
#endif

// Mac OS X:
#if  !defined(MAC32) && !defined(MAC64)
	#if defined(__APPLE__) && defined(__MACH__)
		#if defined(_M_IX86) || defined(__i386__)
			#define MAC32
		#else 
			#define MAC64
		#endif
	#endif
#endif

// this define would work for Intel compilers
// for GNU compiler symbol "__forceinline" should be already defined 
// to "static inline"
#ifdef DEBUG
#define INLINE   static
#else
#define INLINE   __forceinline
#endif

#if defined(WIN32)

	#define ASMCALL  __cdecl

#elif defined(WIN64)

	#define ASMCALL

#elif defined(WINCE)

	#define ASMCALL __cdecl

#elif defined(LIN32)

	#include <stdint.h>
	#include <linux/types.h>
	#include <time.h>

	#define __int64 int_least64_t
	#define ASMCALL __attribute__((cdecl))

#elif defined(LIN64) //linux

	#include <stdint.h>
	#include <linux/types.h>
	#include <time.h>

	#define __int64 int_least64_t
	#define ASMCALL

#elif defined(MAC32)

	#include <stdint.h>
	#include <sys/types.h>
	#include <time.h>

	#define __int64 int_least64_t
	#define ASMCALL __attribute__((cdecl))

#elif defined(MAC64)

	#include <stdint.h>
	#include <sys/types.h>
	#include <time.h>

	#define __int64 int_least64_t
	#define ASMCALL

#else
	//error
#endif

// pointer alignment macro
#define ALIGNPTR(ptr,alignment) (((uintptr_t)(ptr) + (alignment-1))&(~((alignment)-1)))

#define VSSHAPI ASMCALL


// determine compiler
#ifndef V4CC
	#if defined(__INTEL_COMPILER)
		#define V4CC V4CC_ICC
	#elif defined(__GNUC__)
		#define V4CC V4CC_GCC
	#elif defined(_MSC_VER)
		#define V4CC V4CC_MSC
	#endif
#endif


#endif//__V4PLATFORM_H__
