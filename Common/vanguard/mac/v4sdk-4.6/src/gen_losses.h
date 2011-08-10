
/**
 * @file gen_loses.h
 * generate nal units with losses
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __GEN_LOSSES_H__
#define __GEN_LOSSES_H__

//////////////////////////////////////////////////////////////////////////
#define DIE_FILE_NAME	"die.264"

int gen_nal_losses_main(int flag, media_sample_t *ms, int reset);


//////////////////////////////////////////////////////////////////////////

// just copy 1/8 part of NAL from center to the end
static int gen_nal_losses_0(media_sample_t *ms)
{
	byte *buf = (byte*)ms->data;
	if (ms->used_size > 8)
	{
//		int half  = ms->used_size / 2;
		int delta = ms->used_size / 8;
		memcpy(buf + delta, buf + ms->used_size - 1 - delta, delta);
	}
	return 0;
}

// just copy 1/8 part of NAL from end to the center
static int gen_nal_losses_1(media_sample_t *ms)
{
	byte *buf = (byte*)ms->data;
	if (ms->used_size > 8)
	{
		int half  = ms->used_size / 2;
		int delta = ms->used_size / 8;
		memcpy(buf + half - delta, buf + half, delta);
	}
	return 0;
}

// zero last N bytes
static int gen_nal_losses_2(media_sample_t *ms)
{
	byte *buf = (byte*)ms->data;
	int n = 12;
	if (ms->used_size > (n + 1))
	{
		memset(buf + ms->used_size - n - 1, 0, n);
	}
	return 0;
}

// put random byte(s) to random place
static int gen_nal_losses_3(media_sample_t *ms)
{
	byte b;
	int i, n, place;
	byte *buf = (byte*)ms->data;
	if (ms->used_size > 16)
	{
		int part = 1 + (ms->used_size / 2048);
		n = (rand() % part);
		for (i=0; i<n; i++)
		{
			b = (byte)(rand() % 255);
			place = 1 + (rand() % (ms->used_size-1));
			if ((place > 0) && (place < ms->used_size))
			{
				buf[place] = b;
			}
		}
	}
	return 0;
}

// cut off last N bytes
static int gen_nal_losses_4(media_sample_t *ms)
{
	int to_cut;
	if (ms->used_size > 16)
	{
		to_cut = (rand() % ms->used_size/2);
		ms->used_size -= to_cut;
	}
	return 0;
}

static int gen_nal_losses_X(media_sample_t *ms)
{
	return 0;
}

// array of function pointers
typedef int gen_nal_losses_t(media_sample_t *ms);
gen_nal_losses_t *gen_nal_array[] = 
{
	gen_nal_losses_0,
	gen_nal_losses_1,
	gen_nal_losses_2,
	gen_nal_losses_3,
	gen_nal_losses_4,
	gen_nal_losses_X // empty function
};
const int gen_nal_count = sizeof(gen_nal_array) / sizeof(gen_nal_array[0]);

// public main function: choose random method and call it
// flag is currently ignored
int gen_nal_losses_main(int idx, media_sample_t *ms, int reset)
{
	int rc;
	gen_nal_losses_t *func;
	srand((unsigned int)time(0));
	if (idx == 255)
	{
		idx = 1 + (rand() % (gen_nal_count-1));
	}


	// run the function
	if (idx == 256)
	{
		func = gen_nal_losses_X;
	}
	else
	{	
		if ((idx < 1) || (idx > gen_nal_count))
		{
			idx = 1;
		}

		func = gen_nal_array[idx-1];
	}

	rc = func(ms);

	// append before-die file
	{
		static int g_first_run = 0;
		FILE *f;
		if (g_first_run == 0 || reset)
		{
			f = fopen(DIE_FILE_NAME, "wb");
			if (f)
			{
				fclose(f);
			}
			g_first_run = 1;
		}
		f = fopen(DIE_FILE_NAME, "ab");
		if (f)
		{
			static byte startcode[4] = {0,0,0,1};
			fwrite(startcode, 4, 1, f);
			fwrite(ms->data, ms->used_size, 1, f);
			fclose(f);
		}
	}

	return rc;
}

#endif	// __GEN_LOSSES_H__
