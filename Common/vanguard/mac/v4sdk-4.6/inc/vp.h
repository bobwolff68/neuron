
/**
 * @file vp.h
 * VSofts Video Processing Library public API
 *
 * Project:	VSofts H.264 Codec V4
 *
 *
 * (c) Vanguard Software Solutions Inc 1995-2011, All Rights Reserved
 * This document and software are the intellectual property of Vanguard Software Solutions Inc. (VSofts)
 * Your use is governed by the terms of the license agreement between you and VSofts.
 * All other uses and/or modifications are strictly prohibited.
 */

#ifndef __VP_H__
#define __VP_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VPAPI
	#undef VPAPI
#endif

#ifdef WIN32
	#define VPAPI __cdecl
#else
	#define VPAPI
#endif

/** @name VP result codes
*/
// @{
#define VP_OK                 0
#define VP_WRONG_PARAMETER   -2
#define VP_UNKNOWN_FILTER    -3
#define VP_NULL_POINTER      -5
#define VP_IO_OPERATION_FAILED  -10

// @}

/// colorspace specification
typedef enum vp_colorspace_e
{
	VP_UNKNOWN = -1,
	VP_IYUV    =  0,	///< YUV 4:2:0 planar (our internal)
	VP_YV12    =  1,	///< YUV 4:2:0 planar (U&V swapped)
	VP_YUY2    =  2,	///< YUV 4:2:2 packed
	VP_YVYU    =  3,	///< YUV 4:2:2 packed
	VP_UYVY    =  4,	///< YUV 4:2:2 packed
	VP_RGB555  =  5,	///< RGB 5:5:5
	VP_RGB565  =  6,	///< RGB 5:6:5
	VP_RGB24   =  7,	///< RGB 24 bit
	VP_RGB32   =  8,	///< RGB 24 bit + alpha channel
	VP_YUV400  =  9,	///< YUV 4:0:0 planar
	VP_YUV422  = 10,	///< YUV 4:2:2 planar
	VP_TOTAL   = 11		///< total number of color spaces
} vp_colorspace_e;

/// frame specification
typedef struct vp_frame_t
{
	int              width;			///< image width, pixels;
	int              height;		///< image height, pixels;
	vp_colorspace_e  colorspace;	///< image colorspace;
	int              bytes_per_pel;	///< number of bytes per pixel (1/2);
	int              luma_bits;		///< number of bits per luma value (8..14);
	int              chroma_bits;	///< number of bits per chroma value (8..14);
	// data pointers and stride in bytes for frames:
	//		planar formats: data[0]=Y, data[1]=C1, data[2]=C2;
	//			where: C1=first chroma, C2=second chroma plane;
	//			example IYUV: data[0]=Y, data[1]=U, data[2]=V;
	//			example YV12: data[0]=Y, data[1]=V, data[2]=U;
	//		packed & RGB formats: data[0]=image;
	// data pointers and stride in bytes for separate fields:
	//		planar formats: data[0]=Y0, data[1]=C10, data[2]=C20, data[3]=Y1, data[4]=C11, data[5]=C21;
	//		packed & RGB formats: data[0]=field0, data[1]=field1;
	void            *data[6];
	int              stride[6];
	int              valid_rows_count;	///< number of rows to be processed together
} vp_frame_t;

/**
* callback function declaration
* @param ctx - application-defined context;
* @param data - data buffer to write to / read from;
* @param size - number of bytes to process;
* @return number of bytes processed;
*/
typedef long VPAPI vp_data_callback_f(void *ctx, void *data, long size);

struct vp_frame_factory_t;

/// frame operation function prototype
typedef int VPAPI frame_op_f(struct vp_frame_factory_t *factory, vp_frame_t *frame);

/// close frame factory function prototype
typedef int VPAPI close_frame_factory_f(struct vp_frame_factory_t *factory);

/// frame factory instance
typedef struct vp_frame_factory_t
{
	void          *context;
	frame_op_f    *alloc_frame;    ///< alloc one more @ref vp_frame_t
	frame_op_f    *free_frame;     ///< free @ref vp_frame_t
	frame_op_f    *read_frame;     ///< read @ref vp_frame_t from external storage
	frame_op_f    *write_frame;    ///< write @ref vp_frame_t to external storage
	close_frame_factory_f  *close; ///< free allocated resources, raw frames should be already freed
} vp_frame_factory_t;

