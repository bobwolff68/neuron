
/**
 * @file sample_transcode.c
 * Example of AVC Encoder console application
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

// codec headers
#include "v4e_api.h"
#include "v4d_api.h"

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

#include "v4timer.h"
#include "v4info.h"
#include "v4args.h"
#include "v4file.h"


#define MAX_FILE_NAME 1024

typedef struct transcode_data_t
{
	//Public members, set from command line or cfg file
	//Input
	int frame_width; //Dimensions of the highest resolution for encoder
	int frame_height; //this resolution can differ from resolution of input stream
	int max_frames; //max number of input frames to process
	int frame_start; //start frame to code
	int mt_dec; //mt parameter for decoder
	int transcode; //transcode mode (0 - simple recode; 1 - transcode
	int verbose; //level of verbose messages (0/1/2)"
	char *input_file;  //Name of input h.264 file
	char *orig_file;   //Name of input original yuv-file. used to substitude decoded pixels by original one
	char *output_file; //Name of output h.264 file
	char *config_file; //Name of encoding gfg file
	v4e_settings_t enc_settings; //encoder settings
	//Output
	int status; //current transcoding status. 0 - means all OK; or error code. 


	//Private members used during transcoding
	FILE *file_in; //input h.264 file 
	FILE *orig_in; //input yuv file. Must be yuv of the same size as decoded yuv 
	byte *orig_pixels;   //buffer for original pixels
	int orig_pixels_size;
	int num_out_files; //number of output h.264 files (calculated fro encoder settings)
	FILE *files_out[MAX_AVC_STREAMS]; //output files; several files in case of Parallelstream encoding 
	v4d_settings_t dec_settings; //decoder settings
	void *enc_handle; //encoder handle
	void *dec_handle; //decoder handle
	void *nal_extractor; //nal extractor for decoder
} transcode_data_t;
//functions to work with the above structure

// ---- Public functions ----------- 
//open input file and creates nal extractor and decoder
//Called after all public members are set
//Return 0 - OK or error code
int sample_transcode_init(transcode_data_t *transcode_data);

//run all transcoding (called after above function is called and returns OK)
//Note, that encoder init will be done somewhere inside this function.
//This is done because dimension of frames in input stream is not defined yet
void sample_transcode_run(transcode_data_t *transcode_data);

//close all
void sample_transcode_close(transcode_data_t *transcode_data);


// ----------------------------- Implementation -----------------------------------
#define READ_BUF 4096
#define INIT_NAL_SIZE 4096
static byte input_block[READ_BUF];

static void output_files_init(transcode_data_t *transcode_data)
{
	v4e_settings_t *s = &transcode_data->enc_settings;
	int i, num_ms_streams;
	char ms_draft[MAX_FILE_NAME];
	char *dot;

	if (transcode_data->output_file == NULL)
	{
		verbose_print(0, "Warning: No output file specified\n");
	}
	else if (s->svc.num_layers <= 0 || s->svc.multistream_mode != MULTISTREAM_MODE_AVC)
	{
		transcode_data->num_out_files = 1;
		transcode_data->files_out[0] = fopen(transcode_data->output_file, "wb");
		if(transcode_data->files_out[0] == NULL)
			verbose_print(0, "Warning: can't open output file [%s]\n", transcode_data->output_file);
	}
	else
	{
		transcode_data->num_out_files = num_ms_streams = s->svc.num_layers + 1;
		strcpy(ms_draft, transcode_data->output_file);
		dot = strchr(ms_draft, '.');
		if (dot != NULL) *dot = '\0';
		for(i = 0; i < num_ms_streams; i++)// Generate stream names and open output files
		{
			int width, height, bitrate, qp;
			char ms_name[MAX_FILE_NAME];
			char suffix[MAX_FILE_NAME];

			if(i == 0)
			{	// from base layer settings
				width   = s->frame_width;
				height  = s->frame_height;
				bitrate = s->rc.kbps;
				qp      = s->rc.qp_intra;
			}
			else
			{	// from svc layers settings
				width   = s->svc.layer[i-1].frame_width;
				height  = s->svc.layer[i-1].frame_height;
				bitrate = s->svc.layer[i-1].kbps;
				qp      = s->svc.layer[i-1].qp_intra;
			}

			if(s->svc.multistream_mode == MULTISTREAM_MODE_MVC)
				sprintf(suffix, "%d.264", i);
			else
			{
				if(s->rc.type != RATE_CONTROL_QP)
					sprintf(suffix, "%d_%dx%d_%dkbps.264", i+1, width, height, bitrate);
				else
					sprintf(suffix, "%d_%dx%d_qp%d.264", i+1, width, height, qp);
			}
			strcpy(ms_name, ms_draft);
			strncat(ms_name, suffix, sizeof(suffix));
			transcode_data->files_out[i] = fopen(ms_name, "wb");
			if(transcode_data->files_out[i] == NULL)
				verbose_print(0, "Warning: can't open multi-stream file [%s]\n", ms_name);
		}
	}
}


static void output_files_close(transcode_data_t *transcode_data)
{
	int i;

	for(i = 0; i < transcode_data->num_out_files; i++)
	{
		if(transcode_data->files_out[i] != NULL)
			fclose(transcode_data->files_out[i]);
		transcode_data->files_out[i] = NULL;
	}
}


static void prepare_dec_settings(transcode_data_t *transcode_data)
{
	v4d_settings_t *dec_settings = &transcode_data->dec_settings;
	v4d_default_settings(dec_settings, transcode_data->mt_dec == 0);
	if (transcode_data->mt_dec > 0)
		dec_settings->mt_settings.num_threads = transcode_data->mt_dec;
}

int sample_transcode_init(transcode_data_t *transcode_data)
{
	int rc;
	prepare_dec_settings(transcode_data);
	// create NAL parser
	if (transcode_data->input_file == NULL)
	{
		verbose_print(0,"Error: No input file specified\n");
		transcode_data->status = -1;
		return -1;
	}

	transcode_data->file_in = fopen(transcode_data->input_file, "rb");

	if (transcode_data->orig_file != NULL) 
		transcode_data->orig_in = fopen(transcode_data->orig_file, "rb");


	if (transcode_data->file_in == NULL)
	{
		verbose_print(0," Error: Can't open file: %s\n", transcode_data->input_file);
		transcode_data->status = -1;
		return -1;
	}

	output_files_init(transcode_data); //not required

	transcode_data->nal_extractor = v4d_nal_extractor_create_ex(INIT_NAL_SIZE, 0);
	if (transcode_data->nal_extractor == NULL)
	{
		verbose_print(0,"Error: v4d_nal_extractor_create() returned NULL\n");
		transcode_data->status = -1;
		return -1;
	}

	// create decoder instance
	rc = v4d_open(&transcode_data->dec_handle, &transcode_data->dec_settings);
	if (rc != VSSH_OK)
	{
		verbose_print(0,"Error: v4d_open() failed (%d): %s\n", rc, v4_error_text(rc));
		transcode_data->status = -2;
		return -2;
	}

	return 0;
}

//this function is called for each encoded NAL
static int receive_nal_callback(void *context, media_sample_t *ms)
{
	transcode_data_t *transcode_data = context;
	if (ms)
	{
		static byte start_code[4] = {0,0,0,1};
		frame_info_t *frame_info = (frame_info_t *)(&ms->extra_data);
		int n = 0;

		if (transcode_data->num_out_files > 1) n = frame_info->num_stream;

		if (n < transcode_data->num_out_files && transcode_data->files_out[n] != NULL)
		{
			fwrite(start_code, 1, 4, transcode_data->files_out[n]);
			fwrite(ms->data, 1, ms->used_size, transcode_data->files_out[n]);
		}
		v4_free_media_sample(ms);
	}
	return 0;
}


static void init_encoder(transcode_data_t *transcode_data, yuv_frame_t *dec_frame)
{
	v4_receive_nal_t    nal_receive;
	int rc;
	//TODO replace check below by proper update of preproc settings
	if (dec_frame->image_width != transcode_data->enc_settings.input.width || dec_frame->image_height != transcode_data->enc_settings.input.height)
	{
		//add resize step
		int i;
		if (dec_frame->image_width < transcode_data->enc_settings.input.width || dec_frame->image_height < transcode_data->enc_settings.input.height)
		{
			verbose_print(0, "Error: Upscaling is not allowed\n");
			transcode_data->status = -1;
			return;
		}
		assert(transcode_data->enc_settings.preproc.step[6].type == S_NONE);
		for (i = 6; i > 0; i--)
			transcode_data->enc_settings.preproc.step[i] = transcode_data->enc_settings.preproc.step[i-1];
		transcode_data->enc_settings.preproc.step[0].type = RESIZE;
		transcode_data->enc_settings.preproc.step[0].param0 = transcode_data->enc_settings.input.width;
		transcode_data->enc_settings.preproc.step[0].param1 = transcode_data->enc_settings.input.height;
		transcode_data->enc_settings.input.width = dec_frame->image_width;
		transcode_data->enc_settings.input.height = dec_frame->image_height;
	}
	nal_receive.context = transcode_data;
	nal_receive.callback = receive_nal_callback;
	rc = v4e_open_ex(&transcode_data->enc_handle, &transcode_data->enc_settings, NULL, &nal_receive);
	if (rc > VSSH_OK)
	{
		verbose_print(0, "Warning: v4e_open_ex(): %d [%s]\n", rc, v4_error_text(rc));
	}
	if (rc < VSSH_OK)
	{
		verbose_print(0, "Error: v4e_open_ex(): %d [%s]\n", rc, v4_error_text(rc));
		transcode_data->status = rc;
		return;
	}
}

static void pass_yuv_to_encoder(transcode_data_t *transcode_data, yuv_frame_t *dec_frame)
{
	if (transcode_data->status == 0 && transcode_data->enc_handle == NULL && dec_frame)
	{
		init_encoder(transcode_data, dec_frame);
		if (transcode_data->status != 0)
			verbose_print(0,"Error: init_encoder() failed (%d): %s\n", transcode_data->status, v4_error_text(transcode_data->status));
		if (transcode_data->orig_in) //we will substitude decoded pixels by original pixels. Alloc buffer to read original pixels
		{
			transcode_data->orig_pixels_size = 3*dec_frame->image_height*dec_frame->image_width/2; //TODO 10-bit and 422
			transcode_data->orig_pixels = malloc(transcode_data->orig_pixels_size);
		}

	}
	if (transcode_data->status != 0)
		return;

	if (dec_frame)
	{
		vp_raw_frame_t vp_raw_frame;
		media_sample_t *mbs_data = NULL;
		assert(dec_frame->image_width == transcode_data->enc_settings.input.width);
		assert(dec_frame->image_height == transcode_data->enc_settings.input.height);
		memset(&vp_raw_frame, 0, sizeof(vp_raw_frame_t));
		vp_raw_frame.vp_frame.luma_bits = 8; //This is not needed. TODO remove when VPL will be fixed
		if (transcode_data->orig_pixels != NULL) //read original pixels to pass to transcoder
		{
			int size = (int)fread(transcode_data->orig_pixels, 1, transcode_data->orig_pixels_size, transcode_data->orig_in);
			if (size > 0)
			{
				vp_raw_frame.vp_frame.data[0] = transcode_data->orig_pixels;
				vp_raw_frame.vp_frame.data[1] = transcode_data->orig_pixels + dec_frame->image_height*dec_frame->image_width;
				vp_raw_frame.vp_frame.data[2] = transcode_data->orig_pixels + 5*dec_frame->image_height*dec_frame->image_width/4;
				vp_raw_frame.vp_frame.stride[0] = dec_frame->image_width;
				vp_raw_frame.vp_frame.stride[1] = dec_frame->image_width/2;
				vp_raw_frame.vp_frame.stride[2] = dec_frame->image_width/2;
			}
			else //nothing to read from file. Finish with it
			{
				free(transcode_data->orig_pixels);
				transcode_data->orig_pixels = NULL;
			}
		}
		if (transcode_data->orig_pixels == NULL) //pass decoded pixels
		{
			vp_raw_frame.vp_frame.data[0] = dec_frame->y;
			vp_raw_frame.vp_frame.data[1] = dec_frame->u;
			vp_raw_frame.vp_frame.data[2] = dec_frame->v;
			vp_raw_frame.vp_frame.stride[0] = dec_frame->stride_y;
			vp_raw_frame.vp_frame.stride[1] = dec_frame->stride_uv;
			vp_raw_frame.vp_frame.stride[2] = dec_frame->stride_uv;
		}
		//TODO timstamps and other
		if (transcode_data->transcode)
			v4d_get_frame_macroblocks_data(transcode_data->dec_handle, dec_frame, &mbs_data);
		v4e_set_vp_frame_and_mbs_data(transcode_data->enc_handle, &vp_raw_frame, mbs_data, 1);
	}
	else if (transcode_data->enc_handle)
		v4e_set_flush(transcode_data->enc_handle);
}


void sample_transcode_run(transcode_data_t *transcode_data)
{
	media_sample_t *ms;
	yuv_frame_t *dec_frame;
	timer_clock_t start_time;
	timer_delta_t delta_time;
	int total_frames_read, total_frames_coded, total_ms, end_of_stream, end_of_ms;
	int64_t ts, dts;

	timer_init();
	timer_start(&start_time);

	ts = 0;
	dts = 1;
	total_frames_read = total_frames_coded = 0;
	for (end_of_stream = 0; !end_of_stream && transcode_data->status == 0; )
	{
		int read_size = (int)fread(input_block, 1, READ_BUF, transcode_data->file_in);
		if (read_size <= 0)	// end of file
		{
			// inform parser about end of stream to allow the last NAL unit be treated as completed
			v4d_nal_extractor_feed_data(transcode_data->nal_extractor, NULL, 0,  0, NO_TIME_STAMP);
			end_of_stream = 1;
		}
		else
		{
			// feed next portion of H.264 stream into input parser
			v4d_nal_extractor_feed_data(transcode_data->nal_extractor, input_block, read_size,  0, ts);
			ts += dts;
		}
		///////// extract all available NAL units from parser and decode them
		for (end_of_ms=0; !end_of_ms; )
		{
			// extract next NAL unit from parser
			ms = v4d_nal_extractor_get_nalu(transcode_data->nal_extractor);
			if (ms)
				v4d_set_nal_unit(transcode_data->dec_handle, ms);
			else 
			{
				end_of_ms = 1;
				if (end_of_stream) // tell decoder that it is the last NAL unit
					v4d_set_nal_unit(transcode_data->dec_handle, NULL);
			}
			// now try to receive all decoded YUV frames
			while (VSSH_OK == v4d_get_frame(transcode_data->dec_handle, &dec_frame, NULL))
			{
				total_frames_read++;
				if (total_frames_read >= transcode_data->frame_start)
				{
					pass_yuv_to_encoder(transcode_data, dec_frame);
					total_frames_coded++;
				}
				verbose_print(0, "Frames: Read:%10d    Coded:%10d         \r", total_frames_read, total_frames_coded);
				if (total_frames_coded == transcode_data->max_frames)
				{
					end_of_ms = end_of_stream = 1;
					break;
				}
			}
		}
	}
	pass_yuv_to_encoder(transcode_data, NULL);

	delta_time = timer_delta(&start_time);
	total_ms = timer_delta_ms(delta_time);

	verbose_print(0, "%20s: %d\n", "total frames", total_frames_coded);
	verbose_print(0, "%20s: %d\n", "total time, ms", total_ms);
	if (total_ms > 0)
		verbose_print(0, "%20s: %d\n", "total speed, fps", (total_frames_coded*1000 + total_ms/2)/total_ms);
}

void sample_transcode_close(transcode_data_t *transcode_data)
{
	output_files_close(transcode_data);
	if (transcode_data->file_in) fclose(transcode_data->file_in);
	if (transcode_data->orig_in) fclose(transcode_data->orig_in);
	if (transcode_data->dec_handle) v4d_close(transcode_data->dec_handle);
	if (transcode_data->enc_handle) v4e_close(transcode_data->enc_handle);
	if (transcode_data->nal_extractor) v4d_nal_extractor_close(transcode_data->nal_extractor);
}

// --------------- end of transcode module implementation -------------------



// ------------- main function stuff ---------------- 

static transcode_data_t g_transcode_data = {0}; //static structure



//////////////////////////////////////////////////// command line arguments
// map input parameters to command line options  
static cmd_arg_t all_args[] =
{
	{ "i", "yf", "input-file",   1, &g_transcode_data.input_file,  0, "name of input H.264 file" },
	{ "o", "hf", "output-file",  1, &g_transcode_data.output_file, 0, "name of output H.264 file" },
	{ "-", "of", "orig-file",    1, &g_transcode_data.orig_file,   0, "name of input original yuv file" },
	{ "c", "cf", "config-file",  1, &g_transcode_data.config_file, 0, "name of encoder config file" },
	{ "w", "fw", "frame-width",  0, &g_transcode_data.frame_width, 0, "input frame width, pixels (divisible by 2)" },
	{ "h", "fh", "frame-height", 0, &g_transcode_data.frame_height,0, "input frame height, pixels (divisible by 2)" },
	{ "n", "fn", "frame-count",  0, &g_transcode_data.max_frames,  0, "max number of input frames to process" },
	{ "s", "fs", "frame-start",  0, &g_transcode_data.frame_start, 0, "frame to start encoding" },
	{ "v", "vb", "verbose",      0, &g_transcode_data.verbose,     1, "level of verbose messages (0/1/2)" },
	{ "m", "mt", "threads",      0, &g_transcode_data.mt_dec,     -1, "number of threads in decoder(-1=auto, 0=disable)" },
	{ "t", "tm", "transcode",    0, &g_transcode_data.transcode,   1, "transcode mode (0 - simple recode; 1-transcode)" },
};

static const int all_args_cnt = sizeof(all_args)/sizeof(all_args[0]);


//////////////////////////////////////////////////////////////////////////
static void usage(void)
{
	verbose_print(0, "USAGE:\n");
	verbose_print(0, "\t > sample_transcode [-option value] [param=value]\n");
	verbose_print(0, "WHERE:\n");
}


////////////////////////////////////////////////////////////////////////// dump encoder params
static int dump_nondef_params(void *ctx, char *name, char *fmt, int value, int def)
{
	char sformat[256];
	if (value != def)
	{
		sprintf(sformat, "%%s = %s\n", fmt);
		verbose_print(1, sformat, name, value);
	}
	return 0;
}

static int dump_all_params(void *ctx, char *name, char *fmt, int value, int def)
{
	char sformat[256];
	sprintf(sformat, "%%s = %s\n", fmt);
	verbose_print(1, sformat, name, value);
	return 0;
}

static void dump_encoder_settings(v4e_settings_t *settings, int verbose)
{
	// use callback function to dump encoder settings to standard output
	switch (verbose)
	{
	case 1:
		v4e_settings2buf_ex(settings, NULL, dump_nondef_params);
		break;
	case 2:
		v4e_settings2buf_ex(settings, NULL, dump_all_params);
		break;
	default:
		return;
	}
	verbose_print(1, "\n");
}


//////////////////////////////////////////////////////////////////////////
int prepare_enc_settings(transcode_data_t *transcode_data, int argc, char *argv[])
{
	v4e_settings_t *settings = &transcode_data->enc_settings; 
	int i, rc;
	// in order to avoid collisions application has to specify size of settings structure
	settings->size = sizeof(v4e_settings_t);
	rc = v4e_default_settings(settings);
	if (rc != VSSH_OK)
	{
		if (rc == VSSH_ERR_ARG)
			verbose_print(0, "Error: invalid version of encoder settings linked in (%d)\n", rc);
		else
			verbose_print(0, "Error: v4e_default_settings() failed (%d)\n", rc);
		return rc;
	}

	// read encoder config file
	if (transcode_data->config_file)
	{
		rc = v4e_read_config_file(settings, transcode_data->config_file);
		if (rc != VSSH_OK)
		{
			verbose_print(0, "Error: can't open config file [%s]\n", transcode_data->config_file);
			return rc;
		}
	}

	// command line arguments override encoder settings:
	if (transcode_data->frame_width > 0)
		settings->input.width  = transcode_data->frame_width;
	if (transcode_data->frame_height > 0)
		settings->input.height = transcode_data->frame_height;

	// now scan all command line arguments for a pairs like "name=value"
	// and try to apply them to encoder settings structure
	for (i=1; i<argc; i++)
	{
		char *param = argv[i];
		if (strchr(param, '='))
		{
			int len = (int)strlen(param);
			int rc = v4e_buf2settings(settings, param, len);
			if (rc == VSSH_OK)
			{
				verbose_print(1, "*** override [%s]\n", param);
			}
		}
	}
	return 0;
}
//////////////////////////////////////////////////////////////////////////
/*
This program demonstrates trancoding from input H.264 file
*/


