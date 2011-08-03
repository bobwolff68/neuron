
/**
 * @file v4_nalu.h
 * NAL Unit public API
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */



#ifndef __V4_NALUNIT_H__
#define __V4_NALUNIT_H__

#include "v4_types.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MAXSPS	 32
#define MAXPPS	256
#define MAX_VALUE_OF_CPB_CNT	32
#define TMP_MAX_NUM_REFS	33 //One extra element is needed for convinience
#define MAX_NUM_SLICE_GROUPS	 8
#define MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE  256

/// NALU (network abstraction layer unit) types
typedef enum
{
	NALU_TYPE_EMPTY				= 0,	// empty NAL unit
	NALU_TYPE_SLICE				= 1,
	NALU_TYPE_DPA				= 2,
	NALU_TYPE_DPB				= 3,
	NALU_TYPE_DPC				= 4,
	NALU_TYPE_IDR				= 5,
	NALU_TYPE_SEI				= 6,
	NALU_TYPE_SPS				= 7,
	NALU_TYPE_PPS				= 8,
	NALU_TYPE_PD				= 9,
	NALU_TYPE_ESEQ				= 10,
	NALU_TYPE_ESTRM				= 11,
	NALU_TYPE_FILL				= 12,
	NALU_TYPE_PREFIX_EXT		= 14,
	NALU_TYPE_SUBSPS			= 15,
	/* 16...19 -> Reserved */
	NALU_TYPE_SLICE_EXT			= 20,
	NALU_TYPE_MVC_PD            = 24,       //0x18 delimiter in MVC dependant stream 
	/* 24...29 -> RFC-3984 */
	NALU_TYPE_STAP_A			= 24,		// Single-time aggregation packet
	NALU_TYPE_STAP_B			= 25,		// Single-time aggregation packet
	NALU_TYPE_MTAP16			= 26,		// Multi-time aggregation packet
	NALU_TYPE_MTAP24			= 27,		// Multi-time aggregation packet
	NALU_TYPE_FU_A				= 28,		// Fragmentation unit
	NALU_TYPE_FU_B				= 29,		// Fragmentation unit
	/* 30..31 -> draft-ietf-avt-rtp-svc-18 */
	NALU_TYPE_PACSI				= 30,		// Payload Content Scalability Information
	NALU_TYPE_EXTENSION			= 31	// see next octet (byte) for Subtype
	//+---------------+
	//|0|1|2|3|4|5|6|7|
	//+-+-+-+-+-+-+-+-+
	//| Subtype |J|K|L|
	//+---------------+
} nalu_type_e;

/**
 * NALU (network abstraction layer unit) structure
 * Note: This structure is obsolete and is defined for compatibility with VSS codec version 3.
 * In version 4 it is better to use media_sample_t instead.
*/
typedef struct nal_unit_t
{
	uint32_t	 len;	///< length of the NALU in bytes (excluding start code, which does not belong to the NALU)
	byte		 *buf;		///< points to the first byte of the EBSP (right after startcode)
	int64_t		 timestamp; ///< timestamp of this nal_unit
} nal_unit_t;

/**
 * nal unit destructor function prototype
 * appropriate callback function must be set via decoder_settings
 * if decoder is opened in "external nals" mode
*/
typedef void release_nal_unit_t(nal_unit_t *nal_unit);

/// macro to determine NAL unit type
#define NAL_UNIT_TYPE(nal_unit)     ((nal_unit)->buf[0]&0x1f)

/// macro to determine NAL unit type by the first byte
#define NALU_TYPE(first_byte)     (first_byte&0x1f)
/// macro to determine NAL unit reference IDC by the first byte
#define NALU_RIDC(first_byte)     ((first_byte>>5)&0x3)

/// macro to determine NAL unit reference idc
#define NAL_REFERENCE_IDC(nal_unit) (((nal_unit)->buf[0]>>5)&3)

#define NAL_SVC_EXTENSION_FLAG(nal_unit) ((nal_unit)->buf[1]>>7)

/** @name Extract NAL header
 * Macros below help to extract from compress media samples which represent H.264 NAL-units.
 * For instance, to determine if NAL is IDR and reference slice use the following code:
 * <pre><code>
 *     media_sample_t *ms;
 *     v4e_get_nal(handle, &ms);
 *     if (ms) {
 *	       int is_idr = (NAL_TYPE(ms->data) == NALU_TYPE_SLICE);
 *         int is_ref = NAL_IS_REF(ms->data);
 *         .....
 * </code></pre>
 */
