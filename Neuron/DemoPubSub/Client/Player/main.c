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

// RTI item.
//#ifdef RTI_STYLE
//#define DDS_long DDS_Long
//#endif

#include "../x264_vfr_module/x264_vfr_module.h"
//---------------------------------------- DEFINES ------------------------------------------------
#define VID_W	640
#define VID_H	480

#define SCHED_SRCLIST_REFRESH( pvrfm, delay_ms ) \
		SDL_AddTimer( delay_ms, refresh_srclist_callback, pvrfm )
//---------------------------------------- GLOBALS ------------------------------------------------
TSDPtr *ptsd_global;
//--------------------------------------- STRUCTURES ----------------------------------------------
typedef struct
{
	VFRModule		*pvfrm;
	TSDPtr			ptsd;
	NeuronGuiObject	*pNGObj;
} SelectRowUserData;

typedef SelectRowUserData *SRUDPtr;
//--------------------------------------- MISC FUNCS ----------------------------------------------
Uint32 refresh_srclist_callback( Uint32 interval, void *opaque )
{
	SDL_Event event;
	event.type = FF_REFRESH_SRCLIST_EVENT;
	event.user.data1 = opaque;
	SDL_PushEvent( &event );
	return 0;
}

void refresh_src_list( NeuronGuiObject *pNGObj, void *data )
{
	int			i;
	char		*srcListEntry[2];
	VFRModule 	*pvfrm = (VFRModule *) data;
	
	// Refresh source list
	if( !strcmp( pvfrm->video_src_id_str, "-1" ) )
	{
		srcListEntry[0] = (char *) malloc( sizeof(char)*256 );
		srcListEntry[1] = (char *) malloc( sizeof(char)*100 );
	
		// Refresh entries in Clist widget
		gtk_clist_freeze( GTK_CLIST(pNGObj->srcList) );
		gtk_clist_clear( GTK_CLIST(pNGObj->srcList) );
		for( i=0; i<srcNameListLen; i++ )
		{
			strcpy( srcListEntry[0], srcNameList[i] );
			strcpy( srcListEntry[1], srcVidStats[i] );
			gtk_clist_append( GTK_CLIST(pNGObj->srcList), srcListEntry );
		}
		
		gtk_clist_thaw( GTK_CLIST(pNGObj->srcList) );	
		free( srcListEntry[0] );
		free( srcListEntry[1] );
		SCHED_SRCLIST_REFRESH( pvfrm, 1000 );
	}
	return;
}

void SDL_refresh_loop( gpointer data )
{
	SDL_Event 		event;
	NeuronGuiObject *pNGObj = (NeuronGuiObject *) data;

	SDL_WaitEvent( &event );
	switch( event.type )
	{
		case FF_REFRESH_EVENT:
								refresh_display( pNGObj, event.user.data1 );
								break;
								
		case FF_REFRESH_SRCLIST_EVENT:
										refresh_src_list( pNGObj, event.user.data1 );
										break;
	}
  	return;
}

