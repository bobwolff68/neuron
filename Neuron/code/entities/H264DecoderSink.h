#ifndef H264DECODERSINK_H_
#define H264DECODERSINK_H_

#include "entity.h"
#include "H264BufferParser.h"
#include "DDSInputObject.h"
#include "H264DecoderOutputObject.h"

class H264DecoderSink : public SessionEntity,public EventHandlerT<H264DecoderSink>,public ThreadSingle
{
    private:

        int                         epId;
        int                         srcId;          //Id of relay proxy, if present, of src epId.
        int                         bSEIWritten;
        int                         bSPSWritten;
        int                         bPPSWritten;
        int                         prevLayerULimit;
        int                         curLayerULimit;
        int                         curFrameType;
        int                         count;
        std::string                 DecInFifoName;
        std::string                 curLayerPartition;
        ReorderMap                 *pReorderMap;
        H264BufferParser           *pParser;
        DDSInputObject             *pInputObj;
        H264DecoderOutputObject    *pOutputObj;

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
            int spsIdxMap[3] = {SPSQ_STREAM_INDEX,SPSH_STREAM_INDEX,SPSF_STREAM_INDEX};
            const char  *frmTypes[5] = {"i","I","P","B","b"};
            map<long,ReorderMapElement*> OrderedSamples;

            if(pOutputObj==NULL)
                pOutputObj = new H264DecoderOutputObject(id,DecInFifoName.c_str());

            pReorderMap->InsertSampleSet(pSampleSet);
            pReorderMap->GetOrderedSamples(OrderedSamples);
            
            while(!OrderedSamples.empty())
            {
                com::xvd::neuron::media::DataUnit *pSample = OrderedSamples.begin()->second->pSample;
                unsigned char  *pBuf = reinterpret_cast<unsigned char *>((*pSample).payload.get_contiguous_buffer());
                int             bufSize = (int) (*pSample).payload.length();

                if(!pParser->ParseBuffer(pBuf,bufSize,0))   return;

                if(pParser->type==X264_TYPE_IDR || pParser->type==X264_TYPE_I || pParser->type==X264_TYPE_P)
                {
                    //If IDR or I frame, write SEI header if not written
                    if(pParser->type==X264_TYPE_IDR || pParser->type==X264_TYPE_I)
                    {
                        if(!bSEIWritten)
                        {
                            if(pParser->vh.streamBuf[SEI_STREAM_INDEX]!=NULL)
                            {
                                std::cout << "SEI Header (Size: " << pParser->vh.size[SEI_STREAM_INDEX] << ")" << std::endl;
                                pOutputObj->Write(pParser->vh.streamBuf[SEI_STREAM_INDEX],pParser->vh.size[SEI_STREAM_INDEX]);
                                bSEIWritten = true;
                            }
                        }
                    }

                    //Determine start of a new GOP by comparing frame types, and if true,
                    //send sps if frame rate change requested
                    if(pParser->type<=curFrameType)
                    {
                        //std::cout << "1" << std::endl;
                        if(!bSPSWritten || prevLayerULimit!=curLayerULimit)
                        {
                            if(prevLayerULimit!=curLayerULimit)
                            {
                                std::string newLayerPartition;
                                prevLayerULimit = curLayerULimit;
                                newLayerPartition = ToString<int>(srcId) + "/[0-" + ToString<int>(curLayerULimit) + "]";
                                pInputObj->SetLayerReaderPartition(curLayerPartition.c_str(),newLayerPartition.c_str());
                                curLayerPartition = newLayerPartition;
                            }

                            if(pParser->vh.streamBuf[spsIdxMap[curLayerULimit]]!=NULL)
                            {
                                std::cout << "SPS Header (Index: " << spsIdxMap[curLayerULimit] << ", Size: " 
                                          << pParser->vh.size[spsIdxMap[curLayerULimit]] << ")" << std::endl;
                                pOutputObj->Write(pParser->vh.streamBuf[spsIdxMap[curLayerULimit]],pParser->vh.size[spsIdxMap[curLayerULimit]]);
                                bSPSWritten = true;
                            }
                        }
                    }

                    //If IDR or I frame, write PPS header if not written
                    if(pParser->type==X264_TYPE_IDR || pParser->type==X264_TYPE_I)
                    {
                        if(!bPPSWritten)
                        {
                            if(pParser->vh.streamBuf[PPS_STREAM_INDEX]!=NULL)
                            {
                                std::cout << "PPS Header (Size: " << pParser->vh.size[PPS_STREAM_INDEX] << ")" << std::endl;
                                pOutputObj->Write(pParser->vh.streamBuf[PPS_STREAM_INDEX],pParser->vh.size[PPS_STREAM_INDEX]);
                                bPPSWritten = true;
                            }
                        }
                    }

                    //Write frame
                    if(bSEIWritten && bSPSWritten && bPPSWritten)
                    {
                        //std::cout << (*pSampleSet->pSeqData)[i].seqNum << " :\t" << frmTypes[pParser->type-1] << " :\t" << bufSize << std::endl;
                        pOutputObj->Write(pBuf,bufSize);
                        curFrameType = pParser->type;
                    }
                }
                else if(pParser->type==X264_TYPE_BREF || pParser->type==X264_TYPE_B)
                {
                    //Write frame
                    if(bSEIWritten && bSPSWritten && bPPSWritten)
                    {
                        //std::cout << (*pSampleSet->pSeqData)[i].seqNum << " :\t" << frmTypes[pParser->type-1] << " :\t" << bufSize << std::endl;
                        pOutputObj->Write(pBuf,bufSize);
                        curFrameType = pParser->type;
                    }
                }

                delete OrderedSamples.begin()->second;
                OrderedSamples.erase(OrderedSamples.begin());
            }

