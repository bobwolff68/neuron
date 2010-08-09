#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "x264_stream_layer_mux.h"

// RTI item.
//#define DDS_long DDS_Long

int main( int argc, char *argv[] )
{
	//char 			app_signal;
	int				mux_id;
	x264_slmux_ptr  pMuxX264 = (x264_slmux_ptr) malloc( sizeof(x264_stream_layer_mux) );
	 
	if (argc < 3)
	{
	  printf("Usage: %s <videoname-without-Lx> <publisher-name> [L]\n", argv[0]);
	  exit(-1);
	}

	pMuxX264->x264mux_input_fn_prefix = argv[1];
	//pMuxX264->sink_id_str = argv[2];
	//sscanf( argv[3], "%d", &mux_id );
	//pMuxX264->ndp.dp_id = (DDS_long) mux_id;
	
	if( argc>=4 )	pMuxX264->loop_flag = 1;
	else			pMuxX264->loop_flag = 0;
	
	//NeuronDP_create_dp_factory( &(pMuxX264->ndp) );
	x264_stream_layer_mux_thread_run( (void *) pMuxX264, argv[2] );
	
	free( pMuxX264 );
	
	return 0;
}

