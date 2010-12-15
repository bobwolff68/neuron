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
        int                         bSEIWritten;
        int                         bSPSWritten;
        int                         bPPSWritten;
        int                         prevLayerULimit;
        int                         curLayerULimit;
        int                         curFrameType;
        int                         count;
        const char                 *decInFifoName;
        std::string                 curLayerPartition;
        H264BufferParser           *pParser;
        DDSInputObject             *pInputObj;
        H264DecoderOutputObject    *pOutputObj;

        void EventHandleLoop(void)
        {
            const char *layerPartitions[3] = {"*","[0-1]","0"};
            const int   layerULimit[3] = {2,1,0};

            while(!isStopRequested)
            {
                if(HandleNextEvent())
                {
                    count++;
                    if(count%1000==0)
                    {
                        int i = count/1000;
                        std::cout << layerPartitions[(i-1)%3] << " ---> " << layerPartitions[i%3] << std::endl;
                        pInputObj->SetLayerReaderPartition((ToString<int>(epId)+"/"+layerPartitions[(i-1)%3]).c_str(),
                                                           (ToString<int>(epId)+"/"+layerPartitions[i%3]).c_str());

                        curLayerULimit = layerULimit[i%3];
                        curLayerPartition = ToString<int>(epId)+"/"+layerPartitions[i%3];
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
            int spsIdxMap[3] = {SPSQ_STREAM_INDEX,SPSH_STREAM_INDEX,SPSF_STREAM_INDEX};
            const char  *frmTypes[5] = {"i","I","P","B","b"};

            if(pOutputObj==NULL)
                pOutputObj = new H264DecoderOutputObject(id,decInFifoName);

            for(int i=0; i<pSampleSet->pSeqData->length(); i++)
            {
                if((*pSampleSet->pSeqInfo)[i].valid_data)
                {
                    unsigned char  *pBuf = reinterpret_cast<unsigned char *>((*pSampleSet->pSeqData)[i].payload.get_contiguous_buffer());
                    int             bufSize = (int) (*pSampleSet->pSeqData)[i].payload.length();

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
                                //std::cout << "2" << std::endl;
                                prevLayerULimit = curLayerULimit;
                                if(pParser->vh.streamBuf[spsIdxMap[curLayerULimit]]!=NULL)
                                {
                                    std::cout << "SPS Header (Index: " << spsIdxMap[curLayerULimit] << ", Size: " << pParser->vh.size[spsIdxMap[curLayerULimit]] << ")" << std::endl;
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

        H264DecoderSink(int idP,int epIdP,int ownerIdP,int sessionIdP,const char *decInFifoNameP,
                        DDSDomainParticipant *pOwnerDPP,DDSTopic *pTopicP,const char *layerRegExp):
        SessionEntity(pOwnerDPP,idP,ownerIdP,sessionIdP,ENTITY_KIND_H264DECODERSINK),EventHandlerT<H264DecoderSink>(),ThreadSingle()
        {
            epId = epIdP;
            curFrameType = X264_TYPE_B;
            prevLayerULimit = 0;
            curLayerULimit = 2;
            curLayerPartition = ToString<int>(epId)+"/"+layerRegExp;
            bSEIWritten = bSPSWritten = bPPSWritten = false;
            pParser = new H264BufferParser();
            pInputObj = new DDSInputObject(idP,this,pOwnerDPP,pTopicP);
            AddHandleFunc(&H264DecoderSink::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            pInputObj->AddLayerReader(curLayerPartition.c_str());
            decInFifoName = decInFifoNameP;
            pOutputObj = NULL;
        }

        ~H264DecoderSink()
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


#endif // H264DECODERSINK_H_
