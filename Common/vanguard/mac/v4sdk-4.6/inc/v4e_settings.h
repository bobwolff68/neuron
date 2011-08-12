
/**
 * @file v4e_settings.h
 * Encoder settings structures
 *
 * Project:	VSofts H.264 Codec V4
 * Module: Encoder
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */



#ifndef __VSSH_SETTINGS_H__
#define __VSSH_SETTINGS_H__

#include "v4_types.h"
#include "v4e_preproc_settings.h"
#include "v4_nalu.h"


#ifdef __cplusplus
extern "C" {
#endif

// build number of codec
#define V4E_SETTINGS_BUILD 4300
// "magic" number (yyyymmdd) of last change to encoder settings
#define V4E_SETTINGS_MAGIC 20090402

/// encoder operation speed selector
typedef struct speed_settings_t
{
	int i;	///< compression speed/efficiency ratio values [0..7] (0=max compression, 7=max speed)
	int p;	///< compression speed/efficiency ratio values [0..7] (0=max compression, 7=max speed)
	int b;	///< compression speed/efficiency ratio values [0..7] (0=max compression, 7=max speed)
	int automatic; ///< 0=disable; 1=automatic;
} speed_settings_t;

/**
* Macroblock subdivision flags.
* To be combined by "|".
*/
typedef enum
{
	MODE_16X16 = 1,
	MODE_16X8  = 2,
	MODE_8X16  = 4,
	MODE_8X8   = 8,
	MODE_8X4   = 16,
	MODE_4X8   = 32,
	MODE_4X4   = 64
} subdiv_flags_e;


#define ALL_SUBDIV_MODES (MODE_16X16+MODE_16X8+MODE_8X16+MODE_8X8+MODE_8X4+MODE_4X8+MODE_4X4)
#define SQR_SUBDIV_MODES (MODE_16X16+MODE_8X8+MODE_4X4)
#define MPG_SUBDIV_MODES (MODE_16X16+MODE_8X8)

/// Macroblock Subdivision modes
typedef enum
{
	SD_MODE_16X16 = 0,
	SD_MODE_16X8  = 1,
	SD_MODE_8X16  = 2,
	SD_MODE_8X8   = 3,
	SD_MODE_8X4   = 4,
	SD_MODE_4X8   = 5,
	SD_MODE_4X4   = 6
} subdiv_modes_e;


/// Types of Rate Control
typedef enum
{
	RATE_CONTROL_QP  = 0,	///< fixed quality (fixed QP);
	RATE_CONTROL_VBR = 1,	///< VBR (HRD: not conforming);
	RATE_CONTROL_CBR = 2,	///< CBR (HRD: VBR mode);
	RATE_CONTROL_CBR_WITH_FILLER = 3,	///< CBR (HRD: CBR mode);
	RATE_CONTROL_DUAL_PASS_0 = 4,	///< first pass of dual pass mode;
	RATE_CONTROL_DUAL_PASS_1 = 5,	///< second pass of dual pass mode;
	RATE_CONTROL_INTRA = 6,	///< intra-only mode
	RATE_CONTROL_INTRA_WITH_FILLER = 7 ///< intra-only mode with filler
} rate_control_types_e;

#define IS_RC_CBR(type) ((type) == RATE_CONTROL_CBR || (type) == RATE_CONTROL_CBR_WITH_FILLER ||\
	                     (type) == RATE_CONTROL_INTRA || (type) == RATE_CONTROL_INTRA_WITH_FILLER)

#define IS_RC_INTRA(type) ((type) == RATE_CONTROL_INTRA || (type) == RATE_CONTROL_INTRA_WITH_FILLER)

