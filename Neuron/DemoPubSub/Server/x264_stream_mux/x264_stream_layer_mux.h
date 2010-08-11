#include "x264_stream_parser.h"

//#define MUX_FRM_OP_CLK_PERIOD_MUS(throttle) (25000*(throttle))
#define TTW_PUBLISH	20
		
#define H264MUX_KILL_SIGNAL		3
#define H264MUX_MAX_THROTTLE	1
#define H264MUX_MED_THROTTLE	2
#define	H264MUX_PAUSE_SIGNAL	0	

typedef struct
{
	frameStreamContainer	fl[3];
	int64_t					frm_publish_period_mus;					
	char					*x264mux_input_fn_prefix;
	int						loop_flag;
} x264_stream_layer_mux;

typedef x264_stream_layer_mux *x264_slmux_ptr;

int x264_stream_layer_mux_init( x264_slmux_ptr, char *, char *, double );
int x264_stream_layer_mux_thread_run( void *, char *, char *, double );

