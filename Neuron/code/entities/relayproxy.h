#ifndef RELAYPROXY_H_
#define RELAYPROXY_H_

#include "entity.h"
#include "DDSInputObject.h"
#include "DDSOutputObject.h"

class RelayProxy : public SessionEntity,public EventHandlerT<RelayProxy>,public ThreadSingle
{
    private:

        int epId;
        int nLayers;
        DDSInputObject *pInputObj;
        DDSOutputObject *pOutputObj;

        void EventHandleLoop(void)
        {
            /*int             i = 1;
            int             count = 0;
            const char     *layerPartitions[4] = { "*","0","[23]","[14]" };*/
            while(!isStopRequested)
            {
                count++;
                HandleNextEvent();

                /*if(count%10==0)
                {
                    pInputObj->SetLayerReaderPartition(layerPartitions[(i-1)%4],layerPartitions[i%4]);
                    std::cout << "Changing partition from " << layerPartitions[(i-1)%4] << " to " << layerPartitions[i%4] << std::endl;
                    i++;
                }*/
            }

            while(!NoEvents())
                HandleNextEvent();

            return;
        }

        void DetermineSamplePartition(char *layerPartitionName,DataSampleSet *pSampleSet,int sampIdx)
        {
            int num = *(reinterpret_cast<int*>((*pSampleSet->pSeqData)[sampIdx].payload.get_contiguous_buffer()));
#ifdef VERBOSE_OUTPUT
            std::cout << "Writing " << num << " in partition " << num%nLayers << std::endl;
#endif
            sprintf(layerPartitionName,"%d",num%nLayers);

            return;
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            DataSampleSet *pSampleSet = reinterpret_cast<MediaInputEvent<DataSampleSet*>*>(pEvent)->GetData();

            for(int i=0; i<pSampleSet->pSeqData->length(); i++)
            {
                if((*pSampleSet->pSeqInfo)[i].valid_data)
                {
                    char layerPartitionName[10];
                    std::cout << "Num: " << *(reinterpret_cast<int*>((*pSampleSet->pSeqData)[i].payload.get_contiguous_buffer())) << std::endl;
                    DetermineSamplePartition(layerPartitionName,pSampleSet,i);
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

        RelayProxy(DDSDomainParticipant *pOwnerDPP,DDSTopic *pTopicP,int idP,int epIdP,int ownerIdP,int sessionIdP, int nLayersP):
        EventHandlerT<RelayProxy>(),SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_RELAYPROXY),ThreadSingle()
        {
            epId = epIdP;
            nLayers = nLayersP;
            AddHandleFunc(&RelayProxy::HandleMediaInputEvent,MEDIA_INPUT_EVENT);

            pInputObj = new DDSInputObject(id,this,pOwnerDP,pTopicP);
            pInputObj->AddLayerReader("*");

            pOutputObj = new DDSOutputObject(id,pOwnerDP,pTopicP);
            for(int i=0; i<nLayers; i++)
            {
                char layerPartitionName[10];
                sprintf(layerPartitionName,"%d",i);
                pOutputObj->AddLayerWriter(layerPartitionName);
            }
        }

        ~RelayProxy()
        {
            delete pInputObj;
            delete pOutputObj;
        }
};

#endif // RELAYPROXY_H_
