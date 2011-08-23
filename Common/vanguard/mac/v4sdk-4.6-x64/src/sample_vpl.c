
/**
 * @file sample_vpl.c
 * Example of Video Processing Library console application
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#define _CRT_SECURE_NO_DEPRECATE //to avoid warning C4996: 'fopen' was declared deprecated

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef WIN32
#ifdef _DEBUG
#include <crtdbg.h>
#endif
#endif

#include "v4file.h"
#include "vp.h"
int error_no = VP_OK;

static void check_error(int ret_code)
{
	if(error_no == VP_OK)
	{
		error_no = ret_code;
	}
}

static void print_usage(const char *message)
{

	if(message)
	{
		verbose_print(0,"!!!%s\n\n", message);
	}

	verbose_print(0,"sample_vpl.exe [-yabc... | -uabc... | -vabc... | -r_w_h | -cab | -d | -b_y_u_v] -p_l_t_w_h -w width -h height -n frames_number -i input.yuv [-o output.yuv] [-e] -l \n");
	verbose_print(0,"    -e: emulate file operations;\n");
	verbose_print(0,"    -s: skip frames counter;\n");
	verbose_print(0,"    -y, -u, -v: describe luma and chroma processing chains;\n");
	verbose_print(0,"    -r: enables image resize;\n");
	verbose_print(0,"    -c: select input and output colorspaces;\n");
	verbose_print(0,"    -l: treat input file as interlaced;\n");
	verbose_print(0,"	 -g: turn RGB upside down;\n");

	{
		const char *link_types[] =
		{
			"blur3x3",
			"blur5x5",
			"sharpen3x3",
			"sharpen5x5",
			"median3x3",
			"median5x3",
			"temporal denoise  -g_s_b, where s=strength [1..10], b=buffer [2..7];"
		};
		int i;
		verbose_print(0,"\n\t-y[xyz...]  -u[xyz...] -v[xyz...] subkey values:\n");
		for(i = 0; i < sizeof(link_types)/sizeof(link_types[0]); i += 1)
		{
			verbose_print(0,"\t\t%c: %s\n", 'a' + i, link_types[i]);
		}
	}

	verbose_print(0,"\t-d[t|b|a]: deinterlace:\n");
	verbose_print(0,"\t\tt = duplicate top field;\n");
	verbose_print(0,"\t\tb = duplicate bottom field;\n");
	verbose_print(0,"\t\ta = adaptive blur;\n");
	verbose_print(0,"\t-r_w_h] resize (-r_234_128 = resize to 234x128)\n");
	verbose_print(0,"\t-p_l_t_w_h: crop source frame (left, top, width, height);\n");
	verbose_print(0,"\t-b_y_u_v: set bit depth for y u and v planes, [8,14], default values 8,8,8;\n");



	{
		char *colorspaces[] =
		{
			"colorspace IYUV",//    a=  0,	///< YUV 4:2:0 planar (our internal)
			"colorspace YV12",//    b=  1,	///< YUV 4:2:0 planar (U&V swapped);
			"colorspace YUY2",//    c=  2,	///< YUV 4:2:2 channel
			"colorspace YVYU",//    d=  3,	///< YUV 4:2:2 channel
			"colorspace UYVY",//    e=  4,	///< YUV 4:2:2 channel
			"colorspace RGB555",//  f=  5,	///< RGB 5:5:5
			"colorspace RGB565",//  g=  6,	///< RGB 5:6:5
			"colorspace RGB24",//   h=  7,	///< RGB 24 bit
			"colorspace RGB32",//   i=  8,	///< RGB 24 bit + alpha channel
			"colorspace YUV400",//  j=  9,	///< YUV 4:0:0 planar
			"colorspace YUV422"//   k= 10	//
		};
		int i;
		verbose_print(0,"\n\t-c[xy]: select colorspace (x = input, y = output), default IYUV->IYUV;\n");
		for(i = 0; i < VP_TOTAL; i += 1)
		{
			verbose_print(0,"\t\t%c: %s\n", 'a' + i, colorspaces[i]);
		}
	}
	verbose_print(0,"\nNOTE:\n");
	verbose_print(0,"\tkeys -y -u -v and -r could be specified several times;\n");
	exit(-1);
}

//------------------------------------------------------------------------------
// read number from string in such format _123_43ased
// it returns 123 and modifies _str pointer to _43ased
static int read_digit(char **_str)
{
	int res = 0;
	unsigned int i = 0;
	size_t len = strlen(*_str);
	if(len > 1)
	{
		char number[20];
		char *str = *_str;
		str += 1; // skip _
		while(str[i] >= '0' && str[i] <= '9' && i < (len-1))
		{
			number[i] = str[i];
			i += 1;
		}
		number[i] = 0;
		*_str += i+1;
		res = atoi(number);
	}
	else
	{
		print_usage("digit read error");
	}
	return res;
}


static void build_chain(vp_batch_t *batch, vp_target_e target, char *nodes)
{
	while(nodes[0] != '\0')
	{
		switch(nodes[0])
		{
		case 'a':
			vp_add_filter(batch, target, VP_BLUR3x3, 0, 0); nodes += 1;
			break;
		case 'b':
			vp_add_filter(batch, target, VP_BLUR5x5, 0, 0);  nodes += 1;
			break;
		case 'c':
			vp_add_filter(batch, target, VP_SHARPEN3x3, 0, 0);  nodes += 1;
			break;
		case 'd':
			vp_add_filter(batch, target, VP_SHARPEN5x5, 0, 0);  nodes += 1;
			break;
		case 'e':
			vp_add_filter(batch, target, VP_MEDIAN3x3, 0, 0);  nodes += 1;
			break;
		case 'f':
			vp_add_filter(batch, target, VP_MEDIAN5x3, 0, 0);  nodes += 1;
			break;
		case 'g':
			nodes += 1;
			{
				int strength    = read_digit(&nodes);
				int plane_count = read_digit(&nodes);

				vp_add_filter(batch, target, VP_TEMPORAL_DENOISE, strength, plane_count);
			}
			break;
		default:
			print_usage("error: wrong filter type for y, u or v color plane");
			break;
		}
	}
}

static void read_color_conversion(char *str,
								  vp_colorspace_e *in_colorspace,
								  vp_colorspace_e *out_colorspace)
{
	vp_colorspace_e colorspaces[] =
	{
		VP_IYUV,//    =  0,	///< YUV 4:2:0 planar (our internal)
		VP_YV12,//    =  1,	///< YUV 4:2:0 planar (U&V swapped);
		VP_YUY2,//    =  2,	///< YUV 4:2:2 channel
		VP_YVYU,//    =  3,	///< YUV 4:2:2 channel
		VP_UYVY,//    =  4,	///< YUV 4:2:2 channel
		VP_RGB555,//  =  5,	///< RGB 5:5:5
		VP_RGB565,//  =  6,	///< RGB 5:6:5
		VP_RGB24,//   =  7,	///< RGB 24 bit
		VP_RGB32,//   =  8,	///< RGB 24 bit + alpha channel
		VP_YUV400,//  =  9,	///< YUV 4:0:0 planar
		VP_YUV422//  = 10	//
	};
	int in_idx  = -1;
	int out_idx = -1;
	if(strlen(str) > 0)
	{
		out_idx = in_idx = (int)(str[0]) - (int)'a';
	}
	if(strlen(str) > 1)
	{
		out_idx = (int)(str[1]) - (int)'a';
	}
	if((in_idx < 0 || in_idx >= VP_TOTAL) || (out_idx < 0 || out_idx >= VP_TOTAL))
	{
		print_usage("error: wrong color space conversion");
	}
	*in_colorspace = colorspaces[str[0] - 'a'];
	*out_colorspace = colorspaces[str[1] - 'a'];
}
//------------------------------------------------------------------------------
//
static void resize_image(vp_batch_t *batch,
						 char *_str)
{
	char *str = _str;
	int width  = read_digit(&str);
	int height = read_digit(&str);

	check_error(vp_add_resize(batch, width, height));
}
//------------------------------------------------------------------------------
//
static void read_conversion_params(int arg_c, char *arg_v[],
								   vp_batch_t *batch)
{
	int i;
	for(i = 1; i < arg_c; i += 1)
	{
		if(arg_v[i][0] == '-')
		{
			switch(arg_v[i][1])
			{
			case 'y':
				build_chain(batch, VP_Y, arg_v[i]+2);
				break;

			case 'u':
				build_chain(batch,  VP_U, arg_v[i]+2);
				break;

			case 'v':
				build_chain(batch,  VP_V,arg_v[i]+2);
				break;

			case 'r':
				resize_image(batch, arg_v[i]+2);
				break;

			case 'd':
				{
					switch(arg_v[i][2])
					{
					case 't':
						vp_add_deinterlace(batch, VP_DEINTERLACE_TOP);
						break;
					case 'b':
						vp_add_deinterlace(batch, VP_DEINTERLACE_BOTTOM);
						break;
					case 'a':
						vp_add_deinterlace(batch, VP_DEINTERLACE_ADAPTIVE);
						break;
					default:
						print_usage("error: wrong deinterlace type");
					}
				}
				break;
			}
		}
	}
}
//------------------------------------------------------------------------------
//
#define MAX_NAME_LEN 256

static void read_input_params(int arg_c,
							  char *arg_v[],
							  int  *in_width,
							  int  *in_height,
							  int  *frame_count,
							  int  *frame_skip,
							  int  *cr_left,
							  int  *cr_top,
							  int  *cr_width,
							  int  *cr_height,
							  char *input_name,
							  char *output_name,
							  int  *y_bit_depth,
							  int  *u_bit_depth,
							  int  *v_bit_depth,
							  vp_colorspace_e *in_colorspace,
							  vp_colorspace_e *out_colorspace,
							  int *interlace,
							  int *rgb_switch,
							  int *read_frames)
{
	int i;

	for(i = 1; i < arg_c; i += 1)
	{
		if(arg_v[i][0] == '-')
		{
			switch(arg_v[i][1])
			{
			case 'w':
				i += 1; if(i == arg_c) break;
				*in_width = atoi(arg_v[i]);
				if(*cr_width  == 0)
				{
					*cr_width = *in_width;
				}
				break;
			case 'h':
				i += 1;	 if(i == arg_c) break;
				*in_height = atoi(arg_v[i]);
				if(*cr_height  == 0)
				{
					*cr_height = *in_height;
				}
				break;
			case 'n':
				i += 1;  if(i == arg_c) break;
				*frame_count = atoi(arg_v[i]);
				break;
			case 's':
				i += 1;  if(i == arg_c) break;
				*frame_skip = atoi(arg_v[i]);
				break;
			case 'i':
				i += 1;  if(i == arg_c) break;
				if(strlen(arg_v[i]) >= MAX_NAME_LEN)
				{
					verbose_print(0,"file name too long %s\n", arg_v[i]);
					exit(-1);
				}
				strcpy(input_name, arg_v[i]);

				break;
			case 'o':
				i += 1;	  if(i == arg_c) break;
				if(strlen(arg_v[i]) >= MAX_NAME_LEN)
				{
					verbose_print(0,"file name too long %s\n", arg_v[i]);
					exit(-1);
				}
				strcpy(output_name, arg_v[i]);

				break;
			case 'c':
				read_color_conversion(arg_v[i] + 2, in_colorspace, out_colorspace);
				break;
			case 'p':
				{
					char *str = arg_v[i] + 2;
					*cr_left   = read_digit(&str);
					*cr_top    = read_digit(&str);
					*cr_width  = read_digit(&str);
					*cr_height = read_digit(&str);
				}
				break;
			case 'b':
				{
					char *str = arg_v[i] + 2;
					*y_bit_depth   = read_digit(&str);
					*u_bit_depth   = read_digit(&str);
					*v_bit_depth   = read_digit(&str);
				}
				break;
			case 'l':
				*interlace = 1;
				break;
			case 'e':
				*read_frames = 0;
				break;
			case 'g':
				*rgb_switch	= 1;
				break;
			}
		}
	}
}


static long read_frame(void *ctx, void *data, long size)
{
	(void)ctx;
	return (long)read_input_file(data, size);
}

static long write_frame(void *ctx, void *data, long size)
{
	(void)ctx;
	return (long)write_output_file(data, size);
}

int main(int arg_c, char *argv[])
{
	int res = 0;
	clock_t tmp_time = 0;
	clock_t time = 0;
	int i;
	int fr_count  = 100000;
	int fr_skip   = 0;
	int width     = 0;
	int height    = 0;
	int cr_left   = 0;
	int cr_top    = 0;
	int cr_width  = 0;
	int cr_height = 0;
	vp_colorspace_e in_cs  = VP_IYUV;
	vp_colorspace_e out_cs = VP_IYUV;
	char in_name[MAX_NAME_LEN]  = {0};
	char out_name[MAX_NAME_LEN] = {0};
	int y_bit_depth = 8;
	int u_bit_depth = 8;
	int v_bit_depth = 8; 
	vp_frame_factory_t src_factory;
	vp_frame_factory_t dst_factory;
	vp_frame_t src_frame;
	vp_frame_t dst_frame;
	int interlace = 0;
	vp_batch_t batch;
	int rgb_switch = 0;
	int read_frames = 1;
#ifdef WIN32
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_LEAK_CHECK_DF);
#endif
#endif

	if (arg_c <= 1)
	{
		print_usage(NULL);
	}
	read_input_params(arg_c, argv, &width, &height, &fr_count, &fr_skip, 
		&cr_left, &cr_top, &cr_width, &cr_height,
		in_name, out_name, 
		&y_bit_depth, &u_bit_depth, &v_bit_depth, 
		&in_cs, &out_cs, &interlace, &rgb_switch, &read_frames);
	if(width == 0 || height == 0)
	{
		print_usage("Error: input picture dimension not specified (width of height is zero)\n");
	}

	if(open_std_files(in_name, out_name))
	{
		return -1;
	}

	vp_init_frame_factory(&src_factory, width, height, in_cs, 0, read_frame, write_frame);
	src_factory.alloc_frame(&src_factory, &src_frame);
	vp_open_crop(&batch, &src_frame, cr_left, cr_top, cr_width, cr_height, interlace);
	vp_bit_depth(batch, y_bit_depth, u_bit_depth, v_bit_depth);
	read_conversion_params(arg_c, argv, batch);
	vp_add_colorspace(batch, out_cs);
	vp_get_dst_info(batch, &dst_frame);
	vp_init_frame_factory(&dst_factory, dst_frame.width, dst_frame.height, dst_frame.colorspace,
		0, read_frame, write_frame);
	src_factory.free_frame(&src_factory, &src_frame);
	{
		char *cfg = 0;
		int cfg_size = vp_get_config(batch, 0);
		cfg = malloc(sizeof(char) * cfg_size);
		vp_get_config(batch, cfg);
		verbose_print(0,"%s\n", cfg);
		free(cfg);
	}

	if(error_no < VP_OK)
	{
		fprintf(stderr, "Error :", error_no);
		switch(error_no)
		{
		case VP_WRONG_PARAMETER:
			fprintf(stderr, "wrong pramaeter\n");
			break;
		default:
			fprintf(stderr, "unknowk error\n");
			break;
		}
		exit(-1);
	}

	src_factory.alloc_frame(&src_factory, &src_frame);
	dst_factory.alloc_frame(&dst_factory, &dst_frame);

	for(i = 0; i < fr_skip; i += 1)
	{
		if(read_frames)
		{
			if(VP_IO_OPERATION_FAILED == src_factory.read_frame(&src_factory, &src_frame))
			{
				verbose_print(0,"read input file failed frame #%d\nfile-\"%s\"\n", i + 1, in_name);
				break;
			}
		}
	}

	for(i = 0; i < fr_count; i += 1)
	{
		if(read_frames)
		{
			if(VP_IO_OPERATION_FAILED == src_factory.read_frame(&src_factory, &src_frame))
			{
				verbose_print(0,"read input file failed frame #%d\nfile-\"%s\"\n", i + 1, in_name);
				break;
			}
		}

		if(rgb_switch && 
			((in_cs == VP_RGB24)
			||(in_cs == VP_RGB32)
			||(in_cs == VP_RGB555)
			||(in_cs == VP_RGB565))
			)
		{
			src_frame.height = -src_frame.height;
		}
		tmp_time = clock();
		src_frame.valid_rows_count = 0;
		vp_reset(batch);
		if(i == 0)
		{
			//vp_reset_telecine(batch, VP_NEW_FRAME);
		}
		else
		{
			//vp_reset_telecine(batch, VP_OLD_TOP_NEW_BOT);
		}

		if(rgb_switch && 
			((out_cs == VP_RGB24)
			||(out_cs == VP_RGB32)
			||(out_cs == VP_RGB555)
			||(out_cs == VP_RGB565))
			)
		{
			dst_frame.height = -dst_frame.height;
		}
		do
		{
			src_frame.valid_rows_count += ((rand() % 16) * 2);
			if(src_frame.valid_rows_count > abs(src_frame.height))
			{
				src_frame.valid_rows_count = abs(src_frame.height);
			}
			vp_process(batch, &src_frame, &dst_frame);
		}
		while(src_frame.valid_rows_count < abs(src_frame.height));

		time += (clock() - tmp_time);
		if(read_frames)
		{
			if(VP_IO_OPERATION_FAILED == dst_factory.write_frame(&dst_factory, &dst_frame))
			{
				verbose_print(0,"write output file failed frame #%d\nfile-\"%s\"\n", i, out_name);
				break;
			}
		}
	}

	dst_factory.free_frame(&dst_factory, &dst_frame);
	src_factory.free_frame(&src_factory, &src_frame);

	src_factory.close(&src_factory);
	dst_factory.close(&dst_factory);

	if(i != 0)
	{
		verbose_print(0,"frames counter %d\n", i);
		verbose_print(0,"total time     %6.4f sec\n", (double)time / (double)CLOCKS_PER_SEC );
		if(time == 0) time = 1;
		verbose_print(0,"fps            %6.2f\n", ((double)i * CLOCKS_PER_SEC + time / 2)/(double)time);
	}
	else
	{
		res = -1;
		verbose_print(0, "Error!!! no processed frames\n");
	}
	vp_close(batch);
	close_std_files();

	return res;
}
