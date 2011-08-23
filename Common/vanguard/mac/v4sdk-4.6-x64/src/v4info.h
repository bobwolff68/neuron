
/**
 * @file v4info.h
 * Codec information access example
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4INFO_H__
#define __V4INFO_H__

#include "v4_types.h"
#include "v4file.h"

static void print_codec_info(void)
{
	int rc;
	int build, magic;
	v4_caps_info_t caps;

	// print codec library version
	v4_get_version(&build, &magic);
	verbose_print(0, "VSofts H.264 Codec build %d (%d)\n", build, magic);

	// print capabilities
	rc = v4_get_caps(&caps, sizeof(caps));
	if (rc == VSSH_OK)
	{
		// OS
		char *os = "n/a";
		char *cc = "cc";
		switch (caps.platform_info.os)
		{
		case V4OS_WIN: os = "win"; break;
		case V4OS_LIN: os = "lin"; break;
		case V4OS_MAC: os = "mac"; break;
		}
		switch (caps.platform_info.cc)
		{
		case V4CC_MSC: cc = "msc"; break;
		case V4CC_ICC: cc = "icc"; break;
		case V4CC_GCC: cc = "gcc"; break;
		}
		verbose_print(0,"%s%d/%s: ", os, caps.platform_info.bits, cc);
		verbose_print(0,"OPT=0x%04x SVC=%d MVC=%d EXTRA=%d", caps.opt, caps.svc, caps.mvc, caps.extra);
		if (caps.demo_limits.enabled)
		{
			if (caps.demo_limits.expire_enabled)
				verbose_print(0," EXPIRE=%d", caps.demo_limits.expire_date);
		}
		verbose_print(0,"\n");
	}
}

#endif
