
/**
 * @file v4_error.h
 * Error numbers and messages
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4_ERROR_H__
#define __V4_ERROR_H__


/** @defgroup return_codes Return Codes
* @{
*/

/* function return codes */
/// Everything is OK
#define VSSH_OK 0

/// Warning: data is not ready yet
#define VSSH_WARN_NOT_READY 1

/// Warning: timeout occured
#define VSSH_WARN_TIMEOUT 2

/// Warning: settings do not fit into profile/level limitations
/// (frame size exceed, mbps excced, bitrate exceed)
#define VSSH_WARN_LEVEL_LIMITS 3

/// Warning: end of stream
#define VSSH_WARN_EOS 4

/// Warning: svc layers param error detected
#define VSSH_WARN_SVC_LAYER 5

/// Warning: wrong chroma format idc for MVC
#define VSSH_WARN_WRONG_MVC_CHROMA_FORMAT_IDC 6

/// Warning: too many MGS slices are assigned in current qependency_id
/// (the number of quality layers in current dependency_id is truncated to max value 16)
#define VSSH_WARN_TOO_MANY_MGS_SLICES_ASSIGNED 7

/// Warning: too many MGS slices are assigned in current qependency_id
/// (the number of quality layers in current dependency_id is truncated to max value 16)
#define VSSH_WARN_GPU_UNAVAILABLE 8

/// Error: general fault
#define VSSH_ERR_GENERAL -1

/// Error: not enough memory
#define VSSH_ERR_MEMORY -2

/// Error: wrong function argument value
#define VSSH_ERR_ARG -3

/// Warning: evaluation expired
#define VSSH_ERR_EXPIRED -4

/// Warning: frame skipped
#define VSSH_ERR_SKIP_FRAME -5

/// Error: invalid input frame dimensions specified
#define VSSH_ERR_FRAME_DIMENSIONS -6

/// Error: invalid encoder settings specified
#define VSSH_ERR_SETTINGS -7

/// Error: more input data needed to perform operation
#define VSSH_ERR_MORE_DATA -8

/// Error: file operation failed
#define VSSH_ERR_FILE -9

/// Error in stream: invalid prediction type
#define VSSH_ERR_STREAM_PRED_TYPE -10

/// Error in stream: invalid direct params
#define VSSH_ERR_STREAM_DIRECT -11

/// Error in stream: attempt to read after end of stream
#define VSSH_ERR_STREAM_EOS -12

/// Error in stream: error in CAVLC
#define VSSH_ERR_STREAM_CAVLC -13

/// Error in stream: error in REF_FRAME_NO
#define VSSH_ERR_STREAM_REF_FRAME_NO -14

/// Error in stream: error in CBP
#define VSSH_ERR_STREAM_CBP -15

/// Error in stream: error in subdiv type
#define VSSH_ERR_STREAM_SUBDIV_TYPE -16

/// Error in stream: error in sps type
#define VSSH_ERR_STREAM_SPS -17

/// Error in stream: error in mb type
#define VSSH_ERR_STREAM_MB_TYPE -18

/// Error in stream: error in subdiv 8x8 type
#define VSSH_ERR_STREAM_SUBDIV_8X8_TYPE -19

/// Error in stream: slice before sps or pps
#define VSSH_ERR_STREAM_SLICE_BEFORE_SPS_OR_PPS -20

#define VSSH_ERR_STREAM_WRONG_PPS_ID -21

#define VSSH_ERR_PPS_FMO_6_PARAM -22

#define VSSH_ERR_WRONG_REORDER_CODE -23

#define VSSH_ERR_PROFILE_NOT_SUPPORTED -24

#define VSSH_ERR_NON_REF_IDR_SLICE -25

/// the API function is not implemented
#define VSSH_ERR_NOTIMPL -26

/// Error in rfc3984 FU-nal
#define VSSH_ERR_STREAM_FU -27

/// Error in stream: more macroblocks required
#define VSSH_ERR_STREAM_EXTRA_MBS -28

/// Error in stream: invalid qp delta value
#define VSSH_ERR_STREAM_QP_DELTA -29

/// Error: can't change sps while already decoded frames still persist in buffer
#define VSSH_ERR_SPS_CHANGE -30

/// error in first mb of slice
#define VSSH_ERR_FIRST_MB -31

/// error in SPS_ID. It must be in range [0,31]
#define VSSH_ERR_SPS_ID -32

/// error in LOG2_MAX_FRAME_NUM. It must be in range [4,16]
#define VSSH_ERR_LOG2_MAX_FRAME_NUM -33

