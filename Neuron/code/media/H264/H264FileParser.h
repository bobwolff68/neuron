#ifndef H264FILEPARSER_H_
#define H264FILEPARSER_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define LAYER_TYPE(frm_type)	(((frm_type)>>2)*(1+((frm_type)&1)))

typedef unsigned char uchar_t;
typedef unsigned int  uint_t;

class H264Frame
{
    public:

        int         type;
        int         size;
        char        layerName[3];
        uchar_t    *pBuf;

        H264Frame(int typeP,int sizeP,int layerNumP,uchar_t *pBufP):
        type(typeP),size(sizeP),pBuf(pBufP)
        {
            sprintf(layerName,"%d",layerNumP);
        }

        ~H264Frame()
        {
            free(pBuf);
        }
};

class H264FileParser
{
    class H264Header
    {
        public:

            int         size[VIDEO_HDR_TYPES];
            uchar_t    *streamBuf[VIDEO_HDR_TYPES];
    };

    private:

        int		        nStreamChunks;

    public:

        int 	        type;
        int 	        streamPtr;
        uchar_t        *streamBuf;
        FILE	       *stream;
        H264Header	    vh;


        H264FileParser(const char *fileName)
        {
            if((stream=fopen(fileName,"r"))==NULL)
            {
                std::cout << "H264FileParser[error]: Trouble opening H264 stream " << fileName << std::endl;
            }
            else
            {
                streamPtr = 0;
                nStreamChunks = 0;
                type = -1;
                streamBuf = NULL;
                /*if((streamBuf=(uchar_t *) malloc(sizeof(uchar_t)*STREAM_CHUNK_SIZE))==NULL)
                {
                    std::cout << "H264FileParser[error]: Unable to allocate memory for stream buffer" << std::endl;
                    fclose(stream);
                }
                else
                {*/
                for(int i=0; i<VIDEO_HDR_TYPES; i++)
                    vh.streamBuf[i] = NULL;
                //}
            }
        }

        ~H264FileParser()
        {
            for(int i=0; i<VIDEO_HDR_TYPES; i++)
                if(vh.streamBuf[i]!=NULL)
                    free(vh.streamBuf[i]);

            //streamBuf is not freed because it is supposed to
            //be done by the output object thread.

            if(fclose(stream))
                std::cout << "H264FileParser[error]: FClose error" << std::endl;
        }

