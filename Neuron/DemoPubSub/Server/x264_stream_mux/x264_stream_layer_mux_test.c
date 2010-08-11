#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "x264_stream_layer_mux.h"

double get_vid_stats_str( const char *vid_stats_fn, char *vid_stats_str )
{
	int		vid_width;
	int		vid_height;
	double	vid_fps;
	FILE	*p_file = NULL;
	
	p_file = fopen( vid_stats_fn, "r" );
	fscanf( p_file, "%d,%d,%lf", &vid_width, &vid_height, &vid_fps );
	sprintf( vid_stats_str, "%d,%d,%.2lf", vid_width, vid_height, vid_fps );
	fclose( p_file );
	
	return vid_fps;
}

int main( int argc, char *argv[] )
{
	int				mux_id;
	double			fps;
	char			vid_stats_fn[100];
	char			vid_stats_str[100];
	x264_slmux_ptr  pMuxX264 = (x264_slmux_ptr) malloc( sizeof(x264_stream_layer_mux) );
	 
	if (argc < 3)
	{
	  printf("Usage: %s <videoname-without-Lx> <publisher-name> [<fps>] [L]\n", argv[0]);
	  exit(-1);
	}
	
	pMuxX264->x264mux_input_fn_prefix = argv[1];
	strcpy(vid_stats_fn,argv[1]);
	strcat(vid_stats_fn,"ST");
	
	if( access( vid_stats_fn, F_OK )<0 )
	{
		printf( "Video stats file '%s' does not exist...\n", vid_stats_fn );
		exit( -1 );
	}
	
	fps = get_vid_stats_str( vid_stats_fn, vid_stats_str );
	if( !isalpha(argv[3][0]) )
	{
		sscanf( argv[3], "%lf", &fps );
		if( argc>=5 )	pMuxX264->loop_flag = 1;
		else			pMuxX264->loop_flag = 0;
	}
	else
	{
		if( argc>=4 )	pMuxX264->loop_flag = 1;
		else			pMuxX264->loop_flag = 0;		
	}
	
	x264_stream_layer_mux_thread_run( (void *) pMuxX264, argv[2], vid_stats_str, fps );
	
	free( pMuxX264 );
	return 0;
}

