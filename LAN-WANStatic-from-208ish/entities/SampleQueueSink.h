#ifndef SAMPLEQUEUESINK_H_
#define SAMPLEQUEUESINK_H_

#include "entity.h"
#include "H264BufferParser.h"
#include "DDSInputObject.h"
#include "SampleQueueOutputObject.h"


class SampleQueueSink : public SessionEntity,public EventHandlerT<SampleQueueSink>,public ThreadSingle
{
    private:

        int                         epId;
        int                         srcId;
        int                         curSeqNum;
        std::string                 curLayerPartition;
        DDSInputObject             *pInputObj;
        SampleQueueOutputObject    *pOutputObj;

        void EventHandleLoop(void)
        {
            while(!isStopRequested)
                HandleNextEvent();

            while(!NoEvents())
                HandleNextEvent();

            return;
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            DataSampleSet *pSampleSet = reinterpret_cast<MediaInputEvent<DataSampleSet*>*>(pEvent)->GetData();

            for(int i=0; i<pSampleSet->pSeqData->length(); i++)
                if((*pSampleSet->pSeqInfo)[i].valid_data)
                {
                    if(((*pSampleSet->pSeqData)[i].seqNum)==0 || curSeqNum>=((*pSampleSet->pSeqData)[i].seqNum))
                    {
                        pOutputObj->Clear();
                        std::cout << std::endl << "--------STATS FOR NEW STREAM--------" << std::endl;
                    }

                    pOutputObj->Write((*pSampleSet->pSeqData)[i].seqNum);
                    curSeqNum = (*pSampleSet->pSeqData)[i].seqNum;
                }

            delete pSampleSet;

            return;
        }

        int workerBee(void)
        {
            EventHandleLoop();
            return 0;
        }

    public:

        SampleQueueSink(int idP,int epIdP,int ownerIdP,int sessionIdP,int maxQueueLenP,DDSDomainParticipant *pOwnerDPP,
                        DDSTopic *pTopicP,const char *layerRegExp):
        SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_SAMPLEQUEUESINK),EventHandlerT<SampleQueueSink>(),ThreadSingle()
        {
            epId = epIdP;
            srcId = epIdP;
            curLayerPartition = ToString<int>(srcId)+"/"+layerRegExp;
            pInputObj = new DDSInputObject(idP,this,pOwnerDPP,pTopicP);
            AddHandleFunc(&SampleQueueSink::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            pInputObj->AddLayerReader(curLayerPartition.c_str());
            pOutputObj = new SampleQueueOutputObject(idP,maxQueueLenP);
        }

        ~SampleQueueSink()
        {
            delete pInputObj;
            delete pOutputObj;
        }

        void UpdateSource(int newSrcId)
        {
            srcId = newSrcId;
            std::string LayerRegExp = curLayerPartition.substr(curLayerPartition.find('/'));
            pInputObj->SetLayerReaderPartition(curLayerPartition.c_str(),(ToString<int>(srcId)+LayerRegExp).c_str());
            curLayerPartition = (ToString<int>(srcId)+LayerRegExp).c_str();
            std::cout << "Partition changed to: " << curLayerPartition << std::endl;
            return;
        }
};

#endif // SAMPLEQUEUESINK_H_
