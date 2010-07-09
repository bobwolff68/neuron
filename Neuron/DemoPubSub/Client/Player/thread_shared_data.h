/*-------------------------------------------------
	Project:	XvdPlayer
	File:		thread_shared_data.h
	Author:		Manjesh Malavalli
	Date:		02/19/2010
-------------------------------------------------*/
//--------------------------------------- INCLUDES ------------------------------------------------
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
//---------------------------------------- DEFINES ------------------------------------------------
#define MAX_PCQ_SIZE_TIME_MS	1500
#define MED_PCQ_SIZE_TIME_MS	600
#define MIN_PCQ_SIZE_TIME_MS	300
#define MAX_PCQ_LEN				50
#define H264MUX_MAX_THROTTLE	'1'
#define H264MUX_MED_THROTTLE	'2'
#define	H264MUX_PAUSE_SIGNAL	'0'	
//---------------------------------------- GLOBALS ------------------------------------------------

//--------------------------------------- STRUCTURES ----------------------------------------------
typedef struct
{
	int		len;
	int64_t	len_time_ms;
	int		read_pos;
	int		write_pos;
	int32_t	pts_delta_ms[MAX_PCQ_LEN];
	
	AVFrame		*pFrameList[MAX_PCQ_LEN];
	SDL_mutex	*mutex;
	SDL_cond	*cond;
} PresentCQ;

typedef PresentCQ *PCQPtr;

typedef struct
{
	int		vid_stream_idx;
	int 	quit_flag;
	char	h264mux_throttle_signal;
	int64_t	cur_pts_mus;
	int64_t pts_ref_time_mus;
	int64_t decode_frame_count;
	int64_t	disp_frame_count;
	
	AVFormatContext	*pFmtCtx;
	AVCodecContext	*pCodecCtx;
	PCQPtr		 ppcq;
	SDL_Thread	*decode_thread;
	SDL_Thread	*vfrm_thread;
} ThreadSharedData;

typedef ThreadSharedData *TSDPtr;

//------------------------------------- PCQ FUNCTIONS ---------------------------------------------
int 	PCQInit( TSDPtr );
int 	PCQEnqueue( TSDPtr, AVFrame *, uint32_t );
int64_t	PCQDequeue( TSDPtr );
int 	PCQPrintStats( TSDPtr, int );
int 	PCQCleanUp( TSDPtr );
//-------------------------------------------------------------------------------------------------

