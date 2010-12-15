#ifndef RELAYPROXY_H_
#define RELAYPROXY_H_

#include "entity.h"
#include "DDSInputObject.h"
#include "DDSOutputObject.h"
#include "H264BufferParser.h"

class RelayProxy : public SessionEntity,public EventHandlerT<RelayProxy>,public ThreadSingle
{
    private:

        int                 epId;
        int                 nLayers;
        std::string         curLayerPartition;
        DDSInputObject     *pInputObj;
        DDSOutputObject    *pOutputObj;
        H264BufferParser   *pParser;

        void EventHandleLoop(void)
        {
            while(!isStopRequested)
                HandleNextEvent();

            while(!NoEvents())
                HandleNextEvent();

            return;
        }

        void DetermineSamplePartition(char *layerPartitionName,DataSampleSet *pSampleSet,int sampIdx)
        {
            int             bufSize = (int) (*pSampleSet->pSeqData)[sampIdx].payload.length();
            unsigned char  *pBuf = reinterpret_cast<unsigned char *>((*pSampleSet->pSeqData)[sampIdx].payload.get_contiguous_buffer());

            if(!pParser->ParseBuffer(pBuf,bufSize,0))   return;
            strcpy(layerPartitionName,(ToString<int>(id)+"/"+ToString<int>(LAYER_TYPE(pParser->type))).c_str());
            return;
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            DataSampleSet *pSampleSet = reinterpret_cast<MediaInputEvent<DataSampleSet*>*>(pEvent)->GetData();

            for(int i=0; i<pSampleSet->pSeqData->length(); i++)
            {
                if((*pSampleSet->pSeqInfo)[i].valid_data)
                {
                    char layerPartitionName[50];
                    DetermineSamplePartition(layerPartitionName,pSampleSet,i);
                    std::cout << "Partition: " << layerPartitionName << std::endl;
                    pOutputObj->Write(layerPartitionName,(*pSampleSet->pSeqData)[i]);
                }
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

        RelayProxy(int idP,int epIdP,int ownerIdP,int sessionIdP,DDSDomainParticipant *pOwnerDPP,DDSTopic *pTopicP,int nLayersP,const char *layerRegExp):
        EventHandlerT<RelayProxy>(),SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_RELAYPROXY),ThreadSingle()
        {
            epId = epIdP;
            nLayers = nLayersP;
            curLayerPartition = ToString<int>(epId)+"/"+layerRegExp;
            AddHandleFunc(&RelayProxy::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            pParser = new H264BufferParser();
            pInputObj = new DDSInputObject(id,this,pOwnerDP,pTopicP);
            pInputObj->AddLayerReader(curLayerPartition.c_str());
            pOutputObj = new DDSOutputObject(id,pOwnerDP,pTopicP);
            for(int i=0; i<nLayers; i++)
                pOutputObj->AddLayerWriter((ToString<int>(id)+"/"+ToString<int>(i)).c_str());
        }

        ~RelayProxy()
        {
            delete pParser;
            delete pInputObj;
            delete pOutputObj;
        }

        void UpdateVideoSource(int newSrcId)
        {
            epId = newSrcId;
            std::string LayerRegExp = curLayerPartition.substr(curLayerPartition.find('/'));
            pInputObj->SetLayerReaderPartition(curLayerPartition.c_str(),(ToString<int>(epId)+LayerRegExp).c_str());
            curLayerPartition = (ToString<int>(epId)+LayerRegExp).c_str();
            std::cout << "Partition changed to: " << curLayerPartition << std::endl;
            return;
        }
};

#endif // RELAYPROXY_H_
