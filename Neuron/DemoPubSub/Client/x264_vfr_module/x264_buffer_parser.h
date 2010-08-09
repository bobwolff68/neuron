/* x264 stream parser header file.
   Author: Manjesh Malavalli
   Date:   01/23/2010
*/

#include <stdio.h>
#include <stdlib.h>
//#include "../../DDS/NeuronDP.h"
#include "../../DDS_RTI/videoPubSubAPI.h"

#define STREAM_CHUNK_SIZE 8192

#define X264_TYPE_HDR		0x0000
#define X264_TYPE_IDR		0x0001
#define X264_TYPE_I     	0x0002
#define X264_TYPE_P     	0x0003
#define X264_TYPE_BREF		0x0004
#define X264_TYPE_B     	0x0005

#define NAL_TYPE_USER		0x0000
#define NAL_TYPE_SEI		0x0006
#define NAL_TYPE_SPS		0x0007
#define NAL_TYPE_PPS		0x0008
#define NAL_TYPE_AUD		0x0009
#define NAL_AUD_REF_IDC 	0x0000
#define NAL_VHDR_REF_IDC	0x0003
#define NAL_AUD_SIZE		0x0006

#define VIDEO_HDR_TYPES		0x0005
#define SEI_STREAM_INDEX	0x0000
#define	SPSF_STREAM_INDEX	0x0001
#define	PPS_STREAM_INDEX	0x0002
#define	SPSH_STREAM_INDEX	0x0003
#define	SPSQ_STREAM_INDEX	0x0004

#define SPS_HDR_IDX( fps_choice ) ((fps_choice)=='f' ? SPSF_STREAM_INDEX : ((fps_choice)=='h' ?\
								    SPSH_STREAM_INDEX : SPSQ_STREAM_INDEX))

typedef unsigned char uchar_t;
typedef unsigned int  uint_t;

typedef struct
{
	int		size[VIDEO_HDR_TYPES];
	uchar_t *stream[VIDEO_HDR_TYPES];
} vHdr;

typedef struct
{
	int 	type;
	int 	streamPtr;
	int		streamBufSize;
	uchar_t *streamBuf;
	vHdr	vh;
} frameStreamContainer;

typedef frameStreamContainer *fscPtr;

int		fscInit( fscPtr );
int		fscParseVideoHeader( fscPtr, int, uint_t, char );//,  NeuronDP *, const char *, char [][10] );
int		fscParseFrame( fscPtr, uint_t, char );//, NeuronDP *, const char *, char [][10] );
int		fscReadFrame( fscPtr, char );//, NeuronDP *, const char *, char [][10] );
int 	fscWriteVideoHeader( fscPtr, int, int );
int		fscWriteFrame( fscPtr, int );
void	fscReset( fscPtr );
int		fscClose( fscPtr );

