#ifndef ENTITY_H_
#define ENTITY_H_

#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "entityinfo.h"
#include "entityinfoSupport.h"
#include "MediaEvent.h"

#define ENTITY_KIND_NATNUMSRC       0
#define ENTITY_KIND_STDOUTSINK      1
#define ENTITY_KIND_RELAYPROXY      2
#define ENTITY_KIND_H264FILESRC     3
#define ENTITY_KIND_H264DECODERSINK 4
#define ENTITY_KIND_SAMPLEQUEUESINK 5

#define MEDIA_TOPIC_NAME(topicStr,prefix,sessionId)\
        {\
            sprintf(topicStr,"%s",prefix);\
            strcat(topicStr,ToString<int>(sessionId).c_str());\
        }

class EntityInfoListener : public DDSDataReaderListener
{
	private:

		EventHandler	*pOwnerEventHandler;

	public:

		EntityInfoListener(EventHandler *pOwnerEventHandlerP)
		{
			pOwnerEventHandler = pOwnerEventHandlerP;
		}

		~EntityInfoListener()
		{
		}

		void on_data_available(DDSDataReader *pGenericReader)
		{
			DDS_ReturnCode_t								retCode;
			DDS_SampleInfoSeq								seqInfo;
			com::xvd::neuron::media::EntityInfoSeq			seqEntInfo;
			com::xvd::neuron::media::EntityInfoDataReader  *pEntInfoReader = NULL;

            //Obtain the specific type reader from generic reader
            pEntInfoReader = com::xvd::neuron::media::EntityInfoDataReader::narrow(pGenericReader);

            retCode = pEntInfoReader->read(seqEntInfo,seqInfo,DDS_LENGTH_UNLIMITED,DDS_ANY_SAMPLE_STATE,DDS_ANY_VIEW_STATE,DDS_ANY_INSTANCE_STATE);
            if(retCode!=DDS_RETCODE_NO_DATA)
            {
            	if(retCode!=DDS_RETCODE_OK)
            	{
            		cout << "EntityInfoListener::on_data_available(): Error in pEntInfoReader->read()" << endl;
            		return;
            	}

            	for(int i=0; i<seqEntInfo.length(); i++)
            	{
            		if(seqInfo[i].valid_data)
            		{
            			com::xvd::neuron::media::EntityInfoTypeSupport::print_data(&seqEntInfo[i]);
            			EntInfoInputEvent *pEvent = new EntInfoInputEvent(seqEntInfo[i]);
            			pOwnerEventHandler->SignalEvent(pEvent);
            		}
            		else
            		{
            			if(seqInfo[i].instance_state==DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)
            			{
            				UplineEntLostEvent *pEvent = new UplineEntLostEvent();
            				pOwnerEventHandler->SignalEvent(pEvent);
            			}
            			else if(seqInfo[i].instance_state==DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE)
            			{
            				UplineEntShutdownEvent *pEvent = new UplineEntShutdownEvent();
            				pOwnerEventHandler->SignalEvent(pEvent);
            			}
            		}
            	}
            }
            pEntInfoReader->return_loan(seqEntInfo,seqInfo);
		}

};

class SessionEntity
{
    protected:

        int id;
        int ownerId;
        int sessionId;
        int kind;
        DDSDomainParticipant *pOwnerDP;

        //Entity Info DDS Entities
        DDSTopic		*pEntInfoTopic;
        DDSPublisher 	*pEntInfoPub;
        DDSDataWriter	*pEntInfoWriter;
        DDSSubscriber	*pEntInfoSub;
        DDSDataReader	*pEntInfoReader;

        com::xvd::neuron::media::EntityInfo	   		   *pInfo;
        com::xvd::neuron::media::EntityInfo				UplineEntityInfo;
        DDS_InstanceHandle_t							EntInfoInstanceHandle;
        map<int,com::xvd::neuron::media::EntityInfo*>	EntitiesInSession;

