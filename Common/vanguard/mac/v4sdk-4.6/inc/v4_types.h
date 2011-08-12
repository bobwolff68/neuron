
/**
 * @file v4_types.h
 * Types definitions used in all other code.
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __V4_TYPES_H__
#define __V4_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "v4_platform.h"
#include "v4_pragmas.h"

//#ifndef byte_defined
//typedef unsigned char byte;	///<  8 bit unsigned
//#define byte_defined 1
//#endif

#ifndef timestamp_defined
#define timestamp_defined 1
typedef int64_t timestamp_t;
#endif

#define NO_TIME_STAMP (-1) ///< This means that timestamp is not passed

#if defined(WIN64)
	#define SCONST3264(a) (a##LL)
	#define UCONST3264(a) (a##ULL)
#else
	#define SCONST3264(a) (a##L)
	#define UCONST3264(a) (a##UL)
#endif

// structure member alignment is 8 bytes
#pragma pack(push, v4_types, 8)

// return/error codes
#include "v4_error.h"


/** @name API Version info
* @{
*/
#define V4_API_VERSION_MAGIC		20081107
#define V4_API_VERSION_MAJOR		4
#define V4_API_VERSION_MINOR		1
#define V4_API_VERSION_BUILD		7
/** @} */

/// VSofts H.264 Codec library demo limitations
typedef struct v4_demo_limits_t
{
	int	    enabled;	///< demo limitations enabled? (0/1);
	int	 max_frames;	///< maximum frames processed in one session (0=n/a);
	int	 watermark;	///< watermark enabled? (0/1);
	int 	 expire_enabled;	///< expiration enabled? (0/1);
	int	 expire_date;	///< expiration date, yyyymmdd
} v4_demo_limits_t;

/// platform information
#define V4OS_WIN 1
#define V4OS_LIN 2
#define V4OS_MAC 3
#define V4BITS_32 32
#define V4BITS_64 64
#define V4CC_MSC 1
#define V4CC_ICC 2
#define V4CC_GCC 3

typedef struct v4_platform_info_t
{
	int	 os;		///< OS (0=n/a, 1=win, 2=lin, 3=mac);
	int	 bits;	///< number of bits (0=n/a, 32=32 bits, 64=64 bits);
	int	 cc;		///< compiler used (0=n/a, 1=msc, 2=icc, 3=gcc);
} v4_platform_info_t;

/// VSofts H.264 Codec library capabilities info
typedef struct v4_caps_info_t
{
	int					 size;	///< this structure size, bytes;
	int					 opt;	///< assembly optimizations flags (0=disabled, 1=enabled);
	int					 er;		///< error resilience (encoder) flags (0=disabled, 1=enabled);
	int					 ec;		///< error concealment (decoder) flags (0=disabled, 1=enabled);
	int					 svc;	///< SVC extension enabled (0/1);
	int                  mvc; 	///< MVC extension enabled (0/1);
	int                  extra;		///< EXTRA_BIT_DEPTH enabled (0/1);
	v4_demo_limits_t	 demo_limits;	///< demo limitations (if applied);
	v4_platform_info_t	 platform_info;	///< platform information;
} v4_caps_info_t;


/// Types of slices
typedef enum
{
	P_SLICE = 0,
	B_SLICE,
	I_SLICE,
	SP_SLICE,
	SI_SLICE,
	NUMBER_SLICETYPES
} slice_type_e;

/// "is_last_in_pict" param possible values
typedef enum
{
	NOT_LAST      = 0, ///< not last in any sense
	LAST_IN_LAYER = 1, ///< last in some layer (used in SVC coding
	LAST_IN_PICT  = 2, ///< last in one field (picture), but not last in frame
	LAST_IN_FRAME = 3, ///< last in frame, but not last in "chunk"
	LAST_IN_CHUNK = 4 ///< last in "chunk"
} last_in_pict_e;

typedef struct media_sample_t
{
	int64_t		 timestamp;		///< external timestamp
	void		 *data;				///< data buffer
	struct		 media_sample_t *next; ///< to be used internally
	int			 allocated_size;		///< total allocated size, bytes
	int			 used_size;			///< total used size, bytes
	int			 startcode_emulation_removed;	///< flag signaling whether start code emulation bytes removed or not
	int			 is_last_in_pict;	///< see last_in_pict_e enum
	char		 extra_data[64];	///< can be used for support information
} media_sample_t;

