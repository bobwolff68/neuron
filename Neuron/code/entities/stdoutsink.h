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
        ReorderMap         *pReorderMap;

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
                    /*if(count%10==0)
                    {
                        std::string CurReaderPartition = ToString<int>(epId) + "/" + layerPartitions[(i-1)%4];
                        std::string NxtReaderPartition = ToString<int>(epId) + "/" + layerPartitions[i%4];
                        pInputObj->SetLayerReaderPartition(CurReaderPartition.c_str(),NxtReaderPartition.c_str());
#ifdef VERBOSE_OUTPUT
                        std::cout << MOO_LOG_PROMPT(id) << ": " << CurReaderPartition << "-->" << NxtReaderPartition << std::endl;
#endif
                        i++;
                    }*/
                }
            }

            while(!NoEvents())
                HandleNextEvent();

            return;
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            DataSampleSet *pSampleSet = reinterpret_cast<MediaInputEvent<DataSampleSet*>*>(pEvent)->GetData();
            map<long,ReorderMapElement*> OrderedSamples;

            pReorderMap->InsertSampleSet(pSampleSet);
            pReorderMap->GetOrderedSamples(OrderedSamples);
            while(!OrderedSamples.empty())
            {
                int num = *(reinterpret_cast<int*>((*(OrderedSamples.begin()->second->pSample)).payload.get_contiguous_buffer()));
                int seqNum = (*(OrderedSamples.begin()->second->pSample)).seqNum;
                
                cout << "seqNum: " << seqNum << " ";
                pOutputObj->Write(num);
                delete OrderedSamples.begin()->second;
                OrderedSamples.erase(OrderedSamples.begin());
            }

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
            pReorderMap = new ReorderMap();
            AddHandleFunc(&StdOutSink::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            pInputObj->AddLayerReader(ReaderRegExp.c_str());
        }

        ~StdOutSink()
        {
            delete pReorderMap;
            delete pInputObj;
            delete pOutputObj;
        }
};

#endif // STDOUTSINK_H_
