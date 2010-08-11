#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "x264_stream_layer_mux.h"

int	clk_multiple = 1;
int ctrl_c_hit = 0;

int64_t av_gettime(void)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

int nanosleep_ttw( int64_t time_mus, int64_t mux_period_mus )
{
	struct timespec	time_to_wait;
	int64_t			ttw_mus;
	
	ttw_mus = av_gettime()-time_mus;
	ttw_mus = clk_multiple*mux_period_mus - ttw_mus;
	ttw_mus = ttw_mus*((int64_t) (ttw_mus>0));
	time_to_wait.tv_sec = (time_t) ttw_mus/1000000;
	time_to_wait.tv_nsec = (long) (ttw_mus%1000000)*1000;
	nanosleep( &time_to_wait, NULL ); 
	
	return 1;
}

int x264_stream_layer_mux_init( x264_slmux_ptr mux, char *name, char *vid_stats_str, double fps )
{
	char	*x264_layer_fn[3];
	char 	*x264_layer_suffix[3] = { "L0","L1","L2" };
	int		i;
	
	// Initialize mux
	if( mux->loop_flag==0 || mux->loop_flag==1 )
	{
		printf( "Video Stats: %s...\n", vid_stats_str );
		NVPStartup( name, vid_stats_str );
	}
	
	// Calculate frame publish period
	mux->frm_publish_period_mus = (int64_t) (1000000.0/fps);
	
	for( i=0; i<3; i++ )
	{
		x264_layer_fn[i] = (char *) malloc( sizeof(char)*strlen(mux->x264mux_input_fn_prefix)+3 );
		if( x264_layer_fn[i]==NULL )
		{
			perror( "x264_stream_layer_mux (malloc())" );
			return 0;
		}
		
		strcpy( x264_layer_fn[i], mux->x264mux_input_fn_prefix );
		strcat( x264_layer_fn[i], x264_layer_suffix[i] );
		if( !fscInit( &(mux->fl[i]), x264_layer_fn[i] ) ) 
			return 0;
	}

	free( x264_layer_fn[0] );
	free( x264_layer_fn[1] );
	free( x264_layer_fn[2] );
	
	return 1;
}

void sig_handle_ctrl_c( int sig )
{
	if( sig==2 )
	{
		printf( "\nCTRL+C (FORCE TERMINATION)\n" );
		ctrl_c_hit = 1;
	}

	return;
}

int x264_stream_layer_mux_thread_run( void *userdata, char *name, char *vid_stats_str, double fps )
{
	int	 	i;
	int64_t	time_mus;
//	int64_t	tmus;
	
	x264_slmux_ptr			mux = (x264_slmux_ptr) userdata;

	printf( "Neuron Concept server started (%s)....\n", (mux->loop_flag==0) ? "one time playback"
														: "loop playback" );

	do
	{
		x264_stream_layer_mux_init( mux, name, vid_stats_str, fps );
		
		if( mux->loop_flag==0 || mux->loop_flag==1 )
		{
			printf( "Waiting for subscriptions for %d seconds (pub period: %lld ms)...\n", TTW_PUBLISH,
															  		mux->frm_publish_period_mus/1000 );
			sleep( TTW_PUBLISH );
			printf( "Publishing started...\n" );
			(void) signal( SIGINT, sig_handle_ctrl_c );
		}
		
		// Send header info first
		for( i=0; i<VIDEO_HDR_TYPES; i++ )
		{
			time_mus = av_gettime();
			if( !fscExtractVideoHeader( &(mux->fl[0]), i, 0 ) )
				return 0;
			
			if( mux->loop_flag==0 || mux->loop_flag==1 )
			{
				nanosleep_ttw( time_mus, mux->frm_publish_period_mus );
				fscWriteFrame( &(mux->fl[0]) );
			}
		}
		
		// Send frames
		while( 
			   !feof( mux->fl[0].x264_stream ) && 
			   !feof( mux->fl[1].x264_stream ) && 
			   !feof( mux->fl[2].x264_stream ) &&
			   !ctrl_c_hit
			 )
		{
			// Write one L0 frame
			time_mus = av_gettime();
//			tmus = time_mus;		
			if( !fscExtractFrame( &(mux->fl[0]), 0 ) )	break;
			nanosleep_ttw( time_mus, mux->frm_publish_period_mus );
			fscWriteFrame( &(mux->fl[0]) );
//			fprintf( stderr, "Time: %lld ms\n", (av_gettime()-tmus)/1000 );
//			tmus = av_gettime();
			// If IDR/I frame, parse and write next P frame.
			if( mux->fl[0].type==X264_TYPE_IDR || mux->fl[0].type==X264_TYPE_I )
			{
				clk_multiple++;
				if( !fscExtractFrame( &(mux->fl[0]), 0 ) )	break;
				nanosleep_ttw( time_mus, mux->frm_publish_period_mus );
				fscWriteFrame( &(mux->fl[0]) );
//				fprintf( stderr, "Time: %lld ms\n", (av_gettime()-tmus)/1000 );
			}
			// Extract the next B frame
//			tmus = av_gettime();
			if( !fscExtractFrame( &(mux->fl[1]), 0 ) )	break;
			clk_multiple++;
			nanosleep_ttw( time_mus, mux->frm_publish_period_mus );
			fscWriteFrame( &(mux->fl[1]) );
//			fprintf( stderr, "Time: %lld ms\n", (av_gettime()-tmus)/1000 );
			// Extract the next 2 b frames and write if fps_op == fps_orig
			for( i=0; i<2; i++ )
			{
//				tmus = av_gettime();
				if( !fscExtractFrame( &(mux->fl[2]), 0 ) )	break;
				clk_multiple++;
				nanosleep_ttw( time_mus, mux->frm_publish_period_mus );
				fscWriteFrame( &(mux->fl[2]) );
//				fprintf( stderr, "Time: %lld ms\n", (av_gettime()-tmus)/1000 );
			}		
			if( i<2 )	break;
			clk_multiple = 1;
		}
	
		fscClose( &(mux->fl[0]) );
		fscClose( &(mux->fl[1]) );
		fscClose( &(mux->fl[2]) );
		
		if( mux->loop_flag==1 ) mux->loop_flag++;
	} while( mux->loop_flag>1 && !ctrl_c_hit );
	
	NVPPublishFrame( name, 1, 0 );
	sleep( 1 );
	NVPDestroy();
	printf( "Server exiting....\n" );
	return 1;	
}