#define SEI_MS_TYPE(ms) ((ms)->extra_data[0])
#define SEI_MS_FLAGS(ms) ((ms)->extra_data[1])
#define SEI_MS_STREAM_ID(ms) ((ms)->extra_data[2])

//define size of buffer additional block to control errors
#define NAL_ERR_PROTECTION_SIZE 1024

/// Generic frame information
typedef struct frame_info_t
{
	int64_t				timestamp;    ///< 64-bit field to hold user-provided timestamp value
	media_sample_t		*sei_list; ///< list of attached sei-messages
	int32_t				frame_num;    ///< logical number of the frame
	int32_t				poc;          ///< poc
	int32_t				num_bits;     ///< number of encoded bits in slice
	int16_t				num_intra_mb; ///< number of intra mbs in encoded frame
	int8_t				idr_flag;     ///< 1=IDR (key) frame, 0=regular frame;
	int8_t				qp_used;      ///< qp used for the frame
	int8_t				slice_type;   ///< type of the slice (see slice_type_e)
	int8_t				is_ref;       ///< 0 - non-ref; 1 - short-term ref; 2 - long term ref
	int8_t				error_no;     //<<!=0 indicates that some error was detected while processing
	int8_t				pic_struct;   ///< picture structure (see standard Table D-1)
	uint8_t				num_stream;	  ///< stream number in multi-stream mode (0 ... MAX_AVC_STREAMS-1)
} frame_info_t;


typedef enum chroma_format_idc_e
{
	YUV_400_IDC = 0,
	YUV_420_IDC = 1,
	YUV_422_IDC = 2,
	YUV_444_IDC = 3,
	YUV_UNKNOWN_FORMAT_IDC = 4
} chroma_format_idc_e;

/// YUV pixel domain image arrays for uncompressed video frame
typedef struct yuv_frame_t
{
	short                width;              ///< luma (allocated) buffer width in pixels, always divisible by 16;
	short                height;             ///< luma (allocated) buffer height in pixels;
	short                image_width;        ///< actual image width in pixels, less or equal to buffer width;
	short                image_height;       ///< actual image height in pixels, less or equal to buffer height;
	short                image_width_uv;     ///< actual image width in pixels, less or equal to buffer width;
	short                image_height_uv;    ///< actual image height in pixels, less or equal to buffer height;
	short                stride_y;           ///< stride of luma lines allocation in pixels!!!
	short                stride_uv;          ///< stride of chroma lines allocation in pixels!!!
	void				 *y;                  ///< pointer to Y component data
	void				 *u;                  ///< pointer to U component data
	void				 *v;                  ///< pointer to V component data
	chroma_format_idc_e  chroma_format_idc;  ///< chroma format idc value
	frame_info_t         info;               ///< frame info structure
	uint32_t             bytes_per_pel;///< sizeof(pel_cmp_t)
	uint32_t			 reserved;
} yuv_frame_t;

/// known colorspace formats
typedef enum
{
	COLORSPACE_E_UNKNOWN = -1,
	COLORSPACE_E_IYUV    =  0,	///< YUV 4:2:0 planar (our internal)
	COLORSPACE_E_YV12    =  1,	///< YUV 4:2:0 planar (U&V swapped);
	COLORSPACE_E_YUY2    =  2,	///< YUV 4:2:2 channel
	COLORSPACE_E_YVYU    =  3,	///< YUV 4:2:2 channel
	COLORSPACE_E_UYVY    =  4,	///< YUV 4:2:2 channel
	COLORSPACE_E_RGB555  =  5,	///< RGB 5:5:5
	COLORSPACE_E_RGB565  =  6,	///< RGB 5:6:5
	COLORSPACE_E_RGB24   =  7,	///< RGB 24 bit
	COLORSPACE_E_RGB32   =  8,	///< RGB 24 bit + alpha channel
	COLORSPACE_E_YUV400  =  9,	///< YUV 4:0:0 planar
	COLORSPACE_E_YUV422  = 10	///< YUV 4:2:2 planar
} colorspace_e;

