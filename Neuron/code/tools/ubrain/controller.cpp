//! \file controller.cpp
//!
//! \brief An example controller
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
//! The purpose of this example is to illustrate how an Controller may function
//!
//! Specifically. this code is provided "as is", without warranty, expressed
//! or implied.

#include "controller.h"

#include <string>
#include <sstream>

// Session listener
typedef void (*ControllerListener_onNewSession)(int srcId,int sessionId);
typedef void (*ControllerListener_onNewSessionState)(int srcId,
											int sessionId,
												com::xvd::neuron::ObjectState state);
typedef bool (*ControllerListener_onNewSessionFilter)(int srcId,int sessionId);
typedef void (*ControllerListener_onNewSessionEvent)(int srcId);
typedef void (*ControllerListener_onNewSessionMetric)(int srcId);

//! \brief Not-quite-so-naive function to break a string into an argv list.
//!
//! Everything between " is considered a single argument
//!

void CreateArgvList(char *str_in,int *argc,char **argv)
{
	string arg;
	stringstream strm(str_in);
	static char buf[10000];
	int index;
	
	index = 0;
	*argc = 0;
	
	while (strm.good())
	{
		while (strm.peek()==' ' && strm.good())
			strm.get();
		
		// Trailing spaces would do this. They are chopped off and then we pop out.
		if (!strm.good())
			break;
		
		// Now get argument which may start with '"'
		if (strm.peek()=='\"')
		{
			strm.get();	// Kill the quote so we can get the guts of the arg.
			strm.getline(buf, 9999, '\"');	// Assumes there WILL be another closing '"'
			arg = "\"";
			arg += buf;
			arg += "\"";	// Putting the 'eaten' '"' back in the argument for length purposes.
		}
		else
		{
			strm >> arg;
		}
		// Now we have an arg. So stuff it and increment *argc
		// If we find the length of 'arg', we know where to STUFF a NULL in the inbound string and assign.
		argv[*argc] = str_in + index;
		index += arg.length();
		
		str_in[index] = 0;
		index++;	// Skip past the null terminator.
		
		(*argc)++;
	}
}

//! \brief Create a new Controller
Controller::Controller(int appId, 
					   int domaindId, 
					   map<string,string> &nvPairs,
					   ControllerListener *listener)
{
	map<string,string>      PropertyPairsACP;
	map<string,DDS_Boolean> PropagateDiscoveryFlagsACP;
	map<string,string>      PropertyPairsSCP;
	map<string,DDS_Boolean> PropagateDiscoveryFlagsSCP;
	
	pSessionEventHandler = new ControllerEventHandler(this);
	
	pListener = listener;
	
	m_appId = appId;
	
	bool bUseLANOnly = (nvPairs["use_lan_only"]=="true");
	
	if (!bUseLANOnly)
	{
		// The Controller serves as the admin master for all SFs, thus connect as master
		PropertyPairsACP["dds.transport.wan_plugin.wan.transport_instance_id"] = nvPairs["ubrain_acp_id"];
		PropagateDiscoveryFlagsACP["dds.transport.wan_plugin.wan.transport_instance_id"] = DDS_BOOLEAN_FALSE;
		PropertyPairsACP["dds.transport.wan_plugin.wan.server"] = nvPairs["stun_ip"];
		PropagateDiscoveryFlagsACP["dds.transport.wan_plugin.wan.server"] = DDS_BOOLEAN_FALSE;
	}
	
	PropertyPairsACP["CPInterfaceType"] = "ACP:Master";
	PropagateDiscoveryFlagsACP["CPInterfaceType"] = DDS_BOOLEAN_TRUE;
	PropertyPairsACP["Id"] = ToString<int>(appId);
	PropagateDiscoveryFlagsACP["Id"] = DDS_BOOLEAN_TRUE;
	
	if (!bUseLANOnly)
		// WAN Case
		pACPMaster = new ACPMaster(pSessionEventHandler,appId,domaindId,"Controller::ACPMaster", PropertyPairsACP, PropagateDiscoveryFlagsACP, "ACP");
	else
		// LAN Case
		pACPMaster = new ACPMaster(pSessionEventHandler,appId,domaindId,"Controller::ACPMaster", PropertyPairsACP, PropagateDiscoveryFlagsACP, "ACPLAN");
	
	if (!bUseLANOnly)
	{
		// The Controller manages sessions, thus connect to the SCP as master
		PropertyPairsSCP["dds.transport.wan_plugin.wan.transport_instance_id"] = nvPairs["ubrain_scp_id"];
		PropagateDiscoveryFlagsSCP["dds.transport.wan_plugin.wan.transport_instance_id"] = DDS_BOOLEAN_FALSE;
		PropertyPairsSCP["dds.transport.wan_plugin.wan.server"] = nvPairs["stun_ip"];
		PropagateDiscoveryFlagsSCP["dds.transport.wan_plugin.wan.server"] = DDS_BOOLEAN_FALSE;
	}
	
	PropertyPairsSCP["CPInterfaceType"] = "SCP:Master";
	PropagateDiscoveryFlagsSCP["CPInterfaceType"] = DDS_BOOLEAN_TRUE;
	PropertyPairsSCP["Id"] = ToString<int>(appId);
	PropagateDiscoveryFlagsSCP["Id"] = DDS_BOOLEAN_TRUE;
	
	if (!bUseLANOnly)
		// WAN Case
		pSCPMaster = new SCPMaster(pSessionEventHandler,appId,domaindId,"Controller::SCPMaster", PropertyPairsSCP, PropagateDiscoveryFlagsSCP, "SCP");
	else
		// LAN Case
		pSCPMaster = new SCPMaster(pSessionEventHandler,appId,domaindId,"Controller::SCPMaster", PropertyPairsSCP, PropagateDiscoveryFlagsSCP, "SCPLAN");
	
	m_domainId = domaindId;
	
	pthread_mutexattr_t attr;  
 	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE); 
	pthread_mutex_init(&lock, &attr); 
 
	// This is using RTI APIs to spawn an event handler thread. Please
	// replace with something more appropriate
	eventThread = RTIOsapiThread_new("controlThread",
									 RTI_OSAPI_THREAD_PRIORITY_NORMAL,
									 0,
									 PTHREAD_STACK_MIN*16,
									 NULL,
									 eventLoop,
									 pSessionEventHandler);
}