/**
* initialize frame factory instance for further frame operations;
* @param factory - instance to be created;
* @param width  - frame width, pixels;
* @param height - frame height, pixels;
* @param colorspace - frame colorspace specification;
* @param caller_context - optional caller data to be passed in callbacks;
* @param read_data  - caller callback function to read frame data;
* @param write_data - caller callback function to write frame data;
* @return VP result
*/
int VPAPI vp_init_frame_factory(vp_frame_factory_t *factory,
							 	int width,
							 	int height,
							 	vp_colorspace_e colorspace,
							 	void *caller_context,
							 	vp_data_callback_f *read_data,
							 	vp_data_callback_f *write_data);

/**
* initialize frame factory instance with extra bit depth;
* @param factory - instance to be created;
* @param width  - frame width, pixels;
* @param height - frame height, pixels;
* @param bytes_per_pel - number of bytes per pixel (default is "1");
* @param colorspace - frame colorspace specification;
* @param caller_context - optional caller data to be passed in callbacks;
* @param read_data  - caller callback function to read frame data;
* @param write_data - caller callback function to write frame data;
* @return VP result
*/
int VPAPI vp_init_frame_factory_ex(vp_frame_factory_t *factory,
								int width,
								int height,
								int bytes_per_pel,
								vp_colorspace_e colorspace,
								void *caller_context,
								vp_data_callback_f *read_data,
								vp_data_callback_f *write_data);

/// batch is a frame operation unit
typedef void* vp_batch_t;

/**
* create batch instance;
* @param batch - instance to be created;
* @param frame - source frame definition (width height colorspace);
* @param is_interlace - interlace flag (0/1);
* @return VP result
*/
int VPAPI vp_open(vp_batch_t *batch, vp_frame_t *frame, int is_interlace);

/**
* create batch instance with cropping;
* @param batch - instance to be created;
* @param frame - source frame definition (width height colorspace);
* @param cr_left   - pixels to be cropped from the left;
* @param cr_top    - pixels to be cropped from the top;
* @param cr_width  - resulting width, pixels;
* @param cr_height - resulting height, pixels;
* @param is_interlace - interlace flag (0/1);
* @return VP result
*/
int VPAPI vp_open_crop(vp_batch_t *batch,
					   vp_frame_t *frame,
					   int         cr_left,
					   int         cr_top,
					   int         cr_width,
					   int         cr_height,
					   int         is_interlace);

/**
* set bit depth;
* @param batch - batch instance created by any of vp_open();
* @param y_bit_depth - Y-plane bit depth;
* @param u_bit_depth - U-plane bit depth;
* @param v_bit_depth - V-plane bit depth;
* @return VP result
*/
int VPAPI vp_bit_depth(vp_batch_t *batch,
					   int         y_bit_depth,
					   int         u_bit_depth,
					   int         v_bit_depth);

/**
* close (free) batch instance;
*/
int VPAPI vp_close(vp_batch_t batch);

/// specify part of image to apply filter to
typedef enum vp_target_e
{
	VP_Y   = 1,
	VP_U   = 2,
	VP_V   = 4,
	VP_UV  = ((VP_U)  | (VP_V)),
	VP_YUV = ((VP_UV) | (VP_Y))
} vp_target_e;

/// specify type of filter
typedef enum vp_filter_type_e
{
	VP_BLUR3x3           = 0x10,
	VP_BLUR5x5           = 0x11,

	VP_SHARPEN3x3        = 0x20,
	VP_SHARPEN5x5        = 0x21,

	VP_MEDIAN3x3         = 0x30,
	VP_MEDIAN5x3         = 0x31,

	VP_TEMPORAL_DENOISE  = 0x40,

	VP_TELECINE          = 0x50,  // target must be VP_YUV, param0 and param1 ignored
	VP_DEINTERLACE       = 0x60,  // target must be VP_YUV, param0 is deinterlace type, param1 ignored
	VP_RESIZE            = 0x70,  // target must be VP_YUV, param0 width, param1 height
	VP_COLORSPACE        = 0x80,  // target must be VP_YUV, param0 colorspace, param1 ignored

	VP_POSTFILTER_2D     = 0x90,  // target must be VP_YUV, param0 filter_size_x, param1 filter_size_y
	VP_POSTFILTER_1D     = 0x91   // target must be VP_YUV, param0 filter_size, param1 ignored
} vp_filter_type_e;

