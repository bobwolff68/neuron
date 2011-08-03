
/**
 * @file v4d_settings.h
 * Decoder settings structures.
 *
 * Project:	VSofts H.264 Codec V4
 * Module:	Decoder
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */


#ifndef __VSSH_DEC4_SETTINGS_H__
#define __VSSH_DEC4_SETTINGS_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "v4_types.h"
#include "v4_media_sample.h"

/// decoder flags
typedef enum
{
	/**
	 * This flag will tell decoder to decode sei-messages
	 * Application can access to it via sei-list, attached to the output frame
	 * If this flag is not set, sei messages are attached to frame "as is", without
	 * separating and decoding
	 */
	DEC_DECODE_SEI = 1,


	/**
	 * This flag controls how decoder will deal with pic_struct received by timing SEI.
	 * if this flag is 0 nothing is done.
	 * if this flag is 1 decoder interpret this parameter inside. I.e. make telecine and other actions.
	 */
	DEC_INTERP_PIC_STRUCT = 2,

	/**
	 * enable "postprocessing" deblocking when "inloop" deblocking is disabled
	 */
	DEC_POST_DEBLOCK = 4,

	/**
	 * enable full error concealment for lost data;
	 */
	DEC_FULL_ERR_CONC = 8,

	/**
	 * output bottom field first (effective only when DEC_INTERP_PIC_STRUCT is set);
	 */
	DEC_BOTTOM_FIELD_FIRST = 0x10,


	/**
	 * this flag disable actual frames decoding. Used for stream analysing
	 * If opened with this flag, decoder will not produce output frames.
	 *It also will not handle NAL samples release. Caller must handle it itself.
	 */
	DEC_ANALYSE_ONLY = 0x80,

	/**
	 * this flag force decoder to skip deblocking for nonreference pictures
	 * can be used for decoder speedup
	 */
	DEC_SKIP_NON_REF_DEBLOCKING = 0x100,

	/**
	 * this flag force decoder to skip deblocking for reference pictures
	 * can be used for decoder speedup
	 */
	DEC_SKIP_REF_DEBLOCKING = 0x200,
	/**
	 * this flag force decoder to skip decoding of pictures, coded using FMO
	 */
	DEC_DISABLE_FMO = 0x400,
	/**
	 * this flag force decoder to depack frame accoriding to frame packing sei-message
	 */
	DEC_INTERP_FRAME_PACKING_SEI = 0x800
} dec_flags_e;


/// All decoder settings
typedef struct mt_dec_settings_t
{
	int disable;
	int num_threads;
	int num_nal_units; ///<  max number of nal_units in decoder core buffer
	int max_picts_in_parallel;
	//Can be expanded
} mt_dec_settings_t;


typedef struct v4d_settings_t
{
	unsigned int dec_flags; ///< bitwise combination of flags to control decoder behaviour (see flags values above)
	int typical_nal_size;	///< select typical NAL unit size in bytes, 0 means default value;
	int skip_non_ref_frames; ///< 0 - don't skip, 1 - skip 1 from sequence, 2 - skip 2 from sequence ...
	int output_delay;       ///< maximum decoded frame output delay
	mt_dec_settings_t mt_settings;
} v4d_settings_t;

/** @addtogroup dec_func */
// @{

/**
 * Set default decoder settings
 */
void v4d_default_settings(v4d_settings_t *settings, int mt_disable);

// @}

#ifdef __cplusplus
}
#endif

#endif	//__VSSH_DEC4_SETTINGS_H__