//! \brief Destroy this Controller
Controller::~Controller()
{
	if (pACPMasterObject)
		pACPMaster->DeleteMasterObject(pACPMasterObject);

	pthread_mutex_destroy(&lock); 
	
	delete pSCPMaster;
	delete pACPMaster;
	delete pSessionEventHandler;
}

bool Controller::AddSCPMasterPeer(const char* descriptor)
{
	//TODO: (tron): Can this ever happen? pACMaster is created in constructor
	if (!pSCPMaster)
	{
	    cout << "Controller::AddSCPMasterPeer() - No SCP Master yet." << endl;
		return false;
	}
	
	return pSCPMaster->AddPeerAndWaitForDiscovery(descriptor,5000);
}

bool Controller::AddACPMasterPeer(const char* descriptor)
{
	//TODO: (tron): Can this ever happen? pACMaster is created in constructor
	if (!pACPMaster)
	{
	    cout << "Controller::AddACPMasterPeer() - No ACP Master yet." << endl;
		return false;
	}
	
	return pACPMaster->AddPeerAndWaitForDiscovery(descriptor,5000);
}

//! \brief Handle events detetecd on the SCP
void Controller::RemoteSessionStateUpdate(com::xvd::neuron::scp::State *state, DDS_SampleInfo *info)
{
	Controller::Session *session = NULL;
	SessionMap::iterator sit;
	RemoteSessionFactorySessionMap::iterator it;
	
	sit = sessions.find(state->sessionId);
	
	if (sit == sessions.end())
	{
		if (pListener)
		{
			if (pListener->onSessionDetect(this, state->sessionId)) 
			{		
				session = new Session(pSCPMaster,state->sessionId);
				sessions[state->sessionId] = session;
			}
		}
	} 
	else
	{
		session = sit->second;
	}

	if (session == NULL)
	{
		return;
	}
	
	if (state->state == com::xvd::neuron::OBJECT_STATE_OFFERSRC ||
		state->state == com::xvd::neuron::OBJECT_STATE_SELECTSRC)
	{
		printf("Payload: %s\n",state->payload);
	}
	
	if (pListener)
	{
		if (pListener->onSFSessionStateChange(this,state->sessionId, state->srcId,state)) 
		{
			session->UpdateSFSessionState(state->srcId,state);
		}
	}	
}

//! \brief Handle state updates from a SF
void Controller::RemoteSessionDeleted(int sessionId,int srcId)
{	
	SessionMap::iterator sit;
	bool deleteObject = false;
	
	sit = sessions.find(sessionId);
	
	if (sit != sessions.end())
	{
		if (!sit->second->IsSFDeleted(srcId))
		{
			if (pListener)
			{
				deleteObject = pListener->onSFSessionTerminate(this,sessionId,srcId);
			}
		}
		else
		{
			if (pListener)
			{
				deleteObject = pListener->onSFSessionDelete(this,sessionId,srcId);
			}
		}
	}
	
	if (deleteObject) 
	{
		sit->second->DeleteSF(srcId);
		if (sit->second->IsDeleted()) 
		{
			if (pListener && pListener->onSessionDelete(this,sessionId)) 
			{
				sessions.erase(sessionId);
				delete sit->second;
			}
		}
	}
}	