/// some colorspaces are duplicated
#define COLORSPACE_I420		COLORSPACE_E_IYUV
#define COLORSPACE_IYUV		COLORSPACE_E_IYUV
#define COLORSPACE_YV12		COLORSPACE_E_YV12
#define COLORSPACE_YUY2		COLORSPACE_E_YUY2
#define COLORSPACE_YUYV		COLORSPACE_E_YUY2
#define COLORSPACE_YVYU		COLORSPACE_E_YVYU
#define COLORSPACE_UYVY		COLORSPACE_E_UYVY
#define COLORSPACE_RGB555	COLORSPACE_E_RGB555
#define COLORSPACE_RGB565	COLORSPACE_E_RGB565
#define COLORSPACE_RGB24	COLORSPACE_E_RGB24
#define COLORSPACE_RGB32	COLORSPACE_E_RGB32
#define COLORSPACE_YUV400   COLORSPACE_E_YUV400
#define COLORSPACE_YUV422   COLORSPACE_E_YUV422
#define COLORSPACE_TOTAL	11

#define MAX_SVC_LAYERS 7  ///< Maximum number of SVC layers in encoder
#define MAX_AVC_STREAMS (MAX_SVC_LAYERS + 1) ///< Maximum number of AVC streams in multistream encoding mode
#define MAX_MVC_VIEWS (MAX_SVC_LAYERS + 1) ///< Maximum number of MVC views in MVC encoding mode

/// This structure could be used by encoding application to modify default slice encoding parameters.
/// Negative values like (-1) mean no modification
typedef struct frame_modifier_t
{
	int8_t		 slice_type;	///< type of the slice to use (see slice_type_e)
	int8_t		 idr_flag;	///< keyframe flag: 0 means no key frame, 1 means key frame
	int8_t		 qp;			///< Q(uant) P(arameter) for this frame
	int8_t		 svc_layers_qp[MAX_SVC_LAYERS]; //qp for svc-layers (used only in svc coding)
} frame_modifier_t;

/// Typical values for profile IDC
typedef enum
{
	PROFILE_IDC_BASELINE = 66,
	PROFILE_IDC_MAIN     = 77,
	PROFILE_IDC_EXTENDED = 88,
	PROFILE_IDC_HIGH = 100,
	PROFILE_IDC_HIGH_10 = 110,
	PROFILE_IDC_HIGH_422 = 122,
	PROFILE_IDC_HIGH_444 = 244,
	PROFILE_IDC_CAVLC_444_INTRA = 44,
	PROFILE_IDC_SCALABLE_BASELINE = 83,
	PROFILE_IDC_SCALABLE_HIGH = 86,
	PROFILE_IDC_MULTIVIEW_HIGH = 118,
	PROFILE_IDC_STEREO_HIGH = 128
} profile_idc_e;



typedef enum
{
	SM_4_INTRA_Y = 0,
	SM_4_INTRA_U = 1,
	SM_4_INTRA_V = 2,
	SM_4_INTER_Y = 3,
	SM_4_INTER_U = 4,
	SM_4_INTER_V = 5,
	SM_8_INTRA_Y = 6,
	SM_8_INTER_Y = 7
} scaling_matrix_type_e;


/// Typical values for level IDC
typedef enum
{
	LEVEL_IDC_12 = 12,
	LEVEL_IDC_32 = 32,
	LEVEL_IDC_40 = 40
} level_idc_e;

/// Interlace mode
typedef enum
{
	INTERLACE_MODE_NONE = 0,
	INTERLACE_MODE_ALL_FIELD_TOP_FIRST = 1,
	INTERLACE_MODE_ALL_FIELD_BOTTOM_FIRST = 2,
	INTERLACE_MODE_FRAME_MBAFF = 3
} interlace_mode_e;

typedef enum
{
	MB_TRANSFORM_8X8_OFF = 0,
	MB_TRANSFORM_8X8_ADAPTIVE = 1,
	MB_TRANSFORM_8X8_ONLY = 2,
	MB_TRANSFORM_8X8_ONLY_NO_INTRA_16X16 = 3
} transform8x8_usage_e;


