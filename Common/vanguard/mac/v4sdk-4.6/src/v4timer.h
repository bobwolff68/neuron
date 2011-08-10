
/**
 * @file v4timer.h
 * high resolution timer
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#if defined(WIN32) || defined(WIN64)

#include <windows.h>
typedef __int64 timer_clock_t;
typedef __int64 timer_delta_t;

#else	// Linux & Mac

#include <sys/time.h>
typedef struct timeval timer_clock_t;
typedef long timer_delta_t;

#endif

/**
 * High resolution timer support
 */

static timer_delta_t s_timer_freq = 1000;

void timer_init(void)
{
#ifdef WIN32
	QueryPerformanceFrequency((LARGE_INTEGER*)&s_timer_freq);
#else
	s_timer_freq = 1000;	// milliseconds
#endif
}

/**
 * Start the timer, return current time
 */
static void timer_start(timer_clock_t *start)
{
#ifdef WIN32
	QueryPerformanceCounter((LARGE_INTEGER*)start);
#else
	gettimeofday(start, 0);
#endif
}

/**
 * @return time delta in original platform-dependent units
 */
static timer_delta_t timer_delta(timer_clock_t *start)
{
	timer_clock_t finish;
#ifdef WIN32
	QueryPerformanceCounter((LARGE_INTEGER*)&finish);
	return (finish - *start);
#else
	gettimeofday(&finish, 0);
	return (finish.tv_sec - start->tv_sec) * 1000
		+ (finish.tv_usec - start->tv_usec) / 1000;
#endif
}

/**
 * @return time delta in seconds
 */
static int timer_delta_sec(timer_delta_t delta)
{
	if (delta == 0) delta = 1;
	if (s_timer_freq > 0)
		return (int)(delta / s_timer_freq);
	else
		return 0;
}

/**
 * @return time delta in seconds
 */
static double timer_delta_sec_dbl(timer_delta_t delta)
{
	if (s_timer_freq > 0)
		return (double)delta / (double)s_timer_freq;
	else
		return 0;
}

/**
 * @return time delta in milliseconds
 */
static int timer_delta_ms(timer_delta_t delta)
{
	if (s_timer_freq > 1000)
		return (int)(delta / (s_timer_freq/1000));
	else
		return (int)(delta*1000 / s_timer_freq);
}

/**
 * @return time delta in microseconds
 */
static int timer_delta_mks(timer_delta_t delta)
{
	if (s_timer_freq > 1000000)
		return (int)(delta / (s_timer_freq/1000000));
	else
		return (int)(delta*1000000 / s_timer_freq);
}

#endif	//__TIMER_H__