//@{

/// Extracts NAL unit type (see nalu_type_e enum)
#define NAL_TYPE(data) (((byte *)(data))[0]&0x1f)

/// Determines if it is reference NAL unit. Result can be 0,1,2,3;  0 � means non-reference
#define NAL_IS_REF(data) ((((byte *)(data))[0]>>5)&3)

//@}


/** @name Extract NAL SVC
 * Macros below help to extract data from SVC NAL units. They can be used
 * only for units of type NALU_TYPE_PREFIX_EXT and NALU_TYPE_SLICE_EXT
 */
//@{

#define  NAL_EXT_SVS_EXTENSION_FLAG(data) ((((byte *)(data))[1]>>7)&1)
#define	 NAL_EXT_IDR(data) ((((byte *)(data))[1]>>6)&1)
#define	 NAL_EXT_PRIORITY_ID(data) (((byte *)(data))[1]&0x3f)
#define	 NAL_EXT_NO_INTER_LAYER_PREDICTION(data) (((byte *)(data))[2]>>7)
#define	 NAL_EXT_DEPENDENCY_ID(data) ((((byte *)(data))[2]>>4)&7)
#define	 NAL_EXT_QUALITY_ID(data) (((byte *)(data))[2]&0xf)
#define	 NAL_EXT_DQ_ID(data) (((byte *)(data))[2]&0x7f)
#define	 NAL_EXT_TEMPORAL_ID(data) (((byte *)(data))[3]>>5)
#define	 NAL_EXT_USE_REF_BASE_PIC(data) ((((byte *)(data))[3]>>4)&1)
#define	 NAL_EXT_DISCARDABLE(data) ((((byte *)(data))[3]>>3)&1)
#define	 NAL_EXT_OUTPUT(data) ((((byte *)(data))[3]>>2)&1)

#define  NAL_MVC_EXT_NON_IDR(data) ((((byte *)(data))[1]>>6)&1)
#define	 NAL_MVC_EXT_PRIORITY_ID(data) (((byte *)(data))[1]&0x3f)
#define  NAL_MVC_EXT_VIEW_ID(data) (((((byte *)(data))[3]>>6)&3) | (((byte *)(data))[2] << 2))
#define  NAL_MVC_EXT_TEMPORAL_ID(data) ((((byte *)(data))[3]>>3)&7)
#define  NAL_MVC_EXT_ANCHOR_PIC(data)  ((((byte *)(data))[3]>>2)&1)
#define  NAL_MVC_EXT_INTER_VIEW(data)  ((((byte *)(data))[3]>>1)&1)
#define  NAL_MVC_EXT_RESERVED_BIT(data) (((byte *)(data))[3]&1)

//@}


#define MAX_NUM_REF_FRAMES_IN_PIC_ORDER_CNT_CYCLE  256

/** @name MMCO
* memory management control operation
*/
//@{

typedef struct mmco_node_t
{
	uint16_t	 val;  ///< parameter of operation
	byte		 lt_idx;  ///< extra param for mmco_3 operation
	byte		 code; ///< code of the mmco operation
} mmco_node_t;

typedef struct mmco_list_t
{
	mmco_node_t	 *mmco; ///< array of mmco operations
	int			 curr_num;
	int			 allocated_num;
} mmco_list_t;

// @}

///how many items to add at each realloc
#define REALLOC_SIZE 64

typedef struct wp_params_t
{
	uint32_t	 luma_log2_weight_denom;          // ue(v)
	uint32_t	 chroma_log2_weight_denom;        // ue(v)

	byte		 luma_weight_l0_flag[TMP_MAX_NUM_REFS];
	byte		 chroma_weight_l0_flag[TMP_MAX_NUM_REFS];
	byte		 luma_weight_l1_flag[TMP_MAX_NUM_REFS];
	byte		 chroma_weight_l1_flag[TMP_MAX_NUM_REFS];

	int16_t		 weight_list0[TMP_MAX_NUM_REFS][3];     // se(v)
	int16_t		 weight_list1[TMP_MAX_NUM_REFS][3];     // se(v)
	int16_t		 offset_list0[TMP_MAX_NUM_REFS][3];  // se(v)
	int16_t		 offset_list1[TMP_MAX_NUM_REFS][3];  // se(v)

	int			 is_weighted_fwd;
	int			 is_weighted_bwd;

	int			 wp_round_luma;
	int			 wp_round_chroma;
} wp_params_t;