//! \brief Handle state updates from a SF
void Controller::RemoteSFUpdate(com::xvd::neuron::acp::State *state,DDS_SampleInfo *info)
{
	RemoteSessionFactoryMap::iterator it;
	RemoteSessionFactory *rsf;
	bool ignoreSF = false;
	
 	it = remoteSF.find(state->srcId);
	
	if (it == remoteSF.end())
	{
		// A new SF has been detected (it was either created manually or by thee
		// sf create ... command. The rsf is optimiscally created so if the
		// listener accepts it we know we cannot fail. If the listener 
		// rejects it we just delete the object.
		rsf = new RemoteSessionFactory(state->srcId,
									   info->disposed_generation_count,
									   pACPMaster);
		if (rsf == NULL) 
		{
			// TODO: Log error
			return;
		}
		
		if ((pListener != NULL)  && !pListener->onSFDetect(this,state->srcId)) 
		{
			// TODO: Log that SF was rejected
			delete rsf;
			return;
		}
		
		remoteSF[state->srcId] = rsf;
		
		// This can be refactored after change in responsibilties between
		// controller and upper layer, but kept close to the original 
		// in case there is a need to keep things separated.
		if (state->state == com::xvd::neuron::OBJECT_STATE_STANDBY)
		{
			// This is the state we would expect if the SF is running
			// and has not been enabled by someone else. Note that the
			// controller do not enable the SF. It is up to the higher level
			// Manager to do so. However, ping it so that it can be deleted.
			rsf->ping();
		}
		else if (state->state == com::xvd::neuron::OBJECT_STATE_READY)
		{
			// This is the state we would expect if the SF is already running
			// and has been enabled by someone else. However, if the SF is running
			// and no-one is managing it, we ping it to make sure that it "discover"
			// this master (we currently only support have shared ownership). If
			// It has not discovered the master, a dispose will be ignored. In this
			// application there is no real different between a enable and ping,
			// but in a real application there likely is a difference
			rsf->ping();
		}
		else
		{
			// If it is being deleted or in another state we don't
			// want to deal with it as a valid SF
		}				
	}
	else
	{		
		// We already know about this SF. In this case we act upon the
		// state change. If it has been deleted, we remove it from
		// our list of remote SFs only if the listener accpets it. If we 
		// initiated the delete isShutdown() returns true, it is all good. 
		// However, if someone exited it but not us we still remove it from 
		// our list of remote SFs.
		//
		// TODO: In this case we need to go through all the current sessions
		//       using using this SF and determine what to do.
		//
		// TODO: Log this as info
		//printf("Update to SF detected, state = %d/%d\n",state->state,info->disposed_generation_count);
		
		RemoteSessionFactoryMap::iterator rsfit;
		rsfit = remoteSF.find(state->srcId);
		
		if ((state->state == com::xvd::neuron::OBJECT_STATE_STANDBY) ||
				 (state->state == com::xvd::neuron::OBJECT_STATE_READY))
		{
			if (!rsfit->second->isSame(info->disposed_generation_count))
			{
				// This is the state we would expect if the SF was started
				// clean again after we have discovered it or it has been been
				// enabled by someone else after being restarted. We would
				// expect that this is a different incarnation, so we check
				// for that.
				printf("A new incarnation of SF %d was expected!!\n",state->srcId);
			}
			else
			{
				//TODO: Notify the upper layer in both of these cases
				if (state->state == com::xvd::neuron::OBJECT_STATE_STANDBY)
				{
					//rsfit->second->enable();
				}
				else
				{
					// If it already ready, just ping it in case the previous
					// controller was restarted. In that case we want to
					// ensure the key is rediscovered
					rsfit->second->ping();
				}
			}
		}
		else
		{
			// Currently there is no error state, this should be added
			// TODO: What would the error state be?
		}		
	}
	
	if (pListener && !ignoreSF)
	{
		pListener->onSFStateChange(this,state->srcId,state);
	}	
}

//! \brief Handle state updates from a SF
void Controller::RemoteSFDeleted(int srcId)
{
	RemoteSessionFactoryMap::iterator it;
	
 	it = remoteSF.find(srcId);
	
	if (it != remoteSF.end())
	{
		bool deleteOk = true;
		if (!it->second->isShutdown()) 
		{
			// This SF was shutdown without our knowledge. Let the 
			// listener know and decide
			if (pListener && !pListener->onSFTerminate(this,srcId))
			{
				deleteOk = false;
			}
		}
		else 
		{
			if (pListener && !pListener->onSFDelete(this,srcId))
			{
				deleteOk = false;
			}					
		}
		if (deleteOk)
		{
			remoteSF.erase(srcId);
			SFList.erase(srcId);
			delete it->second;
		}				
	}			
	else 
	{
		// Don't know anything about this SF
	}
	
}	

//TODO: We should probably document this elsewhere?
//TODO: Ask for document generation, find an apppropriate place
//! \brief Handle help command
void Controller::CmdHelp()
{
	printf("Interactive controller shell:\n\n");
	
	// Commands that manage SFs
	printf("acp ls                             - List detected SFs\n");
	printf("acp shutdown <sfId>+               - Shutdown listed SFs\n");
	printf("acp enable <sfId>+                 - Enable listed SFs\n");
	printf("acp send <sfId>+ <script>          - Send script to SFs\n");

	// Commands that manage Sessions
	printf("scp create <sId> <sfId>+ <script>  - Create a new session\n");
	printf("scp update <sId> <sfId>+ <script>  - Update session \n");
	printf("scp delete <sId> <sfId>*           - Delete a session, optionally only from the listed SFs\n");
	printf("scp ls                             - List known sessions on standard output\n");
	
	// Commands that manages SFs	
	printf("sf create <sId>                    - Spawn a new SF\n");
	printf("sf delete <sId>                    - Delete a SF\n");
	printf("sf ls                              - List locally created SFs\n");
	printf("\n");
	printf("quit                               - Exit with no cleanup\n");
	printf("\n");
}

