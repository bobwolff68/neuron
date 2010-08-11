/*-------------------------------------------------
	Project:	XvdPlayer
	File:		thread_shared_data.c
	Author:		Manjesh Malavalli
	Date:		02/19/2010
-------------------------------------------------*/
//--------------------------------------- INCLUDES ------------------------------------------------
#include <unistd.h>
#include "thread_shared_data.h"
//---------------------------------------- DEFINES ------------------------------------------------

//---------------------------------------- GLOBALS ------------------------------------------------

//--------------------------------------- FUNCTIONS -----------------------------------------------
int PCQInit( TSDPtr ptsd )
{
	int i;
	int frame_data_size;
	uint8_t	*frame_data_buf = NULL;
	
	ptsd->ppcq = ( PCQPtr ) av_mallocz( sizeof( PresentCQ ) );
	ptsd->ppcq->len = 0;
	ptsd->ppcq->read_pos = 0;
	ptsd->ppcq->write_pos = 0;
	ptsd->ppcq->mutex = SDL_CreateMutex();
	ptsd->ppcq->cond = SDL_CreateCond();
	ptsd->ppcq->len_time_ms = 0;
	
	frame_data_size = avpicture_get_size( ptsd->pCodecCtx->pix_fmt, ptsd->pCodecCtx->width, 
										  ptsd->pCodecCtx->height );
	
	for( i=0; i<MAX_PCQ_LEN; i++ )
	{
		if( (ptsd->ppcq->pFrameList[i]=avcodec_alloc_frame())==NULL)
		{
			fprintf( stderr, "XvdPlayer Error: Could not allocate memory for Presentation \
							  Queue elements." );
			return 0;
		}
		// Allocate frame data buffer
		frame_data_buf = (uint8_t *) av_mallocz( sizeof( uint8_t )*frame_data_size );
		avpicture_fill( (AVPicture *) ptsd->ppcq->pFrameList[i], 
						frame_data_buf, ptsd->pCodecCtx->pix_fmt, 
						ptsd->pCodecCtx->width, ptsd->pCodecCtx->height );
		ptsd->ppcq->pts_delta_ms[i] = 0;
		frame_data_buf = NULL;
	}
	
	return 1;
}

int PCQEnqueue( TSDPtr ptsd, AVFrame *pFrame, uint32_t pts_del_ms )
{
	AVFrame *pf;
	// Wait until PCQ has space to add frame
	SDL_LockMutex( ptsd->ppcq->mutex );
	while( ptsd->ppcq->len_time_ms>=MAX_PCQ_SIZE_TIME_MS && !ptsd->quit_flag )
		SDL_CondWait( ptsd->ppcq->cond, ptsd->ppcq->mutex );
	SDL_UnlockMutex( ptsd->ppcq->mutex );
	if( ptsd->quit_flag )	return 0;
	// Add frame to PCQ
	pf = ptsd->ppcq->pFrameList[ptsd->ppcq->write_pos];
	av_picture_copy( (AVPicture *) pf, (AVPicture *) pFrame, ptsd->pCodecCtx->pix_fmt,
				  	  ptsd->pCodecCtx->width, ptsd->pCodecCtx->height );
	ptsd->ppcq->pts_delta_ms[ptsd->ppcq->write_pos] = pts_del_ms;
	// Signal display thread that a frame has been added
	ptsd->ppcq->write_pos = (ptsd->ppcq->write_pos+1) % MAX_PCQ_LEN;
	SDL_LockMutex( ptsd->ppcq->mutex );
	ptsd->ppcq->len++;
	ptsd->ppcq->len_time_ms += pts_del_ms;

	//fprintf( stderr, "QLen: %d\n", ptsd->ppcq->len );
	SDL_UnlockMutex( ptsd->ppcq->mutex );	
	return 1;
}

int64_t PCQDequeue( TSDPtr ptsd )
{
	SDL_LockMutex( ptsd->ppcq->mutex );
	ptsd->ppcq->len--;
	//fprintf( stderr, "QLen: %d\n", ptsd->ppcq->len );
	ptsd->ppcq->len_time_ms -= (ptsd->ppcq->pts_delta_ms[ptsd->ppcq->read_pos]);
	SDL_CondSignal( ptsd->ppcq->cond );
	SDL_UnlockMutex( ptsd->ppcq->mutex );
	// Remove first frame
	ptsd->ppcq->read_pos = (ptsd->ppcq->read_pos+1) % MAX_PCQ_LEN;	
	
	return ptsd->ppcq->len_time_ms;
}

int PCQPrintStats( TSDPtr ptsd, int call_thread )
{
	int i;
	char *thread_names[2] = { "decode", "display" };
	printf( "Queue Stats (%s):\n", thread_names[call_thread] );
	printf( "Frame count: %d, Read Pos: %d, Write pos: %d\n", ptsd->ppcq->len, 
															  ptsd->ppcq->read_pos,
															  ptsd->ppcq->write_pos );
															  	  
	return 1;
}

int PCQCleanUp( TSDPtr ptsd )
{
	int i;
	
	for( i=0; i<MAX_PCQ_LEN; i++ )
		if( ptsd->ppcq->pFrameList[i]!=NULL )	
			av_freep( ptsd->ppcq->pFrameList[i] );
	
	SDL_DestroyMutex( ptsd->ppcq->mutex );
	SDL_DestroyCond( ptsd->ppcq->cond );
	return 0;
}
//-------------------------------------------------------------------------------------------------