/**
* Motion Estimation flags.
* Can be combined by "|".
*/
typedef enum
{
	ME_USE_MB_ME_P                = 0x0001,    ///< Use macroblock level me for P-slices (obsolete)
	ME_USE_MB_ME_B                = 0x0002,    ///< Use macroblock level me for B-slices (obsolete)
	ME_USE_SMALL_ME_P             = 0x0004,    ///< Use preliminary me on the reduced pictures P-slices (obsolete)
	ME_USE_SMALL_ME_B             = 0x0008,    ///< Use preliminary me on the reduced pictures B-slices (obsolete)
	ME_LIMIT_B_REFS               = 0x0010,    ///< Set num_refs for B-frames to (1,1) even if max_refs > 1
	ME_DISABLE_PREPROC_ME         = 0x0020,    ///< Disable preproc motion estimation by reduced picture
	ME_DISABLE_PREPROC_COMPLEXITY = 0x0040,    ///< Disable preproc complexity calculation
	ME_USE_ORIGINAL               = 0x0100,    ///< use original pixels for motion estimation (obsolete)
	ME_USE_FULL_LOG_SEARCH_P      = 0x1000,  ///< Use more detailed motion estimation for P frames
	ME_USE_FULL_LOG_SEARCH_B      = 0x2000   ///< Use more detailed motion estimation for B frames
} me_flags_e;


/// Motion Estimation settings
typedef struct me_settings_t
{
	int max_refs; ///< Number of previous frames used for inter motion search (1-5); @see encode.cfg::me.max_refs
	int use_idr_long_term;  ///< Use long term IDR frames (0/1); not used in current version
	int subdiv;       ///< subdiv_flags_e combined by "|"; @see encode.cfg::me.subdiv
	int flags;              ///< bitwise flags; @see me_flags_e; @see encode.cfg::me.flags
	int search_range; ///< motion search range in full pels;
} me_settings_t;

/// Slicing settings
typedef struct slice_settings_t
{
	int mode;  ///< slicing mode (0=off; 1=mbs in slice; 2=bytes in slice; 3=slices in picture); @see encode.cfg::slice.mode
	int param; ///< slice argument (arguments to modes 1,2,3 above); @see encode.cfg::slice.param
	int i_param; ///< override "param" for I-pictures; @see encode.cfg::slice.i_param
	int b_param; ///< override "param" for non-ref pictures; @see encode.cfg::slice.b_param
	int fmo_type;    ///< [TBD] Not used in current version;
} slice_settings_t;

/// Deblock (loop filter) settings
typedef struct deblock_settings_t
{
	int flag; ///< Configure loop filter (0=parameter below ignored, 1=parameters sent); @see encode.cfg::deblock.flag
	int disable; ///< Disable loop filter in slice header (0=enable, 1=disable); @see encode.cfg::deblock.disable
	int alpha_c0; ///< Alpha & C0 offset div. 2, [-6..6]; @see encode.cfg::deblock.alpha_c0
	int beta_c0; ///< Beta offset div. 2, [-6..6]; @see encode.cfg::beta_c0
} deblock_settings_t;


/// GOP (Group Of Pictures) flags
typedef enum
{
	GOP_USE_HIERARCHICAL_B  =    0x1, ///< encoder Hierarchical B-frames (for 3 and more)
	GOP_NO_IDR_ON_SCENE_CHANGE = 0x2 ///< don't set IDR-slice on schene changes 
} gop_flags_e;

/// GOP (group of pictures) settings
typedef struct gop_settings_t
{
	int num_units;	///< num units per tick; @see encode.cfg::gop.num_units
	int time_scale;	///< time scale (field_rate = time_scale/num_units); @see encode.cfg::time_scale
	int keyframes; ///< Period of I-Frames (0=only first); @see encode.cfg::gop.keyframes
	int idr_period; ///< Period of IDR I-Frames (0=only first); @see encode.cfg::gop.idr_period
	int bframes;  ///< number of B-frames (0=disable); @see encode.cfg::gop.bframes
	int min_bframes;  ///< minimal number of B-frames (if != bframes_num encoder will adaptively select best number; -1 means, the same as bframe_num)
	int emulate_b;  ///< enable B-frames emulation using non-ref P-frames (0 - not uesd; 1 -  in a'la B-frames order; 2 - natural order);
	int aud;	///< enable Access Unit Delimiter NAL units for every picture (0/1/2); @see encode.cfg::gop.aud
	int min_intra_period; ///< minimum number of non-intra frames between intra frames
	int sps_period; ///< How often SPS/PPS is included (used for coding with keyframes = 0)
	int flags;  ///< bitwise flags; @see gop_flags_e;
} gop_settings_t;

