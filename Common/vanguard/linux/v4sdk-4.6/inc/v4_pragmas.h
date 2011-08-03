
/**
 * @file v4_pragmas.h
 * Compiler control
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
*/


#ifndef __V4_PRAGMAS_H__
#define __V4_PRAGMAS_H__


/*----------------------------------------------------------------------------*/
/* intel compiler */
#if defined (__INTEL_COMPILER) || defined (__ICL) || defined (__ICC)
	/* enumerated type mixed with another type*/
	/*#pragma warning (disable : 188) */

	/* allow single line comment			   */
	#pragma warning (disable : 991)

	/* disable remark 'parameter was never referenced' */
	#pragma warning (disable : 869) 



/*----------------------------------------------------------------------------*/
/* gcc compiler */
#elif defined (__GNUC__)





/*----------------------------------------------------------------------------*/
/*visual studio compiler */
#else
	/* allow single line comment			   */
	#pragma warning (disable : 4001)

	/* Suppress 'unreferenced formal parameter'  */
	#pragma warning (disable : 4100)

	/* Suppress '... unreferenced inline function has been removed'*/
	/* #pragma warning (disable : 4514) */ 

	/* Suprress 'function': was declared deprecated */ 
	/* #pragma warning (disable : 4996) */

#endif



#endif/*__V4_PRAGMAS_H__*/
