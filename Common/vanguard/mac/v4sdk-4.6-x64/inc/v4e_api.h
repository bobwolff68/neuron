
/**
 * @file v4e_api.h
 * Encoder public API
 *
 * Project:	VSofts H.264 Codec V4
 * Module: Encoder
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4E_API_H__
#define __V4E_API_H__

#include "v4_types.h"
#include "v4e_settings.h"
#include "v4_media_sample.h"
#include "v4_nalu.h"
#include "vp.h"

#ifdef __cplusplus
extern "C" {
#endif

/// number of raw frames on encoder input
#define NUM_RAW_FRAMES (9)
#define NUM_RGB_FRAMES NUM_RAW_FRAMES	///< for backward compatibility

/**
* Application-provided callback functions to release raw_frame or vp_raw_frame
* to be called asynchronously when raw frame is not used any more.
* @param context - application-provided context pointer;
* @param frame - pointer to raw_frame_t or vp_raw_frame_t depending on what input was used
* it is caller responisibility to supply proper release callback
* @return return value is ignored;
*/
typedef int release_raw_frame_callback_t(void *context, void *frame);

/**
* Application-provided callback function to receive ready NAL units immediately.
* Application is responsible to release media sample, immediately or later,
*	using "v4_free_media_sample()".
* Note, that in multi-threading mode this function should not perform any
* time-consuming actions. It is recommended to store NAL unit for further processing
* and return control as soon as possible.
* @param context - application-provided context pointer;
* @param ms - next media sample, NULL means "end of stream" situation;
* @return return value is ignored;
*/
typedef int receive_nal_callback_t(void *context, media_sample_t *ms);

/**
* Define callback functions in conjunction with appropriate context
*/
typedef struct v4_frame_release_t
{
	void *context;	///< application context for frame release callback;
	release_raw_frame_callback_t *callback;	///< frame release callback function;
} v4_frame_release_t;

typedef struct v4_receive_nal_t
{
	void *context;	///< application context for receive nal callback;
	receive_nal_callback_t *callback; ///< receive nal callback function;
} v4_receive_nal_t;

/// structure holding external image and it's characteristics
typedef struct vp_raw_frame_t
{
	int64_t				 timestamp;	///< external timestamp
	//byte				 *image;			///< pointer to image buffer;
	frame_modifier_t	 *modifier; ///< external modifier
	media_sample_t		 *sei_list; ///< Initially must be set to NULL. Used by v4e_attach_sei() function;
	//int32_t				 stride;		///< width of the buffer in bytes (typically aligned by 4 bytes);
	vp_frame_t           vp_frame;
	int32_t				 mediatime;	///< external media time
	int32_t				 reserved; ///< reserved parameter; must be zero
	//Both				 lists are used in field coding one list for top and bottom field
} vp_raw_frame_t;


/// Obsolete structure. provided for backward compatibility
typedef struct raw_frame_t
{
	int64_t				 timestamp;	///< external timestamp
	byte				 *image;			///< pointer to image buffer;
	frame_modifier_t	 *modifier; ///< external modifier
	media_sample_t		 *sei_list; ///< Initially must be set to NULL. Used by v4e_attach_sei() function;
	int32_t				 stride;		///< width of the buffer in bytes (typically aligned by 4 bytes);
	int32_t				 mediatime;	///< external media time
	int32_t				 reserved; ///< reserved parameter; must be zero
} raw_frame_t;



/** @defgroup enc_func Encoder Functions
*
* @{
*/

/**
 * Create a new encoder instance.
 * @param phandle - pointer to a variable which will receive encoder handle
 *		in case of success, can not be NULL;
 * @param settings - pointer to the v4e_settings_t structure
 *	containing initial encoder parameters to be used during this encoding session,
 *	can not be NULL.
 * @note framerate should be assigned via "gop_settings" member;
 * @return VSSH_OK = success,
 *	VSSH_ERR_ARG = invalid arg (NULL),
 *	VSSH_ERR_SETTINGS = incorrect settings,
 *	VSSH_ERR_MEMORY = out of memory;
 *	VSSH_WARN_LEVEL_LIMITS = settings do not fit into profile/level limitations;
 */
int VSSHAPI v4e_open(void **phandle, v4e_settings_t *settings);

