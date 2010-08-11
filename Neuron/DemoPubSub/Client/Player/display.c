/*-------------------------------------------------
	Project:	XvdPlayer
	File:		display.c
	Author:		Manjesh Malavalli
	Date:		02/19/2010
-------------------------------------------------*/
//--------------------------------------- INCLUDES ------------------------------------------------
#include <libswscale/swscale.h>
#include <time.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <unistd.h>
#include "thread_shared_data.h"
#include "display.h"
#include "../x264_vfr_module/x264_vfr_module.h"
//---------------------------------------- DEFINES ------------------------------------------------

//---------------------------------------- GLOBALS ------------------------------------------------
int64_t disp_cur_frame_time, disp_prev_frame_time, disp_int;
int64_t	cur_pts_delta_mus;
//--------------------------------------- STRUCTURES ----------------------------------------------

//---------------------------- Button Press & SDL Refresh Loop FUNCTIONS --------------------------
void button_click( GtkWidget *widget, gpointer data )
{
	char 		*fps_choice = ((char *) data);
	const char	*button_label = gtk_button_get_label( (GtkButton *) widget );

	*fps_choice = button_label[0];
	return;
}

//---------------------------------- Neuron GUI FUNCTIONS -----------------------------------------
void setup_src_list( NeuronGuiObject *nguiObj )
{
	gchar *colTitles[2] = { "Source Name", "Video Stats" };
	
	nguiObj->scrollWin = gtk_scrolled_window_new( NULL, NULL );
	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(nguiObj->scrollWin),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_widget_set_size_request( nguiObj->scrollWin, 250, 150 );
	gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->scrollWin, 10, 15 );
	nguiObj->srcList = gtk_clist_new_with_titles( 2, colTitles );
	gtk_clist_column_titles_passive( GTK_CLIST(nguiObj->srcList) );
	gtk_clist_column_titles_show( GTK_CLIST(nguiObj->srcList) );
	gtk_clist_set_selection_mode( GTK_CLIST(nguiObj->srcList), GTK_SELECTION_SINGLE );
	gtk_container_add( GTK_CONTAINER(nguiObj->scrollWin), nguiObj->srcList );
	
	return;
}

