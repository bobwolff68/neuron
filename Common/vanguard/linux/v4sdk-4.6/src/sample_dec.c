
/**
 * @file sample_dec.c
 * Example of AVC Decoder console application
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
// VSofts H.264 Decoder V4 API
#include "v4_nalu.h"
#include "v4d_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#ifdef WIN32
#include <windows.h>
#else
#include <string.h>
#endif
#include <assert.h>

#include "v4timer.h"
#include "v4info.h"
#include "v4args.h"
#include "v4file.h"
#include "v4_substr_extract.h"

#if defined(WIN32) && defined(DEBUG) && !defined(__GNUC__)
#define _CRT_DEBUG_
#endif
#ifdef _CRT_DEBUG_
	#include <crtdbg.h>
#endif

#ifndef min
	#define min(a,b) ( (a) < (b)? (a) : (b) )
#endif
#ifndef max
	#define max(a,b) ( (a) < (b)? (b) : (a) )
#endif


///////////////////////////////////////////////// video processing library
#include "vp.h"


#include "psnr_ssim.h"

//////////////////////////////////////////////////////////////////////////
#include "md5.h"
md5_state_t md5;
int enable_md5 = 0;
md5_byte_t md5sum[17] = {0};


static void print_hex_string(md5_byte_t *str, int length)
{
	int i;
	for(i = 0; i < length; i += 1)
	{
		verbose_print(0,"%01x", ((str[i]>>4) & 15));
		verbose_print(0,"%01x", (str[i] & 15));
	}
}

static void init_md5_sum(void)
{
	if(enable_md5)
	{
		md5_init(&md5);
	}
}

static void check_md5_sum(void)
{
	if(enable_md5)
	{
		md5_byte_t str_md5[17] = {0};
		md5_finish(&md5, str_md5);
		verbose_print(0,"\nMD5 sum:\n");
		print_hex_string(str_md5, 16);
	}
}

////////////////////////////////////////////////////////////////////////// global variables
char *input_file  = NULL;
char *output_file = NULL;
char *svc_output_file = NULL;
FILE *svc_out = NULL;

// frame dimension
int frame_width  = 0;
int frame_height = 0;

// frame rate
int frame_rate   = 0;

// multi threading
int mt = 0;

// sei messages
int enable_sei = 0;

// frame packing
int fp_flag;

// EC
int enable_ec = 0;

//psnr ssim
psnr_ssim_t psnr_ssim;
int enable_ssim = 0;
int max_psnr = 1; //just enable 
char *ref_file = NULL;
FILE *ref_file_handle;


// select SVC layer
#define SVC_SELECT_ALL  (-1)
#define SVC_SELECT_BASE  (0)

#define MAX_TEMPORAL_ID	7
#define MAX_PRIORITY_ID	63
#define MAX_DQ_ID		127

int target_p_id  = SVC_SELECT_ALL;
int target_t_id  = SVC_SELECT_ALL;
int target_dq_id = SVC_SELECT_ALL;

// target SVC bitrate for Sub-stream Extractor
int svc_bitrate = 0;

int vbv_delay = 0;

// zero means to handle all the frames
int max_frames   = 0;
int start_frame  = 0;


// target bitrate for net buffer calculation, kbps
int bitrate = 0;

int enable_telecine = 0;	// interpret enable_telecine
int pic_struct = 0;	// last decoded pic_struct value
int colorspace = -1; // "-1" means no conversion

// override output dimensions
int output_width = 0;
int output_height = 0;
int forever      = 0;	// run forever
int output_delay = 15;

//////////////////////////////////////////////////// command line arguments
static cmd_arg_t all_args[] =
{
	{ "i", "yf", "input-file",   1, &input_file,      0, "name of input file" },
	{ "o", "hf", "output-file",  1, &output_file,     0, "name of output file" },
	{ "e", "ef", "svc-output-file",1, &svc_output_file, 0, "name of extracted SVC stream output file" },
	{ "r", "rf", "ref-file",     1, &ref_file,        0, "name of reference file for PSNR and SSIM calculations" },
	{ "n", "fn", "frame-count",  0, &max_frames,      0, "max number of input frames to process" },
	{ "-", "fs", "frame-start",  0, &start_frame,     0, "specify number of frame to start writing from" },
	{ "f", "fr", "frame-rate",   0, &frame_rate,      0, "input frame rate (frames per 10,000 sec)" },
	{ "u", "fc", "colorspace",   0, &colorspace,      0, "colorspace: 0=IYUV,I420; 1=YV12; 2=YUYV,YUY2; 3=YVYU; 4=UYVY; 5=RGB555; 6=RGB565; 7=RGB24; 8=RGB32, 9=4:0:0, 10=4:2:2" },
	{ "b", "br", "bitrate",      0, &bitrate,         0, "desired bitrate, kbps" },
	{ "m", "mt", "threads",      0, &mt,             -1, "number of threads (-1=auto, 0=disable)" },
	{ "v", "vb", "verbose",      0, &verbose,         1, "level of verbose messages (0/1/2)" },
	{ "-", "od", "output-delay", 0, &output_delay,    0, "h264 decoder output delay" },
	{ "s", "ds", "sei",          0, &enable_sei,      1, "decode SEI messages (0/1)" },
	{ "t", "dt", "telecine",     0, &enable_telecine, 1, "interpret telecine (0/1)" },
	{ "l", "sl", "select-layer", 0, &target_p_id,     0, "select SVC/MVC layers/views by priority id (-1=all, 0=base, .. 63=max)" },
	{ "-", "st", "select-t-id",  0, &target_t_id,     0, "select SVC/MVC layers/views by temporal id (-1=all, 0=base, .. 7=max)" },
	{ "d", "dq", "select-dq-id", 0, &target_dq_id,    0, "select SVC layers by dq_id ( = 16*dep_id+quality_id) (-1=all, 0=base, .. 127=max)" },
	{ "-", "sb", "svc-bitrate",  0, &svc_bitrate,     0, "target SVC bitrate, kbps" },
	{ "-", "vd", "vbv-delay",    0, &vbv_delay,       0, "target VBV delay, ms (0 means no VBV control)" },
	{ "5", "md", "md5",          0, &enable_md5,      1, "calculate md5 sum for output YUV file (0/1)" },
	{ "-", "ow", "out-width",    0, &output_width,    0, "force output frame width, pixels" },
	{ "-", "oh", "out-height",   0, &output_height,   0, "force output frame height, pixels" },
	{ "-", "ec", "error-concealment", 0, &enable_ec,  1, "enable EC (error concealment)" },
	{ "-", "mp", "max_psnr",     0, &max_psnr,        1, "max psnr value" },
	{ "-", "es", "enable_ssim",  0, &enable_ssim,     1, "enable SSIM calculation" },
	{ "-", "fv", "forever",      0, &forever,        -1, "rewind input stream and run N loops" },
	{ "-", "fp", "fp_flag",	     0, &fp_flag,         0, "enable frame unpacking" },
};

static const int all_args_cnt = sizeof(all_args)/sizeof(all_args[0]);

//////////////////////////////////////////////////////////////////////////
static void usage(void)
{
	verbose_print(0,"USAGE:\n");
	verbose_print(0,"\t > sample_dec [input file] [output file] [-option value] \n");
	verbose_print(0,"WHERE:\n");
}

////////////////////////////////////////////////////////////////////////// mvc extra files support
#define MAX_FILE_NAME 1024
#define NUM_MVC_FILES 8  //maximum number of views in MVC stream
FILE *mvc_file[NUM_MVC_FILES] ={NULL};

static FILE* get_mvc_file(int view_id)
{
	if(mvc_file[view_id] == NULL)
	{
		char name[MAX_FILE_NAME];
		char suffix[16];
		char *dot;

		strcpy(name, output_file);
		dot = strchr(name, '.');
		if (dot != NULL)
			*dot = '\0';

		sprintf(suffix, "%d.yuv", view_id);
		strncat(name, suffix, sizeof(suffix));
		mvc_file[view_id] = fopen(name, "wb");
		if(mvc_file[view_id] == NULL)
		{
			verbose_print(0, "Warning: can't open extra MVC file [%s]\n", name);
		}
	}

	return mvc_file[view_id];
}


static void close_mvc_files(void)
{
	int i;

	for(i = 0; i < NUM_MVC_FILES; i++)
	{
		if(mvc_file[i] != NULL)
			fclose(mvc_file[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////// file support
static int openFiles(void)
{
	if(svc_output_file != NULL)
	{
		if(input_file != NULL && strcmp(input_file, svc_output_file) == 0)
		{
			verbose_print(0, "Error: attempt to write to input file [%s]\n", input_file);
			return -1;
		}
		if(output_file != NULL && strcmp(output_file, svc_output_file) == 0)
		{
			verbose_print(0, "Error: attempt to write to YUV output file [%s]\n", output_file);
			return -1;
		}

		svc_out = fopen(svc_output_file, "wb");

		if(svc_out == NULL)
		{
			verbose_print(0, "Error: can't open SVC output file [%s]\n", svc_output_file);
			return -1;
		}
	}

	return open_std_files(input_file, output_file);
}

static void closeFiles(void)
{
	if(svc_out)
	{
		fclose(svc_out);
	}
	close_std_files();
	close_mvc_files();
}

static int readFile(void *buf, int buf_size)
{
	return read_input_file(buf, buf_size);
}


// 10-bit decoder can decode 8-bit h264 streams
// but to be fully complaint with 8-bit yuv
// it should skip high byte (it is zero)
// for 8-bit decoder skip_high_byte must be zero
int skip_high_byte = 0;


static int writeFile(const void *buf, int buf_size)
{
	int bytes = 0;
	byte *tmp_byte_buf = 0;

	// TODO axne: eliminate malloc/free calls!!!
	if(skip_high_byte)
	{
		int i;
		tmp_byte_buf = malloc(buf_size/2);

		for(i = 0; i < buf_size/2; i += 1)
		{
			tmp_byte_buf[i] = (byte)(((short*)buf)[i]);
		}

		buf = tmp_byte_buf;
		buf_size /= 2;
	}

	bytes = write_output_file(buf, buf_size);

	if(enable_md5)
	{
		md5_append(&md5, buf, buf_size);
	}


	if(tmp_byte_buf)
	{
		free(tmp_byte_buf);
	}

	if(skip_high_byte)
	{
		bytes *= 2;
	}
	return bytes;
}

// Write stream into file with start code avoiding
static int write_svc_file(media_sample_t *ms)
{
	int bytes = 0;
	static byte start_code[4] = {0,0,0,1};

	// Annex B format: 4 bytes start code and NAL unit RBSP contents
	if(svc_out != NULL && ms != NULL)
	{
		bytes += (int)fwrite(start_code, 1, 4, svc_out);
		bytes += (int)fwrite(ms->data, 1, ms->used_size, svc_out);
	}

	return bytes;
}

////////////////////////////////////////////////////////////////////////// VPL manipulations
static long read_frame(void *ctx, void *data, long size)
{
	long io = readFile(data, size);
	return io;
}

static long write_frame(void *ctx, void *data, long size)
{
	long io = writeFile(data, size);
	return io;
}


static int vpl_created = 0;
static vp_frame_factory_t src_factory;
static vp_frame_factory_t dst_factory;
static vp_frame_t src_frame;
static vp_frame_t dst_frame;
static vp_colorspace_e inp_cs;
static vp_colorspace_e out_cs;
static vp_batch_t batch;
static int telecine_state = 0;
static int output_frame = 0;

static vp_frame_factory_t dst_factory1;
static vp_frame_t in_frame;
static int interlace_sbs;
static vp_frame_t dst_frame1;

static vp_colorspace_e get_cs(int csp)
{
	switch (csp)
	{
	case  0: return VP_IYUV;
	case  1: return VP_YV12;
	case  2: return VP_YUY2;
	case  3: return VP_YVYU;
	case  4: return VP_UYVY;
	case  5: return VP_RGB555;
	case  6: return VP_RGB565;
	case  7: return VP_RGB24;
	case  8: return VP_RGB32;
	case  9: return VP_YUV400;
	case 10: return VP_YUV422;
	}
	return VP_UNKNOWN;
}

static long write_frame1(void *ctx, void *data, long size)
{
	long bytes = (long)fwrite(data, 1, size, (FILE*)ctx);
	assert(ctx);

	if(bytes != size)
	{
		printf("write file operation failed!!!\n");
		exit(-1);
	}

	return bytes;
}

static int init_vpl(sps_info_t *sps_info, yuv_frame_t *dec_frame)
{
	int w, h, bytes_per_pel;

	if (vpl_created) return 0;

	w = dec_frame->width;
	h = dec_frame->height;
	bytes_per_pel = dec_frame->bytes_per_pel;

	// determine input and default output color spaces
	switch (sps_info->yuv_format)
	{
	case 0:
		inp_cs = VP_IYUV;	// this is a trick because VPL currently does not support raw 4:0:0 input
		out_cs = VP_YUV400;
		break;
	case 1:
		inp_cs = VP_IYUV;
		out_cs = VP_IYUV;
		break;
	case 2:
		inp_cs = VP_YUV422;
		out_cs = VP_YUV422;
		break;
	default: assert(0);
	}
	// init source frame factory
	vp_init_frame_factory_ex(&src_factory, w, h, bytes_per_pel, inp_cs, NULL, read_frame, write_frame);

	// assign vital source frame characteristics
	memset(&src_frame, 0, sizeof(src_frame));
	src_frame.width  = w;
	src_frame.height = h;
	src_frame.colorspace = inp_cs;

	memcpy(&in_frame, &src_frame, sizeof(vp_frame_t));

	interlace_sbs = sps_info->is_interlace;

	// create VP batch instance with cropping
	if (sps_info->cropping_info_present_flag)
	{
		int cr_left, cr_top, cr_width, cr_height;
		cr_top  = sps_info->cropping_info.luma_offset / sps_info->allocated_frame_width;
		cr_left = sps_info->cropping_info.luma_offset - cr_top * sps_info->allocated_frame_width;
		cr_width  = sps_info->cropping_info.frame_width;
		cr_height = sps_info->cropping_info.frame_height;
		vp_open_crop(&batch, &in_frame, cr_left, cr_top, cr_width, cr_height, sps_info->is_interlace);
	}
	else
	{
		vp_open(&batch, &in_frame, interlace_sbs);
	}

	vp_bit_depth(batch, sps_info->bit_depth_luma, sps_info->bit_depth_chroma, sps_info->bit_depth_chroma);

	// select desired output colorspace
	if (colorspace >= 0)
	{
		out_cs = get_cs(colorspace);
		assert(out_cs != VP_UNKNOWN);
	}
	if (out_cs != inp_cs)
	{
		vp_add_colorspace(batch, out_cs);
	}
	// add telecine
	if (enable_telecine > 0)
	{
		vp_add_telecine(batch);
	}
	// add resize if configured in command line options
	if ((output_width > 0) && (output_height > 0))
	{
		if ((output_width != src_frame.width) || (output_height != src_frame.height))
		{
			vp_add_resize(batch, output_width, output_height);
		}
	}

	// determine output frame dimensions and init appropriate frame factory
	memset(&dst_frame, 0, sizeof(dst_frame));
	vp_get_dst_info(batch, &dst_frame);

	vp_init_frame_factory_ex(&dst_factory, dst_frame.width, dst_frame.height, bytes_per_pel, dst_frame.colorspace, NULL, read_frame, write_frame);
	// allocate output frame buffer
	dst_factory.alloc_frame(&dst_factory, &dst_frame);

	// signal that VPL instance has been created
	vpl_created = 1;
	return 1;
}

static void close_vpl(void)
{
	if (vpl_created)
	{
		dst_factory.free_frame(&dst_factory, &dst_frame);
		src_factory.close(&src_factory);
		dst_factory.close(&dst_factory);
		vp_close(batch);
	}
	vpl_created = 0;
}

static int reinit_vpl_if_needed(sps_info_t *sps_info, yuv_frame_t *dec_frame)
{
	int ret = 0;
	// check whether source frame dimensions have changed
	if ((dec_frame->width != src_frame.width) || (dec_frame->height != src_frame.height))
	{
		close_vpl();
		init_vpl(sps_info, dec_frame);
		ret = 1;
	}
	return ret;
}

static char *get_cmd_name(vp_telecine_cmd_e cmd)
{
	switch (cmd)
	{
	case VP_DIRECT:		return "";
	case VP_OLD_FRAME:	return "T0B0";
	case VP_NEW_FRAME:	return "T1B1";
	case VP_OLD_TOP_NEW_BOT:	return "T0B1";
	case VP_NEW_TOP_OLD_BOT:	return "T1B0";
	}
	return "";
}

//////////////////////////////////////////////////////////////////////////
static void writeFrame(sps_info_t *sps_info, yuv_frame_t *dec_frame)
{
	int i;
	vp_telecine_cmd_e cmd = VP_DIRECT;
	int frame_complete = 0;
	int frame_repeat = 0;

	vp_frame_t out_frame;

	if((dec_frame->bytes_per_pel == 2)
		&& (sps_info->bit_depth_luma == 8)
		&& (sps_info->bit_depth_chroma == 8))
	{
		skip_high_byte = 1;
	}

	// re-init VPL
	reinit_vpl_if_needed(sps_info, dec_frame);

	src_frame.bytes_per_pel = dec_frame->bytes_per_pel;
	src_frame.luma_bits     = sps_info->bit_depth_luma;
	src_frame.chroma_bits   = sps_info->bit_depth_chroma;

	src_frame.data[0] = dec_frame->y;
	src_frame.stride[0] = dec_frame->stride_y * dec_frame->bytes_per_pel;
	if (inp_cs != VP_YUV400)
	{
		src_frame.data[1] = dec_frame->u;
		src_frame.data[2] = dec_frame->v;
		src_frame.stride[1] = dec_frame->stride_uv * dec_frame->bytes_per_pel;
		src_frame.stride[2] = dec_frame->stride_uv * dec_frame->bytes_per_pel;
	}

	memcpy(&in_frame, &src_frame, sizeof(vp_frame_t));
	memcpy(&out_frame, &dst_frame, sizeof(vp_frame_t));

	if (enable_telecine > 0)
	{
		cmd = VP_NEW_FRAME;
		switch (pic_struct)
		{
		case 0:
		case 1:
		case 2:
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			switch (telecine_state)
			{
			case 0:	// buffer is empty
				cmd = VP_NEW_FRAME;
				switch (pic_struct)
				{
				case 3:
				case 4:
					break;
				case 5:
					telecine_state = 1;
					break;
				case 6:
					telecine_state = 2;
					break;
				}
				break;
			case 1:	// top field in buffer
				switch (pic_struct)
				{
				case 3:	// error: skip buffered field
					cmd = VP_NEW_FRAME;
					telecine_state = 0;
					break;
				case 4:
					cmd = VP_OLD_TOP_NEW_BOT;
					telecine_state = 1;
					break;
				case 5:	// error: skip buffered field
					cmd = VP_NEW_FRAME;
					telecine_state = 1;
					break;
				case 6:
					cmd = VP_OLD_TOP_NEW_BOT;
					telecine_state = 0;
					frame_complete = 1;	// frame completed
					break;
				}
				break;
			case 2:	// bottom field in buffer
				switch (pic_struct)
				{
				case 3:
					cmd = VP_NEW_TOP_OLD_BOT;
					telecine_state = 2;
					break;
				case 4:	// error: skip buffered field
					cmd = VP_NEW_FRAME;
					telecine_state = 0;
					break;
				case 5:
					cmd = VP_NEW_TOP_OLD_BOT;
					telecine_state = 0;
					frame_complete = 1;	// frame completed
					break;
				case 6:	// error: skip buffered field
					cmd = VP_NEW_FRAME;
					telecine_state = 2;
					break;
				}
				break;
			default:
				// invalid state!!!
				assert(0);
			} // switch telecine_state
			break;
		case 8:
			frame_repeat++;
		case 7:
			frame_repeat++;
			break;
		} // switch pic_struct
		//verbose_print(0,"%2d: pic_struct [%d] cmd [%s] state [%d]\n", output_frame, pic_struct, get_cmd_name(cmd), telecine_state);
		vp_reset_telecine(batch, cmd);
	} // if enable_telecine
	else
	{
		vp_reset(batch);
	}

	// process frame row by row
	in_frame.valid_rows_count = 2;
	while (in_frame.valid_rows_count <= in_frame.height)
	{
		vp_process(batch, &in_frame, &out_frame);
		in_frame.valid_rows_count += 2;
	}

	// write output frame
	dst_factory.write_frame(&dst_factory, &dst_frame);

	// completed frame
	if (frame_complete)
	{
		//verbose_print(0,"==> complete frame \n");
		vp_reset_telecine(batch, VP_NEW_FRAME);
		vp_process(batch, &src_frame, &dst_frame);
		dst_factory.write_frame(&dst_factory, &dst_frame);
		output_frame++;
	}

	// repeat frame
	for (i=0; i<frame_repeat; i++)
	{
		//verbose_print(0,"==> repeat frame \n");
		dst_factory.write_frame(&dst_factory, &dst_frame);
		output_frame++;
	}
	output_frame++;
}


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

static char *get_profile_name(int idc)
{
	switch (idc)
	{
	case  66: return "baseline";
	case  77: return "main";
	case  88: return "extended";
	case 100: return "high";
	case 110: return "high 10";
	case 122: return "high 422";
	case 244: return "high 444";
	case  44: return "high CALVC 444 intra";
	case  83: return "scalable baseline";
	case  86: return "scalable high";
	case 118: return "multiview high";
	case 128: return "stereo high";
	}
	return "unknown";
}

static char *get_chroma_name(int idc)
{
	switch (idc)
	{
	case  YUV_400_IDC: return "YUV 4:0:0";
	case  YUV_420_IDC: return "YUV 4:2:0";
	case  YUV_422_IDC: return "YUV 4:2:2";
	case  YUV_444_IDC: return "YUV 4:4:4";
	}
	return "unknown";
}

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

static char *get_sei_name(int sei_type)
{
	if (sei_type > 45) return "RESERVED";
	return sei_type_names[sei_type];
}

////////////////////////////////////////////////////////////////////////// SEI support
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
	
	verbose_print(1, " SEI #%d (%d bytes): %s", type, size, get_sei_name(type));
	if (is_compressed)
	{
		verbose_print(1, "[compressed]");
	}
	switch (type)
	{
	case SEI_BUFFERING_PERIOD:
		{
			sei_buffering_period_t *bp = sei->data;
			if (size != sizeof(sei_buffering_period_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(sei_buffering_period_t));
			}
			else
			{
				verbose_print(1,": sps=%d rd=%d rd_offs=%d", bp->seq_parameter_set_id, bp->nal_bp[0].initial_cpb_removal_delay, bp->nal_bp[0].initial_cpb_removal_delay_offset);
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
						verbose_print(0,": CC: count=%d ", cc->cc_count);
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

	case SEI_RECOVERY_POINT:
		{
			sei_recovery_point_t *rp = sei->data;
			if (size != sizeof(sei_recovery_point_t))
			{
				verbose_print(1, ": wrong size, expected=%d", sizeof(sei_recovery_point_t));
			}
			else
			{
				verbose_print(1, ": rec_frame_cnt=%d exact_match=%d", rp->recovery_frame_cnt, rp->exact_match_flag);
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

	case SEI_POST_FILTER:
		{
			sei_post_filter_t *pf = sei->data;
			if (size != sizeof(sei_post_filter_t))
			{
				verbose_print(1,": wrong size, expected=%d", sizeof(sei_post_filter_t));
			}
			else
			{
				verbose_print(1,": filter_hint_type=%d, ...", pf->filter_hint_type);
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
					verbose_print(1, "\nDQId=%2d:", current_dq_id);
					sei = handle_sei(sei);
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
						verbose_print(1, "\nView ALL ");
					else
					{
						verbose_print(1, "\nView ");
						for(i = 0; i <= mvc_nesting->num_view_components_minus1; i++)
							verbose_print(1, "%d ", mvc_nesting->sei_view_id[i]);
					}
				}
				else
				{
					verbose_print(1, "\nView");
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

static void handle_sei_list(media_sample_t *sei)
{
	while (sei != NULL)
	{
		verbose_print(1, "\t");
		sei = handle_sei(sei);
		verbose_print(1, "\n");
	}
}

//////////////////////////////////////////////////////////////////////////
static int dec_cfg(v4d_settings_t *dec_settings)
{
	// Note, that if decoder_settings.release_nal_unit_callback is not set
	// decoder will still copy NALs into internal buffer.
	// In this case caller must release NALs. It can
	// be done immediately after vssh_dec_put_nalu() call
	if (mt == 0)
	{
		v4d_default_settings(dec_settings, 1);
	}
	else
	{
		v4d_default_settings(dec_settings, 0);
		if (mt == -1) mt = 0;
		dec_settings->mt_settings.num_threads = mt;
		verbose_print(0,"*** multi-threading enabled, mt=%d\n", mt);
	}
	// enable SEI decoding
	if (enable_sei)
	{
		verbose_print(0,"*** SEI decoding enabled\n");
		dec_settings->dec_flags |= DEC_DECODE_SEI;
	}
	// enable EC
	if (enable_ec > 0)
	{
		verbose_print(0,"*** error concealment enabled\n");
		dec_settings->dec_flags |= DEC_FULL_ERR_CONC;
	}
	dec_settings->output_delay = output_delay;
	if (fp_flag)
		dec_settings->dec_flags |= DEC_INTERP_FRAME_PACKING_SEI;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
static char *nalu_type_names[] = {
	"ERR",
	"SLICE",
	"DPA",
	"DPB",
	"DPC",
	"IDR",
	"SEI",
	"SPS",
	"PPS",
	"PICSEP",
	"ESEQ",
	"ESTRM",
	"FILL",
	"13",
	"PREFIX",
	"SUBSPS",	// 15
	"16",		// 16
	"17",		// 17
	"18",		// 18
	"19",		// 19
	"SLICEEXT"		// 20
};

static char *get_nalu_name(int nalu_type)
{
	int max_index = sizeof(nalu_type_names) / sizeof(nalu_type_names[0]);
	if (nalu_type >= max_index) return "???";
	return nalu_type_names[nalu_type];
}

//////////////////////////////////////////////////////////////////////////
static int is_nalu_ext(media_sample_t *ms)
{
	int nal_type;
	nal_type = NAL_TYPE(ms->data);
	return ((nal_type == NALU_TYPE_SLICE_EXT) || (nal_type == NALU_TYPE_PREFIX_EXT));
}

static int get_pid(media_sample_t *ms)
{
	return NAL_EXT_PRIORITY_ID(ms->data);
}

static int timestamp2int(int64_t timestamp)
{
	if (timestamp < 10000) return (int)timestamp;
	return (int)(timestamp / 10000);
}

static void report_media_sample(int64_t num, media_sample_t *ms)
{
	const int verbose_level = 2;
	byte first_byte;
	int nalu_type, nalu_ridc;
	frame_info_t *info;
	first_byte = ((byte*)(ms->data))[0];
	nalu_type = NALU_TYPE(first_byte);
	nalu_ridc = NALU_RIDC(first_byte);
	info = (frame_info_t *)ms->extra_data;
	verbose_print(verbose_level, "%6d(%6d): [%d|%2d] %8s",
		(int)num, ms->used_size,
		nalu_ridc, nalu_type, get_nalu_name(nalu_type));
	switch (nalu_type)
	{
	case NALU_TYPE_SLICE_EXT:
		if(NAL_EXT_SVS_EXTENSION_FLAG(ms->data))
		{
			verbose_print(verbose_level," [%d][%d|%d|%d]",
				NAL_EXT_PRIORITY_ID(ms->data),
				NAL_EXT_DEPENDENCY_ID(ms->data),
				NAL_EXT_TEMPORAL_ID(ms->data),
				NAL_EXT_QUALITY_ID(ms->data)
				);
			if (NAL_EXT_IDR(ms->data))
				verbose_print(verbose_level," IDR");
		}
		else
		{
			verbose_print(verbose_level," MVC[%d][%d|%d]",
				NAL_MVC_EXT_PRIORITY_ID(ms->data),
				NAL_MVC_EXT_TEMPORAL_ID(ms->data),
				NAL_MVC_EXT_VIEW_ID(ms->data)
				);
			if (!NAL_MVC_EXT_NON_IDR(ms->data))
				verbose_print(verbose_level," IDR");
			if(NAL_MVC_EXT_ANCHOR_PIC(ms->data))
				verbose_print(verbose_level," ANCHOR");
		}
		break;
	}
	verbose_print(verbose_level, " %d", timestamp2int(ms->timestamp));
	verbose_print(verbose_level,"\n");
#if defined(DEBUG)
	fflush(stderr);
#endif
}

//////////////////////////////////////////////////////////////////////////
static int vbv_size = 0;
static int vbv_send = 0;
static int vbv_max  = 0;
static void report_dec_frame(yuv_frame_t *dec_frame, sps_info_t *sps_info, int64_t num)
{
	const int verbose_level = 1;
	char *slice_name;
	int is_max = 0;

	// report head line
	if (!dec_frame)
	{
		verbose_print(verbose_level," frame	num	type	qp	 bytes	  w x h 	ts");
		if (bitrate > 0)
			verbose_print(verbose_level,"	     vbv");
		verbose_print(verbose_level,"\n");
		return;
	}
	// VBV calculation
	if (bitrate > 0)
	{
		// calculate fps from stream SPS info
		if ((vbv_send == 0) && (sps_info->vui_info.num_units_in_tick > 0))
		{
			// calculate number of bits to send on every frame
			vbv_send = (int)( ((uint64_t)bitrate * 2000 * sps_info->vui_info.num_units_in_tick) / sps_info->vui_info.time_scale );
		}
		if (vbv_send > 0)
		{
			// put frame bits into VBV buffer
			vbv_size += dec_frame->info.num_bits;
			// send bits to network
			if (vbv_size < vbv_send)
			{
				// buffer underflow!
				vbv_size = 0;
			}
			else
			{
				vbv_size -= vbv_send;
			}
			// save maximum VBV buffer value
			if (vbv_max < vbv_size)
			{
				is_max = 1;
				vbv_max = vbv_size;
			}
		}
	}

	// frame report line
	slice_name = get_slice_name(dec_frame->info.slice_type);
	verbose_print(verbose_level,"%6d	%3d	%s	%2d	%6d	%4dx%d %8d",
	   (int)num
	   ,dec_frame->info.frame_num
	   ,slice_name
	   ,dec_frame->info.qp_used
	   ,(dec_frame->info.num_bits / 8)
	   ,dec_frame->image_width
	   ,dec_frame->image_height
	   ,timestamp2int(dec_frame->info.timestamp)
	   );
	if (vbv_size > 0)
	{
		verbose_print(verbose_level," %8d%s", vbv_size, (is_max?" *":""));
	}
	if (ref_file_handle != NULL)
	{
		psnr_ssim_set_frame(&psnr_ssim, 0, sps_info, dec_frame, ref_file_handle);
		if (psnr_ssim.max_psnr)
			verbose_print(verbose_level,"   %.4f %.4f %.4f", psnr_ssim.psnr_frame[0], psnr_ssim.psnr_frame[1], psnr_ssim.psnr_frame[2]);
		if (psnr_ssim.ssim_enabled)
			verbose_print(verbose_level,"      %.4f %.4f %.4f", psnr_ssim.ssim_frame[0], psnr_ssim.ssim_frame[1], psnr_ssim.ssim_frame[2]);
	}
	verbose_print(verbose_level,"\n");

	// report decoding error (if any)
	if (dec_frame->info.error_no != 0)
	{
		// verbose level 0 always print error!!!
		verbose_print(0,"Decode error #%d: %s\n", dec_frame->info.error_no, v4_error_text(dec_frame->info.error_no));
	}
}

//////////////////////////////////////////////////////////////////////////
// we demonstrate here approach which reads input H.264 file by fixed size blocks
// and then uses parser to separate NAL units by start codes
// note that decoder library v4 accepts only complete NAL units
//
#define INIT_NAL_SIZE (8*1024)
#define READ_BUF (4*1024)
// assume 4-byte start codes (informative)
#define STARTCODE_LEN (4)



static void disply_common_stat(int64_t num_frames, int64_t total_bytes, int total_ms)
{
	// display common stats
	verbose_print(0,"---------------------------------------------------------------\n");
	verbose_print(0,"total frames     %I64d\n", num_frames);
	verbose_print(0,"total bytes      %I64d\n", total_bytes);
	verbose_print(0,"total time       %d ms\n", total_ms);
	if (total_ms > 0)
		verbose_print(0,"total speed      %6.2f fps\n", (num_frames*1000.0)/total_ms );
}

int need_svc_report = 0;
static void report_svc_buffer(void *substream_extractor)
{
	if(verbose > 2 && need_svc_report)
	{
		int dq_id, kbps, vbv_size, framerate;
		int vbv_size_bits = v4d_substream_extractor_get_vbv_buffer(substream_extractor, &dq_id, &kbps, &vbv_size, &framerate);
		if(kbps)
			verbose_print(3, "Sub-stream: DQId=%d  VBV size=%d ms (%d bits) bitrate=%d kbps @ %g fps\n", dq_id, vbv_size, vbv_size_bits, kbps, framerate/10000.);
		need_svc_report = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
static int dec_run(void)
{
	byte *input_block = NULL;
	int rc, read_size, end_of_stream, end_of_ms;
	v4d_settings_t dec_settings;
	void *dec_handle;
	void *nal_parser;
	void *substream_extractor = NULL;
	int is_skip_extractor = 0;
	media_sample_t *ms;
	yuv_frame_t  local_frame;
	yuv_frame_t *dec_frame = &local_frame;
	sps_info_t sps_info;
	int64_t total_bytes, num_frames, ms_num;
	timer_clock_t start_time;
	timer_delta_t delta_time;
	int total_ms;
	int64_t ts, dts;
	int view_ix;
	int frame_errors = 0;

	// prepare decoder configuration
	memset(&sps_info, 0, sizeof(sps_info));
	dec_cfg(&dec_settings);

	// create NAL parser
	nal_parser = v4d_nal_extractor_create_ex(INIT_NAL_SIZE, 0);
	if (nal_parser == NULL)
	{
		verbose_print(0,"Error: v4d_nal_extractor_create() returned NULL\n");
		return -1;
	}

	// create decoder instance
	rc = v4d_open(&dec_handle, &dec_settings);
	if (rc != VSSH_OK)
	{
		verbose_print(0,"Error: v4d_open() failed (%d): %s\n", rc, v4_error_text(rc));
		return -2;
	}

	// allocate memory block for input file reading
	input_block = (byte*)v4_malloc(READ_BUF);
	if (input_block == NULL)
	{
		verbose_print(0,"Error: can't allocate input memory (%d bytes)\n", READ_BUF);
		return -3;
	}

	// create Sub-stream Extractor
	is_skip_extractor = (target_p_id == -1 && target_dq_id == -1 && target_t_id == -1 && svc_bitrate == 0);
	if(!is_skip_extractor)
	{
		if(target_p_id == 0 && target_dq_id == -1)
			target_dq_id = 0;	// trick to correct extraction of base layer/view
		substream_extractor = v4d_substream_extractor_create(target_p_id, target_t_id, target_dq_id, 0);
		if(substream_extractor == NULL)
		{
			verbose_print(0,"Error: v4d_substream_extractor_create() returned NULL\n");
			return -4;
		}
		
		if(svc_bitrate)
		{
			v4d_substream_extractor_set_svc_bitrate(substream_extractor, svc_bitrate, vbv_delay, -1, frame_rate);
			bitrate = svc_bitrate;
		}
	}
	target_t_id  &= MAX_TEMPORAL_ID;
	target_p_id  &= MAX_PRIORITY_ID;
	target_dq_id &= MAX_DQ_ID;

	// print head line
	report_dec_frame(NULL, NULL, 0);

	// main file decoding cycle
	timer_init();
	timer_start(&start_time);
	total_bytes = 0;
	num_frames = 0;
	ms_num = 0;
	ts = 0;
	dts = 1;
	for (end_of_stream=0; !end_of_stream; )
	{
		// read next data block from the file
		// note: we do not use time stamps here, but they should be provided to parser
		// in order to track frames
		read_size = readFile(input_block, READ_BUF);
		if (read_size <= 0)	// end of file
		{
			// inform parser about end of stream to allow the last NAL unit be treated as completed
			v4d_nal_extractor_feed_data(nal_parser, NULL, 0,  0, NO_TIME_STAMP);
			end_of_stream = 1;
			if (forever  > 0) forever -= 1;
			if (forever != 0)
			{
				rewind_input_file();
				verbose_print(2, "*** rewind (%d)\n", forever);
				delta_time = timer_delta(&start_time);
				total_ms = timer_delta_ms(delta_time);
				disply_common_stat(num_frames, total_bytes, total_ms);

				end_of_stream = 0;
			}
		}
		else
		{
			// feed next portion of H.264 stream into input parser
			v4d_nal_extractor_feed_data(nal_parser, input_block, read_size,  0, ts);
			ts += dts;
		}
		///////// extract all available NAL units from parser and decode them
		for (end_of_ms=0; !end_of_ms; )
		{
			// extract next NAL unit from parser
			ms = v4d_nal_extractor_get_nalu(nal_parser);
			if (ms)
			{
				if(is_skip_extractor)
				{
					total_bytes += ms->used_size + STARTCODE_LEN;
					report_media_sample(ms_num, ms);
					ms_num++;
					// put NAL unit into svc_out
					write_svc_file(ms);
					// put NAL unit into decoder library
					v4d_set_nal_unit(dec_handle, ms);
				}
				else
				{
					v4d_substream_extractor_feed_nalu(substream_extractor, ms);
					for(;;)//while (1)
					{
						ms = v4d_substream_extractor_get_nalu(substream_extractor);
						if (ms == NULL) break;
						total_bytes += ms->used_size + STARTCODE_LEN;
						report_media_sample(ms_num, ms);
						ms_num++;
						// put NAL unit into svc_out
						write_svc_file(ms);
						// put NAL unit into decoder library
						v4d_set_nal_unit(dec_handle, ms);
						need_svc_report = 1;
					}
					if(svc_bitrate)
						report_svc_buffer(substream_extractor);
				}				
			}
			else 
			{
				end_of_ms = 1;

				// tell decoder that it is the last NAL unit
				if (end_of_stream)
				{
					if (!is_skip_extractor)
					{
						v4d_substream_extractor_feed_nalu(substream_extractor, NULL);
						for(;;)//while (1)
						{
							ms = v4d_substream_extractor_get_nalu(substream_extractor);
							if (ms == NULL) break;
							total_bytes += ms->used_size + STARTCODE_LEN;
							report_media_sample(ms_num, ms);
							ms_num++;
							// put NAL unit into svc_out
							write_svc_file(ms);
							// put NAL unit into decoder library
							v4d_set_nal_unit(dec_handle, ms);
							need_svc_report = 1;
						}
						if(svc_bitrate)
							report_svc_buffer(substream_extractor);
					}

					v4d_set_nal_unit(dec_handle, NULL);
				}
			}
			// now try to receive all decoded YUV frames
			while (VSSH_OK == v4d_get_frame(dec_handle, &dec_frame, &sps_info))
			{
				// print frame information
				yuv_frame_t *frame_to_output = dec_frame;

				// process sei list (if enabled)
				pic_struct = 0;
				if (enable_sei)
				{
					// this function will also assign "pic_struct" value if any
					handle_sei_list(dec_frame->info.sei_list);
					frame_to_output = v4d_film_grain(dec_handle, dec_frame); //apply film_grain if any
					if (frame_to_output == NULL)
						frame_to_output = v4d_post_filter(dec_handle, dec_frame); //apply post-filtering if any
					if (frame_to_output == NULL)
						frame_to_output = dec_frame;
				}
				report_dec_frame(frame_to_output, &sps_info, num_frames);
				if (dec_frame->info.error_no != 0)
				{
					frame_errors += 1;
				}


				// write frame into output file (if any)
				if(output_file)
				{
					if((start_frame == 0) || (num_frames >= start_frame))
					{
						FILE *save_base_file = file_out;
						writeFrame(&sps_info, frame_to_output);
						
						view_ix = 0;
						while (VSSH_OK == v4d_get_next_frame_view(dec_handle, &dec_frame, dec_frame, &sps_info))
						{
							file_out = get_mvc_file(++view_ix);
							writeFrame(&sps_info, dec_frame);
						}
						file_out = save_base_file;
					}
				}
				
				num_frames++;
				// check for maximum frames decoded (written)
				if ((max_frames > 0) && (num_frames >= max_frames))
				{
					end_of_ms = 1;
					end_of_stream = 1;
					break;
				}
			}
		} // for (!end_of_ms)
		//////////////////////////////////////////////////////
	} // for (!end_of_stream)
	
	delta_time = timer_delta(&start_time);
	total_ms = timer_delta_ms(delta_time);

	// release all the stuff
	v4d_close(dec_handle);
	v4d_nal_extractor_close(nal_parser);
	v4_free(input_block);
	if(substream_extractor != NULL)
		v4d_substream_extractor_close(substream_extractor);

	if (vpl_created)
	{
		close_vpl();
	}


	// report totals
	if (sps_info.profile_idc > 0)
	{
		// display SPS info
		verbose_print(0,"---------------------------------------------------------------\n");
		verbose_print(0,"profile          %s (%d) \n", get_profile_name(sps_info.profile_idc), sps_info.profile_idc);
		verbose_print(0,"level            %d.%d (%d)\n", sps_info.level_idc/10, sps_info.level_idc%10, sps_info.level_idc);
		verbose_print(0,"chroma           %s (%d)\n", get_chroma_name(sps_info.yuv_format), sps_info.yuv_format);
		verbose_print(0,"interlace        %s\n", (sps_info.is_interlace?"yes":"no"));
		verbose_print(0,"resolution       %d x %d\n", sps_info.cropping_info.frame_width, sps_info.cropping_info.frame_height);
		if (sps_info.vui_info_present_flag && (sps_info.vui_info.num_units_in_tick > 0))
		{
			double fps = sps_info.vui_info.time_scale / (2. * sps_info.vui_info.num_units_in_tick);
			verbose_print(0,"frame rate       %g (%d/%d)\n", fps,
				sps_info.vui_info.time_scale, sps_info.vui_info.num_units_in_tick);
			if (fps > 0)
			{
				uint64_t time_ms = (2000 * (uint64_t)num_frames * sps_info.vui_info.num_units_in_tick) / sps_info.vui_info.time_scale;
				uint64_t kbits = total_bytes * 8;
				uint64_t bitrate = kbits / time_ms;
				verbose_print(0,"bitrate          %u kbps (%u / %u)\n", (unsigned int)bitrate, (unsigned int)kbits, (unsigned int)time_ms);
			}
		}
	}


	disply_common_stat(num_frames, total_bytes, total_ms);
	// display VBV buffer results
	if (vbv_send > 0)
	{
		verbose_print(0,"---------------------------------------------------------------\n");
		verbose_print(0,"VBV buffer max   %d kbit (send=%d fps=%.3f)\n", (vbv_max+500)/1000, vbv_send, 1000.*bitrate/vbv_send);
	}

	//DISPLAY psnr results
	if (ref_file_handle)
	{
		if (psnr_ssim.max_psnr)
		{
			verbose_print(0, "PSNR        (Y, U, V): %7.4f %7.4f %7.4f\n", 
				psnr_ssim.psnr_total[0] / num_frames,
				psnr_ssim.psnr_total[1] / num_frames,
				psnr_ssim.psnr_total[2] / num_frames);
			verbose_print(0, "Global PSNR (Y, U, V): %7.4f %7.4f %7.4f\n", 
				10.0 * log10((255.*255.)/(psnr_ssim.mse_total[0]/num_frames)),
				10.0 * log10((255.*255.)/(psnr_ssim.mse_total[1]/num_frames)),
				10.0 * log10((255.*255.)/(psnr_ssim.mse_total[2]/num_frames)));
		}
		if (psnr_ssim.ssim_enabled)
			verbose_print(0, "SSIM        (Y, U, V): %7.4f %7.4f %7.4f\n", 
				psnr_ssim.ssim_total[0] / num_frames,
				psnr_ssim.ssim_total[1] / num_frames,
				psnr_ssim.ssim_total[2] / num_frames);

	}

	return frame_errors;
}

//////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	int rc;
	arg_ctx_t arg_ctx;
	int frame_errors;
#ifdef _CRT_DEBUG_
	_CrtMemState _ms1, _ms2, _ms3;
	_CrtMemCheckpoint(&_ms1);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	// print codec library version
	verbose_print(0,"sample decoder application\n");

	// display Codec info
	print_codec_info();

	// process command line arguments
	arg_ctx.args = all_args;
	arg_ctx.cnt  = all_args_cnt;
	if (argc < 2)
	{
		usage();
		print_args(&arg_ctx);
		return 0;
	}
	// first argument without dash is input file
	if (argc > 1 && (strlen(argv[1]) > 1) && (argv[1][0] != '-'))
	{
		input_file = argv[1];
		argc--;
		argv++;
		// second argument without dash is output file
		if (argc > 1 && (strlen(argv[1]) > 1) && (argv[1][0] != '-'))
		{
			output_file = argv[1];
			argc--;
			argv++;
		}
	}
	// default argument handling
	read_args(&arg_ctx, argc, argv);
	if (enable_telecine) enable_sei = 1;
	if (verbose) dump_args(&arg_ctx);

	init_md5_sum();

	// open input & output files
	rc = openFiles();
	if (rc != 0) return -1;

	if (ref_file)
	{
		ref_file_handle = fopen(ref_file, "rb");
		if (ref_file_handle == NULL)
			verbose_print(0, "Can't open reference file [%s]\n", ref_file);
		else
			psnr_ssim_init(&psnr_ssim, max_psnr, enable_ssim);
	}
	//////////////////////// run console decoder
	frame_errors = dec_run();
	////////////////////////

	// close files
	closeFiles();
	if (ref_file_handle != NULL)
		fclose(ref_file_handle);
	check_md5_sum();

#ifdef _CRT_DEBUG_
	_CrtMemCheckpoint(&_ms2);
	_CrtMemDifference(&_ms3, &_ms1, &_ms2);
	_CrtMemDumpStatistics(&_ms3);
	_CrtDumpMemoryLeaks();
#endif

	if(frame_errors != 0)
	{
		verbose_print(0, "\nframe errors %d\n", frame_errors);
	}

	return frame_errors;
}
////////////////////////////////////////////////////////////////////////// eof
