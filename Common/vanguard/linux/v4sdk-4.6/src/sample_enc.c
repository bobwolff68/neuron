
/**
 * @file sample_enc.c
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
#include "vp.h"

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

/*
please do not forget to specify properly following parameters in encoder config file:
input.width
input.height
input.colorspace
input.sample_size
input.significant_bits
*/

//////////////////////////////////////////////////////////////////////////

#define MAX_FILE_NAME 1024

#define MAX_NUM_VIEWS 4  //maximum number of views in MVC encoding
char *input_files_mvc[MAX_NUM_VIEWS-1] = {NULL};
FILE *files_mvc[MAX_NUM_VIEWS-1] = {NULL};

char *input_file  = NULL;
char *output_file = NULL;
char *config_file = NULL;
char *cc_file = NULL;	// CC file name
char *pd_file = NULL;	// PD file name

char *ext_data_string = NULL;

char output_file_ms[MAX_FILE_NAME];

int frame_width  = 0;
int frame_height = 0;
int colorspace   = -1;
int frame_rate   = 0;	// frame rate (frames per 10000 seconds)
int frame_skip   = 0;	// skip every N-th frame on input
int max_frames   = 0;	// zero means to handle all the frames
int frame_start = 0;	// first frame number to encode
int bitrate      = 0;
int mt           = -1;
int forever      = 0;	// run forever
int use_async_feed = 0;	// enable asynchronous encoder input
int use_async_receive = 0;	// enable asynchronous NAL receive via callback
int use_frame_modifier = 0;
int change_bitrate_period = 0;	// every N-th frame
int change_bitrate_ratio = 100;	// 100:1, i.e. 120 means 1.2
int	num_dep_views = 0;	// number of dependent views in MVC coding
int bits_per_pel = 0;	// 0 means not assigned, [8..14]
int fp_type = -1; //frame packing type; -1 - means not used; only side-by-side (3) or top-bottom(4) implemented
int use_preload = 0;
int frame_num = 0;
int rgb_switch = 0;

// additional file manipulators
FILE *file_cc  = NULL;
FILE *file_pd  = NULL;

// Multi-stream mode output file pointers
FILE *file_ms[MAX_AVC_STREAMS] = {NULL};

// Number of streams in multi-stream mode
int num_ms_streams = 0;
int is_write_general_output = 1;

// public variables for receive callback access:
int total_samples = 0;
int eos = 0;
uint64_t total_bytes = 0;

//////////////////////////////////////////////////// command line arguments
static cmd_arg_t all_args[] =
{
	{ "i", "yf", "input-file",   1, &input_file,  0, "name of input raw YUV file" },
	{ "-", "y1", "view-1-file",  1, &input_files_mvc[0],  0, "name of input raw YUV for view 1" },
	{ "-", "y2", "view-2-file",  1, &input_files_mvc[1],  0, "name of input raw YUV for view 2" },
	{ "-", "y3", "view-3-file",  1, &input_files_mvc[2],  0, "name of input raw YUV for view 3" },
	{ "o", "hf", "output-file",  1, &output_file, 0, "name of output H.264 file" },
	{ "c", "cf", "config-file",  1, &config_file, 0, "name of encoder config file" },
	{ "-", "cc", "cc-file",      1, &cc_file,     0, "name of input CC text file (one line per frame)" },
	{ "-", "pd", "pd-file",      1, &pd_file,     0, "name of input private data text file (one line per frame)" },
	{ "w", "fw", "frame-width",  0, &frame_width, 0, "input frame width, pixels (divisible by 2)" },
	{ "h", "fh", "frame-height", 0, &frame_height,0, "input frame height, pixels (divisible by 2)" },
	{ "f", "fr", "frame-rate",   0, &frame_rate,  0, "input frame rate (frames per 10,000 sec)" },
	{ "-", "fs", "frame-start",  0, &frame_start, 0, "first frame number to encode" },
	{ "k", "fk", "frame-skip",   0, &frame_skip,  0, "encode every N-th frame (numbering starts from zero)" },
	{ "n", "fn", "frame-count",  0, &max_frames,  0, "max number of input frames to process" },
	{ "u", "fc", "colorspace",   0, &colorspace,  0, "colorspace: 0=IYUV,I420; 1=YV12; 2=YUYV,YUY2; 3=YVYU; 4=UYVY; 5=RGB555; 6=RGB565; 7=RGB24; 8=RGB32, 9=4:0:0, 10=4:2:2" },
	{ "-", "fb", "bits-per-pel", 0, &bits_per_pel,0, "input frame: bits per pixel (8..14)" },
	{ "b", "br", "bitrate",      0, &bitrate,     0, "desired bitrate, kbps" },
	{ "m", "mt", "threads",      0, &mt,         -1, "number of threads (-1=auto, 0=disable)" },
	{ "v", "vb", "verbose",      0, &verbose,     1, "level of verbose messages (0/1/2)" },
	{ "-", "fv", "forever",      0, &forever,    -1, "rewind input stream and run N loops" },
	{ "-", "--", "async-feed",   0, &use_async_feed,    1, "use asynchronous data feed (0/1)" },
	{ "-", "--", "async-receive",0, &use_async_receive, 1, "use asynchronous data receive (0/1)" },
	{ "-", "--", "preload",      0, &use_preload, -1, "preload N frames on input" },
	{ "-", "--", "change-bitrate-period", 0, &change_bitrate_period, 30, "change bitrate period (every N-th frame)" },
	{ "-", "--", "change-bitrate-ratio",  0, &change_bitrate_ratio, 100, "change bitrate ratio, 100:1 (125=1.25)" },
	{ "-", "fp", "fp-type",       0, &fp_type,   3, "use side-by-side(3) or top-bottom(4) frame packing" },
	{ "-", "ed", "ext_data_string",  1, &ext_data_string, 0, "extra data string (use \" \" for string with spaces)" },
};
static const int all_args_cnt = sizeof(all_args)/sizeof(all_args[0]);