/// RC (Rate Control) flags
typedef enum
{
	RC_IGNORE_VBR_BUFFER_OVERFLOW = 0x1, ///< ignore buffer overflow for VBR coding
	RC_BOUND_MAX_QP_DELTA_B =       0x2, ///< use qp_delta_b as max for automatically calulated delta_b        
	RC_BOUND_MIN_QP_DELTA_B =       0x4, ///< use qp_delta_b as min for automatically calulated delta_b
	RC_ADD_FILLER           =       0x10,///< add filler NALs (can be set in vbr mode if max_kbps is specified)
	RC_SET_CBR_FLAG         =       0x20,///< put cbr_flag into sps (effect only with flag above)
	RC_FIXED_GOP_SIZE       =       0x40 ///< support fixed number of bytes in GOP (overflow is not garanteed)
} rc_flags_e;



/// Rate Control settings
typedef struct rate_control_settings_t
{
	rate_control_types_e type;  ///< rate control mode; @see rate_control_types_e; @see encode.cfg::rc.type
	int flags; ///<bitwise flags; @see rc_flags_e
	int kbps;	 ///< desired bitrate (kbits per sec); @see encode.cfg::rc.kbps
	int qp_intra;     ///< init param for intra frames (0..51); @see encode.cfg::rc.qp_intra
	int qp_modulation;///< enable QP modulation (0/1); @see encode.cfg::rc.qp_modulation
	int scene_detect; ///< scene change detection threshold (0..100), default 70; @see encode.cfg::rc.scene_detect
	int qp_delta_p;		///< basic delta QP between I and P frames (0..51); @see encode.cfg::rc.qp_delta_p
	int qp_delta_b;		///< basic delta QP between P and B frames (0..51); @see encode.cfg::rc.qp_delta_b
	int auto_qp; ///< enable automatic start, max and min qp calculation (0/1); @see encode.cfg::rc.auto_qp
	int qp_max;	///< select maximum allowed QP (0..51), 0 means n/a; @see encode.cfg::rc.qp_max
	int qp_min;	///< select minimum allowed QP (0..51), 0 means n/a; @see encode.cfg::rc.qp_min
	int vbv_length; ///< rate_control buffer length in msec for CBR or max_kbps, default 1000;
	//CPB size in bits in hrd-buffer will be set to (vbv_length*kbps) for CBR or (vbv_length*max_kbps) for VBR
	int initial_cpb_removal_delay; ///< in units of a 90 kHz clock (see standard) default: -1, - calculated as vbv_length/2;
	int mb_update; ///< flag to use rate-control on a macroblock-line level; @see encode.cfg::rc.mb_update
	int look_ahead; //< number of look-ahead frames (0-16) default - 1; @see encode.cfg::look_ahead
	int max_kbps; ///< max allowed bitrate in vbr mode; default - 0 (means not set);  @see encode.cfg::max_kbps
	int dual_pass_param; ///< 0 - 256; 0 - CBR-like; 256 - fixed qp -like  
	int max_intra_frame_bytes; ///< maximum size of intra frames in bytes (0 means no restriction)
	int min_intra_frame_bytes; ///< minimum size of intra frames in bytes (0 means no restriction)
	int gop_bytes; ///< size of GOP in bytes (used with RC_FIXED_GOP_SIZE flag; 0 means will be calulated from kbps or max_kbps)
} rate_control_settings_t;

