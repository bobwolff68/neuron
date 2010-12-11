#ifndef H264BUFFERPARSER_H_
#define H264BUFFERPARSER_H_

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

typedef unsigned char uchar_t;
typedef unsigned int  uint_t;

class H264BufferParser
{
    class H264Header
    {
        public:

            int         size[VIDEO_HDR_TYPES];
            uchar_t    *streamBuf[VIDEO_HDR_TYPES];
    };

    public:

        int 	    type;
        H264Header  vh;

        H264BufferParser()
        {
            type = -1;

            for(int i=0; i<VIDEO_HDR_TYPES; i++)
                vh.streamBuf[i] = NULL;
        }

        ~H264BufferParser()
        {
           for(int i=0; i<VIDEO_HDR_TYPES; i++)
                if(vh.streamBuf[i]!=NULL)
                    free(vh.streamBuf[i]);
        }

        bool ParseBuffer(unsigned char *streamBuf,int streamSize,int verbose)
        {
           	size_t	bytesRead;
            uint_t	nalStartCodePrefix = 0;
            int		nalRefIdc = 0;
            int		nalType = 0;
            char	frameTypeArray[5] = { 'i', 'I', 'P', 'B', 'b' };

            // Extract the next 5 bytes from the stream.
            // Bytes 1..4 ==> NAL start code prefix.
            // Byte 5	  ==> NAL header 0 xx yyyyy ( xx - 2 bit nal_ref_idc, yyyyy - 5 bit nal_type ).
            nalStartCodePrefix = (uint_t) streamBuf[0];
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[1]);
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[2]);
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[3]);

            nalRefIdc = ((uint_t) streamBuf[4]) & 0x60;
            nalRefIdc >>= 5;

            nalType = ((uint_t) streamBuf[4]) & 0x1f;

            if(nalType != NAL_TYPE_AUD)
            {
                int hdrStreamIdx;

                type = X264_TYPE_HDR;
                switch(nalType)
                {
                    case NAL_TYPE_SEI:

                        hdrStreamIdx = SEI_STREAM_INDEX;
                        break;

                    case NAL_TYPE_SPS:

                        if(vh.streamBuf[SPSF_STREAM_INDEX]==NULL)
                            hdrStreamIdx = SPSF_STREAM_INDEX;
                        else
                        {
                            for(hdrStreamIdx=SPSH_STREAM_INDEX;
                                hdrStreamIdx<VIDEO_HDR_TYPES && vh.streamBuf[hdrStreamIdx]!=NULL;
                                hdrStreamIdx++);
                        }
                        break;

                    case NAL_TYPE_PPS:

                        hdrStreamIdx = PPS_STREAM_INDEX;
                        break;
                }

                if(hdrStreamIdx<VIDEO_HDR_TYPES && vh.streamBuf[hdrStreamIdx]==NULL)
                {
                    vh.size[hdrStreamIdx] = streamSize;
                    vh.streamBuf[hdrStreamIdx] = (uchar_t *) malloc(sizeof(uchar_t)*(streamSize));
                    if(vh.streamBuf[hdrStreamIdx]==NULL)
                    {
                        std::cout << "H264BufferParser[error]: Unable to allocate memory for stream buffer" << std::endl;
                        return false;
                    }
                    vh.streamBuf[hdrStreamIdx] = (uchar_t *) memcpy(vh.streamBuf[hdrStreamIdx],streamBuf,streamSize);
                }
            }
            else
            {
                type = ((uint_t) streamBuf[5]) & 0x1c;
                type >>= 2;
            }

            if(verbose)
            {
                // Print results.
                std::cout << "H264BufferParser[info]: NAL start prefix: " << nalStartCodePrefix << "\t";
                std::cout << "NAL ref idc: " << nalRefIdc << "\t";
                std::cout << "NAL type: " << nalType << "\t";
                if(type!=X264_TYPE_HDR)
                    std::cout << "Frame type: " << frameTypeArray[type-1] << "\t";
                else
                    std::cout << "Video Header Info\t";
                std::cout << "Size: " << streamSize << " bytes" << std::endl;
            }

            return true;
        }
};

#endif // H264BUFFERPARSER_H_