/**
 * Extended version of v4e_open()
 * Create a new encoder instance.
 * @param phandle - pointer to a variable which will receive encoder handle
 *		in case of success, can not be NULL;
 * @param settings - pointer to the v4e_settings_t structure
 *	containing initial encoder parameters to be used during this encoding session,
 *	can not be NULL.
 * @param frame_release - describes release callback function, can be NULL;
 * @param receive_nal - describes receive nal callback function, can be NULL;
 * @return VSSH_OK = success,
 *	VSSH_ERR_ARG = invalid arg (NULL),
 *	VSSH_ERR_SETTINGS = incorrect settings,
 *	VSSH_ERR_MEMORY = out of memory;
 *	VSSH_WARN_LEVEL_LIMITS = settings do not fit into profile/level limitations;
 * @note framerate should be assigned via "gop_settings" member;
 */
int VSSHAPI v4e_open_ex(void **phandle, v4e_settings_t *settings,
	v4_frame_release_t *frame_release, v4_receive_nal_t *receive_nal);

/**
 * The function sends given frame to encoder and immediately returns.
 * If blocking is not allowed and there are no more free input slots,
 * this function returns appropriate error code, otherwise it will wait
 * for a next free encoder input slot.
 * Caller must provide “frame release callback” function in "v4e_open_ex()" call
 * to receive “release” events signaling that particular frame is no more used
 * and thus could be reused or freed by application.
 * It is the caller responsibility to guarantee that frames do not be reused before release event.
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param vp_raw_frame - uncompressed frame data, NULL is not allowed;
 * @param blocking - enable blocking [0/1];
 * @return VSSH_OK = succeeded;
 *	VSSH_WARN_NOT_READY = no free data slot available (if blocking disabled);
 */
int VSSHAPI v4e_set_vp_frame(void *handle, vp_raw_frame_t *vp_raw_frame, int blocking);

/**
 * The function sends given frame and its macrobock data to encoder.
 * Intended to be used in transcoding applications.
 * Macroblock data must me received in decoder by v4d_get_frame_macroblocks_data() function
 * Encoder will release this sample, when done with it
 * Other parameters are used as in v4e_set_vp_frame() function	
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param vp_raw_frame - uncompressed frame data, NULL is not allowed;
 * @param mbs_data - media sample with macroblocks data
 * @param blocking - enable blocking [0/1];
 * @return VSSH_OK = succeeded; otherwise error code;
 */
int VSSHAPI v4e_set_vp_frame_and_mbs_data(void *handle, vp_raw_frame_t *vp_raw_frame, media_sample_t *mbs_data, int blocking);

/**
 * Obsolete functions. Better to use v4e_set_vp_frame() instead
 * Provided for backward compatibility
 */
int VSSHAPI v4e_set_frame(void *handle, raw_frame_t *raw_frame);
int VSSHAPI v4e_set_frame_ex(void *handle, raw_frame_t *raw_frame, int blocking);

/**
 * This function signals encoder to process all the pending frames and return
 * all NAL units and EOS flag at the very end.
 * Anyway the encoder is still running and could be used again without
 * stop / start operation.
 * @note EOS state will be reset after next v4e_set_frame() call.
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4e_set_flush(void *handle);

/**
 * Retrieve next available encoded nal unit (synchronous, non-blocking).
 * This function is used only if receive nal callback function is not set
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param pms - pointer to a media sample with ready NAL unit;
 * @param blocking - flag to wait for NAL if it should be ready (0 - don't wait)
 * @return
 *	VSSH_OK = nal unit available;
 *	VSSH_WARN_NOT_READY = nal is not ready, but will be ready if wait
 *  VSSH_ERR_MORE_DATA = nal is not ready and will not be ready if wait (more frames needed)
 *	VSSH_WARN_EOS = end of stream;
 * @note caller must free media sample after use, @see "media_sample.h::v4_free_media_sample()"
 */
int VSSHAPI v4e_get_nal_ex(void *handle, media_sample_t **pms, int blocking);

/**
 * The same as v4e_get_nal_ex(handle, pms, 0)
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param pms - pointer to a media sample with ready NAL unit;
 * @return
 *	VSSH_OK = nal unit available;
 *	VSSH_WARN_NOT_READY = nal is not ready, but will be ready if wait
 *  VSSH_ERR_MORE_DATA = nal is not ready and will not be ready if wait (more frames needed)
 *	VSSH_WARN_EOS = end of stream;
 */
