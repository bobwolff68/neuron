
/**
 * @file v4d_api.h
 * VSofts H.264/AVC Encoder public API
 *
 * Project:	VSofts H.264 Codec V4
 * Module: Decoder
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4D_API_H__
#define __V4D_API_H__

#include "v4_types.h"
#include "v4d_settings.h"
#include "v4_media_sample.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
//////////////////////////////////////////////////////////////////////////////////////////
//	D E C O D E R
//////////////////////////////////////////////////////////////////////////////////////////
*/

/** @defgroup dec_func Decoder Functions
*
* @{
*/

/**
 * Create a new decoder instance.
 * @param phandle  - pointer to a variable which will receive decoder handle
 *		in case of success, can not be NULL;
 * @param settings - pointer to the v4d_settings_t structure
 *	containing initial decoder parameters to be used during this session.
 *	Note: can not be NULL.
 * @return VSSH_OK=success,
 *	VSSH_ERR_ARG=invalid argument (NULL),
 *	VSSH_ERR_SETTINGS=incorrect settings,
 *	VSSH_ERR_MEMORY=out of memory;
 */
int VSSHAPI v4d_open(void **phandle, v4d_settings_t *settings);

/**
 * Check, if decoder can accept new nal unit.
 * @param handle - decoder instance (returned by v4d_open)
 * @return VSSH_OK In this case decoder can accept new nal unit
 * Otherwice - it can't
 */
int VSSHAPI v4d_can_accept_nal_unit(void *handle);

/**
 * Pass next nal unit into decoder.
 * @param handle - decoder instance (returned by v4d_open)
 * @param nal_unit - input NAL unit in RBSP format (no start code at the beginning, no start codes inside),
 *		pass NULL to signal end of stream;
 * @return VSSH_OK=success, In this case decoder will take care to release it when all is done;
 * In case of error the caller must release "nal_unit" by itself;
 */
int VSSHAPI v4d_set_nal_unit(void *handle, media_sample_t *nal_unit);

/**
 * Informs decoder, that all slices of current access unit are passed
 * @param handle - decoder instance (returned by v4d_open)
 * @return VSSH_OK=success, or error code
 */
int VSSHAPI v4d_set_end_of_access_unit(void *handle);


/**
 * Decode SPS (sequence parameter set) NAL unit;
 * @param handle - decoder instance (returned by v4d_open)
 * @param sps_bytes pointer to SPS bytes (RBSP format);
 * @param num_sps_bytes number of SPS bytes;
 * @param sps_info - structure to receive decoded SPS information;
 * @return error code;
 */
int VSSHAPI v4d_decode_sps(void *handle, byte *sps_bytes, int num_sps_bytes, sps_info_t *sps_info);

/**
 * Decode PPS (picture parameter set) NAL unit;
 * @param handle Handle of the decoder instance (returned by v4d_open);
 * @param pps_bytes pointer to PPS bytes (RBSP format);
 * @param num_pps_bytes number of PPS bytes;
 * @param pps_info structure to receive decoded PPS information;
 * @return error code;
 */
int VSSHAPI v4d_decode_pps(void *handle, byte *pps_bytes, int num_pps_bytes, pps_info_t *pps_info);


/**
 * Return the pointer to media sample with encapsulated decoded structure of last NAL-unit,
 * set by v4d_set_nal_unit() function. Depending on NAL-type it can be:
 * pic_parameter_set_t
 * slice_params_t 
 * seq_parameter_set_t
 * NAL type is placed into extra_data[3] byte of returned media_sample_t structure 
 * For SEI NAL type it can be the list of several media samples, returned in the same manner,
 * and with same properties, as attached to decoded frames (see sei_list field of frame_info_t
 * structure and DEC_DECODE_SEI flag)
 * This pointer is valid until next call to v4d_set_nal_unit() function
 * Application shall not free or modify data in this media_sample
 *
 * @param handle Handle of the decoder instance (returned by v4d_open);
 * @return media sample or NULL if errors or not supported NAL
 */
media_sample_t *VSSHAPI v4d_get_decoded_nal_unit(void *handle);

/**
 * Pass next nal unit into decoder, decode it's header and return pointer to encapsulated decoded structure
 * See v4d_get_decoded_nal_unit() for more details about it
 * This function is combination of v4d_set_nal_unit() and v4d_get_decoded_nal_unit() calls
 * @param handle - decoder instance (returned by v4d_open)
 * @param nal_unit - input NAL unit in RBSP format (no start code at the beginning, no start codes inside),
 * @param pdecoded_ms - pointer where to put result
 * @return VSSH_OK=success, In this case decoder will take care to release nal_unit when all is done;
 * In case of error the caller must release "nal_unit" by itself;
 */
int VSSHAPI v4d_set_nal_unit_and_decode(void *handle, media_sample_t *nal_unit, media_sample_t **pdecoded_ms);

/**
 * Film grain filter implementation. 
 * Adds noise to given YUV frame according to film_grain SEI information (if any).
 * Note that DEC_DECODE_SEI flag should be enabled in decoder settings.
 * @param handle  - decoder handle;
 * @param dec_frame - pointer to decoded frame as returned by "v4d_get_frame()";
 * @return pointer to modified frame or NULL if errors
 */
