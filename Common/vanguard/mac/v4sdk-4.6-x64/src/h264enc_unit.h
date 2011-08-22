
/**
 * @file h264enc_unit.h
 * encoder instance for sample_all application
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4ENC_UNIT_H__
#define __V4ENC_UNIT_H__


typedef struct h264enc_ctx_t
{
	void *handle;
} h264enc_ctx_t;

////////////////////////////////////////////////////////////////////////// public API
struct h264enc_unit_t;
typedef void h264enc_close_t(struct h264enc_unit_t *unit);
typedef int  h264enc_get_settings_t(struct h264enc_unit_t *unit, v4e_settings_t *actual);
typedef int  h264enc_set_frame_t(struct h264enc_unit_t *unit, raw_frame_t *frame);
typedef int  h264enc_get_nal_t(struct h264enc_unit_t *unit, media_sample_t **pms);
typedef int  h264enc_get_nal_ex_t(struct h264enc_unit_t *unit, media_sample_t **pms, int blocking);


typedef struct h264enc_unit_t
{
	void *ctx;
	h264enc_close_t *close;
	h264enc_get_settings_t *get_settings;
	h264enc_set_frame_t *set_frame;
	h264enc_get_nal_t *get_nal;
	h264enc_get_nal_ex_t *get_nal_ex;
} h264enc_unit_t;

int h264enc_init(h264enc_unit_t *unit, v4e_settings_t *settings);

////////////////////////////////////////////////////////////////////////// realization
int h264enc_get_settings(h264enc_unit_t *unit, v4e_settings_t *actual)
{
	h264enc_ctx_t *ctx;
	assert(unit);
	ctx = (h264enc_ctx_t*)unit->ctx;
	assert(ctx);
	return v4e_get_current_settings(ctx->handle, actual);
}

int h264enc_set_frame(h264enc_unit_t *unit, raw_frame_t *frame)
{
	int rc;
	h264enc_ctx_t *ctx;
	assert(unit);
	ctx = (h264enc_ctx_t*)unit->ctx;
	assert(ctx);
	if (frame)
	{
		rc = v4e_set_frame(ctx->handle, frame);
	}
	else
	{
		rc = v4e_set_flush(ctx->handle);
	}
	return rc;
}

int h264enc_get_nal(h264enc_unit_t *unit, media_sample_t **pms)
{
	h264enc_ctx_t *ctx;
	assert(unit);
	ctx = (h264enc_ctx_t*)unit->ctx;
	assert(ctx);
	return v4e_get_nal(ctx->handle, pms);
}

int h264enc_get_nal_ex(h264enc_unit_t *unit, media_sample_t **pms, int blocking)
{
	h264enc_ctx_t *ctx;
	assert(unit);
	ctx = (h264enc_ctx_t*)unit->ctx;
	assert(ctx);
	return v4e_get_nal_ex(ctx->handle, pms, blocking);
}

void h264enc_close(h264enc_unit_t *unit)
{
	h264enc_ctx_t *ctx;
	assert(unit);
	ctx = (h264enc_ctx_t*)unit->ctx;
	assert(ctx);
	if (ctx->handle)
	{
		v4e_close(ctx->handle);
	}
	free(ctx);
}

int h264enc_init(h264enc_unit_t *unit, v4e_settings_t *settings)
{
	h264enc_ctx_t *ctx;
	int rc;
	ctx = calloc(1, sizeof(h264enc_ctx_t));
	unit->ctx = ctx;
	unit->get_settings = h264enc_get_settings;
	unit->set_frame = h264enc_set_frame;
	unit->get_nal = h264enc_get_nal;
	unit->get_nal_ex = h264enc_get_nal_ex;
	unit->close = h264enc_close;

	rc = v4e_open_ex(&ctx->handle, settings, NULL, NULL);
	if (rc != VSSH_OK)
	{
		printf("Error: v4e_open_ex() failed (%d): %s\n", rc, v4_error_text(rc));
		return rc;
	}
	return VSSH_OK;
}

//////////////////////////////////////////////////////////////////////////

#endif	// __V4ENC_UNIT_H__