int VSSHAPI v4e_get_nal(void *handle, media_sample_t **pms);

/**
 * Retrieve results of first pass of dual-pass rate-control.
 * To be called after first pass encoding session, before call to "v4e_close()".
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param pms - allocated media sample with first pass log data;
 * @return
 *	VSSH_OK = dual-pass log is available;
 *	VSSH_WARN_NOT_READY = data not ready;
 * @note caller must free media sample after use, @see "media_sample.h::v4_free_media_sample()"
 */
int VSSHAPI v4e_get_dual_pass_log(void *handle, media_sample_t **pms);

/**
 * Provide encoder with first pass results of dual-pass rate-control.
 * To be called before second pass encoding session.
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param pms - allocated media sample with first pass log data;
 * @return
 *	VSSH_OK = all OK;
 *	VSSH_WARN_NOT_READY = wrong data;
 * @note
 *	- use library function for media sample allocation, @see "media_sample.h::v4_alloc_media_sample()";
 *	- caller shall properly assign "pms->used_size";
 *	- put first pass data into "pms->data" buffer;
 */
int VSSHAPI v4e_set_dual_pass_log(void *handle, media_sample_t *pms);

/**
 * This function will impose flush event if not yet flushed to release
 * output thread of application if it is waiting for NAL unit.
 * After that it will stop all the worker threads and free memory and system resources.
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @return VSSH_OK;
 */
int VSSHAPI v4e_close(void *handle);

/**
 * Retrieve encoded Sequence Parameter Set (SPS) NAL unit.
 * @param handle - handle of the encoder instance (returned by vssh_enc_open);
 * @param sps_data - structure to be filled in with SPS information;
 * @note "sps_data.data" pointer will be assigned to point to actual SPS bits;
 * @note "sps_data.size" will be assigned to a size of SPS in bytes;
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4_enc_get_sps(void *handle, v4_spps_data_t *sps_data);

/**
 * Retrieve encoded Picture Parameter Set (PPS) NAL unit.
 * @param handle - handle of the encoder instance (returned by vssh_enc_open);
 * @param pps_data - structure to be filled in with PPS bits;
 * @note "sps_data.data" pointer will be assigned to point to actual PPS bits;
 * @note "sps_data.size" will be assigned to a size of PPS in bytes;
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4_enc_get_pps(void *handle, v4_spps_data_t *pps_data);



/**
 * Retrieve encoded Sequence Parameter Set (SPS) NAL unit for SVC layer.
 * @param handle - handle of the encoder instance (returned by vssh_enc_open);
 * @param sps_data - structure to be filled in with SPS information;
 * @param svc_layer_id - id of svc layer
 * @note "sps_data.data" pointer will be assigned to point to actual SPS bits;
 * @note "sps_data.size" will be assigned to a size of SPS in bytes;
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4_enc_get_svc_subsps(void *handle, v4_spps_data_t *sps_data, int svc_layer_id);



/**
 * Retrieve encoded Picture Parameter Set (PPS) NAL unit.
 * @param handle - handle of the encoder instance (returned by vssh_enc_open);
 * @param pps_data - structure to be filled in with PPS bits;
 * @param svc_layer_id - id of svc layer
 * @note "sps_data.data" pointer will be assigned to point to actual PPS bits;
 * @note "sps_data.size" will be assigned to a size of PPS in bytes;
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4_enc_get_svc_pps(void *handle, v4_spps_data_t *pps_data,  int svc_layer_id);



/**
 * Fill given settings structure with actual encoder settings;
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param settings - structure to fill
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4e_get_current_settings(void *handle, v4e_settings_t *settings);


/**
 * Change current bitrate value on-the-fly;
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param bitrate_kbps - new bitrate value;
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4e_change_bitrate(void *handle, int bitrate_kbps);


/**
 * Change current bitrate and frame rate values on-the-fly;
 * @note parameters can be zero. This mean that the function will not change this parameter;
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param bitrate_kbps - new bitrate value;
 * @param time_scale - new time_scale value; frame_rate = time_scale/(2*num_units_in_tick);
 * @param num_units_in_tick - new num_units_in_tick value; See standard VUI parameters description for details
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4e_change_bitrate_and_framerate(void *handle, int bitrate_kbps, int time_scale, int num_units_in_tick);

/**
 * Change bitrate and frame rate for paricular svc layer values on-the-fly;
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param svc_layer_id - layer id to work with, as listed in svc_settings;
 * @param bitrate_kbps - new bitrate value;
 * @param time_scale - new time_scale value; frame_rate = time_scale/(2*num_units_in_tick);
 * @param num_units_in_tick - new num_units_in_tick value; See standard VUI parameters description for details
 * @return VSSH_OK or error code;
 * @note: parameters bitrate_kbps, time_scale, and num_units_in_tick can be zero.
 * This mean that the function will not change this parameter;
 */
