//-------------------------------------------------------------------------------------------------
// File:   x264_vfr_module.c
// Author: Manjesh Malavalli
// Date:   03/04/2010
//-------------------------------------------------------------------------------------------------
#include <sys/types.h>
#include <sys/stat.h>
#include <SDL/SDL_thread.h>
#include <libavformat/avformat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "x264_vfr_module.h"

#define PROC_STAT_QUERY		"sed -n '1p' /proc/stat |  tr -s \" \" | cut -d\" \" -f 2-5"

//------------------------------GOPQ FUNCTIONS-----------------------------------------------------
int GOPQ_init( GOPQ *pGopq )
{
	pGopq->gop_len = 0;
	pGopq->gop_buf_size = 0;
	pGopq->gop_buf_ptr = 0;
	pGopq->is_pframe_present = 0;
	pGopq->sps_stream_index = 0;
	pGopq->gop_buf = NULL;
	return 1;
}

int GOPQ_reset( GOPQ *pGopq )
{
	pGopq->gop_len = 0;
	pGopq->gop_buf_ptr = 0;
	pGopq->is_pframe_present = 0;
	return 1;
}

int GOPQ_add_frame( GOPQ *pGopq, fscPtr f )
{
	// If empty frame, increment gop_len without adding it to gop
	if( f->streamPtr>NAL_AUD_SIZE )
	{
		// Extend buffer memory if necessary
		if( (pGopq->gop_buf_size-pGopq->gop_buf_ptr)<(f->streamPtr) )
		{
			pGopq->gop_buf_size += f->streamPtr;
			pGopq->gop_buf = (uchar_t *) realloc( pGopq->gop_buf, pGopq->gop_buf_size );
			if( pGopq->gop_buf==NULL )
			{
				fprintf( stderr, "x264 vfr module realloc() error\n" );
				return 0;
			}
		}
		// Add frame to gopq buffer
		memcpy( pGopq->gop_buf+pGopq->gop_buf_ptr, f->streamBuf, f->streamPtr );
		pGopq->gop_buf_ptr += f->streamPtr;
		pGopq->is_pframe_present |= (int) (f->type==X264_TYPE_P);
	}
	pGopq->gop_len++;
	return 1;
}

int GOPQ_flush_to_fifo( GOPQ *pGopq, int ofd )
{
	int bytes_written = 0;
	if( pGopq->gop_len>0 )
	{
		bytes_written = write( ofd, pGopq->gop_buf, pGopq->gop_buf_ptr );
		if( bytes_written<pGopq->gop_buf_ptr )
			fprintf( stderr, "x264 vfr module write error\n" );
		GOPQ_reset( pGopq );
	}
	return bytes_written;
}
//------------------------------VFRM FUNCTIONS-----------------------------------------------------
int VFRM_init( VFRModule *pVfm, char *name )
{	
	bandwidth = (double *) av_mallocz( sizeof(double) );
	cpu_usage = (double *) av_mallocz( sizeof(double) );
	cpu_time = (long long *) av_mallocz( sizeof(long long)*2 );
	proc_time = (long long *) av_mallocz( sizeof(long long)*2 );
	
	*bandwidth = 0.0;
	*cpu_usage = 0.0;
	cpu_time[0] = cpu_time[1] = 0;
	proc_time[0] = proc_time[1] = 0;

	pVfm->quit_flag = 0;
	pVfm->video_src_id_str = (char *) av_mallocz( sizeof(char)*256 );
	strcpy( pVfm->video_src_id_str, "-1" );
	GOPQ_init( &(pVfm->gopq) );
	
	if( !fscInit( &(pVfm->fsc) ) )
		return 0;
	
	NVSStartup( name );
	pVfm->fps_choice = 'f';
	pVfm->new_fps_choice = 'f';

	time_mus = av_gettime();
	mutex = SDL_CreateMutex();
	cond = SDL_CreateCond();
	return 1;
}