//! \brief Handle acp <..> commands
bool Controller::CmdACP(int *argc,int max_argc,char **argv)
{
	bool retval = false;
	
	++(*argc);
	if (*argc == max_argc)
	{
		printf("usage: acp <cmd>\n");
		return retval;
	}
	if (!strcmp(argv[*argc],"ls"))
	{
		ListRemoteSF();
	}
	else if (!strcmp(argv[*argc],"shutdown"))
	{
		int sfId;
		set<int> ids;
		set<int>::iterator it;
		++(*argc);
		if (*argc == max_argc)
		{
			printf("usage: acp shutdown <id>+\n");
			return retval;
		}
		while (*argc < max_argc)
		{
			sfId = strtol(argv[*argc],NULL,0);
			ids.insert(sfId);
			(*argc)++;
		}
		for (it = ids.begin(); it != ids.end(); ++it)
		{
			ShutdownRemoteSF(*it);
		}
	}
	else if (!strcmp(argv[*argc],"send"))
	{
		int sfId;
		set<int> ids;
		set<int>::iterator it;
		RemoteSessionFactoryMap::iterator rsfit;
		++(*argc);
		if (*argc == max_argc)
		{
			printf("usage: acp send <id>+ <cmd>\n");
			return retval;
		}
		while (*argc < max_argc)
		{
			if (argv[*argc][0]=='"')
				break;
			sfId = strtol(argv[*argc],NULL,0);
			ids.insert(sfId);
			(*argc)++;
		}
		if (*argc == max_argc-1)
		{
			for (it = ids.begin(); it != ids.end(); ++it)
			{
				rsfit = remoteSF.find(*it);
				rsfit->second->Send(argv[*argc]);
			}
		}
		else
		{
			printf("usage: acp send <id>+ <cmd>\n");
			return retval;
		}
	}
	else if (!strcmp(argv[*argc],"enable") || !strcmp(argv[*argc],"ping"))
	{
		int sfId;
		set<int> ids;
		set<int>::iterator it;
		RemoteSessionFactoryMap::iterator rsfit;
		bool ping = !strcmp(argv[*argc],"ping");

		++(*argc);
		
		if (*argc == max_argc)
		{
			printf("usage: acp enable | ping <id>+\n");
			return retval;
		}
		while (*argc < max_argc)
		{
			sfId = strtol(argv[*argc],NULL,0);
			ids.insert(sfId);
			(*argc)++;
		}
		if (*argc == max_argc)
		{
			for (it = ids.begin(); it != ids.end(); ++it)
			{
				rsfit = remoteSF.find(*it);
				if (ping)
				{
					rsfit->second->ping();
				}
				else
				{
					rsfit->second->enable();
				}
			}
		}
		else
		{
			printf("usage: acp enable | ping <id>+\n");
			return retval;
		}
	}	
	else
	{
		printf("unknown acp command: %s\n",argv[*argc]);
	}

	retval = true;
	
	return retval; 
}

//! \brief Handle scp <..> commands
bool Controller::CmdSCP(int *argc,int max_argc,char **argv)
{
	++(*argc);
	if (*argc == max_argc)
	{
		printf("usage: scp <cmd>\n");
		return false;
	}
	if (!strcmp(argv[*argc],"create") || !strcmp(argv[*argc],"update"))
	{
		int sId;
		int sfId;
		set<int> sessionSFs;
		bool update = !strcmp(argv[*argc],"update");
		
		++(*argc);
		if (*argc == max_argc)
		{
			printf("usage: scp [create | update] <sessionId> <sfID>+ <script>\n");
			return false;
		}
		sId = strtol(argv[*argc],NULL,0);
		
		++(*argc);
		while (*argc < max_argc)
		{
			if (argv[*argc][0]=='"')
				break;
			sfId = strtol(argv[*argc],NULL,0);
			sessionSFs.insert(sfId);
			(*argc)++;
		}
		if (*argc == max_argc-1)
		{
			CreateOrUpdateSession(sId,&sessionSFs,argv[*argc],update);
		}
		else
		{
			CreateOrUpdateSession(sId,&sessionSFs,NULL,update);
		}
	}
	else if (!strcmp(argv[*argc],"delete"))
	{
		int sId,sfId;
		set<int> sessionSFs;

		++(*argc);
		if (*argc == max_argc)
		{
			printf("usage: scp delete <sessionId>\n");
			return false;
		}
		sId = strtol(argv[*argc],NULL,0);
		
		++(*argc);
		while (*argc < max_argc)
		{
			sfId = strtol(argv[*argc],NULL,0);
			sessionSFs.insert(sfId);
			(*argc)++;
		}
		
		DeleteSession(sId,sessionSFs);
	}
	else if (!strcmp(argv[*argc],"ls"))
	{
		ListSession();
	}
	else
	{
		printf("unknown scp command: %s\n",argv[*argc]);
	}
	return true;
}

//! \brief Handle sf <..> commands
bool Controller::CmdSF(int *argc,int max_argc,char **argv)
{
	int sId;
	//SessionFactory *aSF;
	
	++(*argc);
	if (*argc == max_argc)
	{
		printf("usage: sf <cmd>\n");
		return false;
	}
	if (!strcmp(argv[*argc],"create"))
	{
		++(*argc);
		if (*argc == max_argc)
		{
			printf("usage: sf create <id>\n");
			return false;
		}
		while (*argc < max_argc)
		{
			pid_t pid;
			sId = strtol(argv[*argc],NULL,0);
			char did[10];
			char aid[10];
			sprintf(did,"%d",m_domainId);
			sprintf(aid,"%d",m_appId);
			pid = fork();
			if (pid != 0)
			{
				printf("Created SF: %d/%d\n",sId,pid);
				SFList[sId] = pid;
				(*argc)++;
			}
			else
			{
				execl("../../bin/sf",
					  "../../bin/sf",
					  argv[*argc],
					  "SessionFactory",
					  aid,
					  did,(char *)0);
				
				/*execl("bin/sf",
				 "bin/sf",
				 "-appId",
				 argv[*argc],
				 "-domainId",
				 did,(char *)0);*/
				exit(0);
			}
		}
	}
	else if (!strcmp(argv[*argc],"ls"))
	{
		ListSF();
	}
	else if (!strcmp(argv[*argc],"delete"))
	{
		std::map<int,pid_t>::iterator it;
		++(*argc);
		if (*argc == max_argc)
		{
			printf("usage: sf delete <id>\n");
			return false;
		}
		sId = strtol(argv[*argc],NULL,0);
		it = SFList.find(sId);
		//delete it->second;
		SFList.erase(sId);
	}
	
	return true;
}