//////////////////////////////////////////////////////////////////////////
typedef struct raw_frame_ex_t
{
	vp_raw_frame_t vp_views[MAX_NUM_VIEWS];
	int num_views;
	int busy;
} raw_frame_ex_t;

int num_raw_frames = 1;
raw_frame_ex_t      raw_frames[NUM_RGB_FRAMES] = {0};
vp_frame_factory_t  vpfactory[MAX_NUM_VIEWS] = {0};

// preload support
int preload_frames = 0;
vp_frame_t *preload_buffer[MAX_NUM_VIEWS] = {NULL};

//////////////////////////////////////////////////////////////////////////
static void usage(void)
{
	verbose_print(0, "USAGE:\n");
	verbose_print(0, "\t > sample_enc [-option value] [param=value]\n");
	verbose_print(0, "WHERE:\n");
}

///////////////////////////////////////////////////////////////////////////////// file support
static int openFiles(void)
{
	int rc, i, num_extra_files;
	rc = open_std_files(input_file, output_file);
	if (rc != 0) return rc;

	num_extra_files = num_dep_views;

	for (i = 0; i < num_extra_files; i++)
	{
		if (input_files_mvc[i] == NULL)
		{
			verbose_print(0, "Error: input dependent view-%d file not specified\n", i+1);
			return -1;
		}
		files_mvc[i] = fopen(input_files_mvc[i], "rb");
		if (files_mvc[i] == NULL)
		{
			verbose_print(0, "Error: can't open input file [%s]\n", input_files_mvc[i]);
			return -1;
		}
	}
	if (cc_file)
	{
		file_cc = fopen(cc_file, "rt");
		if (file_cc == NULL)
		{
			verbose_print(0, "Warning: can't open CC file [%s]\n", cc_file);
		}
	}
	if (pd_file)
	{
		file_pd = fopen(pd_file, "rt");
		if (file_pd == NULL)
		{
			verbose_print(0, "Warning: can't open PD file [%s]\n", pd_file);
		}
	}
	return 0;
}

static void seekFile(int num_frames, int image_size)
{
	int step = (1<<30)/image_size;
	int i, steps = num_frames/step;
	// skip frames using max allowed steps
	for(i=0;i<steps;i++)
	{
		fseek(file_in, image_size*step, SEEK_CUR);
	}
	// skip remaining frames
	fseek(file_in, image_size*(num_frames-steps*step), SEEK_CUR);
}

static int readFile(void *buf, int buf_size)
{
	return read_input_file(buf, buf_size);
}

static int writeFile(void *buf, int buf_size)
{
	return write_output_file(buf, buf_size);
}

static void closeFiles(void)
{
	int i;
	close_std_files();
	if (file_cc  != NULL) fclose(file_cc);
	if (file_pd  != NULL) fclose(file_pd);
	for (i = 0; i < num_dep_views; i++)
	{
		fclose(files_mvc[i]);
	}
}

static char *readCC(char *buf, int buf_size)
{
	if (file_cc == NULL) return NULL;
	return fgets(buf, buf_size, file_cc);
}

static char *readPD(char *buf, int buf_size)
{
	if (file_pd == NULL) return NULL;
	return fgets(buf, buf_size, file_pd);
}

// Multi-stream mode support

static int multistream_init(v4e_settings_t *s)
{
	int i;
	char ms_common[MAX_FILE_NAME];

	if (s->svc.num_layers <= 0) return 0;

	// Select general output file using or not
	switch (s->svc.multistream_mode)
	{
	case MULTISTREAM_MODE_AVC:
		is_write_general_output = 0;
		break;
	case MULTISTREAM_MODE_MVC:
		is_write_general_output = 1;
		break;
	default:
		is_write_general_output = 1;
	}

	num_ms_streams = ( (s->svc.multistream_mode == MULTISTREAM_MODE_AVC) || (s->svc.multistream_mode == MULTISTREAM_MODE_MVC)) ? 
		s->svc.num_layers + 1 : 0;
	
	if(num_ms_streams == 1)
	{
		num_ms_streams = 0;
		verbose_print(0, "*** Warning: multi-stream mode is disabled: use 'svc.num_layers' > 0\n");
	}
	
	if(num_ms_streams < 2)
	{
		return 0;
	}

	if(output_file_ms[0] == 0)
	{
		return 0;
	}

	// Prepare file name's common part
	strcpy(ms_common, output_file_ms);
	// Remove file extension (if exists)
	for(i = (int)strlen(ms_common)-1; i > 0; i--)
	{
		if(ms_common[i] == '.')
		{
			ms_common[i] = '\0';
			break;
		}
		if(ms_common[i] == '\\')
			break;
	}
	// Generate stream names and open output files
	for(i = 0; i < num_ms_streams; i++)
	{
		int width;
		int height;
		int bitrate;
		int qp;
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
		{
			sprintf(suffix, "%d.264", i);
		}
		else
		{
			if(s->rc.type != RATE_CONTROL_QP)
			{
				sprintf(suffix, "%d_%dx%d_%dkbps.264", i+1, width, height, bitrate);
			}
			else
			{
				sprintf(suffix, "%d_%dx%d_qp%d.264", i+1, width, height, qp);
			}
		}
		strcpy(ms_name, ms_common);
		strncat(ms_name, suffix, sizeof(suffix));
		file_ms[i] = fopen(ms_name, "wb");
		if(file_ms[i] == NULL)
		{
			verbose_print(0, "Warning: can't open multi-stream file [%s]\n", ms_name);
		}
	}
	
	return 0;
}