int VFRM_thread_run( void *userdata )
{
	int	 		i;
	int			bytes_read;
	int			sps_idx;
	int			pps_sent = 0;
	int			eos = 0;
	double		total_bytes_read = 0.0;
	char		prog_bar_text[50];
	int64_t		new_time_mus;
	VFRModule	*pVfm = (VFRModule *) userdata;
	pid_t		proc_id = getpid();

	NVSSetupFrameListener();
	pVfm->opfd = open( pVfm->vfrm_output_fn, O_WRONLY );
	// Read header info first
	for( i=0; i<VIDEO_HDR_TYPES; i++ )
	{
		if( !fscParseVideoHeader( &(pVfm->fsc), i, 0 ) )
			return 0;
		total_bytes_read += ((double) pVfm->fsc.streamPtr);
	}
	// Send sei
	if( fscWriteVideoHeader( &(pVfm->fsc), SEI_STREAM_INDEX, pVfm->opfd )<0 )
	{
		perror( "x264 vfr module write_vid_hdr()" );
		return 0;
	}
	// Read first frame
	if( !fscParseFrame( &(pVfm->fsc), 0 ) )	return 0;
	
	while( !eos && !pVfm->quit_flag )
	{
		// Read GOP
		while( !pVfm->gopq.is_pframe_present || (pVfm->fsc.type!=X264_TYPE_IDR && 
			   pVfm->fsc.type!=X264_TYPE_I && pVfm->fsc.type!=X264_TYPE_P)
			 )
		{
			total_bytes_read += ((double) pVfm->fsc.streamPtr);
			GOPQ_add_frame( &(pVfm->gopq), &(pVfm->fsc) );
			if( pVfm->fsc.type==X264_TYPE_IDR || pVfm->fsc.type==X264_TYPE_I )
			{
				if( !fscParseFrame( &(pVfm->fsc), 0 ) )
				{
					eos = 1;
					break;
				}
				total_bytes_read += ((double) pVfm->fsc.streamPtr);
				GOPQ_add_frame( &(pVfm->gopq), &(pVfm->fsc) );
			}
			if( !fscParseFrame( &(pVfm->fsc), 0 ) )
			{
				eos = 1;
				break;
			}
		}
		// If sps[sps_idx] is different from previously transmitted sps,
		// frame rate has changed, so write new sps
		sps_idx = SPS_STREAM_IDX( pVfm->gopq.gop_len );
		sps_idx = SPS_STREAM_IDX_MODIFY( sps_idx );
		if( pVfm->gopq.sps_stream_index!=sps_idx )
		{
			pVfm->gopq.sps_stream_index = sps_idx;
			if( fscWriteVideoHeader( &(pVfm->fsc), sps_idx, pVfm->opfd )<0 )
			{
				perror( "x264 vfr module write_vid_hdr()" );
				return 0;
			}
		}
		// If PPS not sent, send
		if( !pps_sent )
		{
			if( fscWriteVideoHeader( &(pVfm->fsc), PPS_STREAM_INDEX, pVfm->opfd )<0 )
			{
				perror( "x264 vfr module write_vid_hdr()" );
				return 0;
			}
			pps_sent = 1;
		} 
		// Flush gop
		if( GOPQ_flush_to_fifo( &(pVfm->gopq), pVfm->opfd )<0 )
		{
			perror( "x264 vfr module write_vid_frm()" );
			eos = 1;
		}
		
		if( pVfm->fps_choice != pVfm->new_fps_choice )
		{
			NVSChangeVDSPartition( pVfm->new_fps_choice );
			pVfm->fps_choice = pVfm->new_fps_choice;
		}

		new_time_mus = av_gettime();
		if( (new_time_mus-time_mus)>=STATS_CALC_CLK_PERIOD_MUS )
		{
			SDL_CondWait( cond, mutex );
			SDL_LockMutex( mutex );
			// Bandwidth in Kb/s
			*bandwidth = total_bytes_read*(8e-3) / (((double)(new_time_mus-time_mus))*(1e-6));
			VFRM_get_cpu_usage( proc_id );
			
			SDL_CondSignal( cond );
			SDL_UnlockMutex( mutex );	
			total_bytes_read = 0.0;
			time_mus = new_time_mus;
		}
	}

	pVfm->quit_flag = 1;
	close( pVfm->opfd );
	fprintf( stdout, "VFRM finished\n" );
	return 1;
}

void VFRM_get_cpu_usage( pid_t proc_id )
{
	char		proc_pid_stat_query[100];
	char		buf[100];
	FILE		*queryReply;
	long long	val, val1, val2, val3;

	sprintf( proc_pid_stat_query, "cat /proc/%d/stat | tr -s \" \" | cut -d\" \" -f 14-15", 
			 proc_id );
	cpu_time[0] = cpu_time[1];
	queryReply = popen( PROC_STAT_QUERY, "r" );

	if( queryReply!=NULL )
	{
		fscanf( queryReply, "%lld%lld%lld%lld", &val, &val1, &val2, &val3 );
		cpu_time[1] = val + val1 + val2 + val3;
		//Flush out rest of the output of command before closing the pipe
		while( fgets( buf, 100, queryReply)!=NULL );
	}
	else
	{
		perror( "popen(1)" );
		return;
	}
	
	pclose( queryReply );
	proc_time[0] = proc_time[1];
	queryReply = popen( proc_pid_stat_query, "r" );

	if( queryReply !=NULL )
	{
		fscanf( queryReply, "%lld%lld", &val, &val1 );
		proc_time[1] = val + val1;
		//Flush out rest of the output of command before closing the pipe
		while( fgets( buf, 100, queryReply)!=NULL );
	}
	else
	{
		perror( "popen(2)" );
		return;
	}

	pclose( queryReply );
	*cpu_usage = 100.0*((double) (proc_time[1]-proc_time[0]))/((double) (cpu_time[1]-cpu_time[0]));
	return;
}

void VFRM_destroy( VFRModule *pVfm )
{
	free( pVfm->gopq.gop_buf );
	free( pVfm->video_src_id_str );
	free( bandwidth );
	free( cpu_usage );
	free( cpu_time );
	free( proc_time );
	fscClose( &(pVfm->fsc) );
	NVSDestroy();

	return;	
}