//! \brief List detected SFs
void Controller::ListRemoteSF()
{
	RemoteSessionFactoryMap::iterator it;
	
	printf("\nRemote SessionFactories\n");
	printf("=======================\n");
	for (it = remoteSF.begin(); it != remoteSF.end(); ++it)
	{
		it->second->PrintInfo();
	}
	printf("\n");
}

//! \brief List locally created SFs
void Controller::ListSF()
{
	map<int,pid_t>::iterator it;
	
	printf("\nLocal SessionFactories\n");
	printf("======================\n");
	for (it = SFList.begin(); it != SFList.end(); ++it)
	{
		printf("SF %d/%d\n",it->first,it->second);
		//it->second->PrintInfo();
	}
	printf("\n");
}

//! \brief Shutdown a detected SF.
//!
//! Under the hood, this function disposed of the MasterObject
void Controller::ShutdownRemoteSF(int sfId)
{
	RemoteSessionFactoryMap::iterator rsfit;
	
	rsfit = remoteSF.find(sfId);
	if (rsfit != remoteSF.end())
	{
		printf("Shutting down SessionFactory %d ....\n",sfId);
		rsfit->second->Shutdown();
	}
	else
	{
		printf("SessionFactory %d does not exist\n",sfId);
	}
}

//! \brief Create of update a session
void Controller::CreateOrUpdateSession(int sessionId, set<int> *_sfs,const char *script,bool update)
{
	set<int>::iterator it;
	RemoteSessionFactoryMap::iterator rsfit;
	SessionMap::iterator sit;
	Session *session;
	
	sit = sessions.find(sessionId);
	
	// Need to allow separate 'create' commands to add a given session to a set of factories.
	// This means it is ok to send in a session-id more than once. Deal with it gracefully.
	if (!update)
	{
		// Session doesn't exist yet.
		if (sit == sessions.end())
		{
			printf("Create session %d using first-SF=%d ",sessionId, *(_sfs->begin()));
			session = new Session(pSCPMaster,sessionId);
			sessions[sessionId] = session;
		}
		else
		{
			// Now we have the 'found' session here so mark it and don't re-create an existing session.
			session = sit->second;
			
			printf("Adding existing session(%d) on first-SF=%d ", sessionId, *(_sfs->begin()));
		}
	}
	else
	{
		// If the chosen session to be updated doesn't exist....error out.
		if (sit == sessions.end())
		{
			printf("Session %d does not exist for updating.\n", sessionId);
			return;
		}
		
		session = sit->second;
		printf("Update session %d using SF ",sessionId);
	}
	
	for (it = _sfs->begin(); it != _sfs->end(); ++it)
	{
		rsfit = remoteSF.find(*it);
		if (rsfit == remoteSF.end())
		{
			printf("ERROR: SF id does not exist\n");
			return;
		}
		if (!rsfit->second->IsAvailable())
		{
			printf("ERROR: SF %d is not available\n",rsfit->first);
			return;
		}
		if (!update)
		{
			cout << "Adding Slave " << *it << endl;;
			session->AddSlave(*it,script);
		}
		else
		{
			if (!session->UpdateSF(*it,script))
			{
				printf("ERROR: Failed to update SF %d\n",*it);
			}
		}
	}
	if (script != NULL)
	{
		printf("with script %s\n",script);
	}
	else
	{
		printf("\n");
	}
}

//! \brief List active sessions
void Controller::ListSession()
{
	SessionMap::iterator it;
	
	printf("\nSessions\n");
	printf("========\n");
	
	for (it = sessions.begin(); it != sessions.end(); ++it)
	{
		it->second->ReportSessionStatus();
	}
	printf("\n");
}

///
/// \brief Grab a handle to a particular session from outside.
/// \args sessionId input
/// \param [in] sessionId - Desired session ID to retrieve
/// \return
///   - NULL if sessionId is not found.
///   - Pointer to Session class on success.
///
Controller::Session* Controller::GetSession(int sessionId)
{
	SessionMap::iterator it;
	
	for (it = sessions.begin(); it != sessions.end(); ++it)
	{
		if (it->first == sessionId)
			return it->second;
	}
	
	return NULL;
}

//! \brief Delete a session
void Controller::DeleteSession(int sId,set<int> sessionSFs)
{
	SessionMap::iterator it;
	Session *s;
	
	it = sessions.find(sId);
	if (it == sessions.end())
	{
		return;
	}
	
	// If no SFs are specified, delete the entire session
	if (sessionSFs.empty()) 
	{
		s = it->second;
		s->Delete();
		if (s->IsDeleted())
		{
			if (pListener && pListener->onSessionDelete(this,sId)) 
			{
				sessions.erase(sId);
				delete s;
			}			
		}
	}
	else
	{
		set<int>::iterator sfid;
		
		// Only delete the session on the specified SFs. Do not delete the
		// session even if it has no SFs. It is up to the upper layer to do so
		for (sfid = sessionSFs.begin(); sfid != sessionSFs.end(); ++sfid)
		{		
			it->second->RemoveSlave(*sfid);
		}
	}
}

