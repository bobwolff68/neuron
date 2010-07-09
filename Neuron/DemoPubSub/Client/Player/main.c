/*-------------------------------------------------
	Project:	XvdPlayer
	File:		decode.c
	Author:		Manjesh Malavalli
	Date:		02/20/2010
-------------------------------------------------*/
//--------------------------------------- INCLUDES ------------------------------------------------
#include <sys/types.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "thread_shared_data.h"
#include "display.h"
#include "decode.h"
#include "../x264_vfr_module/x264_vfr_module.h"
//---------------------------------------- DEFINES ------------------------------------------------
#define VID_W	640
#define VID_H	480
//---------------------------------------- GLOBALS ------------------------------------------------
TSDPtr *ptsd_global;
//--------------------------------------- STRUCTURES ----------------------------------------------

//--------------------------------------- MISC FUNCS ----------------------------------------------

//-------------------------------------------------------------------------------------------------  
int main( int argc, char *argv[] )
{
	NeuronGuiObject	*ngObj;
	TSDPtr			ptsd;
	AVCodec			*pCodec;
	SDL_Event		event;
	VFRModule		*pvfrm;
	char			vfrm_opfn[100] = "../../Fifos/x264vfrmout";
	char			*key_press_str;
	int 			i;
	int				vfrm_id;

	// Setup communication channel between vfrm and decode/display thread
	strcat( vfrm_opfn, argv[1] );
	strcat( vfrm_opfn, ".264" );
	if( (i = access( vfrm_opfn, F_OK ))<0 )
		mkfifo( vfrm_opfn, S_IRWXU|S_IRWXG );
		
	gtk_init( &argc, &argv );
	ngObj = (NeuronGuiObject *) av_mallocz( sizeof(NeuronGuiObject) );
	ptsd = (TSDPtr) av_mallocz( sizeof( ThreadSharedData ) );
	ptsd->vid_stream_idx = -1;
	ptsd->quit_flag = 0;
	ptsd->h264mux_throttle_signal = H264MUX_MAX_THROTTLE;
	// Start vfr module thread
	pvfrm = (VFRModule *) av_mallocz( sizeof(VFRModule) );
	//pvfrm->video_src_id_str = NULL;
	pvfrm->vfrm_output_fn = vfrm_opfn;
	pvfrm->p_pcq_throt_signal = &(ptsd->h264mux_throttle_signal);
	sscanf( argv[1], "%d", &vfrm_id );
	pvfrm->ndp.dp_id = (DDS_long) vfrm_id;
	NeuronDP_create_dp_factory( &(pvfrm->ndp) );
	
	ptsd->vfrm_thread = SDL_CreateThread( VFRM_thread_run, (void *) pvfrm );
	// Register all available file formats and codecs with the library
	av_register_all();
	// Open file and store container information in format context object
	printf( "Neuron Concept Demo Client starting up....Src Id = %s\n", pvfrm->video_src_id_str );
	if( av_open_input_file( &(ptsd->pFmtCtx), vfrm_opfn, NULL, 0, NULL )!=0 )
	{
		fprintf( stderr, "Could not open file: %s\n", vfrm_opfn );
		return -1;
	}
	printf( "Streaming started....\n" );
	// Retrieve stream information
	/*if( av_find_stream_info( ptsd->pFmtCtx )<0 )
	{
		fprintf( stderr, "Could not retrieve stream info from file: %s\n", vfrm_opfn );
		return -1;
	}*/
	// Dump file info onto stderr
	//dump_format( ptsd->pFmtCtx, 0, vfrm_opfn, 0 );
	// Find the video stream and get pointer to its codec context
	for( i=0; i<(ptsd->pFmtCtx->nb_streams); i++ )
	{
		if( ptsd->pFmtCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO )
		{
			ptsd->vid_stream_idx = i;
			break;
		}
	}
	if( ptsd->vid_stream_idx==-1 )
	{
		fprintf( stderr, "Could not retrieve stream from file: %s\n", vfrm_opfn );
		return -1;
	}
	ptsd->pCodecCtx = ptsd->pFmtCtx->streams[ptsd->vid_stream_idx]->codec;
	ptsd->pCodecCtx->width = VID_W;
	ptsd->pCodecCtx->height = VID_H;
	ptsd->pCodecCtx->pix_fmt = PIX_FMT_YUV422P;
	// Find and open appropriate decoder based on codec context
	pCodec = avcodec_find_decoder( ptsd->pCodecCtx->codec_id );
	if( pCodec==NULL )
	{
		fprintf( stderr, "No suitable decoder found for stream in file %s\n", vfrm_opfn );
		return -1;
	}
	if( avcodec_open( ptsd->pCodecCtx, pCodec )<0 )
	{
		fprintf( stderr, "Could not open decoder\n" );
		return -1;
	}
	// Initialize presentation queue
	PCQInit( ptsd );
  	// Start decode thread
  	ptsd->decode_frame_count = 0;
  	ptsd->decode_thread = SDL_CreateThread( decode_thread_run, (void *) ptsd );
  	if( ptsd->decode_thread==NULL )
  	{
  		fprintf( stderr, "Could not instantiate decoder thread\n" );
		return -1;
  	}
  	ptsd->disp_frame_count = 0;
  	ptsd->cur_pts_mus = 0;
  	// Init display
	SDLDInit( ngObj, ptsd->pCodecCtx->width, ptsd->pCodecCtx->height, 
			  SDL_YV12_OVERLAY, PIX_FMT_YUV420P, &(pvfrm->new_fps_choice) );
  	SCHED_DISP_REFRESH( ptsd, ptsd->pCodecCtx->pts_delta_ms );
  	// Main loop
	gtk_idle_add( SDL_refresh_loop, ngObj );
	gtk_main();
  	// Clean up
  	printf( "Streaming finished, waiting for decoder and VFR module to exit....\n" );
  	ptsd->quit_flag = 1;
  	pvfrm->quit_flag = 1;
  	SDL_WaitThread( ptsd->decode_thread, NULL );
  	SDL_WaitThread( ptsd->vfrm_thread, NULL );
  	printf( "Releasing resources....\n" );
  	SDL_Quit();
  	avcodec_close( ptsd->pCodecCtx );
	av_close_input_file( ptsd->pFmtCtx );
	PCQCleanUp( ptsd );
	av_freep( ptsd );
	printf( "Client exiting....\n" );
	return 0;
}