        bool ExtractHeader(int hdrStreamIndex,uint_t verbose)
        {
           	size_t	bytesRead;
            uint_t	nalStartCodePrefix = 0;
            int		nalRefIdc = 0;
            int		nalType = 0;

            // Reset frame buffer position and size.
            Reset();

            // Extract the next 5 bytes from the stream.
            // Bytes 1..4 ==> NAL start code prefix.
            // Byte 5	  ==> NAL header 0 xx yyyyy ( xx - 2 bit nal_ref_idc, yyyyy - 5 bit nal_type ).
            if(!Read(5,&bytesRead))
                return false;

            nalStartCodePrefix = (uint_t) streamBuf[streamPtr-5];
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[streamPtr-4]);
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[streamPtr-3]);
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[streamPtr-2]);

            nalRefIdc = ((uint_t) streamBuf[streamPtr-1]) & 0x60;
            nalRefIdc >>= 5;

            nalType = ((uint_t) streamBuf[streamPtr-1]) & 0x1f;
            type = X264_TYPE_HDR;

            if(verbose)
            {
                // Print results.
                std::cout << "H264FileParser[info]: NAL start prefix: " << nalStartCodePrefix << "\t";
                std::cout << "NAL ref idc: " << nalRefIdc << "\t";
                std::cout << "NAL type: " << nalType << "\t";
                std::cout << "Video Header\t";
            }

            // Keep on writing until dectection of next start code prefix.
            nalStartCodePrefix = 0xffffffff;
            while(!feof(stream))
            {
                if( nalStartCodePrefix == 0x00000001 )
                {
                    if(fseek(stream,-4,SEEK_CUR))
                    {
                        std::cout << "H264FileParser[error]: Seek error" << std::endl;
                        return false;
                    }
                    streamPtr -= 4;
                    break;
                }
                else
                {
                    if(!Read(1,&bytesRead))     return false;
                    if(!bytesRead)	            break;
                }
                nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[streamPtr-1]);
            }

            if(verbose)
            {
                // streamPtr is the size of the encoded frame.
                std::cout << "Size: " << streamPtr << " bytes" << std::endl;
            }

            // Store stream in the appropriate video header stream.
            vh.size[hdrStreamIndex] = streamPtr;
            vh.streamBuf[hdrStreamIndex] = (uchar_t *) malloc(sizeof(uchar_t)*(streamPtr));
            if(vh.streamBuf[hdrStreamIndex]==NULL)
            {
                std::cout << "H264FileParser[error]: Unable to allocate memory for stream buffer" << std::endl;
                fclose(stream);
                return false;
            }
            vh.streamBuf[hdrStreamIndex] = (uchar_t *) memcpy(vh.streamBuf[hdrStreamIndex],streamBuf,streamPtr);
            // Since SPS headers for half and quarter frame rates are stored in user data NALs,
            // convert these NALs to SPS NALs for later insertion into the stream.
            if(hdrStreamIndex==SPSH_STREAM_INDEX || hdrStreamIndex==SPSQ_STREAM_INDEX)
                vh.streamBuf[hdrStreamIndex][4] = (uchar_t) ( (0x00<<7) | (NAL_VHDR_REF_IDC<<5) | NAL_TYPE_SPS );

            return true;
        }

        bool ExtractFrame(uint_t verbose)
        {
            size_t	bytesRead;
            uint_t	nalStartCodePrefix = 0;
            int		nalRefIdc = 0;
            int		nalType = 0;
            char	frameTypeArray[5] = { 'i', 'I', 'P', 'B', 'b' };

            // Reset frame buffer position and size.
            Reset();

            if(feof(stream))
                return true;

            // Extract the next 6 bytes from the stream.
            // Bytes 1..4 ==> NAL start code prefix.
            // Byte 5	  ==> NAL header 0 xx yyyyy ( xx - 2 bit nal_ref_idc, yyyyy - 5 bit nal_type ).
            // Byte 6	  ==> aaa bbb 00 ( aaa - 3 bit slice type, bbb - 3 bit frame type ).
            if(!Read(6,&bytesRead))
                return false;

            nalStartCodePrefix = (uint_t) streamBuf[streamPtr-6];
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[streamPtr-5]);
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[streamPtr-4]);
            nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[streamPtr-3]);

            nalRefIdc = ((uint_t) streamBuf[streamPtr-2]) & 0x60;
            nalRefIdc >>= 5;

            nalType = ((uint_t) streamBuf[streamPtr-2]) & 0x1f;

            type = ((uint_t) streamBuf[streamPtr-1]) & 0x1c;
            type >>= 2;

            //Do this only for rp chain length, remove later
            streamBuf[streamPtr-1] = (streamBuf[streamPtr-1]>>2)<<2;

            if(nalType != NAL_TYPE_AUD)
                type = X264_TYPE_HDR;

            if(verbose)
            {
                // Print results.
                std::cout << "H264FileParser[info]: NAL start prefix: " << nalStartCodePrefix << "\t";
                std::cout << "NAL ref idc: " << nalRefIdc << "\t";
                std::cout << "NAL type: " << nalType << "\t";
                if(type!=X264_TYPE_HDR)
                    std::cout << "Frame type: " << frameTypeArray[type-1] << "\t";
                else
                    std::cout << "Video Header Info\t";
            }

            // Keep on writing until dectection of next NAL AUD or end of file.
            nalStartCodePrefix = 0xffffffff;
            while(!feof(stream))
            {
                if(nalStartCodePrefix==0x00000001)
                {
                    // Check the nal_type field in the next byte.
                    if(!Read(1,&bytesRead))
                        return false;

                    nalType = ((int) streamBuf[streamPtr-1]) & 0x1f;
                    // If next NAL is an NAL AUD, walk back the stream buffer
                    // and the x264 stream pointer to the start of the NAL.
                    if(nalType==NAL_TYPE_AUD)
                    {
                        if(fseek(stream,-5,SEEK_CUR))
                        {
                            std::cout << "H264FileParser[error]: Seek error" << std::endl;
                            return false;
                        }
                        streamPtr -= 5;
                        break;
                    }
                }
                else
                {
                    if(!Read(1,&bytesRead)) return false;
                    if(!bytesRead)	        break;
                }
                nalStartCodePrefix = (nalStartCodePrefix << 8) | ((uint_t) streamBuf[streamPtr-1]);
            }

            if(verbose)
            {
                // streamPtr is the size of the encoded frame.
                std::cout << "Size: %d bytes " << streamPtr << std::endl;
            }

            return true;
        }

        bool Read(size_t nEle,size_t *bytesRead)
        {
            int streamBufSize = nStreamChunks*STREAM_CHUNK_SIZE;
            int bytesToRead = sizeof(uchar_t)*nEle;

            // If buffer overflows, increase its size.
            if(streamPtr>(streamBufSize-bytesToRead))
            {
                nStreamChunks++;
                streamBuf = (uchar_t *) realloc(streamBuf,sizeof(uchar_t)*(streamBufSize+STREAM_CHUNK_SIZE));
                if(streamBuf==NULL )
                {
                    std::cout << "H264FileParser[error]: Realloc error" << std::endl;
                    return false;
                }
            }

            *bytesRead = fread(&(streamBuf[streamPtr]),sizeof(uchar_t),nEle,stream);
            streamPtr += (*bytesRead);

            if((*bytesRead)<bytesToRead)
            {
                if(ferror(stream))
                {
                    std::cout << "H264FileParser[error]: FRead error" << std::endl;
                    return false;
                }
            }

            return true;
        }

        void Reset(void)
        {
            streamPtr = 0;
            type = -1;

            //streamBuf is set to NULL, and not freed because it is to be accessed
            //and deleted by the output object on another thread.
            nStreamChunks = 0;
            streamBuf = NULL;

            return;
        }
};

#endif // H264FILEPARSER_H_