typedef struct svc_slice_ext_t
{
	byte		 is_svc_slice;
	byte		 dq_id; //(dependency_id*16+quality_id) see standard defenition 
	byte		 base_pred_weight_table_flag;
	byte		 disable_interlayer_deblocking_filter_idc;
	char		 inter_layer_slice_alpha_c0_offset_div2;
	char		 inter_layer_slice_beta_offset_div2;
	byte		 tcoeff_level_prediction_flag;
	byte		 constrained_intra_resampling;
	byte		 base_chroma_phases;
	int			 scaled_base_left_offset;
	int			 scaled_base_top_offset;
	int			 scaled_base_right_offset;
	int			 scaled_base_bottom_offset;
	byte		 slice_skip_flag;
	int8_t		 base_mode_flag; ///< -1 means "adaptive"; if >= 0 - means default
	int8_t		 mv_pred_flag;   ///<  --- " ---
	int8_t		 residual_pred_flag; ///< ---- " -----
	byte		 adaptive_rewrite_flag;
	byte		 no_inter_layer_pred_flag;
	byte		 store_ref_base_pict_flag;
	byte		 use_ref_base_pic_flag;
	byte		 ref_layer_dq_id;
	byte		 scan_idx_start;
	byte		 scan_idx_end;
	byte		 priority_id;
	byte		 temporal_id;
	byte		 discardable;
	byte		 output;
	int			 num_mbs;
	mmco_list_t	 base_mmco_list;
} svc_slice_ext_t;

typedef struct mvc_slice_ext_t
{
	byte         is_mvc_slice;
	byte         priority_id;
	int          view_ix;
	int          view_id;
	byte         temporal_id;
	byte         anchor_pic_flag;
	byte         inter_view_flag;
	int          num_inter_view_refs[2];
	int         *inter_view_refs[2];
} mvc_slice_ext_t;

#define QUALITY_ID(dq_id) ((dq_id)&0xf)
#define DEPENDENCY_ID(dq_id) ((dq_id)>>4)


typedef struct slice_params_t
{
	byte				 slice_type;
	byte				 field_pic_flag;
	byte				 bottom_field_flag;
	byte				 disable_deblocking_filter_idc;
	char				 slice_alpha_c0_offset_div2;
	char				 slice_beta_offset_div2;
	char				 qp;
	byte				 idr_flag;
	int					 first_mb_in_slice;
	int                  expected_last_mb_in_slice;
	byte				 num_ref_pic_active_fwd;
	byte				 num_ref_pic_active_bwd;
	byte				 pic_parameter_set_id;
	byte				 redundant_pic_cnt;
	byte				 direct_spatial_mv_pred_flag;
	byte				 long_term_reference_flag;
	byte				 model_number;
	byte				 no_output_of_prior_pics_flag;
	byte				 adaptive_ref_pic_buffering_flag;
	byte				 ref_pic_list_reordering_flag[2];
	uint16_t			 frame_num;
	uint16_t			 idr_pic_id;
	uint16_t			 pic_order_cnt_lsb;
	uint16_t			 slice_group_change_cycle;
	int					 delta_pic_order_cnt_bottom;
	int					 delta_pic_order_cnt[2];
	mmco_list_t			 mmco_list;
	uint16_t			 remapping_of_pic_nums_idc[2][TMP_MAX_NUM_REFS];
	uint16_t			 abs_diff_pic_num[2][TMP_MAX_NUM_REFS];
	uint16_t             abs_diff_view_idx[2][TMP_MAX_NUM_REFS];
	uint16_t			 long_term_pic_idx[2][TMP_MAX_NUM_REFS];
	byte				 is_ref;
	wp_params_t			 wp_params;
	uint8_t				 is_fmo;
	svc_slice_ext_t		 svc_ext;
	mvc_slice_ext_t      mvc_ext;
} slice_params_t;


/// HRD parameters
typedef struct vssh_hrd_parameters_t
{
	byte		 cpb_cnt;
	byte		 bit_rate_scale;
	byte		 cpb_size_scale;
	uint32_t	 bit_rate_value[MAX_VALUE_OF_CPB_CNT];
	uint32_t	 cpb_size_value[MAX_VALUE_OF_CPB_CNT];
	byte		 cbr_flag[MAX_VALUE_OF_CPB_CNT];
	uint32_t	 initial_cpb_removal_delay_length;
	uint32_t	 cpb_removal_delay_length;
	uint32_t	 dpb_output_delay_length;
	uint32_t	 time_offset_length;
} vssh_hrd_parameters_t;

