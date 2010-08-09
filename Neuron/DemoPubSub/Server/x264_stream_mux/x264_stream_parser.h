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

#define LAYER_TYPE( frm_type )	(((frm_type)>>2)*(1+((frm_type)&1)))

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
	int		nStreamChunks;
	uchar_t *streamBuf;
	FILE	*x264_stream;
	vHdr	vh;
} frameStreamContainer;

typedef frameStreamContainer *fscPtr;

int		fscInit( fscPtr, char * );
int		fscExtractVideoHeader( fscPtr, int, uint_t );
int		fscExtractFrame( fscPtr, uint_t );
int		fscRead( fscPtr, size_t, size_t * );
//int		fscWriteFrame( fscPtr, NeuronDP * );
int		fscWriteFrame( fscPtr );
void	fscReset( fscPtr );
int		fscClose( fscPtr );

