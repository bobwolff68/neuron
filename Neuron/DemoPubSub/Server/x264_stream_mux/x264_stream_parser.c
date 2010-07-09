/* x264 stream parser definition file.
   Author: Manjesh Malavalli
   Date:   01/23/2010
*/

#include <string.h>
#include "x264_stream_parser.h"

int fscInit( fscPtr f, char *x264_stream_filename )
{
	int i;
	
	if( (f->x264_stream = fopen( x264_stream_filename, "r" )) == NULL )
	{
		fprintf( stderr, "x264_stream_parser[error]: Trouble opening x264 stream %s.\n", x264_stream_filename );
		return 0;
	}
	else
	{
		f->streamPtr = 0;
		f->nStreamChunks = 1;
		if( (f->streamBuf = (uchar_t *) malloc( sizeof(uchar_t)*STREAM_CHUNK_SIZE )) == NULL )
		{
			fprintf( stderr, "x264_stream_parser[error]: Unable to allocate memory for stream buffer.\n" );
			fclose( f->x264_stream );
			return 0;
		}
		
		for( i=0; i<VIDEO_HDR_TYPES; i++ )
			f->vh.stream[i] = NULL;
	}
	
	return 1;
}

int fscExtractVideoHeader( fscPtr f, int hdrStreamIndex, uint_t verbose )
{
	size_t	bytesRead;
	uint_t	nalStartCodePrefix = 0;
	int		nalRefIdc = 0;
	int		nalType = 0;
	//int 	i;
	
	// Reset frame buffer position and size.
	fscReset( f );
	// Extract the next 5 bytes from the stream.
	// Bytes 1..4 ==> NAL start code prefix.
	// Byte 5	  ==> NAL header 0 xx yyyyy ( xx - 2 bit nal_ref_idc, yyyyy - 5 bit nal_type ).	
	if( !fscRead( f, 5, &bytesRead ) )
		return 0;
		
	nalStartCodePrefix = (uint_t) f->streamBuf[f->streamPtr-5];
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[f->streamPtr-4]);
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[f->streamPtr-3]);
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[f->streamPtr-2]);
	
	nalRefIdc = ((uint_t) f->streamBuf[f->streamPtr-1]) & 0x60;
	nalRefIdc >>= 5;
	
	nalType = ((uint_t) f->streamBuf[f->streamPtr-1]) & 0x1f;
	f->type = X264_TYPE_HDR;
	
	if( verbose )
	{
		// Print results.
		fprintf( stdout, "x264_stream_parser[info]: NAL start prefix: %x\t", nalStartCodePrefix );
		fprintf( stdout, "NAL ref idc: %d\t", nalRefIdc );	
		fprintf( stdout, "NAL type: %d\t", nalType );
		fprintf( stdout, "Video Header\t" );
	}
	
	// Keep on writing until dectection of next start code prefix.
	nalStartCodePrefix = 0xffffffff;
	while( !feof( f->x264_stream ) )
	{
		if( nalStartCodePrefix == 0x00000001 )
		{
			if( fseek( f->x264_stream, -4, SEEK_CUR ) )
			{
				fprintf( stderr, "x264_stream_parser[error]: Seek error.\n" );
				return 0;
			}
			f->streamPtr -= 4;
			break;
		}
		else
		{
			if( !fscRead( f, 1, &bytesRead ) )
				return 0;
			if( !bytesRead )	break;
		}		
		nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[f->streamPtr-1]);
	}
	
	if( verbose )
	{
		// streamPtr is the size of the encoded frame.
		fprintf( stdout, "Size: %d bytes\n", f->streamPtr );
	}
	
	// Store stream in the appropriate video header stream.
	f->vh.size[hdrStreamIndex] = f->streamPtr;
	f->vh.stream[hdrStreamIndex] = (uchar_t *) malloc( sizeof(uchar_t)*(f->streamPtr) );
	if( f->vh.stream[hdrStreamIndex]==NULL )
	{
		fprintf( stderr, "x264_stream_parser[error]: Unable to allocate memory for stream buffer.\n" );
		fclose( f->x264_stream );
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
int fscExtractFrame( fscPtr f, uint_t verbose )
{
	size_t	bytesRead;
	uint_t	nalStartCodePrefix = 0;
	int		nalRefIdc = 0;
	int		nalType = 0;
	char	frameTypeArray[5] = { 'i', 'I', 'P', 'B', 'b' };
	
	// Reset frame buffer position and size.
	fscReset( f );
	
	if( feof( f->x264_stream ) )
		return 1;
	// Extract the next 6 bytes from the stream.
	// Bytes 1..4 ==> NAL start code prefix.
	// Byte 5	  ==> NAL header 0 xx yyyyy ( xx - 2 bit nal_ref_idc, yyyyy - 5 bit nal_type ).
	// Byte 6	  ==> aaa bbb 00 ( aaa - 3 bit slice type, bbb - 3 bit frame type ).
	if( !fscRead( f, 6, &bytesRead ) )
		return 0;
	
	nalStartCodePrefix = (uint_t) f->streamBuf[f->streamPtr-6];
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[f->streamPtr-5]);
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[f->streamPtr-4]);
	nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[f->streamPtr-3]);		
	
	nalRefIdc = ((uint_t) f->streamBuf[f->streamPtr-2]) & 0x60;
	nalRefIdc >>= 5;
	
	nalType = ((uint_t) f->streamBuf[f->streamPtr-2]) & 0x1f;
	
	f->type = ((uint_t) f->streamBuf[f->streamPtr-1]) & 0x1c;
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
	}		
	
	// Keep on writing until dectection of next NAL AUD or end of file.
	nalStartCodePrefix = 0xffffffff;
	while( !feof( f->x264_stream ) )
	{
		if( nalStartCodePrefix == 0x00000001 )
		{
			// Check the nal_type field in the next byte.
			if( !fscRead( f, 1, &bytesRead ) )
				return 0;
				
			nalType = ((int) f->streamBuf[f->streamPtr-1]) & 0x1f;
			// If next NAL is an NAL AUD, walk back the stream buffer
			// and the x264 stream pointer to the start of the NAL.
			if( nalType==NAL_TYPE_AUD )
			{
				if( fseek( f->x264_stream, -5, SEEK_CUR ) )
				{
					fprintf( stderr, "x264_stream_parser[error]: Seek error.\n" );
					return 0;
				}
				f->streamPtr -= 5;
				break;
			}
		}
		else
		{
			if( !fscRead( f, 1, &bytesRead ) )
				return 0;
			if( !bytesRead )	break;
		}		
		nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) f->streamBuf[f->streamPtr-1]);
	}
	
	if( verbose )
	{
		// streamPtr is the size of the encoded frame.
		fprintf( stdout, "Size: %d bytes\n", f->streamPtr );
	}
	
	return 1;
}