int main(int argc, char *argv[])
{
	int rc;
	arg_ctx_t arg_ctx;

#ifdef _CRT_DEBUG_
	_CrtMemState _ms1, _ms2, _ms3;
	_CrtMemCheckpoint(&_ms1);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	// print codec library version
	verbose_print(0, "sample transcoder application\n");
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

	read_args(&arg_ctx, argc, argv);
	if (verbose) dump_args(&arg_ctx);


	rc = prepare_enc_settings(&g_transcode_data, argc, argv);
	if (rc != 0) return -1;

	rc = v4e_check_settings(&g_transcode_data.enc_settings);
	if (rc < VSSH_OK) 
	{
		verbose_print(0, "Error: from check_settings(%d)\n", rc);
		return -1;
	}
	if (g_transcode_data.enc_settings.preproc.crop.enable)
	{
		verbose_print(0, "Warning: Disable transcoding mode for cropped image\n");
		g_transcode_data.transcode = 0;
	}

	if (g_transcode_data.enc_settings.svc.num_layers > 0)
	{
		if (g_transcode_data.enc_settings.svc.multistream_mode == MULTISTREAM_MODE_MVC) 
		{
			verbose_print(0, "Error: don't support MVC\n");
			return -1;
		}
		if (g_transcode_data.enc_settings.svc.multistream_mode == MULTISTREAM_MODE_SVC && g_transcode_data.transcode)
		{
			verbose_print(0, "Warning: Disable transcoding mode for SVC output\n");
			g_transcode_data.transcode = 0;
		}
	}

	if (0 == sample_transcode_init(&g_transcode_data))
		sample_transcode_run(&g_transcode_data);

	sample_transcode_close(&g_transcode_data);

#ifdef _CRT_DEBUG_
	_CrtMemCheckpoint(&_ms2);
	_CrtMemDifference(&_ms3, &_ms1, &_ms2);
	_CrtMemDumpStatistics(&_ms3);
	_CrtDumpMemoryLeaks();
#endif
	return 0;
}
