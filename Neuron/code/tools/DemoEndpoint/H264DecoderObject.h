#ifndef H264DECODEROBJECT_H_
#define H264DECODEROBJECT_H_

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}
#include <iostream>
#include "MediaEvent.h"

#define MAX_Q_FRAMES 15

class RawVideoFrame
{
    public:

        AVFrame        *pFrame;
        int32_t         ptsDeltaMus;
        PixelFormat     pixelFormat;

        RawVideoFrame(AVCodecContext *pCdcCtx,AVFrame *pFrameP)
        {
            int         frameBufSize;
            uint8_t    *pFrameBuf = NULL;

            if((pFrame=avcodec_alloc_frame())==NULL)
            {
                std::cout << "Memory allocation problem for frame" << std::endl;
                exit(0);
            }

            //Allocate buffer for pFrame
            frameBufSize = avpicture_get_size(pCdcCtx->pix_fmt,pCdcCtx->width,pCdcCtx->height);
            pFrameBuf = (uint8_t *) av_mallocz(sizeof(uint8_t)*frameBufSize);
            avpicture_fill((AVPicture *)pFrame,pFrameBuf,pCdcCtx->pix_fmt,pCdcCtx->width,pCdcCtx->height);
            pFrameBuf = NULL;

            //Copy frame and presentation time stamp
            av_picture_copy((AVPicture *)pFrame,(AVPicture *)pFrameP,pCdcCtx->pix_fmt,pCdcCtx->width,pCdcCtx->height);

            pixelFormat = pCdcCtx->pix_fmt;
            ptsDeltaMus = pCdcCtx->pts_delta_ms*1000;
        }

        ~RawVideoFrame()
        {
            av_freep(pFrame);
        }
};

class H264DecoderObject : public ThreadSingle
{
    private:

        int                 vidStreamIdx;
        AVCodec            *pCodec;
        AVFormatContext    *pFmtCtx;
        AVCodecContext     *pCdcCtx;
        EventHandler       *pOwnerEventHandler;

        int DecodeInterruptCB()
        {
            return(isStopRequested ? 1 : 0);
        }

        int workerBee(void)
        {
          	int                                 frameDecodeDone;
          	AVPacket	                        Packet;
            AVFrame	                           *pFrame;
            MediaInputEvent<RawVideoFrame *>   *pEvent = NULL;

            // Allocate memory for decoded frame
            if((pFrame=avcodec_alloc_frame())==NULL)
            {
                std::cout << "Memory allocation problem for decoded frame" << std::endl;
                return -1;
            }

            // Interrupt blocking functions if quit
            //url_set_interrupt_cb(DecodeInterruptCB);

            // Decode each frame and enqueue it
            while(!isStopRequested && av_read_frame(pFmtCtx,&Packet)>=0)
            {
                if(Packet.stream_index==vidStreamIdx)
                {
                    avcodec_decode_video2(pCdcCtx,pFrame,&frameDecodeDone,&Packet);
                    if(frameDecodeDone)
                    {
                        pEvent = new MediaInputEvent<RawVideoFrame *>(new RawVideoFrame(pCdcCtx,pFrame));

                        //Insert finished frame to event queue
                        while(!isStopRequested && pOwnerEventHandler->NumEvents()>=MAX_Q_FRAMES)
                            usleep(20000);

                        pOwnerEventHandler->SignalEvent((Event *)pEvent);
                    }
                }

                av_free_packet(&Packet);
            }

            std::cout << "Decode ended..." << std::endl;
            return 0;
        }

    public:

        H264DecoderObject(EventHandler *pOwnerEventHandlerP,const char *decInFifoNameP,int resWidthP,int resHeightP) : ThreadSingle()
        {
            pOwnerEventHandler = pOwnerEventHandlerP;

            //Register all available file formats and codecs
            av_register_all();

            //If fifo not present, create it
            if(access(decInFifoNameP,F_OK)<0)
                mkfifo(decInFifoNameP,S_IRWXU|S_IRWXG);

            //Attempt to open the decoder input fifo
            if(av_open_input_file(&pFmtCtx,decInFifoNameP,NULL,0,NULL)!=0)
            {
                std::cout << "Could not open file: " << decInFifoNameP << std::endl;
                exit(0);
            }

            //Find the video stream and get pointer to its codec context
            for(int i=0; i<pFmtCtx->nb_streams; i++)
            {
                if(pFmtCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO)
                {
                    vidStreamIdx = i;
                    break;
                }
            }

            if(vidStreamIdx==-1 )
            {
                std::cout << "Could not retrieve stream from file: " << decInFifoNameP << std::endl;
                exit(0);
            }

            pCdcCtx = pFmtCtx->streams[vidStreamIdx]->codec;
            pCdcCtx->width = resWidthP;
            pCdcCtx->height = resHeightP;
            pCdcCtx->pix_fmt = PIX_FMT_YUV422P;

            // Find and open appropriate decoder based on codec context
            pCodec = avcodec_find_decoder(pCdcCtx->codec_id);
            if(pCodec==NULL)
            {
                std::cout << "No suitable decoder found for stream in file " << decInFifoNameP << std::endl;
                exit(0);
            }

            if(avcodec_open(pCdcCtx,pCodec)<0)
            {
                std::cout << "Could not open decoder" << std::endl;
                exit(0);
            }
        }

        ~H264DecoderObject()
        {
            avcodec_close(pCdcCtx);
            av_close_input_file(pFmtCtx);
        }
};

#endif // H264DECODEROBJECT_H_