/// SEI (supplemental enhancement information) messages settings
typedef struct sei_settings_t
{
	int pic_timing_flag; ///< enable picture timing SEI messages (0/1/2); @see encode.cfg::sei.pic_timing
	int film_grain_flag; ///< enable film grain (0/1/2)
	int film_grain_mode; ///< (0/4) 0 - additive automatic, 4 - manual additive;
	int film_grain_luma_noise_level; ///< for manual mode
	int film_grain_luma_max_frequency; ///< for manual mode (0-15)
	int film_grain_chroma_noise_level; ///< for manual mode
	int film_grain_chroma_max_frequency; ///< for manual mode (0-15)
	int post_filter_flag; ///< enable post-filter hint SEI message
	int post_filter_mode; ///< (0/1/2) 0 - 2D filter, 1 -1D filters, 2 - cross-correlation matrix;
	int post_filter_size; ///< 1 - filter_size=3, 2 - filter_size=5, 3 - filter_size=7;
	int frame_packing_flag; ///< enable frame packing arrangement SEI (0/1/2)
	int frame_packing_type; ///< frame packing type
	int rec_point_flag; ///< enable recovery point sei (0,1,2)
} sei_settings_t;


/// Multi-Threading settings
typedef struct mt_settings_t
{
	int disable;	 ///< flag to disable multi-threading;
	int num_threads; ///< number of worker threads to run, zero means automatic value equal to number of CPUs;
	int max_pict_tasks; ///< number of picture encoding tasks to use [1..MAX_PICT_ENC_TASKS];
	int max_raw_frames; ///< maximum number of input frames to hold for async mode [1..NUM_RAW_FRAMES]; (0 - calculate automatically)
} mt_settings_t;

/// Fidelity Range Extensions (FRExt) settings
typedef struct frext_t
{
	int transform_8x8;  ///< 0 - off; 1 - adaptive; 2 - no 4x4; 3 - 8x8 only
	int second_qp_offset; ///< chroma qp offset @see encode.cfg::frext.second_qp_offset
	int scaling_matrix; ///< enable using custom or default alternative Quant matrix @see encode.cfg::frext.scaling_matrix
} frext_t;

/// Settings for error resilience in encoder
typedef struct error_resilience_settings_t
{
	int enable;	///< enable ER (0/1); @see encode.cfg::error_resilience.enable
	int initial_expected_loss_percent;	///< The initial expected loss rate in percents; @see encode.cfg::error_resilience.initial_expected_loss_percent
	int intra_update_method;	///< 0=none, 1=adaptive,  2=horizontal, 3=random; @see encode.cfg::error_resilience.intra_update_method
	int total_intra_update_period; ///< The period for picture full intra update, frames
	int full_motion_update_period;	///< long update period, frames. 0=none, 1..3=recommended; @see encode.cfg::error_resilience.full_motion_update_period
	int fast_motion_update_period;	///< short update period, frames. 0=none, 5..15=recommended; @see encode.cfg::error_resilience.fast_motion_update_period
	int me_region_height_in_mbs;	///< motion estimation vertical constraint; @see encode.cfg::error_resilience.me_region_height_in_mbs
	int me_region_width_in_mbs;	///< [obsoleted] motion estimation horizontal constraint;
} error_resilience_settings_t;


/******************************************************************************************
  * @name SVC
  * SVC related parameters.
  * Encoder support encoding of the base layer plus several spatial and quality enhancement
  * layers above it in "linear" order. SVC encoding is controlled by "svc_settings_t" structure.
  * This structure defines the number of SVC layers (0 means no SVC) and properties layers.
  * Properties of each SVC layer is defined in the "svc_layer_settings_t" structure.
  */
// @{


/**
* Flags of the SVC coding tools.
* Bitwise combination of these flags defines the tools, used in each SVC layer.
* It can be different for different layers.
*/
typedef enum
{
	SVC_ADAPTIVE_BASEMODE_FLAG = 0x01,
	SVC_ADAPTIVE_RES_PRED_FLAG = 0x02,
	SVC_ADAPTIVE_MV_PRED_FLAG = 0x04,

	SVC_DEFAULT_BASEMODE_FLAG = 0x10,
	SVC_DEFAULT_RES_PRED_FLAG = 0x20,
	SVC_DEFAULT_MV_PRED_FLAG = 0x40
} svc_flags_e;


