#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include "x264_stream_layer_mux.h"

int	clk_multiple = 1;
int ctrl_c_hit = 0;

int64_t av_gettime(void)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

int nanosleep_ttw( int64_t time_mus, int *throttle_mode )
{
	struct timespec	time_to_wait;
	int64_t			ttw_mus;
	
	if( *throttle_mode!=H264MUX_PAUSE_SIGNAL )
	{
		ttw_mus = av_gettime()-time_mus;
		ttw_mus = clk_multiple*MUX_FRM_OP_CLK_PERIOD_MUS(*throttle_mode) - ttw_mus;
		ttw_mus = ttw_mus*((int64_t) (ttw_mus>0));
		time_to_wait.tv_sec = (time_t) ttw_mus/1000000;
		time_to_wait.tv_nsec = (long) (ttw_mus%1000000)*1000;
		nanosleep( &time_to_wait, NULL ); 
	}
	else
	{
		while( *throttle_mode==H264MUX_PAUSE_SIGNAL )
			usleep(1000);
	}
	
	return 1;
}

int x264_stream_layer_mux_init( x264_slmux_ptr mux, char *name )
{
	char	*x264_layer_fn[3];
	char 	*x264_layer_suffix[3] = { "L0","L1","L2" };
	int		i;
	
	// Initialize mux
	if( mux->loop_flag==0 || mux->loop_flag==1 )
	{
		mux->throttle_mode = H264MUX_PAUSE_SIGNAL;
		strcpy( mux->sink_id_str, "-1" );
		NeuronDP_setup( &(mux->ndp), PUB_CHOICE, name );
	}
	
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

int x264_stream_layer_mux_thread_run( void *userdata, char *name )
{
	int	 	i;
	int64_t	time_mus;
	
	x264_slmux_ptr			mux = (x264_slmux_ptr) userdata;
	ThrotMsgListenerData	ldata = { 
									  &(mux->ndp), &(mux->throttle_mode), 
									  mux->sink_id_str, THROT_MSG_TAKE_QUERY 
									};
	(void) signal( SIGINT, sig_handle_ctrl_c );
	printf( "Neuron Concept server started (%s)....\n", (mux->loop_flag==0) ? "one time playback"
														: "loop playback" );

	do
	{
		x264_stream_layer_mux_init( mux, name );
		if( mux->loop_flag==0 || mux->loop_flag==1 )
			NeuronPub_setup_throt_msg_listener( &(mux->ndp), &ldata );
		
		// Send header info first
		for( i=0; i<VIDEO_HDR_TYPES; i++ )
		{
			time_mus = av_gettime();
			if( !fscExtractVideoHeader( &(mux->fl[0]), i, 0 ) )
				return 0;
			
			if( mux->loop_flag==0 || mux->loop_flag==1 )
			{
				nanosleep_ttw( time_mus, &(mux->throttle_mode) );
				fscWriteFrame( &(mux->fl[0]), &(mux->ndp) );
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
			if( !fscExtractFrame( &(mux->fl[0]), 0 ) )	break;
			if( mux->throttle_mode == H264MUX_KILL_SIGNAL ) break;
			nanosleep_ttw( time_mus, &(mux->throttle_mode) );
			fscWriteFrame( &(mux->fl[0]), &(mux->ndp) );
			// If IDR/I frame, parse and write next P frame.
			if( mux->fl[0].type==X264_TYPE_IDR || mux->fl[0].type==X264_TYPE_I )
			{
				clk_multiple++;
				if( !fscExtractFrame( &(mux->fl[0]), 0 ) )	break;
				if( mux->throttle_mode == H264MUX_KILL_SIGNAL ) break;
				nanosleep_ttw( time_mus, &(mux->throttle_mode) );
				fscWriteFrame( &(mux->fl[0]), &(mux->ndp) );
			}
			// Extract the next B frame and write if fps_op > 0.25*fps_orig
			if( !fscExtractFrame( &(mux->fl[1]), 0 ) )	break;
			if( mux->throttle_mode == H264MUX_KILL_SIGNAL ) break;
			clk_multiple++;
			nanosleep_ttw( time_mus, &(mux->throttle_mode) );
			fscWriteFrame( &(mux->fl[1]), &(mux->ndp) );
			// Extract the next 2 b frames and write if fps_op == fps_orig
			for( i=0; i<2; i++ )
			{
				if( !fscExtractFrame( &(mux->fl[2]), 0 ) )	break;
				if( mux->throttle_mode == H264MUX_KILL_SIGNAL ) break;
				clk_multiple++;
				nanosleep_ttw( time_mus, &(mux->throttle_mode) );
				fscWriteFrame( &(mux->fl[2]), &(mux->ndp) );
			}		
			if( i<2 )	break;
			clk_multiple = 1;
		}
	
		fscClose( &(mux->fl[0]) );
		fscClose( &(mux->fl[1]) );
		fscClose( &(mux->fl[2]) );
		
		if( mux->loop_flag==1 ) mux->loop_flag++;
	} while( mux->loop_flag>1 && mux->throttle_mode != H264MUX_KILL_SIGNAL && !ctrl_c_hit );
	
	NeuronPub_write_frame( &(mux->ndp), (char *) mux->sink_id_str, (long) 1, (long) 0 );
	sleep( 1 );
	NeuronDP_destroy( &(mux->ndp), PUB_CHOICE );
	printf( "Server exiting....\n" );
	return 1;	
}

