#ifndef NATNUMSRC_H_
#define NATNUMSRC_H_

#include "entity.h"
#include "NatNumStreamInputObject.h"
#include "DDSOutputObject.h"

class NatNumSrc : public SessionEntity,public EventHandlerT<NatNumSrc>,public ThreadSingle
{
    private:

        int numLayers;
        long seqNum;
        NatNumStreamInputObject *pInputObj;
        DDSOutputObject *pOutputObj;

        void EventHandleLoop(void)
        {
            pInputObj->startThread();

            while(!isStopRequested)
            {
                HandleNextEvent();
            }

            pInputObj->stopThread();
            while(!NoEvents())
                HandleNextEvent();

            return;
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            int num = reinterpret_cast<MediaInputEvent<int> *>(pEvent)->GetData();
            char layerPartitionName[50];

            sprintf(layerPartitionName,"%d/%d",id,num%numLayers);
#ifdef VERBOSE_OUTPUT
            std::cout << MOO_LOG_PROMPT(id) << ": (data=" << num << ",layer=" << layerPartitionName << ")" << std::endl;
#endif
            pOutputObj->Write((const char *)layerPartitionName,seqNum++,
                              (unsigned char *)&num,(int)sizeof(int));

            return;
        }

        int workerBee(void)
        {
            EventHandleLoop();
            return 0;
        }

    public:

        NatNumSrc(int idP,int ownerIdP,int sessionIdP,int uLimitP,int streamPeriodSecsP,int numLayersP,DDSDomainParticipant *pOwnerDPP,DDSTopic *pTopicP):
        SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_NATNUMSRC),EventHandlerT<NatNumSrc>(),ThreadSingle()
        {
            numLayers = numLayersP;
            seqNum = 0;
            pInputObj = new NatNumStreamInputObject(this,idP,uLimitP,streamPeriodSecsP);
            pOutputObj = new DDSOutputObject(idP,pOwnerDPP,pTopicP);
            AddHandleFunc(&NatNumSrc::HandleMediaInputEvent,MEDIA_INPUT_EVENT);

            for(int i=0; i<numLayersP; i++)
            {
                char layerPartitionName[50];
                sprintf(layerPartitionName,"%d/%d",id,i);
                pOutputObj->AddLayerWriter(layerPartitionName);
            }
        }

        ~NatNumSrc()
        {
            delete pInputObj;
            delete pOutputObj;
        }
};

#endif // NATNUMSRC_H_
