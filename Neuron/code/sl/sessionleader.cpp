#include <iostream>
#include <new>
#include <unistd.h>
#include "sessionleader.h"

#ifndef ENDPOINT_SRC_PATH
#define ENDPOINT_SRC_PATH "/home/manjesh/Videos/"
#endif

SessionLeader::SessionLeader(IDType slIdParam,IDType sIdParam,const char *nameParam,int domIdParam,
							 int ownerIdParam) : EventHandlerT<SessionLeader>(),ThreadSingle()
{
	DDS_ReturnCode_t	retCode;
	char				LSCPSlaveName[100];
	char				topicName[50];
	const char 		   *typeName = NULL;
	
	id = slIdParam;
	sessionId = sIdParam;
	ownerId = ownerIdParam;
	strcpy(name,nameParam);
	
	// Create Local Session Control Plane (LSCP) slave
	GEN_CP_INTERFACE_NAME(LSCPSlaveName,name,LSCP_SLAVE_NAME);
	pLSCSlave = new LSCPSlave(this,id,domIdParam,LSCPSlaveName,"LSCP");
	pLSCSlaveObj = pLSCSlave->CreateSlaveObject(sessionId);
	
	// Create Entity Control Plane
	
	
	// Generate hash table of Event Handle Function Ptrs with Event kind as key
	AddHandleFunc(&SessionLeader::HandleNewSessionEvent,LSCP_EVENT_NEW_SESSION);
	AddHandleFunc(&SessionLeader::HandleUpdateSessionEvent,LSCP_EVENT_UPDATE_SESSION);
	AddHandleFunc(&SessionLeader::HandleDeleteSessionEvent,LSCP_EVENT_DELETE_SESSION);

    // Initialize LSCP instances
    control = com::xvd::neuron::lscp::ControlTypeSupport::create_data();
    state = com::xvd::neuron::lscp::StateTypeSupport::create_data();
    event = com::xvd::neuron::lscp::EventTypeSupport::create_data();
    metrics = com::xvd::neuron::lscp::MetricsTypeSupport::create_data();
    
    //Init media plane
    pMediaDP = DDSTheParticipantFactory->create_participant_with_profile(domIdParam,"NEURON","MEDIA",
    																	 NULL,DDS_STATUS_MASK_NONE);
    if(pMediaDP==NULL)
    {
    	std::cout << "Cannot create domain participant" << std::endl;
    	exit(0);
    }
    typeName = com::xvd::neuron::media::DataUnitTypeSupport::get_type_name();
    retCode = com::xvd::neuron::media::DataUnitTypeSupport::register_type(pMediaDP,typeName);
    if(retCode!=DDS_RETCODE_OK)
    {
        std::cout << "Cannot register type" << std::endl;
        retCode = DDSTheParticipantFactory->delete_participant(pMediaDP);
        if(retCode!=DDS_RETCODE_OK)
        	std::cout << "Cannot delete domain participant" << std::endl;
        exit(0);
    }
    
    //Create Session Video Topic
    MEDIA_TOPIC_NAME(topicName,"video_",sessionId);
	TopicList["video"] = pMediaDP->create_topic(topicName,com::xvd::neuron::media::DataUnitTypeSupport::get_type_name(),
												DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);

    SetStateStandby();
}

SessionLeader::~SessionLeader()
{
	DDS_ReturnCode_t	retCode;
	
	//Delete media plane
	std::map<int,SessionEntity*>::iterator it;
	for(it=EntityList.begin(); it!=EntityList.end(); it++)
	{
		switch(it->second->GetKind())
		{
			case ENTITY_KIND_NATNUMSRC:
				
				((NatNumSrc *)(it->second))->stopThread();
				delete (NatNumSrc *)(it->second);
				break;
				
			case ENTITY_KIND_STDOUTSINK:

				((StdOutSink *)(it->second))->stopThread();
				delete (StdOutSink *)(it->second);
				break;

			case ENTITY_KIND_RELAYPROXY:

				((RelayProxy *)(it->second))->stopThread();
				delete (RelayProxy *)(it->second);
				break;

			case ENTITY_KIND_H264FILESRC:
				
				((H264FileSrc *)(it->second))->stopThread();
				delete (H264FileSrc *)(it->second);
				break;

			case ENTITY_KIND_H264DECODERSINK:

				((H264DecoderSink *)(it->second))->stopThread();
				delete (H264DecoderSink *)(it->second);
				break;

			default:
			
				std::cout << "Entity kind " << it->second->GetKind() <<" not valid" << std::endl;
				break;
		}
		
		std::cout << "Deleted entity " << it->first << std::endl;
	}
	EntityList.clear();

	retCode = pMediaDP->delete_topic(TopicList["video"]);
	if(retCode!=DDS_RETCODE_OK)
	{
		std::cout << "Cannot delete topic" << std::endl;
		exit(0);
	}

	TopicList.clear();
	retCode = DDSTheParticipantFactory->delete_participant(pMediaDP);
	if(retCode!=DDS_RETCODE_OK)
	{
		std::cout << "Cannot delete domain participant" << std::endl;
		exit(0);
	}
	
	SetStateDeleted();
	pLSCSlave->DeleteSlaveObject(pLSCSlaveObj);
	delete pLSCSlave;

    // Delete LSCP instances
    com::xvd::neuron::lscp::ControlTypeSupport::delete_data(control);
    com::xvd::neuron::lscp::StateTypeSupport::delete_data(state);
    com::xvd::neuron::lscp::EventTypeSupport::delete_data(event);
    com::xvd::neuron::lscp::MetricsTypeSupport::delete_data(metrics);
}

