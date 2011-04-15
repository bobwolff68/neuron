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
        int                 srcId;
        int                 nLayers;
        std::string         curLayerPartition;
        ReorderMap         *pReorderMap;
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

        void DetermineSamplePartition(string &Partition,com::xvd::neuron::media::DataUnit *pSample)
        {
            int             bufSize = (int) (*pSample).payload.length();
            unsigned char  *pBuf = reinterpret_cast<unsigned char *>((*pSample).payload.get_contiguous_buffer());

            if(!pParser->ParseBuffer(pBuf,bufSize,0))   return;
            Partition = ToString<int>(id)+"/"+ToString<int>(LAYER_TYPE(pParser->type));
            return;
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            DataSampleSet                  *pSampleSet = reinterpret_cast<MediaInputEvent<DataSampleSet*>*>(pEvent)->GetData();
            map<long,ReorderMapElement*>    OrderedSamples;
            string                          Partition;

            pReorderMap->InsertSampleSet(pSampleSet);
            pReorderMap->GetOrderedSamples(OrderedSamples);
            while(!OrderedSamples.empty())
            {
                com::xvd::neuron::media::DataUnit *pSample = OrderedSamples.begin()->second->pSample;
                DetermineSamplePartition(Partition,pSample);
                pOutputObj->Write(Partition.c_str(),*pSample);
                delete OrderedSamples.begin()->second;
                OrderedSamples.erase(OrderedSamples.begin());
            }

            //delete pSampleSet;
            return;
        }

        //New sample of upline entity's info
        void HandleEntInfoInputEvent(Event *pEvent)
        {
        	UplineEntityInfo = reinterpret_cast<EntInfoInputEvent*>(pEvent)->GetEntInfo();
        	pInfo->hopsFromTrueSource = UplineEntityInfo.hopsFromTrueSource + 1;
        	pInfo->uplineSourceId = UplineEntityInfo.entityId;
        	PublishEntityInfo();
        	return;
        }

        void HandleUplineEntLostEvent(Event *pEvent)
        {
        	cout << "Upline Entity(Id: " << pInfo->uplineSourceId << ") lost..." << endl;
        	cout << "Trying to connect to Entity(Id: " << UplineEntityInfo.uplineSourceId << ")..."<< endl;
        	UpdateVideoSource(UplineEntityInfo.uplineSourceId);
        	return;
        }

        void HandleUplineEntShutdownEvent(Event *pEvent)
        {
        	cout << "Upline Entity(Id: " << pInfo->uplineSourceId << ") shut down..." << endl;
        	cout << "Trying to connect to Entity(Id: " << UplineEntityInfo.uplineSourceId << ")..."<< endl;
        	UpdateVideoSource(UplineEntityInfo.uplineSourceId);
        	return;
        }

        int workerBee(void)
        {
            EventHandleLoop();
            return 0;
        }

    public:

        RelayProxy(int idP,int epIdP,int srcIdP,int ownerIdP,int sessionIdP,DDSDomainParticipant *pOwnerDPP,DDSTopic *pTopicP,int nLayersP,const char *layerRegExp):
        EventHandlerT<RelayProxy>(),SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_RELAYPROXY),ThreadSingle()
        {
            epId = epIdP;
            srcId = srcIdP;
            nLayers = nLayersP;
            curLayerPartition = ToString<int>(srcId)+"/"+layerRegExp;
            AddHandleFunc(&RelayProxy::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            AddHandleFunc(&RelayProxy::HandleEntInfoInputEvent,ENTINFO_INPUT_EVENT);
            AddHandleFunc(&RelayProxy::HandleUplineEntLostEvent,UPLINE_ENTITY_LOST_EVENT);
            AddHandleFunc(&RelayProxy::HandleUplineEntShutdownEvent,UPLINE_ENTITY_SHUTDOWN_EVENT);
            StartupEntInfoSub(this,srcId,epId);
            StartupEntInfoPub();
            pParser = new H264BufferParser();
            pInputObj = new DDSInputObject(id,this,pOwnerDP,pTopicP);
            pInputObj->AddLayerReader(curLayerPartition.c_str());
            pOutputObj = new DDSOutputObject(id,pOwnerDP,pTopicP);
            pReorderMap = new ReorderMap();
            for(int i=0; i<nLayers; i++)
                pOutputObj->AddLayerWriter((ToString<int>(id)+"/"+ToString<int>(i)).c_str());
        }

        ~RelayProxy()
        {
        	ShutdownEntInfoSub();
        	ShutdownEntInfoPub();
            delete pReorderMap;
            delete pParser;
            delete pInputObj;
            delete pOutputObj;
        }

        void UpdateVideoSource(int newSrcId)
        {
        	string	Partition("");

        	pInfo->uplineSourceId = newSrcId;
        	Partition = ToString<int>(pInfo->trueSourceId) + "/";
        	Partition += ToString<int>(pInfo->uplineSourceId);
        	SetEntInfoSubPartition(Partition);

            srcId = newSrcId;
            std::string LayerRegExp = curLayerPartition.substr(curLayerPartition.find('/'));
            pInputObj->SetLayerReaderPartition(curLayerPartition.c_str(),(ToString<int>(srcId)+LayerRegExp).c_str());
            curLayerPartition = (ToString<int>(srcId)+LayerRegExp).c_str();
            std::cout << "Partition changed to: " << curLayerPartition << std::endl;
            return;
        }
};

#endif // RELAYPROXY_H_