int	fscRead( fscPtr f, size_t nEle, size_t *bytesRead )
{
	int streamBufSize = f->nStreamChunks * STREAM_CHUNK_SIZE;
	int bytesToRead = sizeof(uchar_t) * nEle;
	
	// If buffer overflows, increase its size.
	if( f->streamPtr > (streamBufSize-bytesToRead) )
	{
		f->nStreamChunks++;
		f->streamBuf = (uchar_t *) realloc( f->streamBuf, sizeof(uchar_t)*(streamBufSize+STREAM_CHUNK_SIZE) );		
		if( f->streamBuf==NULL )
		{
			fprintf( stderr, "x264_stream_parser[error]: Realloc error.\n" );
			return 0;
		}
	}
	
	*bytesRead = fread( &(f->streamBuf[f->streamPtr]), sizeof(uchar_t), nEle, f->x264_stream );
	f->streamPtr += (*bytesRead);
		
	if( (*bytesRead)<bytesToRead )
	{
		if( ferror(f->x264_stream) )
		{
			fprintf( stderr, "x264_stream_parser[error]: FRead error.\n" );
			return 0;
		}
	} 
	
	return 1;
}

int	fscWriteFrame( fscPtr f, NeuronDP *pNdp )
{
	long layer_type = (long) LAYER_TYPE( f->type );
	long size = (long) f->streamPtr;
	
	NeuronPub_write_frame( pNdp, f->streamBuf, size, layer_type );
	return f->type;
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
	if( fclose(f->x264_stream) )
	{
		fprintf( stderr, "x264_stream_parser[error]: FClose error.\n" );
		return 0;
	}
	
	return 1;
}
