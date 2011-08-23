
/**
 * @file h264dec_file.h
 * one decoder instance for sample_mdec application 
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4DEC_FILE_H__
#define __V4DEC_FILE_H__

#include "md5.h"
#include "gen_losses.h"

#define INIT_NAL_SIZE (1*1024)

//////////////////////////////////////////////////////////////////////////
typedef struct h264dec_file_config_t
{
	int instance_num;
	int reset_flag;
	int md5_flag;
	int save_yuv_flag;
	int gen_losses;
} h264dec_file_config_t;

typedef struct h264dec_file_ctx_t
{
	h264dec_file_config_t config;
	char fname[1024];
	void *handle;
	v4d_settings_t settings;
	int is_ready;
	int frame_num;
	FILE *f_264;
	FILE *f_yuv;
	void *nal_parser;
	byte buf[INIT_NAL_SIZE];
	md5_state_t md5;
	md5_byte_t md5sum[17];
	int reset_gen_losses;
} h264dec_file_ctx_t;

////////////////////////////////////////////////////////////////////////// public API
struct h264dec_file_t;
typedef void h264dec_close_t(struct h264dec_file_t *unit);
typedef int  h264dec_set_file_t(struct h264dec_file_t *unit, char *fname);
typedef int  h264dec_decode_t(struct h264dec_file_t *unit);
typedef int  h264dec_ready_t(struct h264dec_file_t *unit);

typedef struct h264dec_file_t
{
	void *ctx;
	h264dec_close_t    *close;
	h264dec_set_file_t *set_file;
	h264dec_decode_t   *decode;
	h264dec_ready_t    *ready;
} h264dec_file_t;

/**
 * @param unit
 * @param settings
 * @param instance_num - ordered number of decoder instance;
 * @param reset - whether to reset decoder instance on every run;
 *
 */
int h264dec_file_init(h264dec_file_t *unit, v4d_settings_t *settings, h264dec_file_config_t *config);


////////////////////////////////////////////////////////////////////////// md5
//////////////////////////////////////////////////////////////////////////

static void print_hex_string(md5_byte_t *str, int length)
{
	int i;
	for(i = 0; i < length; i += 1)
	{
		printf("%01x", ((str[i]>>4) & 15));
		printf("%01x", (str[i] & 15));
	}
}

////////////////////////////////////////////////////////////////////////// realization
int h264dec_file_set_file(h264dec_file_t *unit, char *fname)
{
	int rc = 0;
	h264dec_file_ctx_t *ctx;
	ctx = (h264dec_file_ctx_t*)unit->ctx;

	assert(ctx->is_ready == 1);
	assert(ctx->f_264 == NULL);

	//printf("=> set_file(%d): [%s]\n", ctx->config.instance_num, fname);
	strncpy(ctx->fname, fname, sizeof(ctx->fname));

	// try to open H264 file
	ctx->f_264 = fopen(fname, "rb");
	if (!ctx->f_264)
	{
		return -1;
	}

	// try to open YUV file
	if (ctx->config.save_yuv_flag)
	{
		char yuv_name[1024];
		sprintf(yuv_name, "%s.yuv", fname);
		ctx->f_yuv = fopen(yuv_name, "wb");
		if (!ctx->f_yuv)
		{
			return -1;
		}
	}

	ctx->frame_num = 0;
	md5_init(&ctx->md5);
	memset(ctx->md5sum, 0, sizeof(ctx->md5sum));

	if (ctx->config.reset_flag)
	{
		v4d_close(ctx->handle);
		rc = v4d_open(&ctx->handle, &ctx->settings);
		if (rc != 0) 
			return rc;
	}
	
	ctx->reset_gen_losses = 1;

	// OK
	ctx->is_ready = 0;
	return 0;
}

typedef void (frame_handler_t)(void *ctx, byte *buf, int len);

void md5_frame_handler(void *ctx, byte *buf, int len)
{
	md5_state_t *md5 = (md5_state_t *)ctx;
	md5_append(md5, buf, len);
}

void yuv_frame_handler(void *ctx, byte *buf, int len)
{
	FILE *f_264 = (FILE *)ctx;
	fwrite(buf, len, 1, f_264);
}

static void process_frame(yuv_frame_t *dec_frame, sps_info_t *sps_info, void *ctx, frame_handler_t *func)
{
	int i, lo, lw, lh, ls;
	byte *bytes;
	const int sizeof_pel = dec_frame->bytes_per_pel;

	// Plane Y (luma) handling
	ls = dec_frame->stride_y;
	lo = sps_info->cropping_info.luma_offset;
	lw = sps_info->cropping_info.frame_width;
	lh = sps_info->cropping_info.frame_height;

	bytes = (byte*)dec_frame->y + sizeof_pel*lo;
	for (i = 0; i < lh; i++, bytes += sizeof_pel*ls)
	{
		//md5_append(md5, bytes, sizeof_pel*lw);
		func(ctx, bytes, sizeof_pel*lw);
	}

	// Planes U and V (chroma) handling
	if (sps_info->yuv_format!=YUV_400_IDC)
	{
		int co, cw = 0, ch = 0, cs;
		cs = dec_frame->stride_uv;
		co = sps_info->cropping_info.chroma_offset;
		switch(sps_info->yuv_format)
		{
		case YUV_420_IDC:
			cw = lw / 2;
			ch = lh / 2;
			break;
		case YUV_422_IDC:
			cw = lw / 2;
			ch = lh;
			break;
		case YUV_444_IDC:
			cw = lw;
			ch = lh;
			break;
		default:
			assert(0);
		}
		bytes = (byte*)dec_frame->u + sizeof_pel*co;
		for (i = 0; i < ch; i++, bytes += sizeof_pel*cs)
		{
			//md5_append(md5, bytes, sizeof_pel*cw);
			func(ctx, bytes, sizeof_pel*cw);
		}
		bytes = (byte*)dec_frame->v + sizeof_pel*co;
		for (i = 0; i < ch; i++, bytes += sizeof_pel*cs)
		{
			//md5_append(md5, bytes, sizeof_pel*cw);
			func(ctx, bytes, sizeof_pel*cw);
		}
	}
}