/// Video usability information (see JVT-g050r1)
typedef struct vui_info_t
{
	uint16_t	 sar_width;	///< aspect ratio width;
	uint16_t	 sar_height;	///< aspect ratio height;
	uint32_t	 num_units_in_tick; ///< frame rate divider;
	uint32_t	 time_scale;	///< frames per second(fps) = time_scale/(2*num_units_in_tick);
	uint8_t		 fixed_frame_rate_flag;	///< 0/1;
	uint8_t		 pic_struct_present_flag;
	uint16_t	 reserved;
	//<			 To be expanded... or excluded at all
} vui_info_t;


/// Cropping information of the allocated yuv frames
typedef struct cropping_info_t
{
	int16_t		 luma_offset;	///< offset in pixels of actual luma data;
	int16_t		 chroma_offset;	///< offset in pixels of actual chroma data;
	int16_t		 frame_width;	///< actual frame width;
	int16_t		 frame_height;	///< actual frame height;
} cropping_info_t;

/// Sequence Parameter Set (SPS) information
typedef struct sps_info_t
{
	byte				 seq_parameter_set_id;
	byte				 profile_idc;	///< profile idc
	byte				 level_idc;	///< level
	int8_t				 is_interlace;	///< 0 - progressive; 1 - interlace
	int8_t				 cropping_info_present_flag;	///< whether the cropping info presents
	int8_t				 vui_info_present_flag;	///< whether the vui info presents
	int8_t				 error_no;  ///< non-zero indicates that some error was detected in SPS
	int8_t				 yuv_format; ///< yuv_format (0 = YUV4:0:0, 1 = YUV 4:2:0, 2 = YUV 4:2:2);
	int16_t				 allocated_frame_width;	///< frame width in pixels
	int16_t				 allocated_frame_height;	///< frame height in pixels
	cropping_info_t		 cropping_info;	///< cropping info from sps header
	vui_info_t			 vui_info;	///< vui info (fps and resize can obtained from here);
	uint8_t              bit_depth_luma;
	uint8_t				 bit_depth_chroma;
} sps_info_t;

/// Picture Parameter Set (PPS) information
typedef struct pps_info_t
{
	byte		 pic_parameter_set_id;
	byte		 seq_parameter_set_id;
	byte		 entropy_coding_mode;	///< 0=CAVLC, 1=CABAC;
	byte		 pic_order_present_flag;

	byte		 num_slice_groups;
	int8_t		 pic_init_qp;
	int8_t		 pic_init_qs;
	int8_t		 chroma_qp_index_offset;

	byte		 deblocking_filter_parameters_present_flag;
	byte		 constrained_intra_pred_flag;
	byte		 redundant_pic_cnt_present_flag;
	byte		 weighted_pred_flag;

	byte		 weighted_bipred_idc;
	byte		 transform_8x8_mode_flag;
	byte		 pic_scaling_matrix_present_flag;
	int8_t		 second_chroma_qp_index_offset;
} pps_info_t;

/**
* Structure to receive PPS/SPS NAL units contents
*/
typedef struct v4_spps_data_t
{
	byte		 *data;	///< encoder will place here a pointer to actual (encoded) bits;
	int			 size;	///< number of bytes used;
} v4_spps_data_t;

/// SEI messages
typedef enum
{
	SEI_BUFFERING_PERIOD = 0,
	SEI_PICTURE_TIMING = 1,
	SEI_USER_DATA_REGISTERED_ITU_T_T35 = 4,
	SEI_USER_DATA_UNREGISTERED = 5,
	SEI_RECOVERY_POINT = 6,
	SEI_FILM_GRAIN = 19,
	SEI_DEBLOCKING_FILTER_DISPLAY_PREFERENCE = 20,
	SEI_POST_FILTER = 22,
	// Annex G: SVC
	SEI_SCALABILITY_INFO = 24,
	SEI_SCALABLE_NESTING = 30,
	// Annex H: MVC
	SEI_PARALLEL_DECODING_INFO = 36,
	SEI_MVC_SCALABLE_NESTING = 37,
	SEI_VIEW_SCALABILITY_INFO = 38,
	SEI_MULTIVIEW_SCENE_INFO = 39,
	SEI_MULTIVIEW_ACQUISITION_INFO = 40,
	SEI_NON_REQUIRED_VIEW_COMPONENT = 41,
	SEI_VIEW_DEPENDENCY_CHANGE = 42,
	SEI_OPERATION_POINT_NOT_PRESENT = 43,
	SEI_BASE_VIEW_TEMPORAL_HRD = 44,
	// Annex D
	SEI_FRAME_PACKING_ARRANGEMENT = 45
} sei_type_e;