int VSSHAPI v4e_change_svc_bitrate_and_framerate(void *handle, int svc_layer_id, int bitrate_kbps, int time_scale, int num_units_in_tick);

int VSSHAPI v4e_attach_sei(raw_frame_t *raw_frame, int sei_id, void *sei_data, int sei_size, unsigned int sei_coding_flags);

/**
* Attach custom SEI data to sei list of the input frame
* before calling to "v4e_set_vp_frame()";
* @param sei_list pointer to SEI list. Usially the member vp_raw_frame_t structure
* @param sei_id - SEI identifier, @see "vssh_types.h"::sei_type_e;
* @param sei_data - pointer to appropriate data structure, could be freed just after the call;
* @param sei_size - size of data structure, bytes;
* @param sei_coding_flags - bitwise combination of sei coding flags,  @see "vssh_types.h"::sei_coding_flags_e
* @param stream_id - stream number in ParallelStream mode to attach SEI message to, should not exceeds number of parallel encoding streams
* @return VSSH_OK or error code;
*/
int VSSHAPI v4e_attach_sei_to_list(media_sample_t **sei_list, int sei_id, void *sei_data, int sei_size, unsigned int sei_coding_flags, int stream_id);


/**
 * Obsolete function. Better to use v4e_attach_sei_to_list() instead
 * Provided for backward compatibility
 */
int VSSHAPI v4e_attach_sei(raw_frame_t *raw_frame, int sei_id, void *sei_data, int sei_size, unsigned int sei_coding_flags);
int VSSHAPI v4e_attach_sei_ex(raw_frame_t *raw_frame, int sei_id, void *sei_data, int sei_size, unsigned int sei_coding_flags,  int stream_id);

/**
 * Set custom scaling matrix for specified matrix type
 * Valid only if frext.scaling_matrix parameter is enabled in encoder settings
 * Must be called before any call to v4e_set_frame(),v4e_get_sps() and v4e_get_pps() functions
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param matrix_type - scaling matix type @see "vssh_types.h"::scaling_matrix_type_e;
 * @param scaling_matrix - Coefficients of the custom scaling matrix in progressive scan order.
 * There must be 16 values for types 0-5 and 64 values for types 6-7
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4e_set_scaling_matrix(void *handle, int matrix_type, byte *scaling_matrix);

/**
 * Set vui parameters
 * Note, that only parameters which don’t contradict with encoder settings are actually set.
 * Such parameters as ‘time_scale’, ‘num_units_in_tick’, hrd-buffer related parameters are
 * calculated from encoder settings and remain unchanged.
 * @param handle - handle of the encoder instance (returned by v4e_open());
 * @param vui - vui parameters;
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4e_set_vui_parameters(void *handle, vssh_vui_seq_parameters_t *vui);

/**
 * Resets on-the-fly the encoder error resillience settings according
 * to the new value of the expected loss rate.
 * @param handle - Handle of the encoder instance (returned by v4e_enc_open)
 * @param new_expected_loss_percents - new value of the expected losses (%, [0..100])
 * @return VSSH_OK or error code;
 */
int VSSHAPI v4e_set_current_loss_percents(void *handle, int new_expected_loss_percents);
/** @} */

/**
 * This function checks input encoder settings to be valid
 * invalid values are been corrected in place
 * @return VSSH_OK
 */
int VSSHAPI v4e_check_settings(v4e_settings_t *settings);


/**
 *
 */
typedef int preproc_callback_t(void *context, yuv_frame_t **frames, int count);

typedef struct v4e_preproc_callback_t
{
	void *context;	///< application context for frame release callback;
	preproc_callback_t *callback;	///< 
} v4e_preproc_callback_t;

/**
 *
 */
int VSSHAPI v4e_set_preproc_callback(void *handle, v4e_preproc_callback_t *preproc_callback);

#ifdef __cplusplus
}
#endif

#endif // __V4E_API_H__
