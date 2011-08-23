
/**
 * @file v4e_preproc_settings.h
 * Encoder preprocessing settings and types
 *
 * Project:	VSofts H.264 Codec V4
 * Module: Encoder
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4_PREPROC_SETTINGS_H__
#define __V4_PREPROC_SETTINGS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "v4_types.h"


#define IS_YUV400_FRAME(colorspace) \
	(COLORSPACE_E_YUV400 == (colorspace))

#define IS_YUV420_FRAME(colorspace)\
	(COLORSPACE_E_IYUV == (colorspace) \
	|| COLORSPACE_E_YV12 == (colorspace))

#define IS_YUV422_FRAME(colorspace) \
	(COLORSPACE_E_YUY2 == (colorspace) \
	|| COLORSPACE_E_UYVY == (colorspace) \
	|| COLORSPACE_E_YVYU   == (colorspace) \
	|| COLORSPACE_E_YUV422 == (colorspace))

#define IS_RGB_FRAME(colorspace) \
	(COLORSPACE_E_RGB555 == (colorspace) \
	|| COLORSPACE_E_RGB565 == (colorspace) \
	|| COLORSPACE_E_RGB24  == (colorspace) \
	|| COLORSPACE_E_RGB32  == (colorspace))


/** @name Input Picture
* complete input picture description
*/
//@{

/// Input picture and processing description
typedef struct input_picture_t
{
	int width;	///< picture width, pixels
	int height;	///< picture height, pixels
	colorspace_e colorspace;	///< colorspace specification;
	int sample_size;          ///< size of one pixel sample, bytes (default 1);
	int significant_bits;     ///< number of significant bits (default 8);
} input_picture_t;
//@}


/** @name Preprocessing settings and types
*
*/
//@{

typedef enum
{
    D_NONE              = 0,
    DUPLICATE_FIELD_1   = 1,
    DUPLICATE_FIELD_2   = 2,
    BLEND_FIELDS        = 3,
	DEINTERLACE_TOTAL   = 4
} deinterlace_types_e;

typedef struct crop_info_t
{
	int enable;
	int left;
	int top;
	int right;
	int bottom;
} crop_info_t;


//-----------------------------------------------------------------------------
//
typedef enum preproc_steps_types_e 
{
	S_NONE           = 0x00,

	BLUR_3x3         = 0x10,
	BLUR_5x5         = 0x11,

	SHARPEN_3x3      = 0x20,
	SHARPEN_5x5      = 0x21,

	MEDIAN_3x3       = 0x30,
	MEDIAN_5x3       = 0x31,

	RESIZE           = 0x40,

	TEMPORAL_DENOISE = 0x50

} preproc_steps_types_e;

typedef struct preproc_step_t
{
	preproc_steps_types_e type;
	int param0;
	int param1;
	int param2;
	int param3;
} preproc_step_t;


/**
* Noise levels for temporal denoising.
*
* WARNING: Please note that two of the listed values - "VERY_STRONG_NOISE_LEVEL"
* and "PICTURE_HARDLY_SEEN_NOISE_LEVEL" passed to denoising for HIGH-MOTION SEQUENCES
* may seriously corrupt the sequence and cause significant artifacts.
* For low motion sequences, these two denoising modes may sometimes
* efficiently reconstruct the sequence frames from really catastrophic noise.
* Please use the modes VERY_STRONG_NOISE_LEVEL and PICTURE_HARDLY_SEEN_NOISE_LEVEL
* only for the actually "bad" cases.
*
* The other modes are less sensitive to the motion intensity but
* in any case the lower is motion the better is denoising effect.
*/
typedef enum noise_level_e
{
	N_NONE            = 0,
	WEAK              = 1,
	MODERATE          = 2,
	MIDDLE            = 3,
	STRONG            = 4,
	VERY_STRONG       = 5,         // ! corrupts high-motion sequences
	HARD              = 6, // ! corrupts high-motion sequences
	NOISE_LEVEL_TOTAL = 6
} noise_level_e;

typedef struct me_denoise_t
{
	noise_level_e  level;
	int            skip_luma;
	int            skip_chroma;
} me_denoise_t;
//------------------------------------------------------------------------------
//
#define PREPROC_MAX_STEPS (7)

typedef struct preproc_settings_t
{
	int                  enable;
	crop_info_t          crop;           ///< cropping settings
	int                  source_type;         ///< source type: 0=progressive, 1=top field first, 2=bottom field first;
	deinterlace_types_e  deinterlace;    ///< deinterlace settings
	chroma_format_idc_e  chroma_format_idc;          ///< internal data, do not use it!
	me_denoise_t         me_denoise;
	
	preproc_step_t       step[PREPROC_MAX_STEPS];

	int                  intra_precision;		///< calc intra using orig pixels;
} preproc_settings_t;
// @}


int check_preproc_settings(int *frame_width, 
						   int *frame_height, 
						   input_picture_t    *input,
						   preproc_settings_t *preproc);


#ifdef __cplusplus
}
#endif

#endif//__V4_PREPROC_SETTINGS_H__