int h264dec_file_decode(h264dec_file_t *unit)
{
	int rc = 1;
	int read_bytes;
	int  eos, end_of_ms;
	media_sample_t *ms;
	yuv_frame_t *frame;
	sps_info_t sps;
	h264dec_file_ctx_t *ctx;
	ctx = (h264dec_file_ctx_t*)unit->ctx;

	if (!ctx->is_ready)
	{
		// read next portion from the file
		read_bytes = (int)fread(ctx->buf, 1, INIT_NAL_SIZE, ctx->f_264);
		if (read_bytes > 0)
		{
			v4d_nal_extractor_feed_data(ctx->nal_parser, ctx->buf, read_bytes, 0, 0);
			eos = 0;
		}
		else
		{
			// flush NAL parser
			v4d_nal_extractor_feed_data(ctx->nal_parser, NULL, 0, 0, 0);
			eos = 1;
		}
		// extract NAL units and send to decoder
		for (end_of_ms=0; !end_of_ms; )
		{
			ms = v4d_nal_extractor_get_nalu(ctx->nal_parser);
			if (ms) 
			{
				if (ctx->config.gen_losses > 0)
				{
					gen_nal_losses_main(ctx->config.gen_losses, ms, ctx->reset_gen_losses);
					ctx->reset_gen_losses = 0;
				}
				v4d_set_nal_unit(ctx->handle, ms);
			}
			else
			{
				end_of_ms = 1;
				if (eos)
				{
					v4d_set_nal_unit(ctx->handle, NULL);
				}
			}
			// decode all frames on every NAL
			while (VSSH_OK == v4d_get_frame(ctx->handle, &frame, &sps))
			{
				ctx->frame_num++;
				if (ctx->config.md5_flag)
				{
					process_frame(frame, &sps, &ctx->md5, md5_frame_handler);
				}
				if (ctx->config.save_yuv_flag)
				{
					process_frame(frame, &sps, ctx->f_yuv, yuv_frame_handler);
				}
				//printf("[%d][%s]: frame #%d %dx%d\n", ctx->config.instance_num, ctx->fname, ctx->frame_num, frame->image_width, frame->image_height);
			}
		}// for (end_of_ms)

		if (eos)
		{
			printf("%s", ctx->fname);
			if (ctx->config.md5_flag)
			{
				md5_finish(&ctx->md5, ctx->md5sum);
				printf(" ");
				print_hex_string(ctx->md5sum, 16);
			}
			printf("\n");
			if (ctx->f_264)
				fclose(ctx->f_264);
			ctx->f_264 = NULL;
			if (ctx->f_yuv)
				fclose(ctx->f_yuv);
			ctx->f_yuv = NULL;
			ctx->is_ready = 1;
			ctx->frame_num = 0;
		}
		rc = 0;
	}
	return rc;
}

void h264dec_file_close(h264dec_file_t *unit)
{
	h264dec_file_ctx_t *ctx;
	ctx = (h264dec_file_ctx_t*)unit->ctx;

	assert(ctx->is_ready);
	assert(ctx->f_264 == NULL);

	// close all the stuff
	if (ctx->nal_parser)
		v4d_nal_extractor_close(ctx->nal_parser);
	if (ctx->handle)
		v4d_close(ctx->handle);

	free(ctx);
}

int h264dec_file_ready(h264dec_file_t *unit)
{
//	int rc;
	h264dec_file_ctx_t *ctx;
	ctx = (h264dec_file_ctx_t*)unit->ctx;
	return ctx->is_ready;
}


int h264dec_file_init(h264dec_file_t *unit, v4d_settings_t *settings, h264dec_file_config_t *config)
{
	h264dec_file_ctx_t *ctx;
	int rc;
	ctx = calloc(1, sizeof(h264dec_file_ctx_t));
	memcpy(&ctx->config, config, sizeof(ctx->config));
	ctx->is_ready = 1;
	ctx->f_264 = NULL;
	ctx->reset_gen_losses = 1;
	ctx->nal_parser = v4d_nal_extractor_create(INIT_NAL_SIZE);
	memcpy(&ctx->settings, settings, sizeof(ctx->settings));
	unit->ctx = ctx;

	unit->set_file = h264dec_file_set_file;
	unit->decode = h264dec_file_decode;
	unit->close = h264dec_file_close;
	unit->ready = h264dec_file_ready;

	rc = v4d_open(&ctx->handle, &ctx->settings);
	if (rc != VSSH_OK)
	{
		return rc;
	}
	return VSSH_OK;
}

//////////////////////////////////////////////////////////////////////////

#endif	// __V4DEC_FILE_H__