/// VUI parameters
typedef struct vssh_vui_seq_parameters_t
{
	byte						 aspect_ratio_info_present_flag;
	byte						 aspect_ratio_idc;
	uint16_t					 sar_width;
	uint16_t					 sar_height;

	byte						 overscan_info_present_flag;
	byte						 overscan_appropriate_flag;
	byte						 video_signal_type_present_flag;
	byte						 video_format;
	byte						 video_full_range_flag;

	byte						 colour_description_present_flag;
	byte						 colour_primaries;
	byte						 transfer_characteristics;
	byte						 matrix_coefficients;

	byte						 chroma_loc_info_present_flag;
	byte						 chroma_sample_loc_type_top_field;
	byte						 chroma_sample_loc_type_bottom_field;

	byte						 timing_info_present_flag;
	byte						 fixed_frame_rate_flag;
	byte						 nal_hrd_parameters_present_flag;
	byte						 vcl_hrd_parameters_present_flag;
	byte						 low_delay_hrd_flag;
	byte						 pic_struct_present_flag;

	byte						 bitstream_restriction_flag;
	byte						 motion_vectors_over_pic_boundaries_flag;
	byte						 max_bytes_per_pic_denom;
	byte						 max_bits_per_mb_denom;
	byte						 log2_max_mv_length_vertical;
	byte						 log2_max_mv_length_horizontal;
	byte						 num_reorder_frames;
	byte						 max_dec_frame_buffering;

	uint32_t					 num_units_in_tick;
	uint32_t					 time_scale;
	vssh_hrd_parameters_t		 nal_hrd_parameters;
	vssh_hrd_parameters_t		 vcl_hrd_parameters;
} vssh_vui_seq_parameters_t;

typedef struct svc_sps_ext_t
{
	byte		 is_subsps;
	byte		 interlayer_deblocking_filter_control_present_flag;
	byte		 extended_spatial_scalability;
	byte		 chroma_phases;
	byte		 base_chroma_phases;
	byte		 seq_tcoeff_level_prediction_flag;
	byte		 adaptive_tcoeff_level_prediction_flag;
	byte		 slice_header_restriction_flag;
	int			 scaled_base_left_offset;
	int			 scaled_base_top_offset;
	int			 scaled_base_right_offset;
	int			 scaled_base_bottom_offset;
} svc_sps_ext_t;

typedef struct view_refs_info_t
{
	int view_id;
	int num_anchor_refs_l0;
	int num_anchor_refs_l1;
	int anchor_ref_l0[15];
	int anchor_ref_l1[15];
	int num_non_anchor_refs_l0;
	int num_non_anchor_refs_l1;
	int non_anchor_ref_l0[15];
	int non_anchor_ref_l1[15];
} view_refs_info_t;

typedef struct operation_points_info_t
{
	uint8_t applicable_op_temporal_id;
	uint16_t applicable_op_num_target_views;   // applicable_op_num_target_views <= 1024
	uint16_t *applicable_op_target_view_id;
	uint16_t applicable_op_num_views;          // applicable_op_num_views <= 1024
} operation_points_info_t;

typedef struct level_info_t
{
	uint8_t level_idc;
	uint16_t num_applicable_ops;               // num_applicable_ops <= 1024
	operation_points_info_t *ops_info;
} level_info_t;

typedef struct mvc_sps_ext_t
{
	uint16_t num_views;                        // num_views <= 1024
	view_refs_info_t *view_refs_info;

	uint8_t num_level_values_signalled;       // num_level_values_signalled <= 64
	level_info_t *level_info;
} mvc_sps_ext_t;

