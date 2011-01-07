#ifndef SESSIONLEADER_H_
#define SESSIONLEADER_H_

#include <string>
#include <sstream>
#include <string.h>
//#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "controlplane.h"
#include "MediaParticipant.h"
#include "natnumsrc.h"
#include "stdoutsink.h"
#include "relayproxy.h"
#include "H264FileSrc.h"
#include "H264DecoderSink.h"
#include "SampleQueueSink.h"

#define DOMAIN_ID_LSCP	11

#define SL_LOG_PROMPT(oId,id)	"SF(" << oId << ")->SL(" << id << ")"

typedef	long long IDType;

class RemoteSourceList
{
	private:

		std::map<std::string,int>	List;

	public:

		void Repopulate(const char *srcList)
		{
			char			  	entry[500];
			char			  	srcName[100];
			char				entIdStr[100];
			int					entityId;
			bool				bFlag;
			std::stringstream 	Stream;

			//srcList is of the form [<username>/<srcname>~<entityId>]*
			List.clear();
			Stream << srcList;
			while(!Stream.eof())
			{
				std::stringstream	EntryStream;
				std::string			SrcName;
				std::string			EntIdStr;

				Stream.getline(entry,500,',');
				EntryStream << entry;

				//Source name
				EntryStream.getline(srcName,100,'~');
				SrcName = srcName;

				//Entity id
				EntryStream.getline(entIdStr,100,'~');
				EntIdStr = entIdStr;
				entityId = FromString<int>(EntIdStr,bFlag);

				//Resolution - <width>~<height>
				EntryStream.getline(entIdStr,100);
				SrcName = SrcName + "~" + entIdStr;

				List[SrcName] = entityId;
			}

			std::cout << "SrcNames:" << std::endl;
			for(std::map<std::string,int>::iterator it=List.begin(); it!=List.end(); it++)
				std::cout << "Listing: (" << it->first << "," << it->second << ")" << std::endl;

			return;
		}

		int GetEntityIdForRmtSrc(const char *srcName)
		{
			int			id = -1;
			std::string SrcName = srcName;

			if(List.find(SrcName)!=List.end())
				id = List[SrcName];

			return id;
		}

		void GetRmtSrcEntIdList(std::map<std::string,int> &ListOut)
		{
			ListOut = List;
			return;
		}
};

class SessionLeader : public EventHandlerT<SessionLeader>, public ThreadSingle
{

	private:

		IDType				id;
		IDType				sessionId;
		IDType				ownerId;
		char		   	   	name[100];
		LSCPSlave	   	   *pLSCSlave;
		LSCPSlaveObject	   *pLSCSlaveObj;

		//DDSDomainParticipant		   *pMediaDP;
		MediaParticipant               *pMediaParticipant;
		std::map<int,SessionEntity*> 	EntityList;
		//std::map<std::string,DDSTopic*>	TopicList;
		RemoteSourceList				RmtSrcEntIdList;

		com::xvd::neuron::lscp::Control *control;
    	com::xvd::neuron::lscp::State   *state;
    	com::xvd::neuron::lscp::Event   *event;
    	com::xvd::neuron::lscp::Metrics *metrics;

		void	EventHandleLoop (void);
		int		workerBee		(void)	{ EventHandleLoop(); return 0; }

		/******** Event Handle Functions ************/

		// Session Control Plane Slave Event Handlers
		void	HandleNewSessionEvent		(Event *);
		void	HandleUpdateSessionEvent	(Event *);
		void	HandleDeleteSessionEvent	(Event *);
		void	ProcessScript				(const char *);

	public:

		SessionLeader(IDType,IDType,const char *,int,int);
		~SessionLeader();

		IDType	GetId(void)			{ return id; }

		IDType	GetSessionId(void)	{ return sessionId; }

    	void Update(com::xvd::neuron::lscp::Control *control)
    	{
		    strcpy(this->control->script,control->script);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << "(update): " << control->script << endl;

		    return;
    	}

    	void SetStateStandby(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_STANDBY;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": STDBY (LSCP)" << endl;

		    return;
    	}

    	void SetStateReady(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_READY;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": READY (LSCP)" << endl;

		    return;
    	}

    	void SetStateUpdate(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_UPDATE;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": UPDATE (LSCP)" << endl;

		    return;
    	}

    	void SetStateDelete(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_DELETE;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": DELETE (LSCP)" << endl;

		    return;
    	}

    	void SetStateDeleted(void)
    	{
    		state->state = com::xvd::neuron::OBJECT_STATE_DELETED;
		    pLSCSlaveObj->Send(state);
		    std::cout << SL_LOG_PROMPT(ownerId,sessionId) << ": DELETED (LSCP)" << endl;

		    return;
    	}

    	int GetEntityIdForRmtSrc(const char *srcName)
		{
			return RmtSrcEntIdList.GetEntityIdForRmtSrc(srcName);
		}

		void GetRmtSrcEntIdList(std::map<std::string,int> &ListOut)
		{
			RmtSrcEntIdList.GetRmtSrcEntIdList(ListOut);
			return;
		}

		bool SetH264DecoderSinkSubLayerULimit(int epSrcId,int layerULimit)
		{
			bool									retVal = false;
			std::map<int,SessionEntity*>::iterator 	it;

			for(it=EntityList.begin(); it!=EntityList.end(); it++)
			{
				if(it->second->GetKind()==ENTITY_KIND_H264DECODERSINK)
				{
					H264DecoderSink *pSink = (H264DecoderSink *)(it->second);
					if(pSink->GetEpSrcId()==epSrcId)
					{
						pSink->SetSubLayerULimit(layerULimit);
						retVal = true;
						break;
					}
				}
			}

			return retVal;
		}

		int GetTimesParsed(int epSrcId)
		{
			std::map<int,SessionEntity*>::iterator 	it;

			for(it=EntityList.begin(); it!=EntityList.end(); it++)
			{
				if(it->second->GetKind()==ENTITY_KIND_H264DECODERSINK)
				{
					H264DecoderSink *pSink = (H264DecoderSink *)(it->second);
					if(pSink->GetEpSrcId()==epSrcId)
						return pSink->GetTimesParsed();
				}
			}

			return -1;
		}

		void RemoveH264DecoderSink(int epSrcId)
		{
			std::map<int,SessionEntity*>::iterator 	it;
			std::cout << "Inside sl.remove()" << std::endl;

			for(it=EntityList.begin(); it!=EntityList.end(); it++)
			{
				if(it->second->GetKind()==ENTITY_KIND_H264DECODERSINK)
				{
					std::cout << "Inside sl.remove().if()" << std::endl;
					H264DecoderSink *pSink = (H264DecoderSink *)(it->second);
					if(pSink->GetEpSrcId()==epSrcId)
					{
						std::string	Script = "\"rem " + ToString<int>(it->second->GetId()) + "\"";
						std::cout << "DELETE SCRIPT: " << Script << std::endl;
						ProcessScript(Script.c_str());
						break;
					}
				}
			}
			return;
		}
};

#endif /* SESSIONLEADER_H_ */