void neu_gui_open( NeuronGuiObject *nguiObj, int vwidth, int vheight, char *fps_choice )
{
	int			i;
	char		*button_text[3] = { "full fps", "half (1/2) fps", "quarter (1/4) fps" };
	GtkStyle	*style;
	char		buf[20];

	// Window and fixed frame
	nguiObj->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size( GTK_WINDOW(nguiObj->window), vwidth+205, vheight+10 );
	gtk_window_set_title( GTK_WINDOW(nguiObj->window), "Neuron Concept Mini Demo" );
	nguiObj->fixedFrame = gtk_fixed_new();
	gtk_container_add( GTK_CONTAINER(nguiObj->window), nguiObj->fixedFrame );
	// Source List
	setup_src_list( nguiObj );
	// Video Screen
	nguiObj->videoScreen = gtk_drawing_area_new();
	gtk_widget_set_size_request( nguiObj->videoScreen, vwidth, vheight );
	gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->videoScreen, 200, 5 );
	// FPS buttons
	for( i=0; i<3; i++ )
	{
		nguiObj->fpsButtons[i] = gtk_button_new_with_label( button_text[i] );
		gtk_widget_set_size_request( nguiObj->fpsButtons[i], 120, 25 );
		gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->fpsButtons[i], 40, 15+i*40 );
	}
	// Bandwidth bar
	nguiObj->bandwidthBar = gtk_progress_bar_new();
	style = gtk_style_new();
	gdk_color_parse( "red", &(style->bg[GTK_STATE_PRELIGHT]) );
	gtk_widget_set_style( nguiObj->bandwidthBar, style );
	g_object_unref( style );
	gtk_progress_bar_set_orientation( (GtkProgressBar *) nguiObj->bandwidthBar, 
									  GTK_PROGRESS_BOTTOM_TO_TOP );
	gtk_progress_bar_set_text( (GtkProgressBar *) nguiObj->bandwidthBar, "BW(kbps)\n0.0" );
	gtk_widget_set_size_request( nguiObj->bandwidthBar, 70, vheight/2 );
	gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->bandwidthBar, 28, 5+vheight/2 );
	// Bandwidth bar labels
	nguiObj->bwBarLabel[0] = gtk_label_new("0");
	sprintf( buf, "%d", (int) BANDWIDTH_FRACTION_DENOM );
	nguiObj->bwBarLabel[1] = gtk_label_new( buf );
	gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->bwBarLabel[0], 15, 5+vheight-15 );
	gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->bwBarLabel[1], 1, 5+vheight/2 );
	// CPU bar
	nguiObj->cpuusageBar = gtk_progress_bar_new();
	style = gtk_style_new();
	gdk_color_parse( "green", &(style->bg[GTK_STATE_PRELIGHT]) );
	gtk_widget_set_style( nguiObj->cpuusageBar, style );
	g_object_unref( style );
	gtk_progress_bar_set_orientation( (GtkProgressBar *) nguiObj->cpuusageBar, 
									  GTK_PROGRESS_BOTTOM_TO_TOP );
	gtk_progress_bar_set_text( (GtkProgressBar *) nguiObj->cpuusageBar, "CPU(%%)\n0.0 %" );
	gtk_widget_set_size_request( nguiObj->cpuusageBar, 70, vheight/2 );
	gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->cpuusageBar, 103, 5+vheight/2 );	
	// CPU bar labels
	nguiObj->cpuBarLabel[0] = gtk_label_new("0");
	sprintf( buf, "%d", (int) MAX_PERCENT_CPUBAR );
	nguiObj->cpuBarLabel[1] = gtk_label_new( buf );
	gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->cpuBarLabel[0], 178, 5+vheight-15 );
	gtk_fixed_put( GTK_FIXED(nguiObj->fixedFrame), nguiObj->cpuBarLabel[1], 178, 5+vheight/2 );
	// Show GUI
	gtk_widget_show_all(nguiObj->window);
	g_signal_connect( G_OBJECT(nguiObj->window), "destroy", G_CALLBACK(gtk_main_quit), NULL );

	for( i=0; i<3; i++ )
	{
		g_signal_connect( G_OBJECT(nguiObj->fpsButtons[i]), "clicked", 
						  G_CALLBACK(button_click), fps_choice );
		gtk_widget_hide( nguiObj->fpsButtons[i] );
	}

	gtk_widget_hide( nguiObj->videoScreen );
	return;
}
//---------------------------------- SDLDisplay FUNCTIONS -----------------------------------------
int SDLDInit( NeuronGuiObject *nguiObj, int w, int h, int otype, int pixfmt, char *fps_choice )
{
	char sdl_gtk_hack[50];
	
	nguiObj->pSdlDisp = (SDLDisplay *) av_mallocz( sizeof(SDLDisplay) );
	nguiObj->pSdlDisp->ovl_type = otype;
	nguiObj->pSdlDisp->pix_fmt = pixfmt;
	nguiObj->pSdlDisp->width = w;
	nguiObj->pSdlDisp->height = h;
	
	neu_gui_open( nguiObj, w, h, fps_choice );
	// Hack for SDL to use GTK window
	sprintf( sdl_gtk_hack,"SDL_WINDOWID=%ld", GDK_WINDOW_XWINDOW(nguiObj->videoScreen->window));
	putenv(sdl_gtk_hack);
	// Init SDL window
	if( SDL_Init( SDL_INIT_VIDEO|SDL_INIT_TIMER ) )
	{
		fprintf( stderr, "Could not initialize SDL display: %s\n", SDL_GetError() );
		return 0;
	}
	nguiObj->pSdlDisp->pScreen = SDL_SetVideoMode( w, h, 0, 0 );
	if( nguiObj->pSdlDisp->pScreen==NULL )
	{
		fprintf( stderr, "Could not set video mode\n" );
		return 0;
	}
	nguiObj->pSdlDisp->pOvl = SDL_CreateYUVOverlay( w, h, nguiObj->pSdlDisp->ovl_type, 
													nguiObj->pSdlDisp->pScreen );
	nguiObj->pSdlDisp->dispRect.x = nguiObj->pSdlDisp->dispRect.y = 0;
	nguiObj->pSdlDisp->dispRect.w = nguiObj->pSdlDisp->width;
	nguiObj->pSdlDisp->dispRect.h = nguiObj->pSdlDisp->height;
	return 1;
}

