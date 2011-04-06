#ifndef DDSINPUTOBJECT_H_
#define DDSINPUTOBJECT_H_

#include "MediaInputObject.h"
#include "MediaReader.h"

class ReorderMapElement
{
    public:
    
        com::xvd::neuron::media::DataUnit   *pSample;
        DataSampleSet                       *pSampleSet;
        
        ReorderMapElement(com::xvd::neuron::media::DataUnit *pSampleP,DataSampleSet *pSampleSetP):
        pSample(pSampleP),pSampleSet(pSampleSetP)
        {
        }
        
        ~ReorderMapElement()
        {
            (pSampleSet->nSamplesToBeWritten)--;
            if(pSampleSet->nSamplesToBeWritten<=0)
            {
                //cout << "Deleting sample set..." << endl;
                delete pSampleSet;
            }
        }
};

class ReorderMap
{
    private:
    
        map<long,ReorderMapElement*>    SampleMap;
        long                            latestSeqNum;
        
    public:
    
        ReorderMap(): latestSeqNum(-1)
        {
        }
        
        ~ReorderMap()
        {
            map<long,ReorderMapElement*>::iterator  it;
            
            for(it=SampleMap.begin(); it!=SampleMap.end(); it++)
                delete it->second;
                
            SampleMap.clear();
        }
        
        void InsertSampleSet(DataSampleSet *pSampleSet)
        {
            for(int i=0; i<pSampleSet->pSeqData->length(); i++)
                if((*pSampleSet->pSeqInfo)[i].valid_data)
                {
                    SampleMap[(*pSampleSet->pSeqData)[i].seqNum] = new ReorderMapElement(
                                                                &((*pSampleSet->pSeqData)[i]),
                                                                pSampleSet);
                }
                
            if(latestSeqNum==-1)    
                latestSeqNum = (SampleMap.begin()->first)-1;
        }
        
        void GetOrderedSamples(map<long,ReorderMapElement*> &SmpMap)
        {
            SmpMap.clear();           
            while(!SampleMap.empty() && (SampleMap.begin()->first-latestSeqNum)==1)
            {
                latestSeqNum = SampleMap.begin()->first;
                SmpMap[latestSeqNum] = SampleMap[latestSeqNum];
                SampleMap.erase(SampleMap.begin());
            }
            
            if(SampleMap.size()>=5)
            {
                while(!SampleMap.empty())
                {
                    latestSeqNum = SampleMap.begin()->first;
                    SmpMap[latestSeqNum] = SampleMap[latestSeqNum];
                    SampleMap.erase(SampleMap.begin());
                }    
            }
            
            return;
        }
};

class DDSInputObject : public MediaInputObject
{
    private:

        MediaReader    *pMediaReader;

    public:

        DDSInputObject(int                      ownerIdP,
                       EventHandler            *pOwnerEventHandlerP,
                       DDSDomainParticipant    *pOwnerDPP,
                       DDSTopic                *pTopic):
        MediaInputObject(pOwnerEventHandler,ownerIdP)
        {
            pMediaReader = new MediaReader(pOwnerEventHandlerP,pOwnerDPP,pTopic);
        }

        ~DDSInputObject()
        {
            delete pMediaReader;
        }

        void StreamMedia(void)
        {
            //Listener thread streams media, so explicit function not required
        }

        bool AddLayerReader(const char *layerPartitionRegExp)
        {
            return pMediaReader->AddLayerReader(layerPartitionRegExp);
        }

        bool SetLayerReaderPartition(const char *curRegExp,const char *newRegExp)
        {
            return pMediaReader->SetLayerReaderPartition(curRegExp,newRegExp);
        }
};

#endif // DDSINPUTOBJECT_H_
