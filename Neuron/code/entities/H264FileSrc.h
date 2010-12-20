#ifndef H264FILESRC_H_
#define H264FILESRC_H_

#include "entity.h"
#include "H264FileInputObject.h"
#include "DDSOutputObject.h"

class H264FileSrc : public SessionEntity,public EventHandlerT<H264FileSrc>,public ThreadSingle
{
    private:

        long                    seqNum;
        std::string             FileName;
        H264FileInputObject    *pInputObj;
        DDSOutputObject        *pOutputObj;

        void EventHandleLoop(void)
        {
            pInputObj->startThread();

            while(!isStopRequested)
            {
                HandleNextEvent();
            }

            pInputObj->stopThread();
            return;
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            H264Frame   *pFrame = reinterpret_cast<MediaInputEvent<H264Frame*>*>(pEvent)->GetData();
            const char  *hdrNames[VIDEO_HDR_TYPES] = {"SEI","SPSF","PPS","SPSH","SPSQ"};
            const char  *frmTypes[5] = {"i","I","P","B","b"};

            if(pFrame->type==X264_TYPE_IDR || pFrame->type==X264_TYPE_I)
            {
                for(int i=0; i<VIDEO_HDR_TYPES; i++)
                {
#ifdef VERBOSE_OUTPUT
                    std::cout << seqNum << " :\t" << hdrNames[i] << " :\t" << pInputObj->pParser->vh.size[i] << std::endl;
#endif
                    pOutputObj->Write((ToString<int>(id)+"/0").c_str(),seqNum++,pInputObj->pParser->vh.streamBuf[i],pInputObj->pParser->vh.size[i]);
                }
            }

/*#ifdef VERBOSE_OUTPUT
                    std::cout << seqNum << " :\t" << frmTypes[pFrame->type-1] << " :\t" << pFrame->size << std::endl;
#endif*/
            pOutputObj->Write((ToString<int>(id)+"/"+(pFrame->layerName)).c_str(),seqNum++,pFrame->pBuf,pFrame->size);
            delete pFrame;

            return;
        }

        int workerBee(void)
        {
            EventHandleLoop();
            return 0;
        }

    public:

        H264FileSrc(int idP,int ownerIdP,int sessionIdP,const char *fileNameP,DDSDomainParticipant *pOwnerDPP,DDSTopic *pTopicP):
        SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_H264FILESRC),EventHandlerT<H264FileSrc>(),ThreadSingle()
        {
            double          fps;
            std::string     FpsFileName(fileNameP);
            std::ifstream   in;

            seqNum = 0;
            FileName = fileNameP;
            FpsFileName = FpsFileName + "fps";
            in.open(FpsFileName.c_str(),std::ifstream::in);
            in >> fps;
            in.close();

            std::cout << "FPS selected " << fps << std::endl;
            pInputObj = new H264FileInputObject(this,idP,FileName.c_str(),fps);
            pOutputObj = new DDSOutputObject(idP,pOwnerDPP,pTopicP);
            AddHandleFunc(&H264FileSrc::HandleMediaInputEvent,MEDIA_INPUT_EVENT);

            for(int i=0; i<3; i++)
            {
                char layerPartitionName[50];
                sprintf(layerPartitionName,"%d/%d",id,i);
                pOutputObj->AddLayerWriter(layerPartitionName);
            }

            usleep(1000000);
        }

        ~H264FileSrc()
        {
            delete pInputObj;
            delete pOutputObj;
        }
};

#endif // H264FILESRC_H_