/// maximum number of CC items per frame: 5 bits = [0..31]
#define MAX_CC_ITEMS	(31)

/// maximum allowed length of one Private Data message
#define MAX_PD_LEN (64*1024)

/// ITU_T_T35 data support according to "CS-TSG-659r2.pdf" document;
/// also see ATSC standards: "A/53 part 4", "A/72 part 1" and related documents;
typedef struct cc_item_t
{
	byte		 cc_valid;
	byte		 cc_type;
	byte		 cc_data_1;
	byte		 cc_data_2;
} cc_item_t;

typedef struct cc_data_t
{
	int cc_count;
	cc_item_t cc_items[MAX_CC_ITEMS];
} cc_data_t;

typedef struct bar_data_t
{
	uint8_t		 top_bar_flag;
	uint8_t		 bottom_bar_flag;
	uint8_t		 left_bar_flag;
	uint8_t		 right_bar_flag;
	uint16_t	 line_number_end_of_top_bar;
	uint16_t	 line_number_start_of_bottom_bar;
	uint16_t	 pixel_number_end_of_left_bar;
	uint16_t	 pixel_number_start_of_right_bar;
} bar_data_t;

#define ATSC1_TYPE_CC  0x03
#define ATSC1_TYPE_BAR 0x06

typedef struct atsc1_data_t
{
	uint32_t user_data_type_code;
	union
	{
		cc_data_t  cc_data;
		bar_data_t bar_data;
	} user_data_type_structure;
} atsc1_data_t;

typedef struct afd_data_t
{
	uint8_t		 active_format_flag;
	uint8_t		 active_format;
} afd_data_t;

#define ITU_T_T35_UID_ATSC1 0x47413934
#define ITU_T_T35_UID_AFD   0x44544731

#define BLUE_RAY_UID_CC				0x47413934
#define BLUE_RAY_GOP_MAP			0x47534D50
#define BLUE_RAY_OFFSET_METADATA	0x4F464D44

typedef struct itu_t_t35_t
{
	uint16_t  country_code;
	uint16_t provider_code;
	uint32_t user_identifier;
	union
	{
		atsc1_data_t atsc1_data;
		afd_data_t   afd_data;
	} user_structure;
} itu_t_t35_t;

typedef struct user_data_unregistered_t
{
	uint8_t uuid_iso_iec_11578[16];
	// Blue-ray specific
	uint32_t type_indicator;
	//cc_data_t cc_data;
	//GOP_structure_map_t GOP_structure_map;
	//offset_metadata_t offset_metadata;
} user_data_unregistered_t;

/// picture timestamp fields
typedef struct pic_timestamp_t
{
	int64_t		 timestamp;

	int32_t		 time_offset;

	byte		 clock_timestamp_flag;
	byte		 ct_type;
	byte		 nuit_field_based_flag;
	byte		 counting_type;

	byte		 full_timestamp_flag;
	byte		 discontinuity_flag;
	byte		 cnt_dropped_flag;
	byte		 n_frames;

	byte		 seconds_flag;
	byte		 seconds_value;
	byte		 minutes_flag;
	byte		 minutes_value;

	byte		 hours_flag;
	byte		 hours_value;
	int16_t		 reserved16;

	int32_t		 reserved32;

} pic_timestamp_t;

/// Supplemental Enhancement Information (SEI) message: picture timing
typedef struct sei_pic_timing_t
{
	uint32_t			 pic_struct;
	uint32_t			 cpb_removal_delay;
	uint32_t			 dpb_output_delay;
	uint32_t			 reserved;
	pic_timestamp_t		 pic_timestamp[3];
} sei_pic_timing_t;