void on_video_src_select( GtkWidget *widget, gint row, gint column,GdkEventButton *event, 
						  gpointer data )
{
	long	src_id;
	long	vid_src_id;
	int		i;
	
	SRUDPtr	psrud = (SRUDPtr) data;
	AVCodec	*pCodec;
	
	if( !strcmp( psrud->pvfrm->video_src_id_str, "-1" ) )
	{
		// Extract ID of selected source
		strcpy( psrud->pvfrm->video_src_id_str, srcNameList[row] );
		
		// Hide Source List and Show Buttons
		gtk_widget_hide( widget );
		
		// Display Frame Rate Choice Buttons
		for( i=0; i<3; i++ )
			gtk_widget_show( psrud->pNGObj->fpsButtons[i] );
		
		// Display Video Screen
		gtk_widget_show( psrud->pNGObj->videoScreen );
		
		NVSSetVDSPartition(srcNameList[row]);
		usleep(50000);
		
		// Start vfr module thread
		psrud->ptsd->vfrm_thread = SDL_CreateThread( VFRM_thread_run, (void *) psrud->pvfrm );
		
		// Register all available file formats and codecs with the library
		av_register_all();

		// Open file and store container information in format context object
		if( av_open_input_file( &(psrud->ptsd->pFmtCtx), psrud->pvfrm->vfrm_output_fn, NULL, 0, 
								NULL )!=0 )
		{
			fprintf( stderr, "Could not open file: %s\n", psrud->pvfrm->vfrm_output_fn );
			exit(0);
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
		for( i=0; i<(psrud->ptsd->pFmtCtx->nb_streams); i++ )
		{
			if( psrud->ptsd->pFmtCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO )
			{
				psrud->ptsd->vid_stream_idx = i;
				break;
			}
		}
		if( psrud->ptsd->vid_stream_idx==-1 )
		{
			fprintf( stderr, "Could not retrieve stream from file: %s\n", 
					 psrud->pvfrm->vfrm_output_fn );
			exit(0);
		}
		psrud->ptsd->pCodecCtx = psrud->ptsd->pFmtCtx->streams[psrud->ptsd->vid_stream_idx]->codec;
		psrud->ptsd->pCodecCtx->width = VID_W;
		psrud->ptsd->pCodecCtx->height = VID_H;
		psrud->ptsd->pCodecCtx->pix_fmt = PIX_FMT_YUV422P;
		
		// Find and open appropriate decoder based on codec context
		pCodec = avcodec_find_decoder( psrud->ptsd->pCodecCtx->codec_id );
		if( pCodec==NULL )
		{
			fprintf( stderr, "No suitable decoder found for stream in file %s\n", 
					 psrud->pvfrm->vfrm_output_fn );
			exit(0);
		}
		if( avcodec_open( psrud->ptsd->pCodecCtx, pCodec )<0 )
		{
			fprintf( stderr, "Could not open decoder\n" );
			exit(0);
		}
	
		// Initialize presentation queue
		PCQInit( psrud->ptsd );
  	
  		// Start decode thread
  		psrud->ptsd->decode_frame_count = 0;
  		psrud->ptsd->decode_thread = SDL_CreateThread( decode_thread_run, (void *) (psrud->ptsd) );
  		if( psrud->ptsd->decode_thread==NULL )
  		{
  			fprintf( stderr, "Could not instantiate decoder thread\n" );
			exit(0);
  		}
  		psrud->ptsd->disp_frame_count = 0;
  		psrud->ptsd->cur_pts_mus = 0;
  	
  		// Trigger the first display refresh event
  		SCHED_DISP_REFRESH( psrud->ptsd, psrud->ptsd->pCodecCtx->pts_delta_ms );
	}
	
	return;
}

//-------------------------------------------------------------------------------------------------  
int main( int argc, char *argv[] )
{
	NeuronGuiObject	*ngObj;
	TSDPtr			ptsd;
	SRUDPtr			psrud;
	SDL_Event		event;
	VFRModule		*pvfrm;
	char			vfrm_opfn[100] = "../../Fifos/x264vfrmout";
	char			*key_press_str;
	int 			i;

	// Setup communication channel between vfrm and decode/display thread
	strcat( vfrm_opfn, argv[1] );
	strcat( vfrm_opfn, ".264" );
	if( (i = access( vfrm_opfn, F_OK ))<0 )
		mkfifo( vfrm_opfn, S_IRWXU|S_IRWXG );
		
	gtk_init( &argc, &argv );
	ngObj = (NeuronGuiObject *) av_mallocz( sizeof(NeuronGuiObject) );
	psrud = (SRUDPtr) av_mallocz( sizeof(SelectRowUserData) );
	ptsd = (TSDPtr) av_mallocz( sizeof( ThreadSharedData ) );
	ptsd->vid_stream_idx = -1;
	ptsd->quit_flag = 0;
	pvfrm = (VFRModule *) av_mallocz( sizeof(VFRModule) );
	pvfrm->vfrm_output_fn = vfrm_opfn;

	VFRM_init( pvfrm, argv[1] );
	SDLDInit( ngObj, VID_W, VID_H, 
			  SDL_YV12_OVERLAY, PIX_FMT_YUV420P, &(pvfrm->new_fps_choice) );
	psrud->pvfrm = pvfrm;
	psrud->ptsd = ptsd;
	psrud->pNGObj = ngObj;
	g_signal_connect( G_OBJECT(ngObj->srcList), "select_row", G_CALLBACK(on_video_src_select),
					  psrud );
  	// Main loop
	SCHED_SRCLIST_REFRESH( pvfrm, 1000 );
	gtk_idle_add( (GtkFunction)SDL_refresh_loop, ngObj );
	gtk_main();

  	// Clean up
  	printf( "Streaming finished, waiting for decoder and VFR module to exit....\n" );
  	ptsd->quit_flag = 1;
  	pvfrm->quit_flag = 1;
  	  
  	printf( "Releasing resources....\n" );
  	
  	SDL_Quit();
  
  	if( strcmp( pvfrm->video_src_id_str, "-1" ) )
  	{
	  	SDL_WaitThread( ptsd->decode_thread, NULL );
  		SDL_WaitThread( ptsd->vfrm_thread, NULL );
  		avcodec_close( ptsd->pCodecCtx );
		av_close_input_file( ptsd->pFmtCtx );
		PCQCleanUp( ptsd );
	}
	
	VFRM_destroy( pvfrm );
	av_free( pvfrm );
	av_free( ngObj->pSdlDisp );
	av_free( ngObj );
	av_free( psrud );
	
	printf( "Client exiting....\n" );
	
	return 0;
}

