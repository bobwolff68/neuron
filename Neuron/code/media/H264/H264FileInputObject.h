#ifndef H264FILEINPUTOBJECT_H_
#define H264FILEINPUTOBJECT_H_

#include <time.h>
#include <sys/time.h>
#include "H264FileParser.h"
#include "MediaInputObject.h"
#include "ThreadSingle.h"

class H264FileInputObject : public MediaInputObject,public ThreadSingle
{
    int64_t GetTimeMicrosecs(void)
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);

        return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
    }

    void WaitTTW(int64_t timeMicrosecs,int64_t periodMicrosecs)
    {
        int64_t ttwMicrosecs;

        ttwMicrosecs = periodMicrosecs-GetTimeMicrosecs()+timeMicrosecs;
        ttwMicrosecs = ttwMicrosecs*((int64_t)(ttwMicrosecs>0));

        if(ttwMicrosecs>0)
        {
            struct  timespec ttw;
            ttw.tv_sec = (time_t) ttwMicrosecs/1000000;
            ttw.tv_nsec = (long) (ttwMicrosecs%1000000)*1000;
            nanosleep(&ttw,NULL);
        }
    }

    private:

        int64_t         musPerFrame;
        const char     *fileName;

    public:

        H264FileParser *pParser;

        H264FileInputObject(EventHandler *pOwnerEventHandlerP,int ownerIdP,const char *fileNameP,double fpsP):
        MediaInputObject(pOwnerEventHandlerP,ownerIdP),ThreadSingle()
        {
            musPerFrame = (int64_t)(1000000.0/fpsP);
            fileName = fileNameP;
            pParser = NULL;
        }

        ~H264FileInputObject()
        {
            std::cout << "Deleting iobj" << std::endl;
            if(pParser!=NULL)
                delete pParser;
        }

        void StreamMedia(void)
        {
            int64_t                         timeMicrosecs;
            MediaInputEvent<H264Frame*>    *pEvent = NULL;

            while(!isStopRequested)
            {
                pParser = new H264FileParser(fileName);

                //Send Header Info
                for(int i=0; i<VIDEO_HDR_TYPES; i++)
                {
                    timeMicrosecs = GetTimeMicrosecs();
                    if(!(pParser->ExtractHeader(i,0))) return;
                    /*pEvent = new MediaInputEvent<H264Frame*>(
                                                                new H264Frame(
                                                                                pParser->type,
                                                                                pParser->streamPtr,
                                                                                LAYER_TYPE(pParser->type),
                                                                                pParser->streamBuf
                                                                             )
                                                            );
                    pOwnerEventHandler->SignalEvent((Event *)pEvent);*/
                    WaitTTW(timeMicrosecs,musPerFrame);
                    free(pParser->streamBuf);
                }

                //Send Frames
                while(!feof(pParser->stream) && !isStopRequested)
                {
                    timeMicrosecs = GetTimeMicrosecs();
                    if(!(pParser->ExtractFrame(0))) return;
                    pEvent = new MediaInputEvent<H264Frame*>(
                                                                new H264Frame(
                                                                                pParser->type,
                                                                                pParser->streamPtr,
                                                                                LAYER_TYPE(pParser->type),
                                                                                pParser->streamBuf
                                                                             )
                                                            );
                    pOwnerEventHandler->SignalEvent((Event *)pEvent);
                    WaitTTW(timeMicrosecs,musPerFrame);
                }

                delete pParser;
                pParser = NULL;
            }

            return;
        }

        int workerBee(void)
        {
            StreamMedia();

            return 1;
        }
};

#endif // H264FILEINPUTOBJECT_H_