int SDLDDispPCQFrame( SDLDisplay *pSdlDisp, TSDPtr ptsd )
{
	AVPicture	dispPic;
	AVFrame		*pFrame;
	SDL_Rect	dispRect;
	// Load convertefd frame onto overlay
	SDL_LockYUVOverlay( pSdlDisp->pOvl );
	dispPic.data[0] = pSdlDisp->pOvl->pixels[0];
	dispPic.data[1] = pSdlDisp->pOvl->pixels[2];
	dispPic.data[2] = pSdlDisp->pOvl->pixels[1];
	dispPic.linesize[0] = pSdlDisp->pOvl->pitches[0];
	dispPic.linesize[1] = pSdlDisp->pOvl->pitches[2];
	dispPic.linesize[2] = pSdlDisp->pOvl->pitches[1];
	
	pFrame = ptsd->ppcq->pFrameList[ptsd->ppcq->read_pos];
	img_convert( &dispPic, pSdlDisp->pix_fmt, (AVPicture *) pFrame, ptsd->pCodecCtx->pix_fmt,
				 ptsd->pCodecCtx->width, ptsd->pCodecCtx->height );
	SDL_UnlockYUVOverlay( pSdlDisp->pOvl );
	// Display overlaid frame
	SDL_DisplayYUVOverlay( pSdlDisp->pOvl, &(pSdlDisp->dispRect) );

	return 1;	
}
//--------------------------------- DISPLAY THREAD FUNCTIONS --------------------------------------
Uint32 refresh_callback( Uint32 interval, void *opaque )
{
	SDL_Event event;
	event.type = FF_REFRESH_EVENT;
	event.user.data1 = opaque;
	SDL_PushEvent( &event );
	return 0;
}

