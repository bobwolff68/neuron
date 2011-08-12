
/**
 * @file v4_media_sample.h
 * Media Sample public API
 *
 * Project:	VSofts H.264 Codec V4
 * Module:	Decoder and Encoder
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4_MEDIA_SAMPLE_H__
#define __V4_MEDIA_SAMPLE_H__

#include "v4_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Allocate a new media sample filling structure fields with information from frame info;
* @param data_size - size of data buffer to allocate;
* @param info - frame info to use (can be NULL);
* @return new media sample or NULL;
*/
media_sample_t * VSSHAPI v4_alloc_media_sample(int data_size, frame_info_t *info);

/**
* Allocate a new empty media sample;
* @param data_size - size of data buffer to allocate;
* @return new media sample or NULL;
*/
media_sample_t * VSSHAPI v4_alloc_empty_media_sample(int data_size);

/**
* Re-allocate given media sample to a new size;
* @param new_size - size of data buffer to allocate;
* @param ms - media sample to reallocate;
* @return new media sample or NULL;
*/
media_sample_t * VSSHAPI v4_realloc_media_sample(int new_size, media_sample_t *ms);

/**
* Free previously allocated media sample;
* @param ms - media sample to reallocate;
* @return VSSH_OK;
*/
void VSSHAPI v4_free_media_sample(media_sample_t *ms);

/// typedef for external media sample allocator
typedef media_sample_t * VSSHAPI media_sample_alloc_t(int data_size, void *allocator_data);
typedef void VSSHAPI media_sample_free_t(media_sample_t *ms, void *allocator_data);

#ifdef __cplusplus
}
#endif

#endif