static void multistream_write(media_sample_t *ms)
{
	static byte start_code[4] = {0,0,0,1};
	uint8_t n = ((frame_info_t *)ms->extra_data)->num_stream;
	FILE *file_out;

	if(num_ms_streams < 2)
	{
		return;
	}

	if(n >= (uint8_t)num_ms_streams)
	{
		return;
	}

	file_out = file_ms[n];
	
	if(file_out != NULL)
	{
		// Annex B format: 4 bytes start code and NAL unit RBSP contents
		fwrite(start_code, 1, 4, file_out);
		fwrite(ms->data, 1, ms->used_size, file_out);
	}
}

static void multistream_close(void)
{
	int i;

	for(i = 0; i < num_ms_streams; i++)
	{
		if(file_ms[i] != NULL)
		{
			fclose(file_ms[i]);
		}
	}
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
static int release_raw_frame(void *ctx, void *frame)
{
	raw_frame_ex_t *fr_ex = (raw_frame_ex_t *)frame;
	assert(fr_ex->busy == 1);
	fr_ex->busy = 0;
	return 0;
}

static long frame_read_data(void *ctx, void *data, long size)
{
	long bytes = 0;
	FILE *f = (FILE*)ctx;
	if (f)
	{
		bytes = (long)fread(data, 1, size, f);
	}
	return bytes;
}

static long frame_write_data(void *ctx, void *data, long size)
{
	long bytes = 0;
	FILE *f = (FILE*)ctx;
	if (f)
	{
		bytes = (long)fwrite(data, 1, size, f);
	}
	return bytes;
}

static void init_raw_frames(v4e_settings_t *settings)
{
	int i,j;
	int num_views = num_dep_views + 1;
	colorspace_e colorspace = settings->input.colorspace;
	int width  = settings->input.width;
	int height = settings->input.height;
	int bytes_per_pel = settings->input.sample_size;

	if (use_async_feed)
	{
		num_raw_frames = NUM_RGB_FRAMES;
	}

	// create frame factory per every view
	for (j = 0; j < num_views; j++)
	{
		FILE *f = (j == 0)? file_in : files_mvc[j-1];
		vp_init_frame_factory_ex(&vpfactory[j],
			width, height, bytes_per_pel, colorspace, f, 
			frame_read_data, frame_write_data);
	}

	// allocate raw frames
	for (i = 0; i < num_raw_frames; i++)
	{
		for (j = 0; j < num_views; j++)
		{
			vp_raw_frame_t *vp_rawframe = &raw_frames[i].vp_views[j];
			memset(vp_rawframe, 0, sizeof(vp_raw_frame_t));
			vpfactory[j].alloc_frame(&vpfactory[j], &vp_rawframe->vp_frame);
		}
		raw_frames[i].busy = 0;
		raw_frames[i].num_views = num_views;
	}
}

static void free_raw_frames(void)
{
	int num_views = num_dep_views + 1;
	int i,j;
	for (i = 0; i < num_raw_frames; i++)
	{
		for (j = 0; j < raw_frames[i].num_views; j++)
		{
			vp_frame_t *vpframe  = &raw_frames[i].vp_views[j].vp_frame;
			vpfactory[j].free_frame(&vpfactory[j], vpframe);
		}
	}
	for (j = 0; j < num_views; j++)
	{
		vpfactory[j].close(&vpfactory[j]);
	}
}

static raw_frame_ex_t *get_raw_frame(int frame_num)
{
	int i;
	int index = (use_async_feed)? (frame_num % NUM_RGB_FRAMES) : 0;
	raw_frame_ex_t *raw_frame = &raw_frames[index]; // use frames array as ring buffer
	raw_frame->busy = 1;
	for (i = 0; i < raw_frame->num_views; i++)
	{
		raw_frame->vp_views[i].sei_list = NULL;
	}
	return raw_frame;
}

static int add_CC_PD_vp(vp_raw_frame_t *raw_frame);

static void feed_raw_frame(void *handle, raw_frame_ex_t *raw_frame, int frame_num)
{
	int i;
	frame_modifier_t frame_modifier;

	// add CC & PD custom SEI messages; only for first view now
	add_CC_PD_vp(&raw_frame->vp_views[0]); 

	// customize a frame by using frame modifier
	if (use_frame_modifier)
	{
		frame_modifier.idr_flag = (int8_t)((frame_num % use_frame_modifier) == 0);
		frame_modifier.qp = -1;
		frame_modifier.slice_type = -1;
		for (i=0; i<MAX_SVC_LAYERS; i++)
		{
			frame_modifier.svc_layers_qp[i] = -1;
		}
		raw_frame->vp_views[0].modifier = &frame_modifier;
	}

	v4e_set_vp_frame(handle, raw_frame->vp_views, 1);
}

static void copy_frame(vp_frame_t *dstframe, vp_frame_t *srcframe)
{
	vp_batch_t batch;
	int is_interlace = 0;	// ???
	int rc;
	rc = vp_open(&batch, srcframe, is_interlace);
	srcframe->valid_rows_count = srcframe->height;
	rc = vp_process(batch, srcframe, dstframe);
	vp_close(batch);
}

static int readNextRawFrame(int allow_preload, int framenum, raw_frame_ex_t *rawframe)
{
	int i;
	int rc = 0;
	vp_frame_t *srcframe;
	vp_frame_t *dstframe;

	if (allow_preload && (preload_frames > 0))
	{
		int preload_framenum = framenum % preload_frames;
		for (i = 0; i < rawframe->num_views; i++)
		{
			srcframe = &preload_buffer[i][preload_framenum];
			dstframe = &(rawframe->vp_views[i].vp_frame);
			copy_frame(dstframe, srcframe);
		}
	}
	else
	{
		for (i = 0; i < rawframe->num_views; i++)
		{
			dstframe = &(rawframe->vp_views[i].vp_frame);
			if(rgb_switch)
			{
				dstframe->height = -abs(dstframe->height);
			}
			rc = (int)vpfactory[i].read_frame(&vpfactory[i], dstframe);
		}
	}
	return rc;
}

static void rewindInput(void)
{
	int i;
	rewind_input_file();
	for (i = 0; i < num_dep_views; i++)
	{
		fseek(files_mvc[i], 0, SEEK_SET);
	}
}

static int preloadView(int viewnum)
{
	int numframes;
	int i, bufsize, rc;
	vp_frame_t *vpframe;

	numframes = use_preload;
	if (numframes == -1)
	{
		numframes = max_frames;
	}
	if (numframes <= 0)
	{
		return -1;
	}
	// alloc buffer
	bufsize = numframes * sizeof(vp_frame_t);
	preload_buffer[viewnum] = (vp_frame_t *)v4_malloc(bufsize);
	if (!preload_buffer[viewnum])
	{
		verbose_print(0, "*** preload[%d]: insufficient memory (%d bytes)\n", viewnum, bufsize);
		return -1;
	}

	// alloc & read frames
	verbose_print(1, "*** preload[%d]: %d frames ... ", viewnum, numframes);
	preload_frames = 0;
	for (i=0; i<numframes; i++)
	{
		vpframe = &preload_buffer[viewnum][i];
		rc = vpfactory[viewnum].alloc_frame(&vpfactory[viewnum], vpframe);
		if (rc != 0)
		{
			verbose_print(0, "*** preload[%d]: insufficient memory for frame #%d\n", viewnum, i);
			return -1;
		}
		rc = (int)vpfactory[viewnum].read_frame(&vpfactory[viewnum], vpframe);
		if (rc != 0) break;
		preload_frames += 1;
	}
	verbose_print(1, "%d frames read\n", preload_frames);
	return 0;
}

static void preloadInit(void)
{
	int viewnum;
	int num_views = num_dep_views + 1;
	for (viewnum = 0; viewnum < num_views; viewnum++)
	{
		preloadView(viewnum);
	}
}

static void preloadClose(void)
{
	int rc, i, viewnum;
	vp_frame_t *vpframe;
	int num_views = num_dep_views + 1;

	for (viewnum = 0; viewnum < num_views; viewnum++)
	{
		if (preload_buffer[viewnum] != NULL)
		{
			for (i=0; i<preload_frames; i++)
			{
				vpframe = &preload_buffer[viewnum][i];
				rc = (int)vpfactory[viewnum].free_frame(&vpfactory[viewnum], vpframe);
			}
			v4_free(preload_buffer[viewnum]);
		}
	}
}


////////////////////////////////////////////////////////////////////////// CC & PD support
static int add_CC_PD_to_sei_list(media_sample_t **sei_list)
{
	char *buf;
	char *sei_data;
	int rc, sei_size, sei_flags, i, len;

	sei_flags = SEI_FLAG_SEPARATE_NAL;
	buf = (char*)malloc(MAX_PD_LEN);
	if (!buf) return -1;

	// read CC (closed caption)
	sei_data = readCC(buf, MAX_PD_LEN);
	if (sei_data)
	{
		itu_t_t35_t itu_t_t35;
		atsc1_data_t *atsc1;
		cc_data_t *cc;
		memset(&itu_t_t35, 0, sizeof(itu_t_t35_t));
		itu_t_t35.user_identifier = ITU_T_T35_UID_ATSC1;
		atsc1 = &itu_t_t35.user_structure.atsc1_data;
		atsc1->user_data_type_code = ATSC1_TYPE_CC;
		cc = &atsc1->user_data_type_structure.cc_data;
		len = (int)strlen(sei_data);
		cc->cc_count = len / 2;
		if (cc->cc_count > MAX_CC_ITEMS) cc->cc_count = MAX_CC_ITEMS;
		for (i=0; i<cc->cc_count; i++)
		{
			cc->cc_items[i].cc_valid  = 1;
			cc->cc_items[i].cc_type   = 0;
			cc->cc_items[i].cc_data_1 = (byte)sei_data[i*2+0];
			cc->cc_items[i].cc_data_2 = (byte)sei_data[i*2+1];
		}
		sei_size = sizeof(itu_t_t35_t);
		rc = v4e_attach_sei_to_list(sei_list, SEI_USER_DATA_REGISTERED_ITU_T_T35, &itu_t_t35, sei_size, sei_flags, 0);
		if (rc != VSSH_OK)
			verbose_print(0, "Error: v4e_attach_sei() failed (%d)\n", rc);

		// bar_data example
		{
			bar_data_t *bar = &atsc1->user_data_type_structure.bar_data;
			atsc1->user_data_type_code = ATSC1_TYPE_BAR;
			bar->top_bar_flag = 1;
			bar->line_number_end_of_top_bar = 13;
			rc = v4e_attach_sei_to_list(sei_list, SEI_USER_DATA_REGISTERED_ITU_T_T35, &itu_t_t35, sei_size, sei_flags, 0);
			if (rc != VSSH_OK)
				verbose_print(0, "Error: v4e_attach_sei() failed (%d)\n", rc);
		}
		// AFD example
		{
			afd_data_t *afd = &itu_t_t35.user_structure.afd_data;
			itu_t_t35.user_identifier = ITU_T_T35_UID_AFD;
			afd->active_format_flag = 1;
			afd->active_format = 13;
			rc = v4e_attach_sei_to_list(sei_list, SEI_USER_DATA_REGISTERED_ITU_T_T35, &itu_t_t35, sei_size, sei_flags, 0);
			if (rc != VSSH_OK)
				verbose_print(0, "Error: v4e_attach_sei() failed (%d)\n", rc);
		}
	}
	// read PD (private data)
	sei_data = readPD(buf, MAX_PD_LEN);
	if (sei_data)
	{
		sei_size = (int)strlen(sei_data);
		rc = v4e_attach_sei_to_list(sei_list, SEI_USER_DATA_UNREGISTERED, sei_data, sei_size, sei_flags, 0);
		if (rc != VSSH_OK)
			verbose_print(0, "Error: v4e_attach_sei() failed (%d)\n", rc);
	}
	free(buf);
	return 0;
}


static int add_CC_PD_vp(vp_raw_frame_t *raw_frame)
{
	return add_CC_PD_to_sei_list(&raw_frame->sei_list);
}

//////////////////////////////////////////////////////////////////////////
static char *get_nalu_name(int nalu_type)
{
	switch (nalu_type)
	{
	case NALU_TYPE_SLICE:   return "SLICE";
	case NALU_TYPE_IDR:     return "IDR";
	case NALU_TYPE_SEI:     return "SEI";
	case NALU_TYPE_SPS:     return "SPS";
	case NALU_TYPE_PPS:     return "PPS";
	case NALU_TYPE_PD:      return "PD";
	case NALU_TYPE_FILL:    return "FILL";
	case NALU_TYPE_SUBSPS:  return "SUBSPS";
	case NALU_TYPE_SLICE_EXT:  return "SLICEEXT";
	case NALU_TYPE_PREFIX_EXT: return "PREFIX";
	}
	return "UNKNOWN";
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

static void report_media_sample(int num, media_sample_t *ms)
{
	byte first_byte;
	int nalu_type, nalu_ridc;
	frame_info_t *info;
	first_byte = ((byte*)(ms->data))[0];
	nalu_type = NALU_TYPE(first_byte);
	nalu_ridc = NALU_RIDC(first_byte);
	info = (frame_info_t *)ms->extra_data;
	verbose_print(1, "%6d(%6d): [%d|%2d] %8s",
		num, ms->used_size,
		nalu_ridc, nalu_type, get_nalu_name(nalu_type));
	switch (nalu_type)
	{
	case NALU_TYPE_SLICE_EXT:
		if(NAL_EXT_SVS_EXTENSION_FLAG(ms->data))
		{
			verbose_print(1," [%d][%d|%d|%d]",
				NAL_EXT_PRIORITY_ID(ms->data),
				NAL_EXT_DEPENDENCY_ID(ms->data),
				NAL_EXT_TEMPORAL_ID(ms->data),
				NAL_EXT_QUALITY_ID(ms->data)
				);
			if (NAL_EXT_IDR(ms->data))
				verbose_print(1," IDR");
		}
		else
		{
			verbose_print(1," MVC[%d][%d|%d]",
				NAL_MVC_EXT_PRIORITY_ID(ms->data),
				NAL_MVC_EXT_TEMPORAL_ID(ms->data),
				NAL_MVC_EXT_VIEW_ID(ms->data)
				);
			if (!NAL_MVC_EXT_NON_IDR(ms->data))
				verbose_print(1," IDR");
			if(NAL_MVC_EXT_ANCHOR_PIC(ms->data))
				verbose_print(1," ANCHOR");
		}
	case NALU_TYPE_IDR:
	case NALU_TYPE_SLICE:
		verbose_print(1, " %s[%d] qp=%d ts=%d",
			get_slice_name(info->slice_type),
			info->frame_num,
			info->qp_used,
			(int)(ms->timestamp/10000)
			);
		break;
	}
	verbose_print(1, "\n");
}

//////////////////////////////////////////////////////////////////////////
// this function calculates next frame time stamp using current frame rate value
// we use 10^7 timer frequency as per DirectX nature
#define ONE_SEC (10000000)
static int64_t calc_timestamp(v4e_settings_t *settings, int num)
{
	int64_t ts;
	int num_units = settings->gop.num_units;
	int time_scale = settings->gop.time_scale/2;
	int64_t sec, remain;
	ts = num * num_units;
	ts = ts * ONE_SEC / time_scale;
	sec = ts / ONE_SEC;
	remain = ts % ONE_SEC;
	ts = sec * ONE_SEC + remain;
	return ts;
}

////////////////////////////////////////////////////////////////////////// NAL output

static void on_nal_unit(media_sample_t *ms)
{
	static byte start_code[4] = {0,0,0,1};
	// Annex B format: 4 bytes start code and NAL unit RBSP contents
	if(is_write_general_output != 0)
	{
		writeFile(start_code, 4);
		writeFile(ms->data, ms->used_size);
	}
	total_bytes += (ms->used_size + 4);

	// Write multi-stream files
	multistream_write(ms);

	if (verbose)
		report_media_sample(total_samples, ms);
	if (verbose >= 2)
	{
		if (ms->is_last_in_pict > NOT_LAST)
		{
			verbose_print(2, "[%08d]", (int)(ms->timestamp/10000));
			switch (ms->is_last_in_pict)
			{
			case LAST_IN_LAYER:
				verbose_print(2, "------------> layer\n");
				break;
			case LAST_IN_PICT:
				verbose_print(2, "------------> pict\n");
				break;
			case LAST_IN_FRAME:
				verbose_print(2, "-------------------------------------> frame %d\n", frame_num++);
				break;
			}
		}
	}
	// application responsibility is to release media sample after use
	v4_free_media_sample(ms);
	total_samples++;
}

static int receive_nal_callback(void *context, media_sample_t *ms)
{
	static int s_flag = 0;
	if (s_flag != 0)
	{
		verbose_print(0, "*** receive_nal_callback(): entry violation [%d]!!! \n", s_flag);
		exit(0);
	}
	s_flag += 1;
	////////////////////
	if (ms)
		on_nal_unit(ms);
	else
		eos = 1;
	////////////////////
	s_flag -= 1;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
static int prepareSettings(v4e_settings_t *settings, int argc, char *argv[])
{
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

	// before reading config file setup some default values according to command line arguments
	if (colorspace >= 0)
	{
		switch (colorspace)
		{
		case  COLORSPACE_E_YUV400: settings->preproc.chroma_format_idc = YUV_400_IDC; break;
		case  COLORSPACE_E_YUV422: settings->preproc.chroma_format_idc = YUV_422_IDC; break;
		default: settings->preproc.chroma_format_idc = YUV_420_IDC; break;
		}
	}
	if ((bits_per_pel > 8) && (bits_per_pel <= 14))
	{
		settings->input.sample_size = 2;
		settings->input.significant_bits = bits_per_pel;
		settings->bit_depth_luma = bits_per_pel;
		settings->bit_depth_chroma = bits_per_pel;
	}

	// read encoder config file
	if (config_file)
	{
		rc = v4e_read_config_file(settings, config_file);
		if (rc != VSSH_OK)
		{
			verbose_print(0, "Error: can't open config file [%s]\n", config_file);
			return rc;
		}
	}

	if(settings->input.height < 0)
	{
		rgb_switch = 1;
		settings->input.height = abs(settings->input.height);
	}

	// command line arguments override encoder settings:
	if (mt == 0)
	{
		settings->mt.disable = 1;
	}
	else if (mt > 0)
	{
		settings->mt.disable = 0;
		settings->mt.num_threads = mt;
	}

	if (frame_width > 0)
	{
		settings->input.width  = frame_width;
	}
	if (frame_height > 0)
	{
		settings->input.height = frame_height;
	}

	if (colorspace >= 0)
	{
		settings->input.colorspace = (colorspace_e)colorspace;
	}
	if (frame_rate > 0)
	{
		settings->gop.num_units = 10000;
		settings->gop.time_scale = 2*frame_rate;
	}
	if (bitrate > 0)
	{
		settings->rc.kbps = bitrate;
	}
	if (fp_type >= 0)
	{
		if(fp_type == 3 || fp_type == 4) // make sure that frame_packing_type is supported
		{
			settings->sei.frame_packing_flag = 1;
			settings->sei.frame_packing_type = fp_type;
			settings->svc.num_layers = 0; //make sure that it will AVC encoding only
			settings->svc.multistream_mode = 0;
			if (fp_type == 3 || fp_type == 4) 
				settings->enc_flags |= ENC_FRAME_PACKING; 
		}
	}
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
static void enc_change_bitrate(void *handle, v4e_settings_t *settings, int ratio)
{
	int rc, oldkbps, newkbps, i, svcratio;
	if ((ratio > 0) && (settings->rc.kbps > 0))
	{
		oldkbps = settings->rc.kbps;
		newkbps = settings->rc.kbps * ratio / 100;
		settings->rc.kbps = newkbps;
		verbose_print(2, "*** change-bitrate: %d", newkbps);
		rc = v4e_change_bitrate_and_framerate(handle, newkbps, 0, 0);
		if (settings->rc.kbps > 0)
		{
			for (i=0; i<settings->svc.num_layers; i++)
			{
				svcratio = settings->svc.layer[i].kbps * 100 / oldkbps;
				newkbps = oldkbps * ratio / 100;
				newkbps = newkbps * svcratio / 100;
				settings->svc.layer[i].kbps = newkbps;
				verbose_print(2, ", %d", newkbps);
				rc = v4e_change_svc_bitrate_and_framerate(handle, i, newkbps, 0, 0);
			}
		}
		verbose_print(2, "\n");
	}
}


////////////////////////////////////////////////////////////////////////// dual pass
static char *file_dualpass = "v4enc.dualpass.log";
static int dualpass_before(void *henc, v4e_settings_t *settings)
{
	int rc, fsize;
	media_sample_t *pms;
	// load first pass results before second pass encoding
	if ((settings->rc.type&0xf) == RATE_CONTROL_DUAL_PASS_1)
	{
		FILE *f = fopen(file_dualpass, "rb");
		if (!f)
		{
			verbose_print(0, "*** Error: dual pass log file not found [%s]\n", file_dualpass);
			return VSSH_ERR_GENERAL;
		}
		fseek (f, 0, SEEK_END);
		fsize = ftell(f);
		fseek(f, 0, SEEK_SET);
		pms = v4_alloc_media_sample(fsize, NULL);
		if (!pms)
		{
			verbose_print(0, "Error: can't allocate dual pass buffer [%d] bytes\n", fsize);
			fclose(f);
			return VSSH_ERR_MEMORY;
		}
		fread(pms->data, fsize, 1, f);
		pms->used_size = fsize;
		rc = v4e_set_dual_pass_log(henc, pms);
		fclose(f);
	}
	return VSSH_OK;
}
static int dualpass_after(void *henc, v4e_settings_t *settings)
{
	int rc;
	// save results after first pass encoding
	if ((settings->rc.type&0xf) == RATE_CONTROL_DUAL_PASS_0)
	{
		FILE *f = fopen(file_dualpass, "wb");
		media_sample_t *pms = NULL;
		if (!f)
		{
			verbose_print(0, "*** Error: can't open dual pass log file [%s]\n", file_dualpass);
			return VSSH_ERR_GENERAL;
		}
		rc = v4e_get_dual_pass_log(henc, &pms);
		if (!pms)
		{
			verbose_print(0, "Error: can't retrieve dual pass bytes\n");
			fclose(f);
			return VSSH_ERR_MEMORY;
		}
		fwrite(pms->data, pms->used_size, 1, f);
		v4_free_media_sample(pms);
		fclose(f);
	}
	return VSSH_OK;
}


#ifdef SVC_EXTENSION
//#define VSS_MB_TYPES_DUMP
#endif

// for debugging
#ifdef VSS_MB_TYPES_DUMP
extern int g_dbg_num_layers;
extern int g_num_b_frames;
extern void mb_types_dbg_drawing_close();

#endif

//////////////////////////////////////////////////////////////////////////
/*
This program demonstrates synchronous input approach with several (9) buffers.
Encoder API function "v4e_set_frame_ex()" freely accepts 8 input frames and then starts blocking
the caller waiting while the next frame becomes released. Using exactly (9) buffers guarantees
that after every call to "v4e_set_frame_ex()" next frame in the array will be safe for use (ring buffer).
*/
int main(int argc, char *argv[])
{
	v4e_settings_t settings;
	v4e_settings_t actual_settings;
	void *handle;
	media_sample_t *ms;
	v4_frame_release_t  frame_release_local;
	v4_frame_release_t *frame_release = NULL;
	v4_receive_nal_t    nal_receive_local;
	v4_receive_nal_t   *nal_receive = NULL;
	raw_frame_ex_t *raw_frame;
	int num_frame, total_frames;
	int rc, i, again, eof;
	timer_clock_t start_time;
	timer_delta_t delta;
	int total_ms;
	arg_ctx_t arg_ctx;
	int skip_counter;

#ifdef _CRT_DEBUG_
	_CrtMemState _ms1, _ms2, _ms3;
	_CrtMemCheckpoint(&_ms1);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF);
#endif

	// print codec library version
	verbose_print(0, "sample encoder application\n");
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

	settings.ext_data_string = NULL;

	read_args(&arg_ctx, argc, argv);
	if (verbose) dump_args(&arg_ctx);

	// prepare encoder settings
	rc = prepareSettings(&settings, argc, argv);
	if (rc != 0) return -1;

	if(change_bitrate_period > 0 && settings.sei.pic_timing_flag > 0)
	{
		settings.sei.pic_timing_flag = 0;
		verbose_print(0, "\nWarning: picture timing is disabled due to change-bitrate-period option is in use\n\n");
	}

	if(settings.svc.num_layers > 0 && (settings.svc.multistream_mode == MULTISTREAM_MODE_AVC || settings.svc.multistream_mode == MULTISTREAM_MODE_MVC) )
	{
		if(output_file != NULL)
		{
			strcpy(output_file_ms, output_file);
			if(settings.svc.multistream_mode == MULTISTREAM_MODE_AVC)
			{
				output_file = NULL;
			}
		}
		else
		{
			// Null outputs support
			output_file_ms[0] = 0;
		}
	}

	num_dep_views = settings.svc.multistream_mode != MULTISTREAM_MODE_MVC ? 0 : settings.svc.num_layers;

	if(settings.sei.frame_packing_flag)
	{
		if(settings.sei.frame_packing_type == 3 || settings.sei.frame_packing_type == 4) 
		{
			num_dep_views = 1;
			settings.enc_flags |= ENC_FRAME_PACKING; 
		}
	}


	// open input & output files
	rc = openFiles();
	if (rc != 0) return -1;

	// application MUST provide release frame callback in order to work with
	// v4e_set_frame_ex() function
	if (use_async_feed)
	{
		frame_release_local.context = NULL;
		frame_release_local.callback = release_raw_frame;
		frame_release = &frame_release_local;
	}

	// optional asynchronous NAL receive function
	if (use_async_receive)
	{
		nal_receive_local.context = NULL;
		nal_receive_local.callback = receive_nal_callback;
		nal_receive = &nal_receive_local;
	}

#ifdef _CRT_DEBUG_
	_CrtMemCheckpoint(&_ms1);
#endif

	settings.ext_data_string = ext_data_string;
	rc = v4e_open_ex(&handle, &settings, frame_release, nal_receive);
	if (rc > VSSH_OK)
	{
		verbose_print(0, "Warning: v4e_open_ex(): %d [%s]\n", rc, v4_error_text(rc));
	}
	if (rc < VSSH_OK)
	{
		verbose_print(0, "Error: v4e_open_ex(): %d [%s]\n", rc, v4_error_text(rc));
		return -3;
	}
	v4e_get_current_settings(handle, &actual_settings);
	dump_encoder_settings(&actual_settings, verbose);
	if (dualpass_before(handle, &actual_settings) < VSSH_OK)
	{
		return -1;
	}

	// prepare frame buffers with actual encoder settings 
	init_raw_frames(&settings);

	////////////////////// first time change bitrate
	if (change_bitrate_period > 0)
	{
		enc_change_bitrate(handle, &settings, 100);
	}

	// Multi-stream mode support
	rc = multistream_init(&actual_settings);
	if (rc != 0) return -1;

	////////////////////// file encoding cycle
	num_frame = 0;
	total_frames = 0;
	total_samples = 0;
	total_bytes = 0;
	skip_counter = frame_skip;

	eof = 0;
	eos = 0;

#ifdef VSS_MB_TYPES_DUMP
	{
		g_dbg_num_layers = 0;
		g_num_b_frames = settings.gop.bframes;
#ifdef SVC_EXTENSION
		g_dbg_num_layers = settings.svc.num_layers + 1;
#endif
		assert (g_dbg_num_layers <= 5);
	}
#endif

	// skip to frame_start position...
	raw_frame = get_raw_frame(0);
	for (i = 0; i<frame_start; i++)
	{
		rc = readNextRawFrame(0, 0, raw_frame);
	}
	// preload raw frames
	preloadInit();

	timer_init();
	timer_start(&start_time);
	// main encoding cycle
	for (i = 0; !eos; i++)
	{
		if (!eof)
		{
			// select next input frame from array
			raw_frame = get_raw_frame(i);
			// read next input raw frame and assign eof (end of file) flag
			for (again=1; again>0; again--)
			{
				rc = readNextRawFrame(1, total_frames, raw_frame);
				if ((rc != 0) || ((max_frames > 0) && (num_frame >= max_frames)))
				{
					eof = 1;
					if (forever  > 0) forever -= 1;
					if (forever != 0)
					{
						rewindInput();
						verbose_print(2, "*** rewind (%d)\n", forever);
						eof = 0;
						num_frame = 0;
						again += 1;
					}
				}
			}
			if (eof)
			{
				// signal encoder end of sequence
				v4e_set_flush(handle);
			}
			else
			{
				if ((frame_skip > 0) && (skip_counter > 0))
				{
					skip_counter -= 1;
					verbose_print(2, "%4d: *** skip ***\n", total_frames);
				}
				else
				{
					skip_counter = frame_skip;
					// assign media time and timestamp
					raw_frame->vp_views[0].mediatime = -1 - i;
					raw_frame->vp_views[0].timestamp = calc_timestamp(&settings, total_frames);

					// send raw frame to encoder
					feed_raw_frame(handle, raw_frame, num_frame);
					num_frame += 1;
					verbose_print(2, "%4d: set frame ts=%6d \n", total_frames, (int)(raw_frame->vp_views[0].timestamp/10000));
					//verbose_print(2, "%4d: set vp_frame ts=%6d \n", total_frames, (int)(raw_frame->vp_views[0].timestamp/10000));
				}
				total_frames++;
				if ((change_bitrate_period > 0) && (num_frame % change_bitrate_period == 0))
				{
					enc_change_bitrate(handle, &settings, change_bitrate_ratio);
				}
			}
		}
		// note: async receive will enable "eos" flag externally
		if (!use_async_receive)
		{
			// extract synchronously all the NAL units on every input frame
			// note: NAL units come in RBSP format but without start codes
			for(;;)
			{
				// receive next NAL unit (if ready)
				rc = v4e_get_nal(handle, &ms);

				// VSSH_WARN_EOS is a special code signaling that it is the last NAL unit
				// produced after v4e_set_flush()
				if (rc == VSSH_WARN_EOS) eos = 1;
				if (rc != VSSH_OK) break;

				on_nal_unit(ms);
			}
		}
	} // while(!eos)
	delta = timer_delta(&start_time);
	total_ms = timer_delta_ms(delta);

	verbose_print(0, "%20s: %d\n", "total frames", total_frames);
	verbose_print(0, "%20s: %d\n", "total time, ms", total_ms);
	if (total_ms > 0)
	verbose_print(0, "%20s: %d\n", "total speed, fps", (total_frames*1000 + total_ms/2)/total_ms);
	{
		uint32_t stream_length_ms, time_scale_ms;
		stream_length_ms = 0;
		if (actual_settings.gop.time_scale > 0 && settings.avc_intra_class == 0)
		{
			if (settings.avc_intra_class == 0)
				time_scale_ms = (actual_settings.gop.time_scale + 1000) / 2000;
			else
			{
				if (actual_settings.gop.time_scale == 50 || actual_settings.gop.time_scale == 100)
					time_scale_ms = actual_settings.gop.time_scale / 2;
				else
					time_scale_ms = (actual_settings.gop.time_scale + 1000) / 2000;
			}
			stream_length_ms = total_frames * actual_settings.gop.num_units / time_scale_ms;
		}
		if (stream_length_ms > 0)
		{
			uint32_t bitrate = (uint32_t)(total_bytes * 8 / stream_length_ms);
			verbose_print(0, "%20s: %u \n", "stream length, ms", (uint32_t)stream_length_ms);
			verbose_print(0, "%20s: %u \n", "total bitrate, kbps", bitrate);
		}
	}

	// release encoder instance
	dualpass_after(handle, &actual_settings);
	v4e_close(handle);

	preloadClose();

#ifdef _CRT_DEBUG_
	_CrtMemCheckpoint(&_ms2);
	_CrtMemDifference(&_ms3, &_ms1, &_ms2);
	_CrtMemDumpStatistics(&_ms3);
#endif

	// release input raw frames
	free_raw_frames();

	// close files
	closeFiles();

	// Close multi-stream files
	multistream_close();

#ifdef _CRT_DEBUG_
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
////////////////////////////////////////////////////////////////////////// eof