yuv_frame_t * VSSHAPI v4d_film_grain(void *handle, yuv_frame_t *dec_frame);

/**
 * Post filter implementation. 
 * Improve quality of the given YUV frame with Wiener optimal filter.
 * Note that DEC_DECODE_SEI flag should be enabled in decoder settings.
 * @param handle  - decoder handle;
 * @param dec_frame - pointer to decoded frame as returned by "v4d_get_frame()";
 * @return pointer to modified frame or NULL if errors
 */
yuv_frame_t * VSSHAPI v4d_post_filter(void *handle, yuv_frame_t *dec_frame);

/**
 * Try to take next decoded frame.
 * @param handle  - decoder handle
 * @param p_yuv_frame - where to put pointer to output frame
 * @param sps_info - where to put sps info (can be NULL if not needed)
 * @return VSSH_OK=frame is ready; VSSH_WARN_NOT_READY=frame not ready; VSSH_WARN_EOS=end of stream (no more frames expected)
 */
int VSSHAPI v4d_get_frame(void *handle, yuv_frame_t **p_yuv_frame, sps_info_t *sps_info);

/**
 * Get frame macroblocks data. This info can be used by encoder to make smart transcoding 
 * @param handle  - decoder handle
 * @param yuv_frame - yuv frame
 * @param pmbs_data - where to put pointer to media sample with macroblock data.
 * Caller must release this media sample when done with it
 * @return VSSH_OK or error code
 */
int VSSHAPI v4d_get_frame_macroblocks_data(void *handle, yuv_frame_t *yuv_frame, media_sample_t **pmbs_data);


/**
 * Try to take next view of decoded frame
 * @param handle - decoder handle
 * @param p_yuv_frame_next_view - where to put pointer to next output frame view
 * @param yuv_frame_view - pointer to current output frame view
 * @param sps_info - where to put sps info (can be NULL if not needed)
 * @return VSSH_OK=frame view is ready; VSSH_WARN_NOT_READY=frame view not exist
 */
int VSSHAPI v4d_get_next_frame_view(void *handle, yuv_frame_t **p_yuv_frame_next_view, yuv_frame_t *yuv_frame_view, sps_info_t *sps_info);


/**
 * Change output delay on-the-fly
 * @param handle - decoder instance (returned by v4d_open)
 * @param output_delay - new value of output delay; -1 - means "calulate" from active sps
 * @return VSSH_OK = success, or error code
 */
int VSSHAPI v4d_set_output_delay(void *handle, int output_delay);



/**
 * Close decoder, release memory etc.
 * @param handle - handle of the decoder instance (returned by v4d_open());
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4d_close(void *handle);

/*
//////////////////////////////////////////////////////////////////////////////////////////
//	N A L   E X T R A C T O R
//////////////////////////////////////////////////////////////////////////////////////////
*/

/**
* Init input stream buffer
* @param initial_size - data buffer initial size;
* @param remove_startcode_emulation - whether to remove 0x3 startcode preventing bytes;
* @return pointer to a structure created;
*/
void * VSSHAPI v4d_nal_extractor_create_ex(int initial_size, int remove_startcode_emulation);

/**
 * Init input stream buffer
 * @param initial_size - data buffer initial size;
 * @return pointer to a structure created or NULL in case of out of memory;
 */
void * VSSHAPI v4d_nal_extractor_create(int initial_size);

/**
 * Release stream buffer
 * @param extractor - nal unit extractor instance;
 * @return error code;
 */
int VSSHAPI v4d_nal_extractor_close(void *extractor);

/**
 * Put RBSP bytes into stream buffer.
 * @param extractor - nal unit extractor instance;
 * @param data - pointer to input bytes;
 * @param size - number of bytes; zero means the flush;
 * @param is_nal_unit - indicate whole nal unit or raw bytes:
 *		0 = raw bytes on input, the function will parse startcodes and do the rest;
 *		1 = the function put bytes of one entire NAL unit into stream buffer;
 * the data must not include startcodes. In this case this function will flush
 * the previously accumulated data and copy all bytes into new internal nal units;
 * @param timestamp Timestamp of the data (or NO_TIME_STAMP value if no timestmap)
 * @return error code
 */
int VSSHAPI v4d_nal_extractor_feed_data(void *extractor, byte *data, int size, int is_nal_unit, int64_t timestamp);

/**
 * Flush the extractor by forcing all incomplete NAL units go out on next call
 * to get_nalu()
 * @param extractor - nal unit extractor instance;
 * @return VSSH_OK
 */
int VSSHAPI v4d_nal_extractor_flush(void *extractor);

/**
 * Extract next NAL unit from the buffer (or NULL if nothing available)
 * This nal unit must be either released by caller via v4_free_media_sample() function
 * or be put into decoder, decoder will free it automatically.
 * @param extractor - nal unit extractor instance;
 * @return NAL unit or NULL if nothing ready;
 */
media_sample_t *VSSHAPI v4d_nal_extractor_get_nalu(void *extractor);


/** @} */


#ifdef __cplusplus
}
#endif

#endif // __V4D_API_H__