/// Sequence Parameter Set structure
typedef struct seq_parameter_set_t
{
	int16_t						 error_no;		///< 0 indicates whether the parameter set is valid , 0 means OK;
	uint16_t					 pic_width_in_mbs;
	uint16_t					 frame_height_in_mbs;
	uint16_t					 pic_height_in_map_units;

	byte						 profile_idc;
	byte						 level_idc;
	byte						 seq_parameter_set_id;
	byte						 log2_max_frame_num;

	byte						 pic_order_cnt_type;
	byte						 log2_max_pic_order_cnt_lsb;
	byte						 delta_pic_order_always_zero_flag;
	byte						 num_ref_frames_in_pic_order_cnt_cycle;

	byte						 num_ref_frames;
	byte						 gaps_in_frame_num_value_allowed_flag;
	byte						 frame_mbs_only_flag;
	byte						 mb_adaptive_frame_field_flag;

	byte						 direct_8x8_inference_flag;
	byte						 vui_parameters_present_flag;
	byte						 constrained_set0_flag;
	byte						 constrained_set1_flag;

	byte						 constrained_set2_flag;
	byte						 constrained_set3_flag;
	byte						 constrained_set4_flag;
	byte						 constrained_set5_flag;
	byte						 reserved_zero2;
	byte						 frame_cropping_flag;

	uint16_t					 frame_cropping_rect_left_offset;
	uint16_t					 frame_cropping_rect_right_offset;
	uint16_t					 frame_cropping_rect_top_offset;
	uint16_t					 frame_cropping_rect_bottom_offset;

	int							 offset_for_non_ref_pic;
	int							 offset_for_top_to_bottom_field;

	chroma_format_idc_e			 chroma_format_idc;
	byte						 bit_depth_luma_minus8;
	byte						 bit_depth_chroma_minus8;
	byte						 qpprime_y_zero_transform_bypass_flag;

	byte						 seq_scaling_matrix_present_flag;

	int							 offset_for_ref_frame[256];

	byte						 seq_scaling_list_present_flag[8];
	byte						 scaling_list4x4[6][16];
	byte						 scaling_list8x8[2][64];
	byte						 use_default_scaling_matrix4x4_flag[6];
	byte						 use_default_scaling_matrix8x8_flag[2];

	byte						 dpb_size;

	vssh_vui_seq_parameters_t	 vui_info;	  ///< vui_seq_parameters_t
	svc_sps_ext_t				 svc_ext;

	mvc_sps_ext_t                mvc_ext;
} seq_parameter_set_t;

#define MAX_NUM_SLICE_GROUPS/*_MINUS1*/  8

typedef struct pic_parameter_set_t
{
	int16_t		 error_no;	  ///< indicates the parameter set is valid
	byte		 pic_parameter_set_id;				  // ue(v)
	byte		 seq_parameter_set_id;				  // ue(v)
	byte		 entropy_coding_mode;					  // u(1)
	//			 if( pic_order_cnt_type < 2 )  in the sequence parameter set
	byte		 pic_order_present_flag;					  // u(1)
	byte		 num_slice_groups;					  // ue(v)
	byte		 slice_group_map_type;				  // ue(v)
	byte		 slice_group_change_direction_flag;		  // u(1)
	char		 pic_init_qp;							  // se(v)
	char		 pic_init_qs;							  // se(v)
	char		 chroma_qp_index_offset;					  // se(v)
	byte		 deblocking_filter_parameters_present_flag;// u(1)
	byte		 constrained_intra_pred_flag;			  // u(1)
	byte		 redundant_pic_cnt_present_flag;			  // u(1)
	byte		 weighted_pred_flag;						  // u(1)
	byte		 weighted_bipred_idc;					  // u(2)

	uint16_t	 run_length[MAX_NUM_SLICE_GROUPS];
	uint16_t	 top_left[MAX_NUM_SLICE_GROUPS];
	uint16_t	 bottom_right[MAX_NUM_SLICE_GROUPS];
	//			 else if( slice_group_map_type = = 3 || 4 || 5
	uint32_t	 slice_group_change_rate;			  // ue(v)
	int			 last_slice_group_change_cycle; ///< used to check if we need to racalculate map

	//			 else if( slice_group_map_type = = 6 )
	uint32_t	 num_slice_group_map_units;			  // ue(v)
	byte		 *map_unit_to_slice_group_map; ///< FMO map (alocated here)
	uint32_t	 num_ref_idx_active[2];				  // ue(v)

	byte		 transform_8x8_mode_flag;				  // u(1)
	byte		 pic_scaling_matrix_present_flag;		  // u(1)
	int			 second_chroma_qp_index_offset;			  // se(v)

	byte		 pic_scaling_list_present_flag[8];     // u(1)
	byte		 scaling_list4x4[6][16];                // se(v)
	byte		 scaling_list8x8[2][64];                // se(v)
	byte		 use_default_scaling_matrix4x4_flag[6];
	byte		 use_default_scaling_matrix8x8_flag[2];

} pic_parameter_set_t;

typedef struct scaling_list_t
{
	byte    scaling_list4x4[6][16];
	byte    scaling_list8x8[2][64];
	byte    changing_mask;
} scaling_list_t;

#ifdef __cplusplus
}
#endif

#endif	// __VSSH_NALUNIT_H__
