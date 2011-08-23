
/**
 * @file h264dec_unit.h
 * decoder instance for sample_all application
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4DEC_UNIT_H__
#define __V4DEC_UNIT_H__


typedef struct h264dec_ctx_t
{
	void *handle;
	sps_info_t sps_info;
	yuv_frame_t *dec_frame;
} h264dec_ctx_t;

////////////////////////////////////////////////////////////////////////// public API
struct h264dec_unit_t;
typedef void h264dec_close_t(struct h264dec_unit_t *unit);
typedef int  h264dec_set_nal_t(struct h264dec_unit_t *unit, media_sample_t *ms);
typedef int  h264dec_decode_t(struct h264dec_unit_t *unit, yuv_frame_t **pframe, sps_info_t *sps);

typedef struct h264dec_unit_t
{
	void *ctx;
	h264dec_close_t   *close;
	h264dec_set_nal_t *set_nal;
	h264dec_decode_t  *decode;
} h264dec_unit_t;

int h264dec_init(h264dec_unit_t *unit, v4d_settings_t *settings);

////////////////////////////////////////////////////////////////////////// realization
int h264dec_set_nal(h264dec_unit_t *unit, media_sample_t *ms)
{
	int rc;
	h264dec_ctx_t *ctx;
	int end_of_frame;
	assert(unit);
	ctx = (h264dec_ctx_t*)unit->ctx;
	assert(ctx);

	rc = VSSH_OK;
	if (ms)
	{
		end_of_frame = (ms->is_last_in_pict >= LAST_IN_FRAME);
		rc = v4d_set_nal_unit(ctx->handle, ms);
		if (rc != VSSH_OK)
		{
			// release media sample in case of error
			v4_free_media_sample(ms);
			return rc;
		}
		if (end_of_frame)
		{
			v4d_set_end_of_access_unit(ctx->handle);
		}
	}
	else
	{
		rc = v4d_set_nal_unit(ctx->handle, NULL);
	}
	return rc;
}

int h264dec_decode(h264dec_unit_t *unit, yuv_frame_t **pframe, sps_info_t *sps)
{
	int rc;
	h264dec_ctx_t *ctx;
	ctx = (h264dec_ctx_t*)unit->ctx;
	rc = v4d_get_frame(ctx->handle, pframe, sps);
	return rc;
}

void h264dec_close(h264dec_unit_t *unit)
{
	h264dec_ctx_t *ctx;
	ctx = (h264dec_ctx_t*)unit->ctx;
	if (ctx->handle)
	{
		v4d_close(ctx->handle);
	}
	free(ctx);
}

int h264dec_init(h264dec_unit_t *unit, v4d_settings_t *settings)
{
	h264dec_ctx_t *ctx;
	int rc;
	ctx = calloc(1, sizeof(h264dec_ctx_t));
	unit->ctx = ctx;
	unit->set_nal = h264dec_set_nal;
	unit->decode = h264dec_decode;
	unit->close = h264dec_close;

	rc = v4d_open(&ctx->handle, settings);
	if (rc != VSSH_OK)
	{
		printf("Error: v4d_open() failed (%d): %s\n", rc, v4_error_text(rc));
		return rc;
	}
	return VSSH_OK;
}

//////////////////////////////////////////////////////////////////////////

#endif	// __V4DEC_UNIT_H__