bool Controller::runSingleCommand(const char*cmdIn)
{
	char **argv = (char**)calloc(sizeof(char*),200);
	char thiscmdline[256];
	int argc;
	int s;
 
	pthread_mutex_lock(&lock);
	
	assert(cmdIn);
	
	string cmd(cmdIn);
	
	strcpy(thiscmdline, cmdIn);
	CreateArgvList(thiscmdline,&argc,argv);
	
	// TODO - MUST make sure this is done correctly. INCORRECT now.
	for (s = 0; s < argc; ++s)
	{
		if (!strcmp(argv[s],"help"))
		{
			CmdHelp();
			break;
		}
		else if (!strcmp(argv[s],"acp"))
		{
			CmdACP(&s,argc,argv);
		}
		else if (!strcmp(argv[s],"scp"))
		{
			CmdSCP(&s,argc,argv);
		}
		else if (!strcmp(argv[s],"sf"))
		{
			CmdSF(&s,argc,argv);
		}
		else
		{
			printf("Unknown command: %s\n",argv[s]);
			break;
		}
	}
	
	free((void*)argv);
	
	pthread_mutex_unlock(&lock);
	
	return true;
}

// The constructor takes the SCP master object and scp ID as input. The
// SCP Master attaches to the SCP plane and can manipulate any scp.
Controller::Session::Session(SCPMaster *_sm,int _sid)
{
	sm = _sm;
	
	// The SCPMasterObject is an instance of a session as seen from the
	// SCPMaster. Each MasterObject maintains state for a single session
	// forthis SCPMaster.
	scp = sm->CreateMasterObject(_sid);
	
	// Data-type for the interface to the SCPMasterObject.
	control = com::xvd::neuron::scp::ControlTypeSupport::create_data();
	
	localState = com::xvd::neuron::OBJECT_STATE_INIT;
	
	locallyDeleted = false;
	
	sprintf(control->script,"Created scp %d",scp->GetSessionId());
}

Controller::Session::~Session()
{
	// Return the session object to the SCPMaster
	if ((scp != NULL) && !sm->DeleteMasterObject(scp))
	{
		printf("error deleting master object\n");
	}
	
	// Delete local memory to store control and state
	com::xvd::neuron::scp::ControlTypeSupport::delete_data(control);
}

void Controller::Session::DeleteSF(int sfId)
{
	RemoteSessionFactorySessionMap::iterator it;
	
	it = sfs.find(sfId);
	if (it == sfs.end())
	{
		return;
	}
	
	sfs.erase(sfId);
	delete it->second;
}

// Add a slave to the session
void Controller::Session::AddSlave(int slaveId,const char *script)
{
	RemoteSessionFactorySessionMap::iterator it;
	RemoteSessionFactorySession *rs;
	 
	it = sfs.find(slaveId);
	if (it != sfs.end())
	{
		return;
	}
	
	rs = new RemoteSessionFactorySession();
	rs->_state = com::xvd::neuron::OBJECT_STATE_INIT;
	rs->_expectedState = com::xvd::neuron::OBJECT_STATE_READY;

	sfs[slaveId]=rs;
	
	if (script != NULL)
	{
		sprintf(control->script,"%s",script);
	}
	else
	{
		sprintf(control->script,"end");
	}
	
	if (!scp->Send(control,slaveId))
	{
		printf("failed to send create command to slave %d\n",slaveId);
	}
}

// Limitation: Can only send to a subset of the SFs the session was created with
bool Controller::Session::UpdateSF(int slaveId,const char *script)
{
	RemoteSessionFactorySessionMap::iterator it;
	
	it = sfs.find(slaveId);
	if (it != sfs.end())
	{
		return false;
	}
	
	if (script == NULL)
	{
		sprintf(control->script,"end");
	}
	else
	{
		sprintf(control->script,"%s",script);
	}
	
	if (!scp->Send(control,slaveId))
	{
		printf("failed to update SF %d\n",slaveId);
		return false;
	}
	return true;
}

void Controller::Session::UpdateSFSessionState(int sfId,com::xvd::neuron::scp::State *state)
{
	RemoteSessionFactorySessionMap::iterator it;
	RemoteSessionFactorySession *rs;
	
	it = sfs.find(sfId);
	if (it == sfs.end())
	{
		return;
	}	
	rs = it->second;
	
	if (state->state != rs->_expectedState)
	{
		//TODO: Define what this means. The controller has no knowledge
	}
	rs->_state = state->state;
}

void Controller::Session::RemoveSlave(int sfId)
{
	RemoteSessionFactorySessionMap::iterator it;
	RemoteSessionFactorySession *rs;
	
	it = sfs.find(sfId);
	if (it == sfs.end())
	{
		return;
	}	
	rs = it->second;
	
	sprintf(control->script,"delete session %d",scp->GetSessionId());
	if (!scp->Send(control,sfId))
	{
		printf("failed to send remove command to slave %d\n",sfId);
	}
	
	rs->_expectedState = com::xvd::neuron::OBJECT_STATE_DELETE;
	
	// Do not delete the SF from the session list. Inst
	// session when the state is disposed on the SF, and the upper layer
	// says ok.
}

bool Controller::Session::IsSFDeleted(int sfId)
{
	RemoteSessionFactorySessionMap::iterator it;

	it = sfs.find(sfId);

	if (it == sfs.end())
	{
		return false;
	}
	
	return (it->second->_state == com::xvd::neuron::OBJECT_STATE_DELETE);
}