/**
* Defines the type of scalability for SVC layer.
* Currently only 2x2 spatial scalability and quality scalability is implemented.
*/
typedef enum
{
	SVC_EXTEND_2X2 = 0,  //spatial extend twice in both direction (spatial scalability)
	SVC_EXTEND_1X1 = 1,  //no spatial extend (coarse-grain quality scalability)
	SVC_MGS_EXTEND = 2,  //no spatial extend (medium-grain quality scalability)
	SVC_EXTEND_1_5 = 15, //spatial 1.5 extend in both direction
	SVC_EXTEND_CUSTOM = 100 //Custom spatial scalability.
} svc_spatial_extend_e;

/**
* Properties of one SVC layer coding
* In SVC coding (num_layers > 0) similar parameters of the main
* setting structure (v4e_settings_t) relate to the base layer.
* Coding parameters, not included into this structure, are common
* for all layers and are taken from main settings structure.
*/
typedef struct svc_layer_settings_t
{
	int profile_idc;	///< profile IDC selection; @see profile_idc_e; @see encode.cfg::profile_idc
	int level_idc;		///< level IDC selection; @see level_idc_e; @see encode.cfg::level_idc
	svc_spatial_extend_e extend; ///< spatial extend, comparing to prev layer
	int num_mgs_slices; //number of slices to split coefs (valid only for MGS extend)
	unsigned int mgs_coeffs; // i.e. 0xB73 - means coefs [0-3] into slice0; [4-7] - slice1; [8-11] - slice2; [12-15] slice3
	//zero - means automatic calculation

	unsigned int flags_i; ///< Bitwise combination of svc_flags_e for intra-frames coding
	unsigned int flags_p; ///< Bitwise combination of svc_flags_e for p-frames coding
	unsigned int flags_b; ///< Bitwise combination of svc_flags_e for b-frames coding
	int sym_mode;	///< select symbol mode: 0=UVLC; 1=CABAC;
	int kbps;  ///< desired bitrate (for this plus below levels) Must be greater then kbps for previous level
	int max_kbps; ///< max allowed bitrate in vbr mode; default - 0 (means not set)
	int qp_intra; ///< qp for intra-frames coding (qp_delta_p and qp_delta_b is used from main settings)(used for rc.type = 0)
	speed_settings_t speed; ///< encoding speed settings
	slice_settings_t slice; ///< slicing parameters
	
	byte vui_aspect_ratio_idc; ///< aspect_ratio_idc: 0-auto, 1-16-manual set from Table E-1, 255-Extended_SAR
	byte res[3];
	uint16_t vui_sar_width;        ///< Extended_SAR width
	uint16_t vui_sar_height;       ///< Extended_SAR height

	//In all encoding modes except SVC_EXTEND_CUSTOM, Parameters below are "output" parameters.
	//They will be calculated by encoder in v4e_open() function.
	//If SVC_EXTEND_CUSTOM encoding extend is used in any layer, only last layer dimensions will be calculated.
	//all other dimensions including base-layer must be set by caller.
	int frame_width;	///< encoding frame width, pixels;
	int frame_height;	///< encoding frame height, pixels;
} svc_layer_settings_t;


/// Multi-stream modes
typedef enum
{
	MULTISTREAM_MODE_SVC = 0,	///< SVC mode (default) 
	MULTISTREAM_MODE_AVC = 1,	///< AVC - generate several AVC streams
	MULTISTREAM_MODE_MVC = 2	///< MVC - generate MVC stream
} multistream_modes_e;


/// Multi-stream flags
typedef enum
{
	M_FLAG_MVC_PREFIX     = 1,	///< Put MVC prefix-nal units into stream 
	M_FLAG_MVC_DELIMITERS = 2,	///< Put MVC picture delimiter into stream 
	M_FLAG_AVC_FAST       = 4,	///< Use fast version of ParallelStream 
	M_FLAG_MVC_BLU_RAY_SEI= 8   ///< Generate MVC SEI according to Blu Ray spec
} multistream_flags_e;


