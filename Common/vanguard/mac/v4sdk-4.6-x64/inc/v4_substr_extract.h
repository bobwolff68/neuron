
/**
 * @file v4_substr_extract.h
 * Public SVC/MVC Sub-stream Extractor API
 *
 * For details see:
 *  ITU-T Recommendation H.264 (03/2009 or later)
 *   G.8.8 Specification of bitstream subsets (SVC)
 *   H.8.5 Specification of bitstream subsets (MVC)
 *
 * Project:	VSofts H.264 Codec V4
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

/**
* Create Sub-stream Extractor instance
*
* @param p_id - target priority id, [-1, 0 .. 63],  -1 means don't care;
* @param t_id - target temporal id, [-1, 0 .. 7],   -1 means don't care;
* @param dq_id- target combined dependency/quality id, dq_id = (d_id << 4) + q_id
*									[-1, 0 .. 127], -1 means don't care;
* @param mode - extraction mode, [0, 1]
*		0: Extract by specified p_id, t_id and dq_id;
*		1: Extract first n layers/views, where n = dq_id (not implemented);
*
* @return pointer to a structure created or NULL in case of out of memory;
*/
void * VSSHAPI v4d_substream_extractor_create(int p_id, int t_id, int dq_id, int mode);

/**
* Release Sub-stream Extractor instance
* @param handle - Sub-stream Extractor context pointer;
* @return error code;
*/
int VSSHAPI v4d_substream_extractor_close(void *handle);

/**
 * Put the next NAL unit to the internal buffer. Pass NULL to indicate end of stream.
 * @param handle - Sub-stream Extractor context pointer;
 * @param ms - input NAL unit (or NULL);
 * @return 0=sample accepted, 1=sample cannot be loaded;
 */
int VSSHAPI v4d_substream_extractor_feed_nalu(void *handle, media_sample_t *ms);

/**
 * Get the next NAL unit from the buffer
 * @param handle - Sub-stream Extractor context pointer;
 * @return NAL unit or NULL if no NAL is available;
 */
media_sample_t * VSSHAPI v4d_substream_extractor_get_nalu(void *handle);

/**
* Set desired output bitrate of the SVC stream
*   On-the-fly bitrate change is supported
* @param handle       - Sub-stream Extractor context pointer;
* @param kbps         - Desired bitrate, kbps [0 .. 200,000], should be between base and highest layer bitrates, 0=bypass;
* @param vbv_delay    - VBV size/delay, ms [100 .. 8000], vbv_delay < 100 means auto setting;
* @param vbv_fullness - VBV fullness, ms  [100 .. 8000], vbv_fullness <= 0 means auto setting;
* @param framerate - input frame rate (frames per 10,000 sec), [0 ..180*10000], 0 - use stream settings;
*/
int VSSHAPI v4d_substream_extractor_set_svc_bitrate(void *handle, int kbps, int vbv_delay, int vbv_fullness, int framerate);

/**
* Get current VBV parameters of the SVC stream
* @param handle     - Sub-stream Extractor context pointer;
* @param *dq_id     - max DQId of current access unit at output;
* @param *kbps      - momentary bitrate, kbps;
* @param *vbv_size  - current VBV size, ms;
* @param *framerate - current frame rate (frames per 10,000 sec);
* @return:
*			0 - NULL handle at input
*			> 0 - vbv size, bits
*/
int VSSHAPI v4d_substream_extractor_get_vbv_buffer(void *handle, int *dq_id, int *kbps, int *vbv_size, int *framerate);