            return;
        }

        //New sample of upline entity's info
        void HandleEntInfoInputEvent(Event *pEvent)
        {
        	UplineEntityInfo = reinterpret_cast<EntInfoInputEvent*>(pEvent)->GetEntInfo();
        	pInfo->hopsFromTrueSource = UplineEntityInfo.hopsFromTrueSource + 1;
        	pInfo->uplineSourceId = UplineEntityInfo.entityId;
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

        H264DecoderSink(int idP,int epIdP,int srcIdP,int ownerIdP,int sessionIdP,const char *decInFifoNameP,
                        DDSDomainParticipant *pOwnerDPP,DDSTopic *pTopicP,const char *layerRegExp):
        SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_H264DECODERSINK),EventHandlerT<H264DecoderSink>(),ThreadSingle()
        {
            epId = epIdP;
            srcId = srcIdP;
            curFrameType = X264_TYPE_B;
            prevLayerULimit = 0;
            curLayerULimit = 2;
            DecInFifoName = decInFifoNameP;
            curLayerPartition = ToString<int>(srcId)+"/"+layerRegExp;
            bSEIWritten = bSPSWritten = bPPSWritten = false;
            pParser = new H264BufferParser();
            pParser->isOwnerRP = false;
            pInputObj = new DDSInputObject(idP,this,pOwnerDPP,pTopicP);
            AddHandleFunc(&H264DecoderSink::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            AddHandleFunc(&H264DecoderSink::HandleEntInfoInputEvent,ENTINFO_INPUT_EVENT);
            AddHandleFunc(&H264DecoderSink::HandleUplineEntLostEvent,UPLINE_ENTITY_LOST_EVENT);
            AddHandleFunc(&H264DecoderSink::HandleUplineEntShutdownEvent,UPLINE_ENTITY_SHUTDOWN_EVENT);
            StartupEntInfoSub(this,srcId,epId);
            pInputObj->AddLayerReader(curLayerPartition.c_str());
            pOutputObj = NULL;
            pReorderMap = new ReorderMap();
        }

        ~H264DecoderSink()
        {
        	ShutdownEntInfoSub();
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

        int GetEpSrcId(void)
        {
            return epId;
        }

        void SetSubLayerULimit(int layerULimit)
        {
            curLayerULimit = layerULimit;
            return;
        }

        int GetTimesParsed(void)
        {
            return pParser->timesParsed;
        }
};


#endif // H264DECODERSINK_H_