bool Controller::Session::IsDeleted()
{
	return locallyDeleted && (sfs.begin() == sfs.end());
}

void Controller::Session::Delete()
{
	RemoteSessionFactorySessionMap::iterator it;
	RemoteSessionFactorySession *rs;
	
	if (!sfs.empty())
	{
		for (it = sfs.begin(); it != sfs.end(); ++it)
		{
			rs = it->second;
			rs->_expectedState = com::xvd::neuron::OBJECT_STATE_DELETE;
			// Do not delete the SF from the session list. Inst
			// session when the state is disposed on the SF, and the upper layer
			// says ok.
		}
	}
	// Return the session object to the SCPMaster
	if (scp != NULL){
		sm->DeleteMasterObject(scp);
		scp = NULL;
	}
	
	locallyDeleted = true;
}

bool Controller::Session::SlavesAreReady(set<int> *pending)
{
	com::xvd::neuron::scp::State *state=NULL;
	set<int>::iterator it;
	for (it = pending->begin(); it != pending->end(); ++it)
	{
		state = scp->GetState(*it);
		if (state == NULL)
		{
			continue;
		}
		if (state->state != com::xvd::neuron::OBJECT_STATE_READY)
		{
			continue;
		}
		pending->erase(*it);
	}
	return pending->empty();
}

// Send a command to a SF
bool Controller::Session::SendCommand(const char *script,int slaveId)
{
	RemoteSessionFactorySessionMap::iterator it;
	
	it = sfs.find(slaveId);
	if (it == sfs.end())
	{
		printf("slave %d is not valid\n",slaveId);
		return false;
	}
	
	sprintf(control->script,"execute scp %d: %s\n",
			scp->GetSessionId(),script);
	
	return scp->Send(control,slaveId);
}

void Controller::Session::ReportSessionStatus(void)
{
	RemoteSessionFactorySessionMap::iterator it;

	printf("Session [%d] status\n",scp->GetSessionId());
	
	com::xvd::neuron::scp::State *state=NULL;
	com::xvd::neuron::scp::EventSeq *events=NULL;
	com::xvd::neuron::scp::MetricsSeq *metrics=NULL;
		
	for (it = sfs.begin(); it != sfs.end(); ++it)
	{
		printf("checking state for %d\n",it->first);
		state = scp->GetState(it->first);
		events = scp->GetEvents(it->first);
		metrics = scp->GetMetrics(it->first);
		
		if (state != NULL)
		{
			printf("State: %d, %d,%d\n",state->srcId,state->sessionId,state->state);
		}
		
		if (events != NULL)
		{
			for (int i = 0; i < events->length(); ++i)
			{
				printf("Event[%d]: %d, %d,%d\n",
					   i,
					   (*events)[i].srcId,
					   (*events)[i].sessionId,
					   (*events)[i].eventCode);
			}
		}
		
		if (metrics != NULL)
		{
			for (int i = 0; i < metrics->length(); ++i)
			{
				printf("Metrics[%d]: %d, %d,%d,%d,%d\n",
					   i,
					   (*metrics)[i].srcId,
					   (*metrics)[i].sessionId,
					   (*metrics)[i].entityCount,
					   (*metrics)[i].bytesSent,
					   (*metrics)[i].bytesReceived);
			}
		}
	}
}

Controller::RemoteSessionFactory::RemoteSessionFactory(int appId,DDS_Long _rank,ACPMaster *_pACPMaster)
{
	m_appId = appId;
	control = com::xvd::neuron::acp::ControlTypeSupport::create_data();
	pACPMaster = _pACPMaster;
	pACPMasterObject = pACPMaster->CreateMasterObject(appId);
	shutDown = false;
	localState = INIT;
	rank = _rank;
}

Controller::RemoteSessionFactory::~RemoteSessionFactory()
{
	if (pACPMasterObject != NULL)
	{
		pACPMaster->DeleteMasterObject(pACPMasterObject);
	}
	com::xvd::neuron::acp::ControlTypeSupport::delete_data(control);
}

void Controller::RemoteSessionFactory::PrintInfo()
{
	com::xvd::neuron::acp::State *pACstate;
	com::xvd::neuron::acp::EventSeq *pACeventSeq;
	com::xvd::neuron::acp::MetricsSeq *pACmetricsSeq;
	
	pACstate = pACPMasterObject->GetState(m_appId);
	pACeventSeq = pACPMasterObject->GetEvents(m_appId);
	pACmetricsSeq = pACPMasterObject->GetMetrics(m_appId);
	
	printf("SF[%d]\n",m_appId);
	if (shutDown)
	{
		printf("\tState=Pending Shutdown\n");
		return;
	}
	
	if (pACstate != NULL)
	{
		printf("\tState=%d\n",pACstate->state);
	}
	else
	{
		printf("\tState=Unknown\n");
	}
	
	if (pACeventSeq != NULL)
	{
		for (int i = 0; i < pACeventSeq->length(); ++i)
		{
			printf("\tEvent[%d]: %d, %d\n",
				   i,
				   (*pACeventSeq)[i].srcId,
				   (*pACeventSeq)[i].eventCode);
		}
	}
	
	if (pACmetricsSeq != NULL)
	{
		for (int i = 0; i < pACmetricsSeq->length(); ++i)
		{
			printf("\tMetrics[%d]: %d, %d,%d,%d\n",
				   i,
				   (*pACmetricsSeq)[i].srcId,
				   (*pACmetricsSeq)[i].entityCount,
				   (*pACmetricsSeq)[i].bytesSent,
				   (*pACmetricsSeq)[i].bytesReceived);
		}
	}
	printf("\n");
}