void SessionLeader::HandleNewSessionEvent(Event *pEvent)
{
	LSCPEventNewSession 	*pNewSessEvt = NULL;
	LSCPSlaveObject			*pLSCSlaveObjLocal = NULL;
	
	pNewSessEvt = reinterpret_cast<LSCPEventNewSession*>(pEvent);
	pLSCSlaveObjLocal = (LSCPSlaveObject*)pNewSessEvt->GetSession();
	
	if(pLSCSlaveObj!=NULL)
	{
        if(pLSCSlaveObjLocal->GetSessionId()==sessionId)
        {
        	strcpy(control->script,pNewSessEvt->GetData()->script);
        	SetStateReady();
        }
	}
	else
	{
		std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": Null LSCP slave object" << endl;
		pLSCSlave->DeleteSlaveObject(pLSCSlaveObjLocal);
	}
	
	return;
}

void SessionLeader::HandleUpdateSessionEvent(Event *pEvent)
{
	LSCPEventUpdateSession	*pUpSessEvt = NULL;
	
	if(pLSCSlaveObj!=NULL)
	{
		pUpSessEvt = reinterpret_cast<LSCPEventUpdateSession*>(pEvent);
		if(pUpSessEvt->GetData()->sessionId==sessionId)
		{
			Update(pUpSessEvt->GetData());
			SetStateReady();
			ProcessScript(control->script);
		}
	}
	else
		std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": Null LSCP slave object" << endl;
	
	return;
}

void SessionLeader::HandleDeleteSessionEvent(Event *pEvent)
{
	std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": Deleting" << std::endl;
	//stopThread();
	return;
}