typedef struct svc_settings_t
{
	int num_layers; ///< number of svc-layers (0 means no SVC)
	int key_picture_period; //Used for MGS encoding
	int temporal_mode; // 0 - no temporal scalability; 1 - non-reference frames to separate temporal layer
	int multistream_mode; // See multistream_modes_e enum
	int flags; //Bitwise svc/mvc/multistream encoding flags; see multi_stream_flags_e enum
	/// Last valid element of this array is related to the to the most “rich” layer
	svc_layer_settings_t layer[MAX_SVC_LAYERS];
} svc_settings_t;

// **********  End of SVC related settings **********************************

// @}

/// custom quantization matrices flags
typedef enum
{
	CQM_INTRA_Y_4X4 = 0x01,
	CQM_INTRA_U_4X4 = 0x02,
	CQM_INTRA_V_4X4 = 0x04,
	CQM_INTER_Y_4X4 = 0x08,
	CQM_INTER_U_4X4 = 0x10,
	CQM_INTER_V_4X4 = 0x20,
	CQM_INTRA_Y_8X8 = 0x40,
	CQM_INTER_Y_8X8 = 0x80
} cqm_flags_e;

/// custom quantization matrices methods
typedef enum
{
	CQM_METHOD_SETTINGS = 1,
	CQM_METHOD_AUTO = 2
} cqm_methods_e;

/// custom quantization matrix settings
typedef struct cqm_settings_t
{
	int method;	///< custom quantization matrix method: 0=disabled, 1=settings, 2=auto; @see cqm_methods_e; @see encode.cfg::cqm.method
	int flags;	///< custom quantization matrix bitwise flags: 0=CQM disabled; @see cqm_flags_e; @see encode.cfg::cqm.flags
	unsigned char intra4x4_luma[16]; ///< @see encode.cfg::cqm.intra4x4y
	unsigned char intra4x4_chromau[16]; ///< @see encode.cfg::cqm.intra4x4u
	unsigned char intra4x4_chromav[16]; ///< @see encode.cfg::cqm.intra4x4v
	unsigned char inter4x4_luma[16]; ///< @see encode.cfg::cqm.inter4x4y
	unsigned char inter4x4_chromau[16]; ///< @see encode.cfg::cqm.inter4x4u
	unsigned char inter4x4_chromav[16]; ///< @see encode.cfg::cqm.inter4x4v
	unsigned char intra8x8_luma[64]; ///< @see encode.cfg::cqm.intra8x8y
	unsigned char inter8x8_luma[64]; ///< @see encode.cfg::cqm.inter8x8y
} cqm_settings_t;

/// interlace flags
typedef enum
{
	INT_PREDICT_BOTTOM_FROM_TOP = 1, ///< for I and P only coding predict bottom field only from current top field 
	INT_INTRA_FOR_BOTH_FIELD    = 2, ///< encode both fields as intra slices (default is only first intra)
	INT_BOTTOM_FIRST            = 4, ///< show bottom field first when mbaff, frame or telecine coding (default - top field first)
	INT_FORCE_INTERLACE_PICTS   = 8, ///< force decoder to play frame-encoded stream as interlaced
	INT_MBAFF_BOTTOM_ZERO_POC   = 0x10, ///< put zero POC offsets for both top & bottom fields (for mbaff coding);
	INT_MBAFF_USE_FULL_FF_DECIDE= 0x20, ///< make mbaff frame-field decisions on the macroblock level based on full frame and field pair check
	INT_DISABLE_BOTTOM_FIELD_PREPROC = 0x40,  ///< disable preprocessing for bottom field
	INT_ADD_TELECINE_PICT_STRUCT     = 0x80,  ///< add telecine picture structure for frame-encoded video
	INT_TELECINE_START_WITH_3        = 0x100  ///< 3-2 start with 3. Together with INT_BOTTOM_FIRST defines start position of telecine
} interlace_flags_e;

/// special encoder flags
typedef enum
{
	ENC_DISABLE_VUI =    1,   ///< don't put vui infromation in sps
	ENC_SLICE_TYPE_012 = 2,   ///< encode slice types as 0,1 or 2 (default is 5,6,7)
	ENC_SPS_ONLY_ONCE  = 4,   ///< put SPS only for the first frame of stream
	ENC_REC_POINT_IDR  = 8,    ///< put recovery point SEI for IDR picture (default is only for non-idr Intra)
	ENC_FRAME_PACKING  = 0x10  ///< used together with sei.frame_packing flags. Force encoder to perform actual packing
} enc_flags_e;