typedef struct bp_delay_t
{
	uint32_t	 initial_cpb_removal_delay;
	uint32_t	 initial_cpb_removal_delay_offset;
} bp_delay_t;

typedef struct sei_buffering_period_t
{
	int32_t		 seq_parameter_set_id;
	bp_delay_t	 nal_bp[32];
	bp_delay_t	 vcl_bp[32];
} sei_buffering_period_t;

typedef struct sei_recovery_point_t
{
	byte		 recovery_frame_cnt;
	byte		 exact_match_flag;
	byte		 broken_link_flag;
	byte		 changing_slice_group_idc;
} sei_recovery_point_t;

typedef struct sei_film_grain_t
{
	byte		 film_grain_characteristics_cancel_flag;
	byte		 model_id;

	byte		 separate_colour_description_present_flag;
	byte		 film_grain_bit_depth_luma_minus8;
	byte		 film_grain_bit_depth_chroma_minus8;
	byte		 film_grain_full_range_flag;
	byte		 film_grain_colour_primaries;
	byte		 film_grain_transfer_characteristics;
	byte		 film_grain_matrix_coefficients;

	byte		 blending_mode_id;
	byte		 log2_scale_factor;
	byte		 comp_model_present_flag[3];
	byte		 num_intensity_intervals_minus1[3];
	byte		 num_model_values_minus1[3];
	byte		 intensity_interval_lower_bound[3][256];
	byte		 intensity_interval_upper_bound[3][256];
	unsigned int film_grain_characteristicts_repetition_period;
	int			 comp_model_value[3][256][6];
} sei_film_grain_t;

typedef struct sei_post_filter_t
{
	unsigned int filter_hint_size_y;
	unsigned int filter_hint_size_x;
	byte filter_hint_type;
	byte additional_extension_flag; // should be equal to 0
	int filter_hint[3][16][16]; // [c][cy][cx]
} sei_post_filter_t;

typedef struct sei_frame_packing_arrangement_t
{
	unsigned int frame_packing_arrangement_id;
	byte frame_packing_arrangement_cancel_flag;

	byte frame_packing_arrangement_type;
	byte quincunx_sampling_flag;
	byte content_interpretation_type;
	byte spatial_flipping_flag;
	byte frame0_flipped_flag;
	byte field_views_flag;
	byte current_frame_is_frame0_flag;
	byte frame0_self_contained_flag;
	byte frame1_self_contained_flag;

	byte frame0_grid_position_x;
	byte frame0_grid_position_y;
	byte frame1_grid_position_x;
	byte frame1_grid_position_y;

	byte frame_packing_arrangement_reserved_byte;
	int frame_packing_arrangement_repetition_period;

	byte frame_packing_arrangement_extension_flag;
}sei_frame_packing_arrangement_t;

typedef struct sei_deblocking_filter_display_preference_t
{
	byte		 deblocking_display_preference_cancel_flag;
	byte		 display_prior_to_deblocking_preferred_flag;
	byte		 dec_frame_buffering_constraint_flag;
	byte		 reserves;
	unsigned int deblocking_display_preference_repetition_period;
} sei_deblocking_filter_display_preference_t;