void SessionLeader::ProcessScript(const char *script)
{
	int 						cmdId;
	int 						entityId;
	int 						entityType;
	int 						srcId;
	int							resW;
	int 						resH;
	char						srcName[100];
	std::string					ScriptUnquoted;
	std::string					Operand;
	std::string					EntityType;
	std::stringstream			ScriptStream;
	std::map<std::string,int>	EntityTypeMap;
	
	//Populate entity type map
	EntityTypeMap["natnumsrc"] = ENTITY_KIND_NATNUMSRC;
	EntityTypeMap["stdoutsink"] = ENTITY_KIND_STDOUTSINK;
	EntityTypeMap["rp"] = ENTITY_KIND_RELAYPROXY;
	EntityTypeMap["vfsrc"] = ENTITY_KIND_H264FILESRC;
	EntityTypeMap["vdsink"] = ENTITY_KIND_H264DECODERSINK;
	
	//Remove quotes from script
	ScriptUnquoted = script;
	ScriptUnquoted = ScriptUnquoted.substr(1,ScriptUnquoted.length()-2);
	ScriptStream << ScriptUnquoted;
	
	//Get operand
	ScriptStream >> Operand;
	std::cout << Operand << std::endl;
	
	if(Operand=="add")
	{
		ScriptStream >> entityId;
		ScriptStream >> EntityType;
		entityType = EntityTypeMap[EntityType];
		std::cout << entityId << "," << entityType << std::endl;
		
		switch(entityType)
		{
			case ENTITY_KIND_NATNUMSRC:
			{
				NatNumSrc *pSrc = new NatNumSrc(entityId,id,sessionId,20,1,5,pMediaDP,TopicList["video"]);				
				pSrc->startThread();
				EntityList[entityId] = (SessionEntity *)(pSrc);
				std::cout << "Kind=" << EntityList[entityId]->GetKind() << std::endl;
				break;
			}
					
			case ENTITY_KIND_STDOUTSINK:
			{	
				ScriptStream >> srcId;
				StdOutSink *pSink = new StdOutSink(entityId,srcId,id,sessionId,pMediaDP,TopicList["video"],"*");
				pSink->startThread();
				EntityList[entityId] = (SessionEntity *)(pSink);
				std::cout << "Kind=" << EntityList[entityId]->GetKind() << std::endl;
				break;
			}
				
			case ENTITY_KIND_RELAYPROXY:
			{	
				ScriptStream >> srcId;
				RelayProxy *pRelay = new RelayProxy(entityId,srcId,id,sessionId,pMediaDP,TopicList["video"],3,"*");
				pRelay->startThread();
				EntityList[entityId] = (SessionEntity *)(pRelay);
				std::cout << "Kind=" << EntityList[entityId]->GetKind() << std::endl;
				break;
			}

			case ENTITY_KIND_H264FILESRC:
			{
				char				buf[100];
				std::string			SrcFileName(ENDPOINT_SRC_PATH);
				std::stringstream 	sstream;
				
				ScriptStream >> srcName;
				std::cout << srcName << std::endl;
				
				sstream << srcName;
				//sstream.getline(buf,100,'/');
				sstream.getline(buf,100);
				SrcFileName = SrcFileName + buf;
				SrcFileName = SrcFileName + ".264";
				std::cout << "Source File: " << SrcFileName << std::endl;
				strcpy(buf,SrcFileName.c_str());
				
				H264FileSrc *pSrc = new H264FileSrc(entityId,id,sessionId,buf,pMediaDP,TopicList["video"]);				
				pSrc->startThread();
				EntityList[entityId] = (SessionEntity *)(pSrc);
				std::cout << "Kind=" << EntityList[entityId]->GetKind() << std::endl;
				break;
			}
				
			case ENTITY_KIND_H264DECODERSINK:
			{	
				std::string	DecInFifoName;
				
				ScriptStream >> srcId;
				DecInFifoName = ToString<int>(srcId) + ".264";
				std::cout << "Input Fifo: " << DecInFifoName << std::endl;
				
				std::cout << "Source ID: " << srcId << std::endl;
				H264DecoderSink *pSink = new H264DecoderSink(entityId,srcId,id,sessionId,DecInFifoName.c_str(),pMediaDP,TopicList["video"],"*");
				pSink->startThread();
				EntityList[entityId] = (SessionEntity *)(pSink);
				break;
			}

			default:
			
				std::cout << "Entity kind " << entityType << " not valid" << std::endl;
				break;
		}
	}
	else if(Operand=="rem")
	{
		ScriptStream >> entityId;
		std::cout << entityId << std::endl;
		switch(EntityList[entityId]->GetKind())
		{
			case ENTITY_KIND_NATNUMSRC:
					
				((NatNumSrc *)(EntityList[entityId]))->stopThread();
				delete (NatNumSrc *)(EntityList[entityId]);
				break;
					
			case ENTITY_KIND_STDOUTSINK:
				
				((StdOutSink *)(EntityList[entityId]))->stopThread();
				delete (StdOutSink *)(EntityList[entityId]);
				break;

			case ENTITY_KIND_RELAYPROXY:
				
				((RelayProxy *)(EntityList[entityId]))->stopThread();
				delete (RelayProxy *)(EntityList[entityId]);
				break;

			case ENTITY_KIND_H264FILESRC:
				
				((H264FileSrc *)(EntityList[entityId]))->stopThread();
				delete (H264FileSrc *)(EntityList[entityId]);
				break;
					
			case ENTITY_KIND_H264DECODERSINK:
				((H264DecoderSink *)(EntityList[entityId]))->stopThread();
				delete (H264DecoderSink *)(EntityList[entityId]);
				break;
	
			default:
				
				std::cout << "Entity kind " << entityType << " not valid" << std::endl;
				break;
		}			
		
		EntityList.erase(entityId);
	}
	else if(Operand=="updsrc")		
	{
		ScriptStream >> entityId;
		ScriptStream >> srcId;
		std::cout << entityId << "," << srcId << std::endl;
		switch(EntityList[entityId]->GetKind())
		{
			case ENTITY_KIND_RELAYPROXY:
				
				((RelayProxy *)(EntityList[entityId]))->UpdateVideoSource(srcId);
				break;
					
			case ENTITY_KIND_H264DECODERSINK:
				
				((H264DecoderSink *)(EntityList[entityId]))->UpdateVideoSource(srcId);
				break;
					
			default:
	
				std::cout << "Entity kind " << EntityList[entityId]->GetKind() << " not valid" << std::endl;
				break;
		}
	}
	else if(Operand=="srclist")
	{
		std::string	SrcList;
		ScriptStream >> SrcList;
		RmtSrcEntIdList.Repopulate(SrcList.c_str());
	}
	
	return;
}

void SessionLeader::EventHandleLoop(void)
{
	while(!isStopRequested)
		HandleNextEvent();

	while(!NoEvents())
		HandleNextEvent();
	
	return;
}