/// special encoder flags
typedef enum
{
	ENC_GPU_NO   = 0x00,   ///< 
	ENC_GPU_ME   = 0x01,   ///< 
	ENC_GPU_DEBL = 0x02,   ///< 
	ENC_GPU_ALL_MASK  =  0x03  ///< change this according all actual flags
} cuda_gpu_flags_e;


/// special encoder flags
typedef enum
{
	AVC_I_FORCE_PANASONIC = 1   ///< force all features for Panasonic compatibility
} avc_intra_flags_e;

/// All encoder settings
typedef struct v4e_settings_t
{
	/// put a size of your structure here - to prevent problems with version compatibility
	int size;

	int profile_idc;	///< profile IDC selection; @see profile_idc_e; @see encode.cfg::profile_idc
	int level_idc;		///< level IDC selection; @see level_idc_e; @see encode.cfg::level_idc

	int frame_width;	///< encoding frame width, pixels;  [will be automatically calculated by "check_settings()"]
	int frame_height;	///< encoding frame height, pixels; [will be automatically calculated by "check_settings()"]
	//Note: if custom extend is used in SVC-coding this params shall be set explicitly

	int bit_depth_luma;      ///< luma bit depth
	int bit_depth_chroma;      ///< chroma bit depth

	int sym_mode;	///< select symbol mode: 0=UVLC; 1=CABAC;  @see encode.cfg::symbol_mode
	int interlace_mode; ///< 0=progressive; 1=top field first; 2=bottom field first, 3=MBAFF;  @see encode.cfg::interlace_mode
	int constrained_intra_pred; ///< 1=Inter pixels are not used for Intra macroblock prediction;
	int poc_type;     ///< select POC mode (0/1/2);  @see encode.cfg::poc_type
	int direct_mode; ///< select direct mode for B-frames (0=temporal, 1=spatial); @see encode.cfg::direct_mode

	int chroma_qp_offset; ///< chroma qp offset; @see encode.cfg::chroma_qp_offset

	int interlace_flags; ///< Bitwise interlace flags (@see interlace_flags_e); @see encode.cfg::interlace_flags
	int weighted_pred_flag; ///< weighted prediction flag (0/1); @see encode.cfg::weighted_pred_flag

	int sps_id; ///< select SPS id (0..32);
	int pps_id; ///< select PPS id (0..64);

	/// other flags
	int enc_flags; ///< bitwise combination of special encoder flags; @see enc_flags_e 
	int cpu_disable;
	int gpu_acceleration; ///< 0 - disable, 1 - enable(0 by default)

	// avc intra mode
	int avc_intra_class; ///< avc intra class: 0 - avc intra is off, 50 - class 50, 100 - class 100
	int avc_intra_flags; ///< bitwise combination of avc coding flags; @see avc_intra_flags_e

	input_picture_t input;       ///< input picture description; @see encode.cfg::input
	preproc_settings_t preproc; ///< Preprocessing settings; @see encode.cfg::preproc
	gop_settings_t gop;         ///< GOP settings; @see encode.cfg::gop
	rate_control_settings_t rc; ///< Rate Control settings; @see encode.cfg::rc
	me_settings_t me;           ///< Motion estimation settings; @see encode.cfg::me
	speed_settings_t speed;           ///< speed settings; @see encode.cfg::speed
	slice_settings_t slice;     ///< Slice settings; @see encode.cfg::slice
	deblock_settings_t deblock;  ///< Deblocking filter settings; @see encode.cfg::deblock
	sei_settings_t sei;	///< SEI messages settings; @see encode.cfg::sei
	mt_settings_t mt;	///< multi-threading settings; @see encode.cfg::mt
	frext_t frext;  ///< FRext (fidelity range extensions) settings
	error_resilience_settings_t er;	///< Error resilience settings
	svc_settings_t svc; ///< svc encoding settings
	vssh_vui_seq_parameters_t vui;	///< VUI parameters
	char *ext_data_string;
} v4e_settings_t;