bool Controller::RemoteSessionFactory::IsAvailable()
{
	com::xvd::neuron::acp::State *pACstate;
	com::xvd::neuron::acp::EventSeq *pACeventSeq;
	com::xvd::neuron::acp::MetricsSeq *pACmetricsSeq;
	
	pACstate = pACPMasterObject->GetState(m_appId);
	pACeventSeq = pACPMasterObject->GetEvents(m_appId);
	pACmetricsSeq = pACPMasterObject->GetMetrics(m_appId);
	
	if ((pACstate == NULL) || (pACstate->state != com::xvd::neuron::OBJECT_STATE_READY))
		return false;
	
	// TODO: Add some logic related to metrics
	
	return true;
}

ACPMasterObject* Controller::RemoteSessionFactory::GetACPMasterObject()
{
	return pACPMasterObject;
}

void Controller::RemoteSessionFactory::Send(char *cmd)
{
	sprintf(control->script,"%s",cmd);
	pACPMasterObject->Send(control,m_appId);
}

void Controller::RemoteSessionFactory::Shutdown()
{
	pACPMaster->DeleteMasterObject(pACPMasterObject);
	pACPMasterObject = NULL;
	shutDown = true;
}

bool Controller::RemoteSessionFactory::isShutdown()
{
	return shutDown;
}

void Controller::RemoteSessionFactory::enable()
{
	sprintf(control->script,"start");
	pACPMasterObject->Send(control,m_appId);
}

void Controller::RemoteSessionFactory::ping()
{
	sprintf(control->script,"ping");
	pACPMasterObject->Send(control,m_appId);
}

bool Controller::RemoteSessionFactory::isSame(DDS_Long _rank)
{
	return _rank == rank;
}


// Now for  Controller::ControllerEventHandler::
//
Controller::ControllerEventHandler::ControllerEventHandler(Controller *_controller) : EventHandlerT<ControllerEventHandler>()
{
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_STATE_UPDATE);
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_STATE_LOST);
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_STATE_DISPOSED);
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_METRICS_UPDATE);
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_EVENT);
	
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_STATE_UPDATE);
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_STATE_DISPOSED);
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_METRICS_UPDATE);
	AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_EVENT);
	controller = _controller;
}

void Controller::ControllerEventHandler::MyEventHandler(Event *e)
{
	pthread_mutex_lock(&controller->lock);
	
	// TODO: Don't like the use of reinterpret case. See if there is a better
	//  	 way to do this using with clean polymorphism
	switch (e->GetKind()) {
			
		//--------------------------------------------------------------------//
		case SCP_EVENT_SESSION_STATE_UPDATE:
			state = (reinterpret_cast<SCPEventSessionStateUpdate*>(e))->GetData();
			controller->RemoteSessionStateUpdate(state,(reinterpret_cast<SCPEventSessionStateUpdate*>(e))->GetSampleInfo());
			break;
		case SCP_EVENT_SESSION_STATE_DISPOSED:
			controller->RemoteSessionDeleted((reinterpret_cast<SCPEventSessionStateDisposed*>(e))->GetSessionId(),
											 (reinterpret_cast<SCPEventSessionStateDisposed*>(e))->GetSrcId());
			break;			
		case SCP_EVENT_SESSION_EVENT:
			event = (reinterpret_cast<SCPEventSessionEvent*>(e))->GetData();
			if (controller->pListener) 
			{
				controller->pListener->onSFSessionEvent(controller, event->sessionId, event->srcId,event);
			}
			break;
		case SCP_EVENT_SESSION_METRICS_UPDATE:
			metrics = (reinterpret_cast<SCPEventSessionMetricsUpdate*>(e))->GetData();
			if (controller->pListener) 
			{
				controller->pListener->onSFSessionMetricUpdate(controller, metrics->sessionId, metrics->srcId,metrics);
			}			
			break;
			
		//--------------------------------------------------------------------//
		case ACP_EVENT_SESSION_STATE_UPDATE:
			pACstate = (reinterpret_cast<ACPEventSessionStateUpdate*>(e))->GetData();
			controller->RemoteSFUpdate(pACstate,(reinterpret_cast<ACPEventSessionStateUpdate*>(e))->GetSampleInfo());
			break;
		case ACP_EVENT_SESSION_STATE_DISPOSED:
			controller->RemoteSFDeleted((reinterpret_cast<ACPEventSessionStateDisposed*>(e))->GetDstId());
			break;		
		case ACP_EVENT_SESSION_METRICS_UPDATE:
			break;
		case ACP_EVENT_SESSION_EVENT:
			break;
		default:
			printf("Unknown event: %d\n",e->GetKind());
			break;
	}
	
	pthread_mutex_unlock(&controller->lock);
}

//TODO: Is this method really needed?
void Controller::ControllerEventHandler::EventHandleLoop (void)
{
}

//!
//! \brief Main eventloop
//!
void* Controller::eventLoop(void *param)
{
	ControllerEventHandler *ev = (ControllerEventHandler*)param;
	//TODO: replace true with a flag which is set on destucion, and which
	// deletes its own thread
	while (true)
	{
		// TODO: This is not the best use of a thread. This origially done
		// because the event handler class did not have a way to wake up this
		// thread. THis needs to be fixed.
		usleep(100000);
		ev->HandleNextEvent();
	}
	return NULL;
}