/// error in LOG2_MAX_FRAME_POC. It must be in range [4,16]
#define VSSH_ERR_LOG2_MAX_POC -34

/// error in PIC_ORDER_CNT. It must be in range [0,2]
#define VSSH_ERR_PIC_ORDER_CNT -35

/// error in NUM_REF_FRAME It must be at least not bigger than 16
#define VSSH_ERR_NUM_REF_FRAMES -36

/// Error in stream: some frames before current were dropped
#define VSSH_ERR_FRAME_DROP_DETECTED -37

/// Error in stream: some slices of current frame were dropped
#define VSSH_ERR_SLICE_DROP_DETECTED -38

/// Error in stream: incorrect cabac model number in slice header
#define VSSH_ERR_MODEL_NUMBER -39

/// Error in stream: incorrect number of references in slice header
#define VSSH_ERR_NUM_REFS -40


/// More fatal errors in check_settings function
#define VSSH_ERR_SVC_NOT_SUPPORTED -41
#define VSSH_ERR_MVC_NOT_SUPPORTED -42


/// This is a number of first fatal for whole stream in decoder
#define VSSH_ERR_FIRST_FATAL -100

#define VSSH_ERR_MMCO_CODE  -100
#define VSSH_ERR_NOT_SUPPORTED_SLICE_GROUPS -101
#define VSSH_ERR_NOT_SUPPORTED_SLICE_TYPE  -104

#define VSSH_ERR_NOT_SUPPORTED_FMO      -105
#define VSSH_ERR_NOT_SUPPORTED_BITDEPTH -106
#define VSSH_ERR_NOT_SUPPORTED_CHROMA_FORMAT_IDC -107
#define VSSH_ERR_UNKNOWN_PROFILE_IDC     -108
#define VSSH_ERR_NOT_SUPPORTED_SPATIAL_RESOLUTION_CHANGE -109
#define VSSH_ERR_SVC_INTERLACE_NOT_SUPPORTED -110
#define VSSH_ERR_SVC_TRANSFORM_SIZE8x8_NOT_SUPPORTED -111
#define VSSH_ERR_SVC_NOT_SUPPORTED_MGS_FEATURES -112
#define VSSH_ERR_NOT_SUPPORTED_CONSTRAINED_INTRA_RESAMPLING -113
#define VSSH_ERR_NOT_SUPPORTED_SLICE_TYPE_FOR_CURRENT_PROFILE_IDC -114
#define VSSH_ERR_NOT_SUPPORTED_SVC_IN_EXTRA_BIT_DEPTH_MODE -115
/** @} */

/// Error: invalid encoder settings specified - colorspace and chroma_format_idc mismatch
#define VSSH_ERR_SETTINGS_CHROMA_FORMAT_IDC_MISMATCH -116

/// Error: invalid encoder settings specified - 10-bit encoder don't support 8-bit sources
#define VSSH_ERR_SETTINGS_10BIT_ENC_NOT_SUPPORT_8BIT_SOURCE -117

/// Error: invalid encoder settings specified - 8-bit encoder don't support 10-bit sources
#define VSSH_ERR_SETTINGS_8BIT_ENC_NOT_SUPPORT_10BIT_SOURCE -118

/// Error: invalid encoder settings specified - sample size and significant bits number mismatch
#define VSSH_ERR_SETTINGS_SIGNIFICANT_BITS_MISMATCH -119

/// Error: invalid encoder settings specified - not supported colorspace type
#define VSSH_ERR_SETTINGS_NOT_SUPPORTED_COLORSPACE_TYPE -120

/// Error: invalid encoder settings specified - automatic set multi-stream mode fail
#define VSSH_ERR_SETTINGS_AUTO_MULTISTREAM -121

/// Error: invalid luma or chroma bitdepth for current profile idc
#define VSSH_ERR_SETTINGS_BITDEPTH_PROFILE_MISMATCH -122

/// Error: too many MGS slices are assigned in current qependency_id
/// (several consecutive MGS layers are assigned and common number of slices exceeds 16)
#define VSSH_ERR_SETTINGS_TOO_MANY_MGS_SLICES_ASSIGNED -123

#define VSSH_ERR_INVALID_SEI -124

#define VSSH_ERR_QP_OUTOFRANGE -125

#define VSSH_ERR_RC_MAX_INTRA_FRAME_BYTES -126

/// Error: check multistream resolutions are monotonic increasing failed
#define VSSH_ERR_MULTISTREAM_RESOLUTION -127


#endif	// __V4_ERROR_H__