/**
* add a filter to the batch;
* @param batch - batch instance created by any of vp_open();
* @param target - specify part of the image to apply filtering to;
* @param type - type of filter;
* @param param0 - first filter parameter;
* @param param1 - second filter parameter;
* @return VP result
*/
int VPAPI vp_add_filter(vp_batch_t       batch,
						vp_target_e      target,
						vp_filter_type_e type,
						int              param0,
						int              param1);


/**
* add a colorspace conversion to the batch;
* @param batch - batch instance created by any of vp_open();
* @param colorspace - specify target colorspace;
* @return VP result
*/
int VPAPI vp_add_colorspace(vp_batch_t batch, vp_colorspace_e colorspace);

/**
* add resize operation to the batch;
* @param batch - batch instance created by any of vp_open();
* @param width  - target image width, pixels;
* @param height - target image height, pixels;
* @return VP result
*/
int VPAPI vp_add_resize(vp_batch_t batch, int width, int height);

/**
* add resize operation to the batch;
* @param batch - batch instance created by any of vp_open();
* @param width  - target image width, pixels;
* @param height - target image height, pixels;
* @param grid_position_x - horisontal phase
* @param grid_position_y - vertical phase
* @param grid_position_denom_x - horisontal grid possition accuracy
* @param grid_position_denom_y - vertical grid possition accuracy
* @param quality - (2-8) speed/quality tradeoff parameter: 2 - max speed; 8 - max quality

* @return VP result
* grid_position_x and grid_position_y specify the horizontal and vertical location of the upper left sample of
* resized frame to the upper left sample of original frame in units of (1/grid_position_denom) of the luma
* sample grid spacing between the samples of original frame
* if grid_position_denom is zero, sample location is calculated automatically in order to preserve
* center-of-gravity of original and resized picture in the same position. 
*/
int VPAPI vp_add_resize_ex(vp_batch_t batch,
						   int width, 
						   int height, 
						   int grid_position_x,
						   int grid_position_denom_x,
						   int grid_position_y,
						   int grid_position_denom_y,
						   int quality);

/// deinterlacing types
typedef enum vp_deinterlace_type_e
{
	VP_NO_DEINTERLACE        = 0,
	VP_DEINTERLACE_TOP       = 1,  ///< copy data from top field to bottom
	VP_DEINTERLACE_BOTTOM    = 2,  ///< copy data from bottom field to top
	VP_DEINTERLACE_ADAPTIVE  = 3   ///< modify bottom field to minimize "interlace" artifacts
} vp_deinterlace_type_e;

/**
* deinterlace can be added only if batch was open with interlace flag
* otherwise error code will be return
* add deinterlace operation to the batch;
* @param batch - batch instance created by any of vp_open();
* @param type  - type of deinterlace operation;
* @return VP result
*/
int VPAPI vp_add_deinterlace(vp_batch_t batch, vp_deinterlace_type_e type);

/**
* prepare for further telecine operation in the batch;
* @param batch - batch instance created by any of vp_open();
* @return VP result
*/
int VPAPI vp_add_telecine(vp_batch_t batch);

/**
* obtain resulting frame specification of the batch;
* @param batch - batch instance created by any of vp_open();
* @param dst - rasulting frame specification;
* @return VP result
*/
int VPAPI vp_get_dst_info(vp_batch_t  batch, vp_frame_t *dst);

/**
* reset the batch before frame operation;
* @param batch - batch instance created by any of vp_open();
* @return VP result
*/
int VPAPI vp_reset(vp_batch_t batch);

/// telecine operation details
typedef enum vp_telecine_fields_e
{
	VP_OLD_TOP_FIELD   = 1,
	VP_OLD_BOT_FIELD   = 2,
	VP_NEW_TOP_FIELD   = 4,
	VP_NEW_BOT_FIELD   = 8
} vp_telecine_fields_e;

