//-------------------------------------------------------------------------------------------------
// File:   x264_vfr_module.h
// Author: Manjesh Malavalli
// Date:   03/04/2010
//-------------------------------------------------------------------------------------------------
#include "x264_buffer_parser.h"
//-------------------------------------------------------------------------------------------------
#define DEFAULT_GOP_SIZE 			4
#define BANDWIDTH_FRACTION_DENOM 	500.0
#define MAX_PERCENT_CPUBAR			20.0
#define STATS_CALC_CLK_PERIOD_MUS	2000000
#define SPS_STREAM_IDX( gop_len ) \
		DEFAULT_GOP_SIZE/((gop_len)-((gop_len)==(DEFAULT_GOP_SIZE+1)))
#define SPS_STREAM_IDX_MODIFY( idx )	((idx) + ((idx)==(DEFAULT_GOP_SIZE>>1)))
#define	FRAME_QUERY "layerType < %1"
#define LAYER_LIMIT_STR( fps_choice ) ( (fps_choice)=='q' ? "1" : ((fps_choice)=='h' ? "2" : "3") )
//-------------------------------------------------------------------------------------------------
int64_t		time_mus;
SDL_mutex	*mutex;
SDL_cond	*cond;
double		*bandwidth;
double		*cpu_usage;
long long	*cpu_time;		
long long	*proc_time;

typedef struct
{
	int		gop_len;
	int		gop_buf_size;
	int		gop_buf_ptr;
	int		is_pframe_present;
	int     sps_stream_index;
	uchar_t *gop_buf;
} GOPQ;

typedef struct
{
	int						opfd;
	frameStreamContainer 	fsc;
	NeuronDP				ndp;
	GOPQ					gopq;
	char					*vfrm_output_fn;
	int						quit_flag;
	char					fps_choice;
	char					new_fps_choice;
	char					throt_signal;
	char					*p_pcq_throt_signal;	// Pointer to the PCQ throt signal variable
	char					*video_src_id_str;
} VFRModule;
//------------------------------GOPQ FUNCTIONS-----------------------------------------------------
int GOPQ_init( GOPQ * );
int GOPQ_reset( GOPQ * );
int GOPQ_add_frame( GOPQ *, fscPtr );
int GOPQ_flush_to_fifo( GOPQ *, int );
//------------------------------VFRM FUNCTIONS-----------------------------------------------------
int 	VFRM_init( VFRModule * );
int 	VFRM_thread_run( void * );
void	VFRM_get_cpu_usage( pid_t );
void	VFRM_destroy( VFRModule * );

