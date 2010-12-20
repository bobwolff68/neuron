#ifndef H264STREAMVIEWER_H_
#define H264STREAMVIEWER_H_

#include <time.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "H264DecoderObject.h"
#include "VideoDisplayObject.h"

//NOTE: Each instance has to be instantiated in a forked process
class H264StreamViewer : public EventHandlerT<H264StreamViewer>,public ThreadSingle
{
    int64_t GetTimeMicrosecs(void)
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
    }

    private:

        int                 sigPipeFd;
        int                 resW;
        int                 resH;
        int64_t             curPtsMus;
        int64_t             curPtsDeltaMus;
        int64_t             curDispTimeMus;
        int64_t             prevDispTimeMus;
        const char         *decInFifoName;
        H264DecoderObject  *pDecoderObj;
        VideoDisplayObject *pDispObj;

        void EventHandleLoop(void)
        {
            pDecoderObj->startThread();

            while(!isStopRequested)
            {
                HandleNextEvent();
            }

            pDecoderObj->stopThread();
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            RawVideoFrame      *pRawFrame = reinterpret_cast<MediaInputEvent<RawVideoFrame*>*>(pEvent)->GetData();
            struct timespec     ttw;
            int64_t             dispIntvlMus;
            int64_t             offsetMus;

            if(!curPtsDeltaMus)
            {
                curPtsMus = GetTimeMicrosecs();
                prevDispTimeMus = GetTimeMicrosecs();
            }

            curDispTimeMus = GetTimeMicrosecs();
            dispIntvlMus = curDispTimeMus-prevDispTimeMus;
            offsetMus = (curPtsDeltaMus-dispIntvlMus);

            /*if(offsetMus>0)
            {
                ttw.tv_sec = offsetMus/1000000;
                ttw.tv_nsec = ((offsetMus)%1000000)*1000;
                nanosleep(&ttw,NULL);
            }*/

            curDispTimeMus = GetTimeMicrosecs();
            pDispObj->Write(pRawFrame->pFrame,pRawFrame->pixelFormat);
            prevDispTimeMus = curDispTimeMus;
            curPtsDeltaMus = pRawFrame->ptsDeltaMus;
            curPtsMus += curPtsDeltaMus;

            delete pRawFrame;
            return;
        }

        int workerBee(void)
        {
            pDecoderObj = new H264DecoderObject(this,decInFifoName,resW,resH);
            EventHandleLoop();
            return 0;
        }

    public:

        H264StreamViewer(const char *decInFifoNameP,int resWidthP,int resHeightP,int sigPipeFdP):
        EventHandlerT<H264StreamViewer>(),ThreadSingle()
        {
            char buf;

            curPtsMus = 0;
            prevDispTimeMus = 0;
            curPtsDeltaMus = 0;
            sigPipeFd = sigPipeFdP;
            resW = resWidthP;
            resH = resHeightP;
            decInFifoName = decInFifoNameP;

            AddHandleFunc(&H264StreamViewer::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            pDispObj = new VideoDisplayObject(resWidthP,resHeightP,SDL_YV12_OVERLAY,PIX_FMT_YUV420P);

            //Start the viewer
            startThread();
            read(sigPipeFd,&buf,1);
            std::cout << "VIEWER LOOP ENDS" << std::endl;
            stopThread();
            std::cout << "DONE" << std::endl;
            close(sigPipeFd);

            return;
        }

        ~H264StreamViewer()
        {
            std::cout << "Deleting Viewer" << std::endl;
            delete pDecoderObj;
            delete pDispObj;
        }
};

#endif // H264STREAMVIEWER_H_