/** @addtogroup enc_func */
// @{

/**
 * Provide default encoder settings into given structure
 * @note "settings->size" should be assigned
 */
int VSSHAPI v4e_default_settings(v4e_settings_t *settings);

/**
 * Load encoder settings from a command line arguments
 * Every argument expected to be in the NAME=VALUE form;
 * Parameters found in arguments list will be changed in "settings" structure;
 * @note "settings->size" should be assigned
 */
int VSSHAPI v4e_read_cmd_line(v4e_settings_t *settings, char *argv[]);

/**
 * Load given configuration text file
 * @return 0=OK, -1=the file could not be opened for read;
 * @note "settings->size" should be assigned
 * @note that function loads only first 1023 lines of text and length of line
 * must be less then 1023 characters
 */
int VSSHAPI v4e_read_config_file(v4e_settings_t *settings, char *filename);

/**
 * Print given settings to a file
 * @param settings - pointer to a structure to fill in;
 * @param filename - name of the file to write to, pass NULL to print to stdout;
 * @return 0=OK
 * @note "settings->size" should be assigned
 */
int VSSHAPI v4e_write_config_file(v4e_settings_t *settings, char *filename);

/**
 * check and correct given encoder settings
 * @return VSSH error code;
 * @note "settings->size" should be assigned
 */
int VSSHAPI v4e_check_settings(v4e_settings_t *settings);

// @}

//////////////////////////////////////////////////////////////////////////
/// maximum length of character buffer required to store encoder settings in text mode
#define V4E_MAX_SETTINGS_SIZE (16*1024)

/** @addtogroup enc_func */
// @{

/**
 * Parse given character string buffer into encoder settings
 * @param settings - encoder settings structure to dump;
 * @param buf - character buffer to parse, expected format: "name=value\n"
 * @param len - length of character buffer, bytes;
 * @return 0=OK;
 * @note "settings->size" should be assigned
 */
int VSSHAPI v4e_buf2settings(v4e_settings_t *settings, char *buf, int len);

/**
* Set encoder parameter by name
* @param settings - encoder settings structure;
* @param name - name of parameter (for ex. "rc.kbps")
* @param namelen - length of parameter name
* @param value - new parameter value
* @return 0=OK;
*/
int VSSHAPI v4e_setparambyname(v4e_settings_t *settings, char *name, int namelen, int value);

/**
 * Dump given encoder settings into character string buffer
 * @param settings - encoder settings structure to dump;
 * @param buf - character buffer to receive settings in text form "name=value; ";
 * @param len - length of character buffer, bytes;
 * @return 0=OK;
 * @note "settings->size" should be assigned
 */
int VSSHAPI v4e_settings2buf(v4e_settings_t *settings, char *buf, int len);

/**
* Get encoder parameter by name
* @param settings - encoder settings structure;
* @param name - name of parameter (for ex. "rc.kbps")
* @param namelen - length of parameter name
* @param value - pointer to return current parameter value
* @return 0=OK;
*/
int VSSHAPI v4e_getparambyname(v4e_settings_t *settings, char *name, int namelen, int* value);

/**
 * settings dump callback function definition
 * @param ctx - application provided context pointer;
 * @param name - name of encoder parameter;
 * @param fmt - format of encoder parameter ("%d", "%x");
 * @param value - integer value of parameter;
 * @param def - default parameter value;
 * @return 0=OK, other value=stop;
 */
typedef int v4e_settings_func_t(void *ctx, char *name, char *fmt, int value, int def);

/**
* Dump given encoder settings via callback function
* @param settings - encoder settings structure to dump;
* @param ctx - application context to pass to callback function;
* @param func - callback function to call on each encoder parameter;
* @return 0=OK;
* @note "settings->size" should be assigned
*/
int VSSHAPI v4e_settings2buf_ex(v4e_settings_t *settings, void *ctx, v4e_settings_func_t func);

// @}

#ifdef __cplusplus
}
#endif

#endif	//__VSSH_SETTINGS_H__