int	refresh_display( NeuronGuiObject *pNGObj, void *userdata )
{
	TSDPtr 			ptsd = (TSDPtr) userdata;
	struct timespec time_to_wait;
	
	update_display_stats( pNGObj );
	
	if( ptsd->pFmtCtx->streams[ptsd->vid_stream_idx] )
	{
		if( ptsd->ppcq->len==0 )
			SCHED_DISP_REFRESH( ptsd, (int32_t) (cur_pts_delta_mus/10000)*10 );
		else
		{
			// If first frame, store pts reference time as current time
			if( ptsd->cur_pts_mus==0 )	
			{
				ptsd->pts_ref_time_mus = av_gettime();
				disp_prev_frame_time = GET_REL_TIME_MUS(ptsd->pts_ref_time_mus);
				cur_pts_delta_mus = 0;
			}
			ptsd->disp_frame_count++;
			// Determine wait time for current frame
			disp_cur_frame_time = GET_REL_TIME_MUS(ptsd->pts_ref_time_mus);
			disp_int = disp_cur_frame_time - disp_prev_frame_time;
			//printf( "fc: %lld, ts: %lld (mus), ", ptsd->disp_frame_count, 
			//											    ptsd->cur_pts_mus );
			//printf( "id: %lld (mus), ", cur_pts_delta_mus-disp_int );
			// If early wait before displaying
			if( cur_pts_delta_mus>disp_int )
			{
				time_to_wait.tv_sec = (time_t) (cur_pts_delta_mus-disp_int)/1000000;
				time_to_wait.tv_nsec = (long) ((cur_pts_delta_mus-disp_int)%1000000)*1000;
				nanosleep( &time_to_wait, NULL );
			}
			// Display current frame
			disp_cur_frame_time = GET_REL_TIME_MUS(ptsd->pts_ref_time_mus);
			//printf( "dt: %lld (mus), ", disp_cur_frame_time );
			//printf( "dti: %lld (mus), ", disp_cur_frame_time-disp_prev_frame_time );
			SDLDDispPCQFrame( pNGObj->pSdlDisp, ptsd );
			disp_prev_frame_time = disp_cur_frame_time;
			// Schedule next refresh (interval is multiple of 10 ms)
			cur_pts_delta_mus = ((int64_t) ptsd->ppcq->pts_delta_ms[ptsd->ppcq->read_pos])*1000;
			SCHED_DISP_REFRESH( ptsd, (int32_t) (cur_pts_delta_mus/10000)*10 );
			//printf( "i: %d (%d), ", (int) (cur_pts_delta_mus/1000), 
			//						(int) (cur_pts_delta_mus/10000)*10 );
			ptsd->cur_pts_mus += cur_pts_delta_mus;
			//printf( "q_len_time: %lld ms\n", PCQDequeue( ptsd ) );
			PCQDequeue( ptsd );
		}
	}
	else
		SCHED_DISP_REFRESH( ptsd, 100 );	
	
	return 1;
}

int update_display_stats( NeuronGuiObject *pNGObj )
{
	char 	bandwidth_bar_text[20];
	double	new_fraction;
	
	SDL_CondWait( cond, mutex );
	SDL_LockMutex( mutex );
	
	new_fraction = (*bandwidth)/BANDWIDTH_FRACTION_DENOM;
	if( new_fraction>1.0 ) new_fraction = 1.0;
	gtk_progress_bar_set_fraction( (GtkProgressBar * ) pNGObj->bandwidthBar, new_fraction );
	sprintf( bandwidth_bar_text, "BW(kbps)\n%.2lf", *bandwidth );
	gtk_progress_bar_set_text( (GtkProgressBar * ) pNGObj->bandwidthBar, bandwidth_bar_text );
	
	new_fraction = (*cpu_usage)/MAX_PERCENT_CPUBAR;
	if( new_fraction>1.0 ) new_fraction = 1.0;
	gtk_progress_bar_set_fraction( (GtkProgressBar * ) pNGObj->cpuusageBar, new_fraction );
	sprintf( bandwidth_bar_text, "CPU(%%)\n%3.2lf", *cpu_usage );
	gtk_progress_bar_set_text( (GtkProgressBar * ) pNGObj->cpuusageBar, bandwidth_bar_text );

	SDL_CondSignal( cond );
	SDL_UnlockMutex( mutex );	
	return 1;
}
//--------------------------------------- MISC FUNCTIONS ------------------------------------------
int img_convert( AVPicture *pDstPic, int dst_pix_fmt, AVPicture *pSrcPic, 
				  int src_pix_fmt, int width, int height )
{
	static struct SwsContext *pImgConvCtx = NULL;
	if( pImgConvCtx==NULL )
	{
		pImgConvCtx = sws_getContext( width, height, src_pix_fmt, width, height, dst_pix_fmt, 
									  SWS_BICUBIC, NULL, NULL, NULL );
		if( pImgConvCtx==NULL )
		{
			fprintf( stderr, "Cannot create conversion context\n" );
			exit(1);
		}
	}
	sws_scale( pImgConvCtx, pSrcPic->data, pSrcPic->linesize, 0, height, pDstPic->data,
			   pDstPic->linesize );
			   
	return 1;
}

