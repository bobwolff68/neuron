
/**
 * @file sample_ana.c
 * H.264 stream analysis sample application
 * Intended to analyze H.264 streams in Annex B file format,
 * i.e. NAL units separated with 4-byte startcodes.
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2010, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#define _CRT_SECURE_NO_DEPRECATE //to avoid warning C4996: 'fopen' was declared deprecated

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef WIN32
#include <windows.h>
#else
#include <string.h>
#endif

#include <assert.h>

// enable CRT debug for memory leak detection etc. (available under Windows only)
#if defined(WIN32) && defined(DEBUG) && !defined(__GNUC__)
#define _CRT_DEBUG_
#endif
#ifdef _CRT_DEBUG_
#include <crtdbg.h>
//	#include <vld.h>
#endif


#include "v4_nalu.h"
#include "v4d_api.h"
#include "v4info.h"
#include "v4args.h"
#include "v4file.h"
#include "v4_substr_extract.h"

// read buffer length
#define BUFFER_SIZE 4*1024

// assume 4-byte start codes
#define STARTCODE_LEN (4)

// global settings
char *input_file = NULL;	 // input file name

#define MAX_TEMPORAL_ID	7
#define MAX_PRIORITY_ID	63
#define MAX_DQ_ID		127

int bitrate         = 0;	// VBV buffer analyzer: 0=none, >0=bitrate, kbps
int framerate       = 0;	// VBV buffer analyzer: 0=fps from stream, >0 framerate (frames per 1000 sec)
int buffer_analyser = 0;	// 0=none, 1=std, 2=vcl;
int target_t_id     = -1;	// target max temporal_id for analyze, -1=don't care
int target_p_id     = -1;	// target max priority_id for analyze, -1=don't care
int target_dq_id    = -1;	// target maximum dq_id for analyze; -1=don't care
int stream_type     = 0;	// 0=AVC, 1=SVC, 2=MVC 
int is_skip_extractor = 0;	// flag to skip Sub-stream Extractor
static char *stream_type_names[] = 
{
	"no",
	"SVC",
	"MVC"
};

// internal variables
void *dec_handle = NULL; // handle of the decoder instance.
void *input_buffer = NULL;
void *substream_extractor = NULL;
int pic_number  = 0;
int nalu_number = 0;
int nalu_offset = 0;
int num_errors = 0;
int nal_hrd_flag[32];	// nal_hrd_parameters_present_flag for each sps id
int vcl_hrd_flag[32];	// vcl_hrd_parameters_present_flag for each sps id

//////////////////////////////////////////////////// command line arguments
static cmd_arg_t all_args[] =
{
	{ "i", "yf", "input-file",      1, &input_file,      0, "name of input file" },
	{ "v", "vb", "verbose",         0, &verbose,         1, "enable extended verbose messages (0/1)" },
	{ "b", "ba", "buffer-analyzer", 0, &buffer_analyser, 1, "enable HRD buffer analyzer (1=nal, 2=vcl)" },
	{ "l", "sl", "select-layer",    0, &target_p_id,     0, "select SVC/MVC layers/views by priority id (-1=all, 0=base, .. 63=max)" },
	{ "-", "st", "select-t-id",     0, &target_t_id,     0, "select SVC/MVC layers/views by temporal id (-1=all, 0=base, .. 7=max)" },
	{ "d", "dq", "select-dq-id",    0, &target_dq_id,    0, "select SVC layers by dq_id ( = 16*dep_id+quality_id) (-1=all, 0=base, .. 127=max)" },
	{ "-", "br", "bitrate",         0, &bitrate,         0, "desired bitrate for VBV buffer model, kbps" },
	{ "f", "fr", "framerate",       0, &framerate,       0, "input frame rate (frames per 10,000 sec)" },
};
static const int all_args_cnt = sizeof(all_args)/sizeof(all_args[0]);

static void usage(void)
{
	verbose_print(0,"USAGE:\n	sample_ana [-option value]\n");
	verbose_print(0,"WHERE:\n");
}

static int legend(void)
{
	verbose_print(0,"LEGEND:\n");
	verbose_print(0,"	Every line:\n");
	verbose_print(0,"		file offset(NAL length): [RefIDC|NAL type]\n");
	verbose_print(0,"	SPS[SPS_IDC]:\n");
	verbose_print(0,"	PPS[SPS_IDC,PPS_IDC]:\n");
	verbose_print(0,"		QPU[] - chroma qp offset\n");
	verbose_print(0,"		QPV[] - second chroma qp offset\n");
	verbose_print(0,"		DB    - deblocking parameters present flag\n");
	verbose_print(0,"		CIP   - constrained intra prediction flag\n");
	verbose_print(0,"		WP    - weighted pred flag\n");
	verbose_print(0,"		WB    - weighted bi-pred idc\n");
	verbose_print(0,"		T8x8  - transform 8x8 flag\n");
	verbose_print(0,"		SM    - scaling matrix present flag\n");
	verbose_print(0,"	SLICEEXT: or PREFIX:\n");
	verbose_print(0,"		[Priority][Dependency|Temporal|Quality]\n");
	verbose_print(0,"	SLICEEXT MVC:\n");
	verbose_print(0,"		[Priority][Temporal|View]\n");
	verbose_print(0,"	SEI: (if verbose > 0)\n");
	verbose_print(0,"		SEI #Type (Uncompressed SEI data length): SEI Name: SEI parameters\n");
	return 1;
}
//////////////////////////////////////////////////////////////////////////

static char *nalu_type_names[] = {
	" !ERR!",
	" SLICE",
	"   DPA",
	"   DPB",
	"   DPC",
	"   IDR",
	"   SEI",
	"   SPS",
	"   PPS",
	"PICSEP",
	"ESEQ",
	"ESTRM",
	"FILLER",
	"13",
	"PREFIX",
	"SUBSPS",	// 15
	"16",
	"17",
	"18",
	"19",
	"SLICEEXT",		// 20
	"RESERVED",		// 21
	"RESERVED",		// 22
	"RESERVED",		// 23
	"MVC PD"		// 24
};

static char *sei_type_names[] = 
{
	"Buffering period",							// 0
	"Picture timing",							// 1
	"Pan-scan rectangle",						// 2
	"Filler payload",							// 3
	"User data registered ITU-T Rec. T.35",		// 4
	"User data unregistered",					// 5
	"Recovery point",							// 6
	"Decoded reference picture marking repetition",	// 7
	"Spare picture",							// 8
	"Scene information",						// 9
	"Sub-sequence information",					// 10
	"sub-sequence layer characteristics", 		// 11
	"Sub-sequence characteristics", 			// 12
	"Full frame freeze",						// 13
	"Full frame freeze release",				// 14
	"Full frame snapshot",						// 15
	"Progressive refinement segment start",		// 16
	"Progressive refinement segment end",		// 17
	"Motion constrained slice group set",		// 18
	"Film grain characteristics",				// 19
	"Deblocking filter display preference",		// 20
	"Stereo video information",					// 21
	"Post-filter hint",							// 22
	"Tone mapping information",					// 23
	"SVC scalability information",				// 24
	"SVC sub-picture scalable layer",			// 25
	"SVC non-required layer representation",	// 26
	"SVC priority layer information",			// 27
	"SVC layers not present",					// 28
	"SVC layer dependency change",				// 29
	"SVC scalable nesting",						// 30
	"SVC base layer temporal HRD",				// 31
	"SVC quality layer integrity check",		// 32
	"SVC redundant picture property",			// 33
	"SVC temporal level zero dependency representation index", // 34
	"SVC temporal level switching point",		// 35
	"MVC parallel decoding information",		// 36
	"MVC scalable nesting",						// 37
	"MVC view scalability information",			// 38
	"MVC multiview scene information",			// 39
	"MVC multiview acquisition information",	// 40
	"MVC non-required view component",			// 41
	"MVC view dependency change",				// 42
	"MVC operation point not present",			// 43
	"MVC base view temporal HRD",				// 44
	"Frame packing arrangement"					// 45
};


//////////////////////////////////////////////////////////////////////////
static char *get_slice_name(int slice_type)
{
	switch (slice_type)
	{
	case I_SLICE:  return "I";
	case P_SLICE:  return "P";
	case B_SLICE:  return "B";
	case SI_SLICE: return "SI";
	case SP_SLICE: return "SP";
	}
	return "Unknown";
}
//////////////////////////////////////////////////////////////////////////
static char profile_name_unknown[80];
static char *get_profile_name(byte profile_idc, byte constrained_set3_flag)
{
	switch (profile_idc)
	{
	case PROFILE_IDC_BASELINE:			return "        Baseline";
	case PROFILE_IDC_MAIN:				return "            Main";
	case PROFILE_IDC_EXTENDED:			return "        Extended";

	case PROFILE_IDC_HIGH:
		if(constrained_set3_flag)
										return "   High 10 Intra";
		else
										return "            High";
	case PROFILE_IDC_HIGH_10:
		if(constrained_set3_flag)
										return "   High 10 Intra";
		else
										return "            High";
	case PROFILE_IDC_HIGH_422:
		if(constrained_set3_flag)
										return "High 4:2:2 Intra";
		else
										return "      High 4:2:2";
	case PROFILE_IDC_HIGH_444:
		if(constrained_set3_flag)
										return "High 4:4:4 Intra";
		else
										return "      High 4:4:4";

	case PROFILE_IDC_SCALABLE_BASELINE: return "    SVC Baseline";
	case PROFILE_IDC_SCALABLE_HIGH:
		if(constrained_set3_flag)
										return "  SVC High Intra";
		else
										return "        SVC High";

	case PROFILE_IDC_MULTIVIEW_HIGH:	return "        MVC High";
	case PROFILE_IDC_STEREO_HIGH:		return "     Stereo High";
	}
	
	sprintf(profile_name_unknown, "Unknown (%d)", profile_idc);
	return profile_name_unknown;
}

static char *get_nalu_name(int nalu_type)
{
	if (nalu_type > 24) return "???";
	return nalu_type_names[nalu_type];
}

static char *get_sei_name(int sei_type)
{
	if (sei_type > 45) return "RESERVED";
	return sei_type_names[sei_type];
}

static char level_name[80];
static char *get_level_name(byte level_idc)
{
	sprintf(level_name, "%d.%d", level_idc/10, level_idc%10);
	return level_name;
}

static char bit_depth_name[80];
static char *get_bit_depth(byte profile_idc, byte bit_depth_luma_minus8, byte bit_depth_chroma_minus8)
{
	if(	(profile_idc == 100 || profile_idc == 110 || profile_idc == 122 ||
		profile_idc == 244 || profile_idc == 44 || profile_idc == 83 ||
		profile_idc == 86 || profile_idc == 118 || profile_idc == 128) 
		&& (bit_depth_luma_minus8 > 0 || bit_depth_chroma_minus8 > 0) )
	{
		if(bit_depth_luma_minus8 == bit_depth_chroma_minus8)
			sprintf(bit_depth_name, " %dbits", bit_depth_luma_minus8+8);
		else
			sprintf(bit_depth_name, " Luma/Chroma %d/%dbits", bit_depth_luma_minus8+8, bit_depth_chroma_minus8+8);
	}
	else
	{
		sprintf(bit_depth_name, "");
	}

	return bit_depth_name;
}

static void print_sps(media_sample_t *decoded_ms)
{
	seq_parameter_set_t *sps;
	int sps_id;

	if (decoded_ms == NULL || decoded_ms->used_size != sizeof(seq_parameter_set_t))
	{
		verbose_print(0,"\tFAIL SPS");
		return;
	}
	
	sps = (seq_parameter_set_t *)decoded_ms->data;

	verbose_print(0,"[%d] %s/%s (%3d/%2d)%s %s %dx%d",
		sps->seq_parameter_set_id
		,get_profile_name(sps->profile_idc, sps->constrained_set3_flag)
		,get_level_name(sps->level_idc)
		,sps->profile_idc
		,sps->level_idc
		,get_bit_depth(sps->profile_idc, sps->bit_depth_luma_minus8, sps->bit_depth_chroma_minus8)
		,sps->frame_mbs_only_flag ? "Frames" : sps->mb_adaptive_frame_field_flag ? "Mbaff" : "Fields"
		,sps->pic_width_in_mbs<<4, sps->frame_height_in_mbs<<4
		);

	sps_id = sps->seq_parameter_set_id & 31;
	nal_hrd_flag[sps_id] = 0;
	vcl_hrd_flag[sps_id] = 0;
	if (sps->vui_parameters_present_flag)
	{
		double fps = 0;
		if (sps->vui_info.num_units_in_tick > 0)
			fps = ((double)sps->vui_info.time_scale / (double)sps->vui_info.num_units_in_tick)/2;
		verbose_print(0," @%g(%d/%d)",
			 fps
			,sps->vui_info.time_scale, sps->vui_info.num_units_in_tick
			);
		
		if(sps->vui_info.nal_hrd_parameters_present_flag)
		{
			int i;
			vssh_hrd_parameters_t *h = &sps->vui_info.nal_hrd_parameters;
			nal_hrd_flag[sps_id] = 1;
			verbose_print(1, "\n\t\t\t\tHRD NAL (bitrate/size):");
			for(i = 0; i < h->cpb_cnt; i++)
			{
				int br = (h->bit_rate_value[i]) << (6 + h->bit_rate_scale);
				int sz = (h->cpb_size_value[i]) << (4 + h->cpb_size_scale);
				verbose_print(1, " %d/%d", br, sz);
				if(h->cbr_flag[i] != 0)
					verbose_print(1, "(CBR)");
			}
		}
		if(sps->vui_info.vcl_hrd_parameters_present_flag)
		{
			int i;
			vssh_hrd_parameters_t *h = &sps->vui_info.vcl_hrd_parameters;
			vcl_hrd_flag[sps_id] = 1;
			verbose_print(1, "\n\t\t\t\tHRD VCL (bitrate/size):");
			for(i = 0; i < h->cpb_cnt; i++)
			{
				int br = (h->bit_rate_value[i]) << (6 + h->bit_rate_scale);
				int sz = (h->cpb_size_value[i]) << (4 + h->cpb_size_scale);
				verbose_print(1, " %d/%d", br, sz);
				if(h->cbr_flag[i] != 0)
					verbose_print(1, "(CBR)");
			}
		}
	}
}

static void print_pps(media_sample_t *decoded_ms)
{
	pic_parameter_set_t *pps;
	if (decoded_ms == NULL || decoded_ms->used_size != sizeof(pic_parameter_set_t))
	{
		verbose_print(0," FAIL PPS");
		return;
	}
	pps = (pic_parameter_set_t *)decoded_ms->data;
	verbose_print(0,"[%d,%d] %s ",
		pps->seq_parameter_set_id
		,pps->pic_parameter_set_id
		,pps->entropy_coding_mode? "CABAC":"CAVLC");
	if (pps->chroma_qp_index_offset != 0)
		verbose_print(0,"QPU[%d] ", pps->chroma_qp_index_offset);
	if (pps->second_chroma_qp_index_offset != 0)
		verbose_print(0,"QPV[%d] ", pps->second_chroma_qp_index_offset);
	if (pps->deblocking_filter_parameters_present_flag)
		verbose_print(0,"DB ");
	if (pps->constrained_intra_pred_flag)
		verbose_print(0,"CIP ");
	if (pps->weighted_pred_flag)
		verbose_print(0,"WP ");
	if (pps->weighted_bipred_idc)
		verbose_print(0,"WB ");
	if (pps->transform_8x8_mode_flag)
		verbose_print(0,"T8x8 ");
	if (pps->pic_scaling_matrix_present_flag)
		verbose_print(0,"SM ");
}

static void print_slice(media_sample_t *decoded_ms)
{
	slice_params_t *slice;
	if (decoded_ms == NULL || decoded_ms->used_size != sizeof(slice_params_t))
	{
		verbose_print(0," FAIL Slice");
		return;
	}
	slice = (slice_params_t *)decoded_ms->data;
	verbose_print(0," #%d(%s) qp=%d %s fmb=%d",
		slice->frame_num,
		get_slice_name(slice->slice_type),
		slice->qp,
		slice->field_pic_flag ? (slice->bottom_field_flag ? "botf" : "topf"): "",
		slice->first_mb_in_slice
	);
	if(slice->svc_ext.is_svc_slice && slice->svc_ext.slice_skip_flag)
	{
		verbose_print(0," SKIPPED ");
	}
}

static void print_slice_ext(media_sample_t *ms)
{
	byte *buf;
	buf = (byte*)ms->data;
	
	if(NAL_EXT_SVS_EXTENSION_FLAG(buf))
	{
		verbose_print(0," [%d][%d|%d|%d]",
			NAL_EXT_PRIORITY_ID(buf),
			NAL_EXT_DEPENDENCY_ID(buf),
			NAL_EXT_TEMPORAL_ID(buf),
			NAL_EXT_QUALITY_ID(buf)
			);
		if (NAL_EXT_IDR(buf))
			verbose_print(0," IDR");
		stream_type = 1;
	}
	else
	{
		verbose_print(0," MVC[%d][%d|%d]",
			NAL_MVC_EXT_PRIORITY_ID(buf),
			NAL_MVC_EXT_TEMPORAL_ID(buf),
			NAL_MVC_EXT_VIEW_ID(buf)
			);
		if (!NAL_MVC_EXT_NON_IDR(buf))
			verbose_print(0," IDR");
		if(NAL_MVC_EXT_ANCHOR_PIC(buf))
			verbose_print(0," ANCHOR");
		stream_type = 2;
	}
}

static media_sample_t *handle_sei(media_sample_t *sei)
{
	int type, flags, size, is_compressed;
	
	if (sei == NULL) return NULL;
	if (sei->data == NULL) return NULL;
	if (sei->extra_data == NULL) return NULL;

	type = SEI_MS_TYPE(sei);
	flags = SEI_MS_FLAGS(sei);
	size = sei->used_size;
	is_compressed = ((flags&SEI_FLAG_COMPRESSED)!= 0);

	verbose_print(1, "SEI #%d (%d): %s", type, size, get_sei_name(type));
	if(is_compressed)
	{
		verbose_print(1, " [compressed]");
	}
	
	switch (type)
	{
	case SEI_BUFFERING_PERIOD:
		{
			sei_buffering_period_t *bp = sei->data;
			int sps_id = bp->seq_parameter_set_id & 31;
			if (size != sizeof(sei_buffering_period_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(sei_buffering_period_t));
				fflush(stderr);
			}
			else
			{
				verbose_print(1, ": sps=%d  delay/offset", bp->seq_parameter_set_id);
				if(nal_hrd_flag[sps_id])
					verbose_print(1, " NAL:%d/%d", bp->nal_bp[0].initial_cpb_removal_delay, bp->nal_bp[0].initial_cpb_removal_delay_offset);
				if(vcl_hrd_flag[sps_id])
					verbose_print(1, " VCL:%d/%d", bp->vcl_bp[0].initial_cpb_removal_delay, bp->vcl_bp[0].initial_cpb_removal_delay_offset);
			}
		}
		break;
	
	case SEI_PICTURE_TIMING:
		{
			sei_pic_timing_t *pt = sei->data;
			size = sizeof(sei_pic_timing_t);
			if (size != sizeof(sei_pic_timing_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(sei_pic_timing_t));
				fflush(stderr);
			}
			else
				verbose_print(1, ": pic_struct=%d cpb_rd=%d dpb_rd=%d", pt->pic_struct, pt->cpb_removal_delay, pt->dpb_output_delay);
		}
		break;
	
	case SEI_USER_DATA_REGISTERED_ITU_T_T35:
		{
			itu_t_t35_t *itu_t_t35 = (itu_t_t35_t *)sei->data;
			atsc1_data_t *atsc1;
			afd_data_t *afd;
			cc_data_t  *cc;
			bar_data_t *bar;
			verbose_print(1, ": ");
			if (size != sizeof(itu_t_t35_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(itu_t_t35_t));
				fflush(stderr);
			}
			else
			{
				switch (itu_t_t35->user_identifier)
				{
				case ITU_T_T35_UID_ATSC1:
					atsc1 = &itu_t_t35->user_structure.atsc1_data;
					verbose_print(1, ": ATSC1: ");
					switch (atsc1->user_data_type_code)
					{
					case ATSC1_TYPE_CC:
						cc = &atsc1->user_data_type_structure.cc_data;
						verbose_print(0,"CC: count=%d ", cc->cc_count);
						break;
					case ATSC1_TYPE_BAR:
						bar = &atsc1->user_data_type_structure.bar_data;
						verbose_print(1, ": BAR: ");
						verbose_print(1, "top=%d ", bar->line_number_end_of_top_bar);
						verbose_print(1, "bottom=%d ", bar->line_number_start_of_bottom_bar);
						verbose_print(1, "left=%d ", bar->pixel_number_end_of_left_bar);
						verbose_print(1, "right=%d ", bar->pixel_number_start_of_right_bar);
						break;
					}
					break;
				case ITU_T_T35_UID_AFD:
					afd = &itu_t_t35->user_structure.afd_data;
					verbose_print(1, ": AFD: flag=%d format=%d", afd->active_format_flag, afd->active_format);
					break;
				}
			}
		}
		break;
	
	case SEI_USER_DATA_UNREGISTERED:
		{
			user_data_unregistered_t *udu = (user_data_unregistered_t *)sei->data;
			if (size != sizeof(user_data_unregistered_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(user_data_unregistered_t));
				fflush(stderr);
			}
			else
			{
				switch (udu->type_indicator)
				{
				case BLUE_RAY_UID_CC:
					verbose_print(1, ": type=BD_Closed_Caption");
					break;
				case BLUE_RAY_GOP_MAP:
					verbose_print(1, ": type=BD_GOP_structure_map");
					break;
				case BLUE_RAY_OFFSET_METADATA:
					verbose_print(1, ": type=BD_offset_metadata");
					break;
				default:
					break;
				}

				verbose_print(2, " UUID=");
				verbose_print(2, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
						udu->uuid_iso_iec_11578[ 0],
						udu->uuid_iso_iec_11578[ 1],
						udu->uuid_iso_iec_11578[ 2],
						udu->uuid_iso_iec_11578[ 3],
						udu->uuid_iso_iec_11578[ 4],
						udu->uuid_iso_iec_11578[ 5],
						udu->uuid_iso_iec_11578[ 6],
						udu->uuid_iso_iec_11578[ 7],
						udu->uuid_iso_iec_11578[ 8],
						udu->uuid_iso_iec_11578[ 9],
						udu->uuid_iso_iec_11578[10],
						udu->uuid_iso_iec_11578[11],
						udu->uuid_iso_iec_11578[12],
						udu->uuid_iso_iec_11578[13],
						udu->uuid_iso_iec_11578[14],
						udu->uuid_iso_iec_11578[15]);
			}
		}
		break;

	case SEI_RECOVERY_POINT:
		{
			sei_recovery_point_t *rp = sei->data;
			if (size != sizeof(sei_recovery_point_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(sei_recovery_point_t));
				fflush(stderr);
			}
			else
			{
				verbose_print(1, ": rec_frame_cnt=%d exact_match=%d broken_link=%d changing_slice_group_idc=%d", 
					rp->recovery_frame_cnt, rp->exact_match_flag, rp->broken_link_flag, rp->changing_slice_group_idc);
			}
		}
		break;
	
	case SEI_FRAME_PACKING_ARRANGEMENT:
		{
			sei_frame_packing_arrangement_t *fpa = sei->data;

			if (size != sizeof(sei_frame_packing_arrangement_t))
			{
				verbose_print(1,": wrong size, expected=%d", sizeof(sei_frame_packing_arrangement_t));
			}
			else
			{
				if(!fpa->frame_packing_arrangement_cancel_flag)
				{
					verbose_print(1,": frame_packing_arrangement_type=%d quincunx_sampling_flag=%d", fpa->frame_packing_arrangement_type, fpa->quincunx_sampling_flag);
				}
				else
				{
					verbose_print(1,": frame_packing_arrangement_cancel_flag=%d =%d", fpa->frame_packing_arrangement_cancel_flag);
				}
			}
		}
		break;

	case SEI_FILM_GRAIN:
		{
			sei_film_grain_t *fg = sei->data;
			if (size != sizeof(sei_film_grain_t))
			{
				verbose_print(1,": wrong size, expected=%d", sizeof(sei_film_grain_t));
			}
			else
			{
				verbose_print(1,": model_id=%d, ...", fg->model_id);
			}
		}
		break;

	case SEI_SCALABLE_NESTING:
		{
			sei_scalable_nesting_t *sei_nesting = (sei_scalable_nesting_t *)sei->data;
			int i = 0;
			if(sei->used_size != sizeof(sei_scalable_nesting_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(sei_scalable_nesting_t));
				fflush(stderr);
				return NULL;
			}
			else
			{
				verbose_print(1, ": num_layers=%d", sei_nesting->num_layer_representations_minus1+1);
			}
			sei = sei->next;
			for(i = 0; i <= sei_nesting->num_layer_representations_minus1; i++)
			{
				int current_dq_id = (sei_nesting->sei_dependency_id[i]<<4) + sei_nesting->sei_quality_id[i];
				if(current_dq_id <= target_dq_id)
				{
					verbose_print(1, "\n\t\tDQId=%2d: ", current_dq_id);
					sei = handle_sei(sei);
					fflush(stderr);
				}
				else
				{
					sei = sei->next;
				}
			}
			return sei;
		}
		break;
	case SEI_MVC_SCALABLE_NESTING:
		{
			sei_mvc_scalable_nesting_t *mvc_nesting = (sei_mvc_scalable_nesting_t *)sei->data;
			int i, current_dq_id;
			if(sei->used_size != sizeof(sei_mvc_scalable_nesting_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(sei_mvc_scalable_nesting_t));
				fflush(stderr);
				return NULL;
			}
			if(!mvc_nesting->operation_point_flag)
			{
				if(mvc_nesting->all_view_components_in_au_flag)
					current_dq_id = 0;
				else
					current_dq_id = mvc_nesting->sei_view_id[0];
			}
			else
				current_dq_id = mvc_nesting->sei_op_view_id[0];

			if(current_dq_id <= target_dq_id)
			{
				if(!mvc_nesting->operation_point_flag)
				{
					if(mvc_nesting->all_view_components_in_au_flag)
						verbose_print(1, "\n\t\tView ALL ");
					else
					{
						verbose_print(1, "\n\t\tView ");
						for(i = 0; i <= mvc_nesting->num_view_components_minus1; i++)
							verbose_print(1, "%d ", mvc_nesting->sei_view_id[i]);
					}
				}
				else
				{
					verbose_print(1, "\n\t\t  View");
					for(i = 0; i <= mvc_nesting->num_view_components_op_minus1; i++)
					{
						verbose_print(1, " %d", mvc_nesting->sei_op_view_id[i]);
					}
				}
				verbose_print(1, ": ");
				return handle_sei(sei->next);
			}
			sei = sei->next;
		}
		break;
	}	
	return sei->next;
}

static void print_sei(media_sample_t *sei)
{
	while (sei != NULL)
	{
		verbose_print(1, "\n\t\t\t  ");
		sei = handle_sei(sei);
	}
}

static void print_picsep(media_sample_t *ms)
{
	byte *buf = (byte*)ms->data;
	byte  type = buf[1] >> 5;
	char *ftype = "?";
	switch (type)
	{
	case 0: ftype = "I"; break;
	case 1: ftype = "P"; break;
	case 2: ftype = "B"; break;
	case 3: ftype = "SI"; break;
	case 4: ftype = "SP"; break;
	case 5: ftype = "SI"; break;
	case 6: ftype = "SP"; break;
	case 7: ftype = "SPB"; break;
	}
	verbose_print(0, " type=%s pic#=%04d", ftype, pic_number);
	pic_number++;
}

static void print_mvc_pd(media_sample_t *ms)
{
	byte *buf = (byte*)ms->data;
	byte  primary_pic_type = buf[1] >> 5;
	char *ftype = "?";
	switch (primary_pic_type)
	{
	case 0: ftype = "I"; break;
	case 1: ftype = "P"; break;
	case 2: ftype = "B"; break;
	}
	verbose_print(0, " type=%s", ftype);
}

static int print_media_sample(media_sample_t *ms)
{
	int nalu_type;
	int nalu_ridc;
	int nalu_len;
	media_sample_t *decoded_ms;
	byte *buf = (byte*)ms->data;
	nalu_len = ms->used_size;
	nalu_type = NALU_TYPE(buf[0]);
	nalu_ridc = NALU_RIDC(buf[0]);
	if (nalu_type == NALU_TYPE_PD)
		verbose_print(0,"\n");

	v4d_set_nal_unit(dec_handle, ms);
	decoded_ms = v4d_get_decoded_nal_unit(dec_handle);

	if(nalu_type == NALU_TYPE_SPS) 
		verbose_print(0,"\n");
	
	verbose_print(0, "%06x(%5d): [%d|%2d] %s",
		nalu_offset,
		nalu_len,
		nalu_ridc,
		nalu_type,
		get_nalu_name(nalu_type)
		);

	nalu_offset += (nalu_len + STARTCODE_LEN);

	switch (nalu_type)
	{
	case NALU_TYPE_MVC_PD:
		print_mvc_pd(ms);
		break;
	case NALU_TYPE_SPS:
	case NALU_TYPE_SUBSPS:
		print_sps(decoded_ms);
		break;
	case NALU_TYPE_PPS:
		print_pps(decoded_ms);
		break;
	case NALU_TYPE_PD:
		print_picsep(ms);
		break;
	case NALU_TYPE_PREFIX_EXT:
		print_slice_ext(ms);
		break;
	case NALU_TYPE_IDR:
	case NALU_TYPE_SLICE:
		print_slice(decoded_ms);
		break;
	case NALU_TYPE_SLICE_EXT:
		print_slice_ext(ms);
		print_slice(decoded_ms);
		break;
	case NALU_TYPE_SEI:
		print_sei(decoded_ms);
		break;
	}

	verbose_print(0, "\n");
	fflush(stderr);
	return 1;
}

typedef int handle_media_sample_t(media_sample_t *ms);

static void flush_hrd(void);

static void flush_data(void)
{
	if (buffer_analyser)
		flush_hrd();
}

static void put_data(byte *input_block, int read_size, handle_media_sample_t *handler)
{
	int rc;
	media_sample_t *ms;

	if (read_size <= 0)
	{
		v4d_nal_extractor_feed_data(input_buffer, NULL, 0, 0, 0);
	}
	else
		rc = v4d_nal_extractor_feed_data(input_buffer, input_block, read_size, 0, 0);
	///////// extract all NAL units from input buffer and analyze them
	for(;;)//while (1)
	{
		ms = v4d_nal_extractor_get_nalu(input_buffer);
		if (ms == NULL) break;
		if(is_skip_extractor)
		{
			handler(ms);
			v4_free_media_sample(ms);
			nalu_number++;
		}
		else
		{
			v4d_substream_extractor_feed_nalu(substream_extractor, ms);
			for(;;)//while (1)
			{
				ms = v4d_substream_extractor_get_nalu(substream_extractor);
				if (ms == NULL) break;
				handler(ms);
				v4_free_media_sample(ms);
				nalu_number++;
			}
		}
	}

	if (read_size <= 0 && !is_skip_extractor)
	{
		v4d_substream_extractor_feed_nalu(substream_extractor, NULL);
		for(;;)//while (1)
		{
			ms = v4d_substream_extractor_get_nalu(substream_extractor);
			if (ms == NULL) break;
			handler(ms);
			v4_free_media_sample(ms);
			nalu_number++;
		}
	}
}

static int hrd_decode_nal_unit(media_sample_t *ms); //Forward declaration
static void pre_init_hrd(int use_vcl_flag);


////////////// ---- Buffer Analyzer -----------------------------------------------------
#include <math.h>

#define HRD_S 512 //must be greater than number of pictures in hrd-buffer

static seq_parameter_set_t sps_pool[32];
static pic_parameter_set_t pps_pool[256];
static slice_params_t slice_hdr;
static int last_dq_id = 0;
static int last_mb_num = -1;
static int last_nal_type = -1;

typedef struct access_unit_t
{
	double t_ai;			// initial arrival time
	double t_ai_earliest;	// earliest initial arrival time
	double t_af;			// final arrival time
	double t_rn;			// nominal removal time
	double t_r;				// removal time

	double t_out;
	int poc;

	double size;
} access_unit_t;

typedef struct coded_picture_buffer_t
{
	unsigned int size;
	double fullness;

	access_unit_t pict[HRD_S];
	int cur_pict_num;
	int removed_pict_num;
} coded_picture_buffer_t;

typedef struct vbv_buf_t 
{
	double fps;		// frames per second
	int bitrate;	// bitrate, kbps
	int size_nal;	// current VBV buffer size (NAL mode)
	int size_vcl;	// current VBV buffer size (VCL mode)
	int send;		// current bits sending to network
	int max;		// maximum VBV buffer size, bits
	int max_pict_num; // picture number with maximum VBV buffer size
	int pict_num;	// current picture number
	int is_field;	// field coding mode
} vbv_buf_t;

typedef struct hr_decoder_t 
{
	int sps_here;
	int is_active;
	int use_vcl_flag;
	int delimiter_type;	// current delimiter type
	int cbr_flag;
	seq_parameter_set_t *active_sps;
	pic_parameter_set_t *active_pps;
	slice_params_t *slice_hdr;

	double t_c;	// clock tick
	double bitrate;

	coded_picture_buffer_t cpb;

	unsigned int nalu_offset_nal; // current NAL unit offset
	unsigned int nalu_offset_vcl; // current VCL unit offset
	unsigned int prev_offset; // previous NAL unit offset

	double current_time;

	int initial_cpb_removal_delay;
	int initial_cpb_removal_delay_offset;
	int cpb_removal_delay_n;
	int dpb_output_delay_n;
	int buf_period_flag;
	double t_rn_nb; // nominal removal time of the first access unit of the previous buffering period
	int max_mbps;
	int active_dq_id;	// dq_id that is corresponds with active_sps

	vbv_buf_t vbv;	// VBV buffer parameters

} hr_decoder_t;

static hr_decoder_t hrd_ctx;

static double max_d(double a, double b)
{
	return a>b ? a : b;
}

static void pre_init_hrd(int use_vcl_flag)
{
	hr_decoder_t *hrd = &hrd_ctx; 
	memset(hrd, 0, sizeof(hr_decoder_t));
	hrd->cpb.cur_pict_num = -1;
	hrd->delimiter_type = -1; //not valid type
	hrd->use_vcl_flag = use_vcl_flag;
	hrd->active_dq_id = 0;
	hrd->sps_here = 0;
	hrd->active_sps = NULL;
	hrd->active_pps = NULL;
	memset(sps_pool, -1, sizeof(sps_pool));
	memset(pps_pool, -1, sizeof(pps_pool));
}


static void init_hrd(void)
{
	hr_decoder_t *hrd = &hrd_ctx; 
	seq_parameter_set_t *active_sps = hrd->active_sps;
	vssh_hrd_parameters_t *hrd_params;
	int nal_hrd_flag;
	int vcl_hrd_flag;
	
	if(active_sps == NULL)
	{
		verbose_print(0,"Error: SPS is unavailable, exit\n\n");
		exit(1);
	}
	
	hrd->is_active = 1;
	hrd_params = hrd->use_vcl_flag ? &active_sps->vui_info.vcl_hrd_parameters : &active_sps->vui_info.nal_hrd_parameters;
	nal_hrd_flag = active_sps->vui_parameters_present_flag && active_sps->vui_info.nal_hrd_parameters_present_flag;
	vcl_hrd_flag = active_sps->vui_parameters_present_flag && active_sps->vui_info.vcl_hrd_parameters_present_flag;

	// Init VBV
	hrd->vbv.fps = 0;
	if(active_sps->vui_parameters_present_flag && active_sps->vui_info.timing_info_present_flag)
	{
		hrd->vbv.fps = (double)active_sps->vui_info.time_scale/(active_sps->vui_info.num_units_in_tick<<1);
	}
	else
	{
		hrd->vbv.fps = (double)framerate/10000.;
	}
	hrd->vbv.bitrate = hrd->vbv.fps > 0 ? bitrate : 0;
	if(hrd->vbv.bitrate > 0)
	{
		// calculate number of bits to send on every picture
		hrd->vbv.send = (int)(.5 + ((double)hrd->vbv.bitrate * 1000.) / hrd->vbv.fps);
		hrd->vbv.is_field = 0;
		if(active_sps->frame_mbs_only_flag == 0 && active_sps->mb_adaptive_frame_field_flag == 0)
		{
			hrd->vbv.send = (hrd->vbv.send + 1) >> 1;	// field coding
			hrd->vbv.is_field = 1;
			verbose_print(0, "Field coding is detected\n\n");
		}
	}
	
	if(hrd->use_vcl_flag && !vcl_hrd_flag)
	{
		hrd->is_active = 0;
		if(hrd->vbv.bitrate == 0)
		{
			verbose_print(0,"Error: VCL HRD and timing parameters are not present in SPS\nTo check VBV buffer use -frame-rate fps option to set frame rate manually\n exit\n\n");
			exit(1);
		}
	}

	if(!hrd->use_vcl_flag && !nal_hrd_flag)
	{
		hrd->is_active = 0;
		if(hrd->vbv.bitrate == 0)
		{
			verbose_print(0,"Error: NAL HRD and timing parameters are not present in SPS\nTo check VBV buffer use -frame-rate fps option to set frame rate manually\n exit\n\n");
			exit(1);
		}
	}

	hrd->bitrate = hrd_params->bit_rate_value[0] << (hrd_params->bit_rate_scale + 6);
	hrd->t_c = ((double)active_sps->vui_info.num_units_in_tick) / active_sps->vui_info.time_scale;
	hrd->cpb.size = hrd_params->cpb_size_value[0] << (hrd_params->cpb_size_scale + 4);
	hrd->cbr_flag = hrd_params->cbr_flag[0];

	switch (active_sps->level_idc) //TODO: check if it is different for VCL buffer
	{
	case 10:
		hrd->max_mbps = 1485;
		break;
	case 11:
		hrd->max_mbps = 3000;
		break;
	case 12:
		hrd->max_mbps = 6000;
		break;
	case 13:
	case 20:
		hrd->max_mbps = 11880;
		break;
	case 21:
		hrd->max_mbps = 19800;
		break;
	case 22:
		hrd->max_mbps = 20250;
		break;
	case 30:
		hrd->max_mbps = 40500;
		break;
	case 31:
		hrd->max_mbps = 108000;
		break;
	case 32:
		hrd->max_mbps = 216000;
		break;
	case 40:
	case 41:
		hrd->max_mbps = 245760;
		break;
	case 42:
		hrd->max_mbps = 522240;
		break;
	case 50:
		hrd->max_mbps = 589824;
		break;
	case 51:
		hrd->max_mbps = 983040;
		break;
	}
}

static void update_vbv(hr_decoder_t *hrd)
{
	vbv_buf_t *vbv = &hrd->vbv;
	int is_max = 0;
	int *size = hrd->use_vcl_flag ? &vbv->size_vcl : &vbv->size_nal;

	char *max_names[] = 
	{
		" ",
		"*",
	};

	if(vbv->bitrate == 0) 
		return;

	// send bits to network
	if (*size < vbv->send)
	{
		// buffer underflow!
		*size = 0;
	}
	else
	{
		*size -= vbv->send;
	}

	// save maximum VBV buffer value
	if (vbv->max < *size)
	{
		is_max = 1;
		vbv->max = *size;
		vbv->max_pict_num = vbv->pict_num;
	}
	
	verbose_print(2, "  VBV\t%4d\t%8.f kbits %s\n", vbv->pict_num++, *size/1000., max_names[is_max]);
}

#define PRINT_HEADING \
	if(stream_type == 1)\
		verbose_print(0,"dq_id");\
	else if(stream_type == 2)\
		verbose_print(0,"view");\
	else\
		verbose_print(0,"layer");\
	verbose_print(0,"\t pict\t  t_ai\t\t  t_af\t\t  t_rn\t\t delay\t\toffset\t\t  cpb\n")

#define PRINT_CURRENT_ACCESS_UNIT_INFO_CPB(cpb) \
	verbose_print(0,"%4d\t%4d\t%4.6lf\t%4.6lf\t%4.6lf\t%6d\t\t%6d\t\t%5d\n", hrd->active_dq_id, hrd->cpb.cur_pict_num, curr_pict->t_ai, curr_pict->t_af, curr_pict->t_rn, hrd->initial_cpb_removal_delay, hrd->initial_cpb_removal_delay_offset, cpb)

#define PRINT_CURRENT_ACCESS_UNIT_INFO \
	verbose_print(0,"%4d\t%4d\t%4.6lf\t%4.6lf\t%4.6lf\n", hrd->active_dq_id, hrd->cpb.cur_pict_num, curr_pict->t_ai, curr_pict->t_af, curr_pict->t_rn)


static seq_parameter_set_t* decode_sps(media_sample_t *ms)
{
	media_sample_t *dec_sps;
	int sps_id;

	v4d_set_nal_unit(dec_handle, ms);
	dec_sps = v4d_get_decoded_nal_unit(dec_handle);

	if(dec_sps == NULL || dec_sps->used_size != sizeof(seq_parameter_set_t))
		return NULL;
	
	sps_id = ((seq_parameter_set_t*)dec_sps->data)->seq_parameter_set_id % 32;
	memcpy(&sps_pool[sps_id], dec_sps->data, sizeof(seq_parameter_set_t));
	return &sps_pool[sps_id];
}

static seq_parameter_set_t* find_sps(byte sps_id)
{
	int i;
	seq_parameter_set_t* sps = NULL;

	for(i = 0; i < 32; i++)
	{
		sps = &sps_pool[i];
		if(sps->seq_parameter_set_id == sps_id)
			break;
		sps = NULL;
	}

	if(sps == NULL)
		sps = &sps_pool[0];

	return sps;
}

static pic_parameter_set_t* decode_pps(media_sample_t *ms)
{
	media_sample_t *dec_pps;
	int pps_id;

	v4d_set_nal_unit(dec_handle, ms);
	dec_pps = v4d_get_decoded_nal_unit(dec_handle);

	if(dec_pps == NULL || dec_pps->used_size != sizeof(pic_parameter_set_t))
		return NULL;

	pps_id = ((pic_parameter_set_t*)dec_pps->data)->pic_parameter_set_id % 256;
	memcpy(&pps_pool[pps_id], dec_pps->data, sizeof(pic_parameter_set_t));
	return &pps_pool[pps_id];
}

static pic_parameter_set_t* find_pps(byte pps_id)
{
	int i;
	pic_parameter_set_t* pps = NULL;

	for(i = 0; i < 256; i++)
	{
		pps = &pps_pool[i];
		if(pps->pic_parameter_set_id == pps_id)
			break;
		pps = NULL;
	}

	if(pps == NULL)
		pps = &pps_pool[0];

	return pps;
}

static void decode_slice(media_sample_t *ms, hr_decoder_t *hrd)
{
	media_sample_t *dec_slice;

	// Track of maximum dq_id present in stream
	if(NAL_TYPE(ms->data) == NALU_TYPE_SLICE_EXT)
	{
		int current_dq_id;
		if(NAL_EXT_SVS_EXTENSION_FLAG(ms->data))
		{
			 current_dq_id = NAL_EXT_DQ_ID(ms->data);
		}
		else
		{
			current_dq_id = NAL_MVC_EXT_VIEW_ID(ms->data);
		}
		
		if(hrd->active_dq_id < current_dq_id)
			hrd->active_dq_id = current_dq_id;
	}

	v4d_set_nal_unit(dec_handle, ms);
	dec_slice = v4d_get_decoded_nal_unit(dec_handle);
	if(dec_slice)
	{
		assert(dec_slice->used_size == sizeof(slice_params_t));
		memcpy(&slice_hdr, dec_slice->data, sizeof(slice_params_t));
		hrd->slice_hdr = &slice_hdr;
		hrd->active_pps = find_pps(hrd->slice_hdr->pic_parameter_set_id);
		hrd->active_sps = find_sps(hrd->active_pps->pic_parameter_set_id);
		hrd->sps_here = 1;
	}
	else
	{
		hrd->slice_hdr = NULL;
	}
}

void check_buffer(hr_decoder_t *hrd)
{
	access_unit_t *curr_pict = &hrd->cpb.pict[hrd->cpb.cur_pict_num % HRD_S];
	
	//static FILE *f_trace = NULL;
	//if (f_trace == NULL) f_trace = fopen("d:\\Testing\\dbg\\trace.txt","w");
	//if (f_trace)
	//	fprintf(f_trace, "%10.5f %10.1f %10d %6d %6d\n", hrd->current_time, hrd->cpb.fullness, hrd->cpb.size, hrd->cpb.cur_pict_num, hrd->cpb.removed_pict_num); 
	// ---- 
	if (hrd->cpb.fullness > hrd->cpb.size)
	{
		PRINT_CURRENT_ACCESS_UNIT_INFO;
		verbose_print(0,"Error: buffer_fullness > buffer_size	OVERFLOW\n");
		num_errors++;
	}
}

void check_all(hr_decoder_t *hrd)
{
	int cur_pict_num = hrd->cpb.cur_pict_num;
	access_unit_t *curr_pict = &hrd->cpb.pict[cur_pict_num % HRD_S];
	access_unit_t *picts = hrd->cpb.pict;
 
	if (curr_pict->t_rn < curr_pict->t_af)
	{
		PRINT_CURRENT_ACCESS_UNIT_INFO;
		verbose_print(0,"Error: t_rn[n] < t_af[n]	UNDERFLOW\n");
		num_errors++;
		//curr_pict->t_rn = curr_pict->t_af + 0.0000001;
	}

	if (cur_pict_num > 0)
	{
		access_unit_t *prev_pict = &hrd->cpb.pict[(cur_pict_num - 1) % HRD_S];
		double f_r = 1.0/(172 << hrd->slice_hdr->field_pic_flag);
		int pic_size_in_mbs = hrd->active_sps->pic_height_in_map_units*hrd->active_sps->pic_width_in_mbs;
		if (!hrd->active_sps->frame_mbs_only_flag && !hrd->slice_hdr->field_pic_flag)
			pic_size_in_mbs<<=1; 

		// To avoid LSBit rounding errors let's increase t_rn difference on small value
		// It should be eliminate errors on level limits boundaries
		if ( 1.0000001*(curr_pict->t_rn - prev_pict->t_r) < max_d(((double)pic_size_in_mbs)/hrd->max_mbps, f_r) )
		{
			PRINT_CURRENT_ACCESS_UNIT_INFO;
			verbose_print(0,"Error: t_rn[n] - t_r[n-1] < Max(PicSizeInMbs/MaxMBPS, fR)\n");
			num_errors++;
		}
	}

	while (picts[hrd->cpb.removed_pict_num % HRD_S].t_rn <= curr_pict->t_af && hrd->cpb.removed_pict_num < cur_pict_num) //move to the time when we need remove it
	{
		if (curr_pict->t_ai_earliest <= picts[hrd->cpb.removed_pict_num % HRD_S].t_rn) //add bytes to buffer  
		{
			if (curr_pict->t_ai_earliest > hrd->current_time)
			{
				hrd->current_time = curr_pict->t_ai_earliest;
				check_buffer(hrd);
			}
			hrd->cpb.fullness += (picts[hrd->cpb.removed_pict_num % HRD_S].t_rn - hrd->current_time)*hrd->bitrate;
		}
		hrd->current_time = picts[hrd->cpb.removed_pict_num % HRD_S].t_rn;
		check_buffer(hrd);
		while (hrd->current_time == picts[hrd->cpb.removed_pict_num % HRD_S].t_rn && hrd->cpb.removed_pict_num < cur_pict_num)
		{
			hrd->cpb.fullness -= picts[hrd->cpb.removed_pict_num % HRD_S].size;
			hrd->cpb.removed_pict_num++;
		}
		check_buffer(hrd);
	}
	//pass all remaining data
	if (curr_pict->t_ai_earliest > hrd->current_time)
	{
		hrd->current_time = curr_pict->t_ai_earliest;
		check_buffer(hrd);
	}
	hrd->cpb.fullness += (curr_pict->t_af - hrd->current_time)*hrd->bitrate;
	hrd->current_time = curr_pict->t_af;
	check_buffer(hrd);
}

void check_pic_order(hr_decoder_t *hrd)
{
	int cur_pict_num = hrd->cpb.cur_pict_num;
	access_unit_t *curr_pict = &hrd->cpb.pict[cur_pict_num % HRD_S];
	access_unit_t *prev_pict = &hrd->cpb.pict[(cur_pict_num - 1) % HRD_S];
	int poc_cmp, tout_cmp;
	int max_pic_order_cnt = 1 << hrd->active_sps->log2_max_pic_order_cnt_lsb;

	if ( curr_pict->poc < prev_pict->poc && 
		 (prev_pict->poc - curr_pict->poc) >= (max_pic_order_cnt/2) )
	{
		poc_cmp = ( (curr_pict->poc + max_pic_order_cnt) > prev_pict->poc );
	}
	else
		if ( curr_pict->poc > prev_pict->poc && 
			 (curr_pict->poc - prev_pict->poc) >= (max_pic_order_cnt/2) )
		{
			poc_cmp = ( (prev_pict->poc + max_pic_order_cnt) < curr_pict->poc );
		}
		else
		{
			poc_cmp = (curr_pict->poc > prev_pict->poc);
			if (hrd->slice_hdr->idr_flag) poc_cmp ^= 1;
		}

	tout_cmp = (curr_pict->t_out > prev_pict->t_out);

	if (poc_cmp != tout_cmp)
	{
		PRINT_CURRENT_ACCESS_UNIT_INFO;
		verbose_print(0,"Error: picture order POC not conforming to output time\n");
		num_errors++;
	}
}

void do_all_with_pict(hr_decoder_t *hrd)
{
	int cur_pict_num = hrd->cpb.cur_pict_num;
	access_unit_t *curr_pict = &hrd->cpb.pict[cur_pict_num % HRD_S];
	access_unit_t *prev_pict = &hrd->cpb.pict[(cur_pict_num - 1) % HRD_S];
	int initial_cpb_removal_delay = hrd->initial_cpb_removal_delay;
	int initial_cpb_removal_delay_offset = hrd->initial_cpb_removal_delay_offset;
	int cpb_removal_delay_n = hrd->cpb_removal_delay_n;
	int nalu_offset = hrd->use_vcl_flag ? hrd->nalu_offset_vcl : hrd->nalu_offset_nal;

	curr_pict->size = 8*(nalu_offset - hrd->prev_offset);
	hrd->prev_offset = nalu_offset;

	if (cur_pict_num == 0)
	{
		init_hrd();
		if(hrd->is_active == 0)
		{
			verbose_print(0, "HRD data is unavailable, analyzing VBV buffer only\n\n");
		}
		else
		{
			PRINT_HEADING;
		}
		update_vbv(hrd);
		if(hrd->is_active == 0)
			return;
		curr_pict->t_rn = initial_cpb_removal_delay/90000.0;
		curr_pict->t_ai = 0;
	}
	else
	{
		update_vbv(hrd);

		if(hrd->is_active == 0) 
			return;

		curr_pict->t_rn = hrd->t_rn_nb + hrd->t_c*cpb_removal_delay_n;

		if (curr_pict->t_rn <= prev_pict->t_rn)
		{
			PRINT_CURRENT_ACCESS_UNIT_INFO;
			verbose_print(0,"Error: t_rn[n] <= t_rn[n-1]\n");
			num_errors++;
		}

		curr_pict->t_ai_earliest = curr_pict->t_rn - 
			(initial_cpb_removal_delay + initial_cpb_removal_delay_offset)/90000.0;
		if (hrd->buf_period_flag)
		{
			double dt_g90_n = 90000.0*(curr_pict->t_rn - prev_pict->t_af);
			double over = (initial_cpb_removal_delay - ceil(dt_g90_n))/(hrd->cpb.size*90000.0/hrd->bitrate);
			int cpb = (int)(ceil(dt_g90_n) - initial_cpb_removal_delay);
			PRINT_CURRENT_ACCESS_UNIT_INFO_CPB(cpb);
			if (initial_cpb_removal_delay > ceil(dt_g90_n))
			{
				verbose_print(0,"Error: initial_cpb_removal_delay > ceil(dt_g90_n) by %f\n", over);
				num_errors++;
				initial_cpb_removal_delay = (int)ceil(dt_g90_n);
			}
			if (hrd->cbr_flag && initial_cpb_removal_delay < floor(dt_g90_n))
			{
				double r = floor(dt_g90_n) - initial_cpb_removal_delay;
				verbose_print(0,"Error: cbr_flag && initial_cpb_removal_delay < floor(dt_g90_n) by %f\n", r);
				num_errors++;
			}

//			curr_pict->t_ai_earliest = curr_pict->t_rn - initial_cpb_removal_delay/90000.0;
		}
		if (hrd->cbr_flag && curr_pict->t_ai_earliest > prev_pict->t_af)
		{
			PRINT_CURRENT_ACCESS_UNIT_INFO;
			verbose_print(0,"Error: cbr_flag && t_ai_earliest[n] > t_af[n-1]\n");
			num_errors++;
		}
		curr_pict->t_ai = max_d(curr_pict->t_ai_earliest, prev_pict->t_af);
	}
	curr_pict->t_af = curr_pict->t_ai + ((double)curr_pict->size)/hrd->bitrate;
	
	curr_pict->t_r = curr_pict->t_rn; // low_delay_hrd_flag always equal 0

	if(verbose)
	{
		if(cur_pict_num == 0)
		{
			int cpb = 0;
			PRINT_CURRENT_ACCESS_UNIT_INFO_CPB(cpb);
		}
		else
		{
			PRINT_CURRENT_ACCESS_UNIT_INFO;
		}
	}
	
	check_all(hrd);

	if (hrd->buf_period_flag)
		hrd->t_rn_nb = curr_pict->t_rn;
	hrd->buf_period_flag = 0;

	curr_pict->t_out = curr_pict->t_r + hrd->t_c*hrd->dpb_output_delay_n;
	curr_pict->poc = hrd->slice_hdr->pic_order_cnt_lsb;

	if (cur_pict_num > 0)
	{
		check_pic_order(hrd);
	}
}

static void flush_hrd(void)
{
	// Flush HRD buffer analyzer
	hrd_ctx.cpb.cur_pict_num++;
	do_all_with_pict(&hrd_ctx);
}

static void decode_delimeter(hr_decoder_t *hrd, media_sample_t *ms)
{
	byte *buf = (byte *)ms->data;
	hrd->delimiter_type = buf[1] >> 5;
	if (hrd->sps_here)
	{
		hrd->cpb.cur_pict_num++;
		do_all_with_pict(hrd);
	}
}

// Returns pointer to the next sei message
media_sample_t *hrd_handle_sei(hr_decoder_t *hrd, media_sample_t *sei, int current_dq_id)
{
	int type;
	
	if(sei == NULL) return NULL;
	if (sei->data == NULL) return NULL;
	if (sei->extra_data == NULL) return NULL;

	type = SEI_MS_TYPE(sei);

	if(target_dq_id == MAX_DQ_ID)
	{	// Analyze whole stream i.e. largest layer
		if(hrd->active_dq_id < current_dq_id)
			hrd->active_dq_id = current_dq_id;
	}
	else
	{	// Analyze stream up to target_dq_id
		if(hrd->active_dq_id < target_dq_id)
		{
			if(hrd->active_dq_id < current_dq_id)
				hrd->active_dq_id = current_dq_id;
		}
	}

	switch (type)
	{
	case SEI_BUFFERING_PERIOD:
		{
			sei_buffering_period_t *sei_buffering_period;
			seq_parameter_set_t *sps;
			int sps_id;
			if(sei->used_size != sizeof(sei_buffering_period_t))
			{
				verbose_print(0, "SEI Buffering Period is corrupted, current NAL is skipped\n");
				fflush(stderr);
				return NULL;
			}
			if (current_dq_id != hrd->active_dq_id) return sei->next;
			sei_buffering_period = (sei_buffering_period_t *)sei->data;
			sps_id = sei_buffering_period->seq_parameter_set_id & 31;
			sps = &sps_pool[sps_id];
			hrd->buf_period_flag = 1;
			if (hrd->use_vcl_flag)
			{
				if(sps->vui_parameters_present_flag && sps->vui_info.vcl_hrd_parameters_present_flag)
				{
					hrd->initial_cpb_removal_delay = sei_buffering_period->vcl_bp[0].initial_cpb_removal_delay;
					hrd->initial_cpb_removal_delay_offset = sei_buffering_period->vcl_bp[0].initial_cpb_removal_delay_offset;
				}
			}
			else
			{
				if(sps->vui_parameters_present_flag && sps->vui_info.nal_hrd_parameters_present_flag)
				{
					hrd->initial_cpb_removal_delay = sei_buffering_period->nal_bp[0].initial_cpb_removal_delay;
					hrd->initial_cpb_removal_delay_offset = sei_buffering_period->nal_bp[0].initial_cpb_removal_delay_offset;
				}
			}				
		}
		break;
	case SEI_PICTURE_TIMING:
		{
			sei_pic_timing_t *sei_pic_timing;
			if(sei->used_size != sizeof(sei_pic_timing_t))
			{
				verbose_print(0, "SEI Picture Timing is corrupted, current NAL is skipped\n");
				fflush(stderr);
				return NULL;
			}
			if (current_dq_id != hrd->active_dq_id) return sei->next;
			sei_pic_timing = (sei_pic_timing_t *)sei->data;
			hrd->cpb_removal_delay_n = sei_pic_timing->cpb_removal_delay;
			hrd->dpb_output_delay_n = sei_pic_timing->dpb_output_delay;
		}
		break;
	case SEI_SCALABLE_NESTING:
		{
			int i = 0;
			sei_scalable_nesting_t *sei_nesting = (sei_scalable_nesting_t *)sei->data;
			if(sei->used_size != sizeof(sei_scalable_nesting_t))
			{
				verbose_print(0, "SEI SVC Scalable Nesting is corrupted, current NAL is skipped\n");
				fflush(stderr);
				return NULL;
			}
			sei = sei->next;
			for(i = 0; i <= sei_nesting->num_layer_representations_minus1; i++)
			{
				int current_dq_id = (sei_nesting->sei_dependency_id[i] << 4) + sei_nesting->sei_quality_id[i];
				sei = hrd_handle_sei(hrd, sei, current_dq_id);
			}
			return sei;
		}
		break;
	case SEI_MVC_SCALABLE_NESTING:
		{
			sei_mvc_scalable_nesting_t *mvc_nesting = (sei_mvc_scalable_nesting_t *)sei->data;
			int current_mvc_id;
			if(sei->used_size != sizeof(sei_mvc_scalable_nesting_t))
			{
				verbose_print(0, "SEI MVC Scalable Nesting is corrupted, current NAL is skipped\n");
				fflush(stderr);
				return NULL;
			}
			if(!mvc_nesting->operation_point_flag)
			{
				if(mvc_nesting->all_view_components_in_au_flag)
					current_mvc_id = 0;
				else
					current_mvc_id = mvc_nesting->sei_view_id[0];
			}
			else
				current_mvc_id = mvc_nesting->sei_op_view_id[0];

			current_mvc_id = 1; //Nick: Patch for 2-views only stream. TODO correct fix 

			return hrd_handle_sei(hrd, sei->next, current_mvc_id);
		}
		break;
	}

	return sei->next;
}

static void decode_sei(hr_decoder_t *hrd, media_sample_t *ms)
{
	media_sample_t *sei;

	v4d_set_nal_unit(dec_handle, ms);
	sei = v4d_get_decoded_nal_unit(dec_handle);

	while (sei != NULL)
	{
		sei = hrd_handle_sei(hrd, sei, 0);
	}
}

static int hrd_decode_nal_unit(media_sample_t *ms)
{
	nalu_type_e nalu_type = NAL_TYPE(ms->data);
	hr_decoder_t *hrd = &hrd_ctx;
	int is_last_nal_slice = 
		last_nal_type == NALU_TYPE_IDR || 
		last_nal_type == NALU_TYPE_SLICE || 
		last_nal_type == NALU_TYPE_SLICE_EXT || 
		last_nal_type == NALU_TYPE_FILL;

	if (nalu_type == NALU_TYPE_PD)
	{
		decode_delimeter(hrd, ms);
	}
	else if(is_last_nal_slice != 0 && 
		    (nalu_type == NALU_TYPE_SPS || nalu_type == NALU_TYPE_PPS || nalu_type == NALU_TYPE_SEI))
	{
		hrd->cpb.cur_pict_num++;
		do_all_with_pict(hrd);
	}
	else if(is_last_nal_slice != 0 && 
		    (nalu_type == NALU_TYPE_IDR || nalu_type == NALU_TYPE_SLICE))
	{
		decode_slice(ms, hrd);
		if(hrd->slice_hdr && last_mb_num >= hrd->slice_hdr->first_mb_in_slice)
		{
			hrd->cpb.cur_pict_num++;
			do_all_with_pict(hrd);
		}
	}

	if(nalu_type == NALU_TYPE_SLICE_EXT)
	{
		if(NAL_EXT_SVS_EXTENSION_FLAG(ms->data))
			stream_type = 1;
		else
			stream_type = 2;
	}

	// Count bits
	hrd->nalu_offset_nal += (ms->used_size + STARTCODE_LEN);
	hrd->vbv.size_nal    += 8 * (ms->used_size + STARTCODE_LEN);
	if (nalu_type == NALU_TYPE_SLICE || nalu_type == NALU_TYPE_SLICE_EXT || nalu_type == NALU_TYPE_IDR || nalu_type == NALU_TYPE_FILL)
	{
		hrd->nalu_offset_vcl += ms->used_size;
		hrd->vbv.size_vcl    += 8 * ms->used_size;
	}

	switch (nalu_type)
	{
	case NALU_TYPE_SPS:
		hrd->active_sps = decode_sps(ms);
		break;
	case NALU_TYPE_PPS:
		decode_pps(ms);
		break;
	case NALU_TYPE_IDR:
	case NALU_TYPE_SLICE:
	case NALU_TYPE_SLICE_EXT:
		decode_slice(ms, hrd);
		if(hrd->slice_hdr)
			last_mb_num = hrd->slice_hdr->first_mb_in_slice;
		break;
	case NALU_TYPE_SUBSPS:
		hrd->active_sps = decode_sps(ms);
		break;
	case NALU_TYPE_PREFIX_EXT:
		break;
	case NALU_TYPE_SEI:
		decode_sei(hrd, ms);
		break;
	}

	last_nal_type = nalu_type;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	int result = 0;
	int rc;
	byte *input_block = NULL;
	int read_size;
	v4d_settings_t decoder_settings;
	arg_ctx_t arg_ctx;
	handle_media_sample_t *handle_media_sample;

#ifdef _CRT_DEBUG_
	_CrtMemState _ms1, _ms2, _ms3;
	_CrtMemCheckpoint(&_ms1);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	// print codec library version
	verbose_print(0, "Stream analysis sample application\n");
	print_codec_info();

	// process command line arguments
	arg_ctx.args = all_args;
	arg_ctx.cnt  = all_args_cnt;
	if (argc < 2)
	{
		usage();
		print_args(&arg_ctx);
		legend();
		return 0;
	}
	read_args(&arg_ctx, argc, argv);
	if (verbose) dump_args(&arg_ctx);

	if (0 != open_std_files(input_file, NULL))
	{
		return 1;
	}

#ifdef _CRT_DEBUG_
	_CrtMemCheckpoint(&_ms1);
#endif

	// allocate input block memory
	input_block = (byte*)v4_malloc(BUFFER_SIZE);
	if (input_block == NULL)
	{
		verbose_print(0, "Error: can't allocate input block memory (%d bytes)\n", BUFFER_SIZE);
		return 3;
	}

	// provide decoder settings
	memset(&decoder_settings, 0, sizeof(decoder_settings));
	v4d_default_settings(&decoder_settings, 1);
	decoder_settings.dec_flags = DEC_DECODE_SEI|DEC_ANALYSE_ONLY;
	rc = v4d_open(&dec_handle, &decoder_settings);
	if (rc != VSSH_OK)
	{
		verbose_print(0, "Error: vssh_dec_open() returned NULL\n");
		return 4;
	}
	
	input_buffer = v4d_nal_extractor_create(BUFFER_SIZE);
	if(input_buffer == NULL)
	{
		verbose_print(0, "Error: v4d_nal_extractor_create() returned NULL\n");
		return 5;
	}
	
	handle_media_sample = buffer_analyser > 0 ? hrd_decode_nal_unit : print_media_sample;
	if(buffer_analyser > 0)
		pre_init_hrd(buffer_analyser==2);
	is_skip_extractor = (target_p_id == -1 && target_dq_id == -1 && target_t_id == -1);
	if(!is_skip_extractor)
	{
		if(target_p_id == 0 && target_dq_id == -1)
			target_dq_id = 0;	// trick to correct extraction of base layer/view
		substream_extractor = v4d_substream_extractor_create(target_p_id, target_t_id, target_dq_id, 0);
		if(substream_extractor == NULL)
		{
			verbose_print(0, "Error: v4d_substream_extractor_create() returned NULL\n");
			return 6;
		}
	}
	target_t_id  &= MAX_TEMPORAL_ID;
	target_p_id  &= MAX_PRIORITY_ID;
	target_dq_id &= MAX_DQ_ID;
	
	if (buffer_analyser == 1)
		verbose_print(0, "HRD analyzer: NAL mode\n\n");
	else if (buffer_analyser == 2)
		verbose_print(0, "HRD analyzer: VCL mode\n\n");

	do 
	{
		// read blocks from stream...
		read_size = read_input_file(input_block, BUFFER_SIZE);
		// put data for analysis
		put_data(input_block, read_size, handle_media_sample);
	}
	while (read_size > 0);

	flush_data();

	// close all
	if (input_block != NULL)
		v4_free(input_block);
	if (dec_handle != NULL)
		v4d_close(dec_handle);
	if (input_buffer != NULL)
		v4d_nal_extractor_close(input_buffer);
	if(substream_extractor != NULL)
		v4d_substream_extractor_close(substream_extractor);

	verbose_print(0, "\n");

	if (buffer_analyser)
	{
		if(hrd_ctx.is_active)
			verbose_print(0, "Total HRD errors:   %d\n", num_errors);
		else
			verbose_print(0, "Total HRD errors:   n/a\n");
		verbose_print(0, "Total Access Units: %d\n", hrd_ctx.cpb.cur_pict_num+1);
	}

	verbose_print(0, "Total NAL Units:    %d\n\n", nalu_number);
	verbose_print(0, "H.264 extension:    %s\n\n", stream_type_names[stream_type]);
	
	if(buffer_analyser && hrd_ctx.vbv.bitrate > 0)
	{
		hr_decoder_t *hrd = &hrd_ctx;
		vbv_buf_t *vbv = &hrd->vbv;
		int nalu_offset = hrd->use_vcl_flag ? hrd->nalu_offset_vcl : hrd->nalu_offset_nal;
		double avg_bitrate = nalu_offset * ( vbv->fps*(1+vbv->is_field)/(125.*vbv->pict_num) );

		verbose_print(0, "VBV Buffer max:     %.f kbits at picture %d\n", hrd->vbv.max/1000., hrd->vbv.max_pict_num);
		verbose_print(0, "Average bitrate:    %.f kbps @%g fps\n\n", avg_bitrate, hrd->vbv.fps);
	}

#ifdef _CRT_DEBUG_
	_CrtMemCheckpoint(&_ms2);
	_CrtMemDifference(&_ms3, &_ms1, &_ms2);
	_CrtMemDumpStatistics(&_ms3);
#endif

	close_std_files();

#ifdef _CRT_DEBUG_
	_CrtDumpMemoryLeaks();
#endif

	return result;
}

//////////////////////////////////////////////////////////////////////////
