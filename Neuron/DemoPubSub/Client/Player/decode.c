/*-------------------------------------------------
	Project:	XvdPlayer
	File:		decode.c
	Author:		Manjesh Malavalli
	Date:		02/20/2010
-------------------------------------------------*/
//--------------------------------------- INCLUDES ------------------------------------------------
#include "thread_shared_data.h"
#include "decode.h"
//---------------------------------------- DEFINES ------------------------------------------------

//---------------------------------------- GLOBALS ------------------------------------------------
extern TSDPtr ptsd_global;
//--------------------------------------- STRUCTURES ----------------------------------------------

//--------------------------------- DECODE THREAD FUNCTIONS --------------------------------------
int decode_interrupt_cb() 
{
	return ( ptsd_global && ptsd_global->quit_flag );
}

int decode_thread_run( void *userdata )
{
	int 	frame_done;
	//double	fps = 0.0, fps_prev = 0.0;
	
	AVFrame		*pFrame;
	AVPacket	packet;
	TSDPtr 		ptsd = (TSDPtr) userdata;
	// Allocate memory for decoded frame
	if( (pFrame=avcodec_alloc_frame())==NULL )
	{
		fprintf( stderr, "Memory allocation problem for decoded frame\n" );
		return -1;
	}
	// Interrupt blocking functions if quit
	ptsd_global = ptsd;
  	url_set_interrupt_cb(decode_interrupt_cb);
	// Decode each frame and enqueue it
	while( !ptsd->quit_flag && av_read_frame( ptsd->pFmtCtx, &packet )>=0 )
	{
		if( packet.stream_index==ptsd->vid_stream_idx )
		{
			avcodec_decode_video2( ptsd->pCodecCtx, pFrame, &frame_done, &packet );
			if( frame_done )
			{
				ptsd->decode_frame_count++;			
				if( !ptsd->quit_flag && !PCQEnqueue( ptsd, pFrame, ptsd->pCodecCtx->pts_delta_ms ) )
					break;
			}
		}
		av_free_packet( &packet );
	}

	ptsd->quit_flag = 1;
	fprintf( stderr, "Decode ended...\n" );	
	return 0;
}

