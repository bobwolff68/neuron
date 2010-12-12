#ifndef STDOUTSINK_H_
#define STDOUTSINK_H_

#include "entity.h"
#include "DDSInputObject.h"
#include "StdOutOutputObject.h"

class StdOutSink : public SessionEntity,public EventHandlerT<StdOutSink>,public ThreadSingle
{
    private:

        int epId;
        DDSInputObject     *pInputObj;
        StdOutOutputObject *pOutputObj;

        void EventHandleLoop(void)
        {
            int             i = 1;
            int             count = 0;
            const char     *layerPartitions[4] = { "*","0","[23]","[14]" };

            while(!isStopRequested)
            {
                if(HandleNextEvent())
                {
                    count++;
                    if(count%10==0)
                    {
                        std::string CurReaderPartition = ToString<int>(epId) + "/" + layerPartitions[(i-1)%4];
                        std::string NxtReaderPartition = ToString<int>(epId) + "/" + layerPartitions[i%4];
                        pInputObj->SetLayerReaderPartition(CurReaderPartition.c_str(),NxtReaderPartition.c_str());
#ifdef VERBOSE_OUTPUT
                        std::cout << MOO_LOG_PROMPT(id) << ": " << CurReaderPartition << "-->" << NxtReaderPartition << std::endl;
#endif
                        i++;
                    }
                }
            }

            while(!NoEvents())
                HandleNextEvent();

            return;
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            DataSampleSet *pSampleSet = reinterpret_cast<MediaInputEvent<DataSampleSet*>*>(pEvent)->GetData();

            for(int i=0; i<pSampleSet->pSeqData->length(); i++)
            {
                if((*pSampleSet->pSeqInfo)[i].valid_data)
                {
                    int num = *(reinterpret_cast<int*>((*pSampleSet->pSeqData)[i].payload.get_contiguous_buffer()));
                    pOutputObj->Write(num);
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

        StdOutSink(int idP,int epIdP,int ownerIdP,int sessionIdP,DDSDomainParticipant *pOwnerDPP,DDSTopic *pTopicP,const char *layerRegExp):
        SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_STDOUTSINK),EventHandlerT<StdOutSink>(),ThreadSingle()
        {
            std::string ReaderRegExp = ToString<int>(epIdP) + "/" + layerRegExp;

            epId = epIdP;
            pInputObj = new DDSInputObject(idP,this,pOwnerDPP,pTopicP);
            pOutputObj = new StdOutOutputObject(idP);
            AddHandleFunc(&StdOutSink::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            pInputObj->AddLayerReader(ReaderRegExp.c_str());
        }

        ~StdOutSink()
        {
            delete pInputObj;
            delete pOutputObj;
        }
};

#endif // STDOUTSINK_H_
