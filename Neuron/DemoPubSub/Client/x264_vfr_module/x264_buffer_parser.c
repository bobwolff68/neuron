/* x264 stream parser definition file.
   Author: Manjesh Malavalli
   Date:   01/23/2010
*/

#include <string.h>
#include <unistd.h>
#include "x264_buffer_parser.h"

int fscInit( fscPtr f )
{
	int i;
	
	f->streamPtr = 0;
	f->streamBufSize = 0;
	f->streamBuf = NULL;
		
	for( i=0; i<VIDEO_HDR_TYPES; i++ )
		f->vh.stream[i] = NULL;

	return 1;
}

int fscParseVideoHeader( fscPtr f, int hdrStreamIndex, uint_t verbose, NeuronDP *pNdp,
						 const char *query, char query_param_list[][10] )
{
	size_t	bytesRead;
	uint_t	nalStartCodePrefix = 0;
	int		nalRefIdc = 0;
	int		nalType = 0;

	if( !fscReadFrame( f, pNdp, query, query_param_list ) )
		return 0;
	// Bytes 1..4 ==> NAL start code prefix.
	// Byte 5	  ==> NAL header 0 xx yyyyy ( xx - 2 bit nal_ref_idc, yyyyy - 5 bit nal_type ).	
		
	nalStartCodePrefix = (uint_t) f->streamBuf[0];
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[1]);
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[2]);
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[3]);
	
	nalRefIdc = ((uint_t) f->streamBuf[4]) & 0x60;
	nalRefIdc >>= 5;
	
	nalType = ((uint_t) f->streamBuf[4]) & 0x1f;
	f->type = X264_TYPE_HDR;
	
	if( verbose )
	{
		// Print results.
		fprintf( stdout, "x264_stream_parser[info]: NAL start prefix: %x\t", nalStartCodePrefix );
		fprintf( stdout, "NAL ref idc: %d\t", nalRefIdc );	
		fprintf( stdout, "NAL type: %d\t", nalType );
		fprintf( stdout, "Video Header\t" );
		fprintf( stdout, "Size: %d bytes\n", f->streamPtr );
	}
	
	// Store stream in the appropriate video header stream.
	f->vh.size[hdrStreamIndex] = f->streamPtr;
	f->vh.stream[hdrStreamIndex] = (uchar_t *) malloc( sizeof(uchar_t)*(f->streamPtr) );
	if( f->vh.stream[hdrStreamIndex]==NULL )
	{
		fprintf( stderr, "x264_stream_parser[error]: Unable to allocate memory for stream buffer.\n" );
		//fclose( f->x264_stream );
		return 0;
	}
	f->vh.stream[hdrStreamIndex] = (uchar_t *) memcpy( f->vh.stream[hdrStreamIndex], f->streamBuf, f->streamPtr );	
	// Since SPS headers for half and quarter frame rates are stored in user data NALs,
	// convert these NALs to SPS NALs for later insertion into the stream.
	if( hdrStreamIndex==SPSH_STREAM_INDEX || hdrStreamIndex==SPSQ_STREAM_INDEX )
		f->vh.stream[hdrStreamIndex][4] = (uchar_t) ( (0x00<<7) | (NAL_VHDR_REF_IDC<<5) | NAL_TYPE_SPS );

	return 1;		
}

// Note: x264_stream always points to the start of an NAL AUD.
int fscParseFrame( fscPtr f, uint_t verbose, NeuronDP *pNdp,
				   const char *query, char query_param_list[][10] )
{
	size_t	bytesRead;
	uint_t	nalStartCodePrefix = 0;
	int		nalRefIdc = 0;
	int		nalType = 0;
	char	frameTypeArray[5] = { 'i', 'I', 'P', 'B', 'b' };
	
	if( !fscReadFrame( f, pNdp, query, query_param_list ) )
		return 0;	
	// Bytes 1..4 ==> NAL start code prefix.
	// Byte 5	  ==> NAL header 0 xx yyyyy ( xx - 2 bit nal_ref_idc, yyyyy - 5 bit nal_type ).
	// Byte 6	  ==> aaa bbb 00 ( aaa - 3 bit slice type, bbb - 3 bit frame type ).
	
	nalStartCodePrefix = (uint_t) f->streamBuf[0];
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[1]);
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[2]);
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[3]);		
	
	nalRefIdc = ((uint_t) f->streamBuf[4]) & 0x60;
	nalRefIdc >>= 5;
	
	nalType = ((uint_t) f->streamBuf[4]) & 0x1f;
	
	f->type = ((uint_t) f->streamBuf[5]) & 0x1c;
	f->type >>= 2;
	
	if( nalType != NAL_TYPE_AUD )
		f->type = X264_TYPE_HDR;
	
	if( verbose )
	{
		// Print results.
		fprintf( stdout, "x264_stream_parser[info]: NAL start prefix: %x\t", nalStartCodePrefix );
		fprintf( stdout, "NAL ref idc: %d\t", nalRefIdc );	
		fprintf( stdout, "NAL type: %d\t", nalType );
		if( f->type != X264_TYPE_HDR )
			fprintf( stdout, "Frame type: %c\t", frameTypeArray[f->type-1] );
		else
			fprintf( stdout, "Video Header Info\t" );
		// streamPtr is the size of the encoded frame.
		fprintf( stdout, "Size: %d bytes\n", f->streamPtr );
	}
	
	return 1;
}

int	fscReadFrame( fscPtr f, NeuronDP *pNdp, const char *query, char query_param_list[][10] )
{
	int	layer_type;

	fscReset( f );
	NeuronSub_read_frame( pNdp, &(f->streamBuf), &(f->streamBufSize), &(f->streamPtr), &layer_type,
						  query, query_param_list );
	return f->streamPtr;
}

int fscWriteVideoHeader( fscPtr f, int hdrStreamIndex, int ofd )
{
	int bytesWritten;

	if( (bytesWritten=write( ofd, f->vh.stream[hdrStreamIndex], f->vh.size[hdrStreamIndex] ))<
		(f->vh.size[hdrStreamIndex]) )
	{
		fprintf( stderr, "Buffer Parser WriteFrame() error\n" );
	}
	
	return bytesWritten;
}

int	fscWriteFrame( fscPtr f, int ofd )
{
	int bytesWritten;

	if( (bytesWritten=write( ofd, f->streamBuf, f->streamPtr ))<(f->streamPtr) )
	{
		fprintf( stderr, "Buffer Parser WriteFrame() error" );
	}
	
	return bytesWritten;
}

void fscReset( fscPtr f )
{
	f->streamPtr = 0;
	f->type = 0;
	return;
}

int	fscClose( fscPtr f )
{
	int i;
	
	for( i=0; i<VIDEO_HDR_TYPES; i++ )
		if( f->vh.stream[i]!=NULL )	
			free( f->vh.stream[i] );
		
	free( f->streamBuf );

	return 1;
}