/// telecine command
typedef enum vp_telecine_cmd_e
{
	VP_DIRECT          = 0,										// means do new frame in dst and don't cache it
	VP_OLD_FRAME       = (VP_OLD_TOP_FIELD | VP_OLD_BOT_FIELD), // means copy cached frame to dst frame
	VP_NEW_FRAME       = (VP_NEW_TOP_FIELD | VP_NEW_BOT_FIELD), // means process frame cache it and copy it to dst
	VP_NEW_TOP_OLD_BOT = (VP_NEW_TOP_FIELD | VP_OLD_BOT_FIELD), // means process frame and
	VP_OLD_TOP_NEW_BOT = (VP_OLD_TOP_FIELD | VP_NEW_BOT_FIELD)
} vp_telecine_cmd_e;

/**
* reset the batch before frame+telecine operation;
* @param batch - batch instance created by any of vp_open();
* @param command - telecine command to be performed;
* @return VP result
*/
int VPAPI vp_reset_telecine(vp_batch_t batch, vp_telecine_cmd_e command);

/**
* reset the batch before post-filtering;
* @param batch - batch instance created by any of vp_open();
* @param data  - pointers to post-filter data(y,u,v filter coeff.);
* @return VP result
*/
int VPAPI vp_reset_post_filter(vp_batch_t batch, int *data[3]);

/**
* process a next part of the batch as signalled by "src.valid_rows_count" value;
* @param batch - batch instance created by any of vp_open();
* @param src - source frame;
* @param dst - destination frame;
* @return VP result
*/
int VPAPI vp_process(vp_batch_t batch, vp_frame_t *src, vp_frame_t *dst);

/**
* get config returns batch configuration
* if cfg param equals zero then return value is the size of batch configuration
* if return value is negative then it is VP result otherwise it is configuration size
* if cfg param non zero then function writes configuration to cfg pointer  retrun VP reslut
*/
int VPAPI vp_get_config(vp_batch_t batch, char *cfg);




//------------------------------------------------------------------------------
// TODO axen: vqm refactoring!!!
typedef struct vqm_frame_t
{
	void *y;
	void *u;
	void *v;
	int stride_y;
	int stride_uv;
} vqm_frame_t;

typedef struct vqm_result_t
{
	double vqm;
	//can be expanded
} vqm_result_t;

// vqm models
typedef enum
{
	VQM_DEVELOPER_MODEL = 0,
	VQM_GENERAL_MODEL  = 1
	//can be expanded
} vqm_models_e;

typedef struct vqm_settings_t
{
	vqm_models_e num_model;
	int depth;
	int frame_width;
	int frame_height;
	int num_iterations;
	//can be expanded
} vqm_settings_t;


struct vqm_context_t; //private data. Will be allocated by vqm engine 

typedef struct vqm_for_dec_t
{
	double VQMp;
	double psnr;
	int num_p;
	int i;
	vqm_frame_t orig;
	vqm_frame_t rec;
	struct vqm_context_t *vqm_context;
} vqm_for_dec_t;

//init structure, that support vqm calculation
struct vqm_context_t *vqm_init(vqm_settings_t *vqm_settings);

// show next frame pair to calulator ...
// returns 0 - OK, 1 - OK, but no more frames will be accepted before result is retrieved; or error code
int vqm_next_frame_pair(struct vqm_context_t *vqm_context, vqm_frame_t *orig, vqm_frame_t *rec);

// show next frame pair to calulator ...
// returns 0 - OK, 1 - OK, but no more frames will be accepted before result is retrieved; 
// 2 - OK,  but detected scene change; 
// or error code;
int vqm_next_frame_pair_modif(struct vqm_context_t *vqm_context, vqm_frame_t *orig, vqm_frame_t *rec);

// return result in vqm_result_t structure
// returns 0 - OK; or error code
int vqm_get_result(struct vqm_context_t *vqm_context, vqm_result_t *result);

// close vqm_calculator
// returns 0 - OK; or error code
int vqm_close(struct vqm_context_t *vqm_context);
//------------------------------------------------------------------------------
//

#ifdef __cplusplus
}
#endif

#endif//__VP_H__