        void SetEntInfoSubPartition(string &Partition)
        {
        	DDS_SubscriberQos	subQos;
        	DDS_ReturnCode_t	retCode;

        	retCode = pEntInfoSub->get_qos(subQos);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity::StartupEntInfoSub(): Error in pEntInfoSub->get_qos()" << endl;
        		exit(0);
        	}
            subQos.partition.name.ensure_length(1,1);
            subQos.partition.name[0] = DDS_String_dup(Partition.c_str());
            retCode = pEntInfoSub->set_qos(subQos);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity::StartupEntInfoSub(): Error in pEntInfoSub->set_qos()" << endl;
        		exit(0);
        	}

        	return;
        }

        void SetEntInfoPubPartition(string &Partition)
        {
        	DDS_PublisherQos	pubQos;
        	DDS_ReturnCode_t	retCode;

        	retCode = pEntInfoPub->get_qos(pubQos);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity::StartupEntInfoPub(): Error in pEntInfoPub->get_qos()" << endl;
        		exit(0);
        	}
            pubQos.partition.name.ensure_length(1,1);
            pubQos.partition.name[0] = DDS_String_dup(Partition.c_str());
            retCode = pEntInfoPub->set_qos(pubQos);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity::StartupEntInfoSub(): Error in pEntInfoPub->set_qos()" << endl;
        		exit(0);
        	}

        	return;
        }

        void StartupEntInfoSub(EventHandler *pOwnerEventHandlerP,int uplineSourceId,int trueSourceId)
        {
        	DDS_ReturnCode_t	retCode;
        	string				Partition("");
        	EntityInfoListener *pListener = NULL;

        	pInfo->uplineSourceId = uplineSourceId;
        	pInfo->trueSourceId = trueSourceId;

        	pEntInfoSub = pOwnerDP->create_subscriber(DDS_SUBSCRIBER_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
        	if(pEntInfoSub==NULL)
        	{
        		cout << "SessionEntity::StartupEntInfoSub(): Error in pOwnerDP->create_subscriber()" << endl;
        		exit(0);
        	}

        	//Subscribe to the up-line source's entity info
        	Partition = ToString<int>(trueSourceId)+"/";
        	Partition += ToString<int>(uplineSourceId);
        	SetEntInfoSubPartition(Partition);

        	//Create data reader for entity info samples
        	pListener = new EntityInfoListener(pOwnerEventHandlerP);
        	pEntInfoReader = pEntInfoSub->create_datareader_with_profile(pEntInfoTopic,"NEURON","ENTITYINFO",pListener,DDS_STATUS_MASK_ALL);
        	if(pEntInfoReader==NULL)
        	{
        		cout << "SessionEntity::StartupEntInfoSub(): Error in pEntInfoSub->create_datareader_with_profile()" << endl;
        		exit(0);
        	}

        	return;
        }

        void ShutdownEntInfoSub(void)
        {
        	DDS_ReturnCode_t	retCode;

        	retCode = pEntInfoSub->delete_contained_entities();
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "~SessionEntity()::ShutdownEntInfoSub(): Error in pEntInfoSub->delete_contained_entities()" << endl;
        		exit(0);
        	}

        	retCode = pOwnerDP->delete_subscriber(pEntInfoSub);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity()::ShutdownEntInfoSub(): Error in pOwnerDP->delete_subscriber()" << endl;
        		exit(0);
        	}

        	return;
        }

        void StartupEntInfoPub(void)
        {
        	DDS_ReturnCode_t								retCode;
        	string											Partition("");
        	com::xvd::neuron::media::EntityInfoDataWriter  *pWriter = NULL;

        	pEntInfoPub = pOwnerDP->create_publisher(DDS_PUBLISHER_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
        	if(pEntInfoPub==NULL)
        	{
        		cout << "SessionEntity::StartupEntInfoPub(): Error in pOwnerDP->create_publisher()" << endl;
        		exit(0);
        	}

        	Partition = ToString<int>(pInfo->trueSourceId)+"/";
        	Partition += ToString<int>(id);
        	SetEntInfoPubPartition(Partition);

        	//Create data writer for entity info samples
        	pEntInfoWriter = pEntInfoPub->create_datawriter_with_profile(pEntInfoTopic,"NEURON","ENTITYINFO",NULL,DDS_STATUS_MASK_NONE);
        	if(pEntInfoWriter==NULL)
        	{
        		cout << "SessionEntity::StartupEntInfoPub(): Error in pEntInfoPub->create_datawriter_with_profile()" << endl;
        		exit(0);
        	}

        	//Tell reader to ignore local writer
        	pOwnerDP->ignore_publication(pEntInfoWriter->get_instance_handle());

        	//Register EntityInfo instance
        	pWriter = com::xvd::neuron::media::EntityInfoDataWriter::narrow(pEntInfoWriter);
        	EntInfoInstanceHandle = pWriter->register_instance(*pInfo);
        	if(DDS_InstanceHandle_is_nil(&EntInfoInstanceHandle))
        	{
        		cout << "SessionEntity::StartupEntInfoPub(): Error in pWriter->register_instance()" << endl;
        		exit(0);
        	}

        	return;
        }

        void ShutdownEntInfoPub(void)
        {
        	DDS_ReturnCode_t								retCode;
        	com::xvd::neuron::media::EntityInfoDataWriter  *pWriter = NULL;

        	pWriter = com::xvd::neuron::media::EntityInfoDataWriter::narrow(pEntInfoWriter);
        	retCode = pWriter->get_key_value(*pInfo,EntInfoInstanceHandle);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity()::ShutdownEntInfoPub(): Error in pWriter->get_key_value()" << endl;
        		exit(0);
        	}
        	retCode = pWriter->dispose(*pInfo,EntInfoInstanceHandle);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity()::ShutdownEntInfoPub(): Error in pWriter->dispose()" << endl;
        		exit(0);
        	}

        	retCode = pEntInfoPub->delete_contained_entities();
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity()::ShutdownEntInfoPub(): Error in pEntInfoPub->delete_contained_entities()" << endl;
        		exit(0);
        	}

        	retCode = pOwnerDP->delete_publisher(pEntInfoPub);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity()::ShutdownEntInfoPub(): Error in pOwnerDP->delete_publisher()" << endl;
        		exit(0);
        	}

        	return;
        }

        void PublishEntityInfo(void)
        {
        	DDS_ReturnCode_t								retCode;
        	com::xvd::neuron::media::EntityInfoDataWriter  *pWriter = NULL;

        	pWriter = com::xvd::neuron::media::EntityInfoDataWriter::narrow(pEntInfoWriter);
        	retCode = pWriter->write(*pInfo,EntInfoInstanceHandle);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "SessionEntity::PublishEntityInfo(): Error in pWriter->write()" << endl;
        		exit(0);
        	}

        	return;
        }

    public:

        SessionEntity(DDSDomainParticipant *pOwnerDPP,int idP,int ownerIdP,int sessionIdP,int kindP)
        {
        	char entInfoTopicName[50];

            id = idP;
            pOwnerDP = pOwnerDPP;
            ownerId = ownerIdP;
            sessionId = sessionIdP;
            kind = kindP;

            pEntInfoPub = NULL;
            pEntInfoWriter = NULL;
            pEntInfoSub = NULL;
            pEntInfoReader = NULL;
            pInfo = com::xvd::neuron::media::EntityInfoTypeSupport::create_data();
            pInfo->entityId = id;
            MEDIA_TOPIC_NAME(entInfoTopicName,"entinfo_",sessionId);
            pEntInfoTopic = pOwnerDP->find_topic(entInfoTopicName,DDS_DURATION_INFINITE);
        }

        ~SessionEntity()
        {
        	DDS_ReturnCode_t	retCode;

        	retCode = pOwnerDP->delete_topic(pEntInfoTopic);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "~SessionEntity(): Error in pOwnerDP->delete_topic()" << endl;
        		exit(0);
        	}

        	retCode = com::xvd::neuron::media::EntityInfoTypeSupport::delete_data(pInfo);
        	if(retCode!=DDS_RETCODE_OK)
        	{
        		cout << "~SessionEntity(): Error in EntityInfoTypeSupport::delete_data()" << endl;
        		exit(0);
        	}
        }

        bool AddPeer(const char *peerIP[],int srcEntityId)
        {
            DDS_ReturnCode_t retCode;

            if(id==srcEntityId)
            {
                retCode = pOwnerDP->add_peer(peerIP[1]);
                if(retCode!=DDS_RETCODE_OK)
                {
                    std::cout << "Cannot add to peer list: " << peerIP[1] << std::endl;
                    return false;
                }
            }
            else
            {
                retCode = pOwnerDP->add_peer(peerIP[0]);
                if(retCode!=DDS_RETCODE_OK)
                {
                    std::cout << "Cannot add to peer list: " << peerIP[0] << std::endl;
                    return false;
                }
            }

            return true;
        }

        int GetKind(void)
        {
            return kind;
        }

        int GetId(void)
        {
            return id;
        }

        com::xvd::neuron::media::EntityInfo &GetUplineEntityInfo(void)
        {
        	return UplineEntityInfo;
        }

        bool GetUplineEntityInfo(int uplineEntityId)
        {
        	com::xvd::neuron::media::EntityInfo 			EntInfo;
        	com::xvd::neuron::media::EntityInfoDataReader  *pReader = NULL;
        	DDS_InstanceHandle_t							InstHandle;

        	if(uplineEntityId>-1)
        	{
				pReader = com::xvd::neuron::media::EntityInfoDataReader::narrow(pEntInfoReader);
				EntInfo.entityId = uplineEntityId;
				InstHandle = pReader->lookup_instance(EntInfo);
				if(!DDS_InstanceHandle_is_nil(&InstHandle))
				{
					com::xvd::neuron::media::EntityInfoSeq	seqEntInfo;
					DDS_SampleInfoSeq						seqSampInfo;
					DDS_ReturnCode_t						retCode;

					//Reader has history.depth = 1
					retCode = pReader->read_instance(seqEntInfo,seqSampInfo,(DDS_Long)1,InstHandle,
													 DDS_ANY_SAMPLE_STATE,DDS_ANY_VIEW_STATE,
													 DDS_ANY_INSTANCE_STATE);
					if(retCode!=DDS_RETCODE_NO_DATA)
					{
						if(retCode!=DDS_RETCODE_OK)		return false;
						if(!seqSampInfo[0].valid_data)	return false;
						UplineEntityInfo = seqEntInfo[0];
						pReader->return_loan(seqEntInfo,seqSampInfo);
						return true;
					}

					pReader->return_loan(seqEntInfo,seqSampInfo);
				}
        	}
        	return false;
        }
};

#endif // ENTITY_H_