typedef struct sei_layer_info_t
{
	byte layer_id; //ue(v)
	byte priority_id; //u(6)
	byte discardable_flag; //u(1)
	byte dependency_id; //u(3)
	byte quality_id; //u(4)
	byte temporal_id; //u(3)
	byte sub_pic_layer_flag; //u(1)
	byte sub_region_layer_flag; //u(1)
	byte iroi_division_info_present_flag; //u(1)
	byte profile_level_info_present_flag; //u(1)
	byte bitrate_info_present_flag; //u(1)
	byte frm_rate_info_present_flag; //u(1)
	byte frm_size_info_present_flag; //u(1)
	byte layer_dependency_info_present_flag; //u(1)
	byte parameter_sets_info_present_flag; //u(1)
	byte bitstream_restriction_info_present_flag; //u(1)
	byte exact_inter_layer_pred_flag; //u(1)
	byte exact_sample_value_match_flag; //u(1)
	byte layer_conversion_flag; //u(1)
	byte layer_output_flag; //u(1)

	//if (profile_level_info_present_flag[ i ])
	int layer_profile_level_idc;  //u(24)
	//if( bitrate_info_present_flag[ i ] ) {
	uint16_t avg_bitrate; //u(16)
	uint16_t max_bitrate_layer; //u(16)
	uint16_t max_bitrate_layer_representation; //u(16)
	uint16_t max_bitrate_calc_window;//u(16)
	//}
	//if( frm_rate_info_present_flag[ i ] ) {
	byte constant_frm_rate_idc; // u(2)
	uint16_t avg_frm_rate; //u(16)
	//}
	//if( frm_size_info_present_flag[ i ] || iroi_division_info_present_flag[ i ] ) {
	byte frm_width_in_mbs_minus1; // ue(v)
	byte frm_height_in_mbs_minus1; // ue(v)
	//}

	//if (layer_dependency_info_present_flag[i])
	byte num_directly_dependent_layers; //ue(v)
	byte directly_dependent_layer_id_delta_minus1[MAX_SVC_LAYERS]; //ue(v)
	//else
	byte layer_dependency_info_src_layer_id_delta; //ue(v)
	//if(!parameter_sets_info_present_flag[ i ])
	byte parameter_sets_info_src_layer_id_delta; //ue(v)


} sei_layer_info_t;

typedef struct scalable_sei_info_t
{
	int temporal_id_nesting_flag; //u(1)
	int priority_layer_info_present_flag; //u(1)
	int priority_id_setting_flag; //u(1)
	int num_layers_minus1; //ue(v)
	sei_layer_info_t layers[255]; //maximal number of SVC layers in this structure
} scalable_sei_info_t;

typedef struct sei_scalable_nesting_t
{
	byte				 all_layer_representations_in_au_flag;
	byte				 sei_dependency_id[255]; //MAX_SVC_LAYERS+1 is enough
	byte				 num_layer_representations_minus1;
	byte				 sei_quality_id[255];
	byte				 sei_temporal_id;
	byte				 sei_reserved[3];
	media_sample_t		 *next_sei_message;
} sei_scalable_nesting_t;

typedef struct sei_mvc_scalable_nesting_t
{
	byte				operation_point_flag;
	byte				all_view_components_in_au_flag;
	byte				num_view_components_minus1;
	uint16_t			sei_view_id[MAX_MVC_VIEWS];
	byte				num_view_components_op_minus1;
	uint16_t			sei_op_view_id[MAX_MVC_VIEWS];
	byte				sei_op_temporal_id;
	media_sample_t		*single_sei_message;
} sei_mvc_scalable_nesting_t;

typedef enum
{
	SEI_FLAG_BOTTOM_FIELD = 1,  ///< sei is related to bottom field
	SEI_FLAG_COMPRESSED   = 2,  ///< data in sei media sample contains compressed data (otherwise the structure above)
	SEI_FLAG_SEPARATE_NAL = 4   ///< this sei must be coded in separate nal-unit
} sei_coding_flags_e;



#pragma pack(pop, v4_types)

/**
 * recommended allocation method for media frames
 * @param len - size of requested memory block in bytes
 * @return pointer to allocated aligned memory block, NULL=not enough memory;
 */
void * VSSHAPI v4_malloc(int len);

/**
 * recommended free method for memory blocks created by v4_malloc()
 * @param ptr - memory block created by v4_malloc();
 * @return void
 */
void  VSSHAPI v4_free(void *ptr);

/**
 * Obtain library version information
 * @param pbuild - pointer to variable to receive 4-digit build number (nnnn);
 * @param pmagic - pointer to variable to receive build date (yyyymmdd);
 * @return 0=OK;
 */
int VSSHAPI v4_get_version(int *pbuild, int *pmagic);

/**
* Obtain library capabilities info
* @param caps - address of structure to receive capabilities information, not NULL;
* @param size - size of structure, bytes;
* @return VSSH_OK, VSSH_ERR_ARG, VSSH_ERR_MEMORY when size of structures does not match;
*/
int VSSHAPI v4_get_caps(v4_caps_info_t *caps, int size);

/**
* @return text string describing given return code
*/
char * VSSHAPI v4_error_text(int rc);


#ifdef __cplusplus
}
#endif

#endif //__V4_TYPES_H__
