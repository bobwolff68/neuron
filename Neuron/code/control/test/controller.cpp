
//!
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
//!

#include "controller.h"

#include <string>
#include <sstream>

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
      strm >> arg;

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
    Controller::Controller(int appId, int domaindId, map<string,string> &nvPairs)
    {
	map<string,string>      PropertyPairsACP;
    	map<string,DDS_Boolean> PropagateDiscoveryFlagsACP;
    	map<string,string>      PropertyPairsSCP;
    	map<string,DDS_Boolean> PropagateDiscoveryFlagsSCP;

        pSessionEventHandler = new ControllerEventHandler(this);

        pCallback = NULL;

        m_appId = appId;

        // The Controller serves as the admin master for all SFs, thus connect as master
	PropertyPairsACP["dds.transport.wan_plugin.wan.transport_instance_id"] = nvPairs["ubrain_acp_id"];
	PropagateDiscoveryFlagsACP["dds.transport.wan_plugin.wan.transport_instance_id"] = DDS_BOOLEAN_FALSE;
	pACPMaster = new ACPMaster(pSessionEventHandler,appId,domaindId,"Controller::ACP", PropertyPairsACP, PropagateDiscoveryFlagsACP, "ACP");

        // The Controller manages sessions, thus connect to the SCP as master	
	PropertyPairsSCP["dds.transport.wan_plugin.wan.transport_instance_id"] = nvPairs["ubrain_scp_id"];
	PropagateDiscoveryFlagsSCP["dds.transport.wan_plugin.wan.transport_instance_id"] = DDS_BOOLEAN_FALSE;
    pSCPMaster = new SCPMaster(pSessionEventHandler,appId,domaindId,"Contoller::SCPMaster", PropertyPairsSCP, PropagateDiscoveryFlagsSCP, "SCP");

	m_domainId = domaindId;
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

        delete pSCPMaster;
        delete pACPMaster;
        delete pSessionEventHandler;
    }

    bool Controller::AddSCPMasterPeer(const char* descriptor)
    {
        if (!pSCPMaster)
            return false;

        return pSCPMaster->AddPeer(descriptor);
    }

    bool Controller::AddACPMasterPeer(const char* descriptor)
    {
        if (!pACPMaster)
            return false;

        return pACPMaster->AddPeer(descriptor);
    }

    //! \brief Handle events detetecd on the SCP
    void Controller::RemoteSessionUpdate(com::xvd::neuron::scp::State *state, DDS_SampleInfo *info)
    {
        printf("State update: sf: %d, scp: %d, state=%d\n",state->srcId,state->sessionId,state->state);

        if (pCallback)
        	pCallback->NewSessionState(state);

        if(state->state==com::xvd::neuron::OBJECT_STATE_OFFERSRC||state->state==com::xvd::neuron::OBJECT_STATE_SELECTSRC)
        	printf("Payload: %s\n",state->payload);
    }

    //! \brief Handle state updates from a SF
    void Controller::RemoteSFUpdate(com::xvd::neuron::acp::State *state,DDS_SampleInfo *info)
    {
        RemoteSessionFactoryMap::iterator it;
        RemoteSessionFactory *rsf;

        // TODO MANJESH - this becomes a CRASH if the remote SF has DIED. How can we handle this type of problem?
        it = remoteSF.find(state->srcId);

        if (it == remoteSF.end())
        {
            // A new SF has been detected (it was either created manually or by the
            // sf create ... command. In either case it will be added as a managed
            // SF.
//            printf("New SF detected. State=%d\n", (int)state->state);
            rsf = new RemoteSessionFactory(state->srcId,
                                           info->disposed_generation_count,
                                           pACPMaster);
            remoteSF[state->srcId] = rsf;

            if (state->state == com::xvd::neuron::OBJECT_STATE_STANDBY)
            {
                // This is the state we would expect if the SF is running
                // and has not been enabled by someone else
                rsf->enable();
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

            // Informative call back ...
            if (pCallback)
                pCallback->NewSFDetected(state->srcId);

            // Informative callback for upper watcher.
            if (pCallback)
                pCallback->NewSFState(state);
        }
        else
        {
            // We already know about this SF. In this case we act upon the
            // state change. If it has been deleted, we remove it from
            // our list of remote eSFs. If we initiated the delete,
            // isShutdown() returns true, it is all good. However, if someone
            // exited it we not this but still remove it from out list
            // of remote SFs.
            //
            // TODO: In this case we need to go through all the current sessions
            //       using using this SF and determine what to do.
            printf("Update to SF detected, state = %d/%d\n",state->state,info->disposed_generation_count);

            RemoteSessionFactoryMap::iterator rsfit;
            rsfit = remoteSF.find(state->srcId);
            if (state->state == com::xvd::neuron::OBJECT_STATE_DELETED)
            {
                if (rsfit != remoteSF.end())
                {
                    if (!rsfit->second->isShutdown()) {
                        printf("Unexpected delete received\n");
                    }
                    delete rsfit->second;
                    remoteSF.erase(state->srcId);
                    SFList.erase(state->srcId);
                }
                else
                {
                    // If by some chance we detect the
                    printf("SessionFactory %d does not exist\n",state->srcId);
                }
            }
            else if ((state->state == com::xvd::neuron::OBJECT_STATE_STANDBY) ||
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
                    if (state->state == com::xvd::neuron::OBJECT_STATE_STANDBY)
                    {
                        rsfit->second->enable();
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
            }

            // Informative callback for upper watcher.
            if (pCallback)
                pCallback->NewSFState(state);

        }
    }

    //! \brief Handle help command
    void Controller::CmdHelp()
    {
        printf("Interactive controller shell:\n\n");
        printf("acp ls                           - List detected SFs\n");
        printf("acp shutdown <sfId>+             - Shutdown listed SFs\n");
        printf("acp send <sfId>+ <script>        - Send script to SFs\n");
        printf("scp create <sId> <sfId>+ <script>  - Create a new session\n");
        printf("scp update <sId> <sfId>+ <script> - Update session \n");
        printf("scp delete <sId>                 - Delete a session\n");
        printf("scp ls                           - List known sessions\n");
        printf("sf create <sId>                  - Spawn a new SF\n");
        printf("sf delete <sId>                  - Delete a SF\n");
        printf("sf ls                            - List locally created SFs\n");
        printf("\n");
        printf("quit                             - Exit with no cleanup\n");
        printf("\n");
    }

    //! \brief Handle acp <..> commands
    void Controller::CmdACP(int *argc,int max_argc,char **argv)
    {
        ++(*argc);
        if (*argc == max_argc)
        {
            printf("usage: acp <cmd>\n");
            return;
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
                return;
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
                return;
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
                return;
            }
        }
        else
        {
            printf("unknown acp command: %s\n",argv[*argc]);
        }
    }

    //! \brief Handle scp <..> commands
    void Controller::CmdSCP(int *argc,int max_argc,char **argv)
    {
        ++(*argc);
        if (*argc == max_argc)
        {
            printf("usage: scp <cmd>\n");
            return;
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
                return;
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
            int sId;
            ++(*argc);
            if (*argc == max_argc)
            {
                printf("usage: scp delete <sessionId>\n");
                return;
            }
            sId = strtol(argv[*argc],NULL,0);
            DeleteSession(sId);
        }
        else if (!strcmp(argv[*argc],"ls"))
        {
            ListSession();
        }
        else
        {
            printf("unknown scp command: %s\n",argv[*argc]);
        }

    }

    //! \brief Handle sf <..> commands
    void Controller::CmdSF(int *argc,int max_argc,char **argv)
    {
        int sId, pId;
        //SessionFactory *aSF;

        ++(*argc);
        if (*argc == max_argc)
        {
            printf("usage: sf <cmd>\n");
            return;
        }
        if (!strcmp(argv[*argc],"create"))
        {
            ++(*argc);
            if (*argc == max_argc)
            {
                printf("usage: sf create <id>\n");
                return;
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
                return;
            }
            sId = strtol(argv[*argc],NULL,0);
            it = SFList.find(sId);
            //delete it->second;
            SFList.erase(sId);
        }
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
				printf("Create session %d using SF ",sessionId);
				session = new Session(pSCPMaster,sessionId);
				sessions[sessionId] = session;
        	}
        	else
        	{
        		// Now we have the 'found' session here so mark it and don't re-create an existing session.
                session = sit->second;

        		printf("Adding existing session on SF ", sessionId);
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
    void Controller::DeleteSession(int sId)
    {
        SessionMap::iterator it;

        it = sessions.find(sId);
        if (it != sessions.end())
        {
            printf("Delete session %d\n",sId);
            delete it->second;
            sessions.erase(sId);
        }
        else
        {
            printf("Session %d does not exist\n",sId);
        }
    }

    bool Controller::runSingleCommand(const char*cmdIn)
    {
        char **argv = (char**)calloc(sizeof(char*),200);
    	char thiscmdline[256];
    	int argc;
    	int s;

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

        return true;
    }

#ifndef ONLY_CONTROLLER_CLASS
    void Controller::run()
    {
        int quit = 0;
        char *line;
        char **argv = (char**)calloc(sizeof(char*),200);
        int argc;
        int s;
	char thiscmdline[256];

        using_history();

        while (!quit)
        {
            line = readline("controller>");
            if (!strcmp(line,"quit"))
            {
                quit = 1;
                continue;

            }

	    if (!strlen(line))
	      continue;

            add_history(line);

	    strcpy(thiscmdline, line);
            CreateArgvList(thiscmdline,&argc,argv);

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
        }
    }
#endif

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

            sprintf(control->script,"Created scp %d",scp->GetSessionId());
        }

        Controller::Session::~Session()
        {
            // Return the session object to the SCPMaster
            if (!sm->DeleteMasterObject(scp))
            {
                printf("error deleting master object\n");
            }

            // Delete local memory to store control and state
            com::xvd::neuron::scp::ControlTypeSupport::delete_data(control);
        }

        // Add a slave to the session
        void Controller::Session::AddSlave(int slaveId,const char *script)
        {
            sfs.insert(slaveId);
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
            std::set<int>::iterator it;

            it = sfs.find(slaveId);
            if (it == sfs.end())
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

        void Controller::Session::RemoveSlave(int slaveId)
        {
            sprintf(control->script,"delete scp %d",scp->GetSessionId());
            if (!scp->Send(control,slaveId))
            {
                printf("failed to send remove command to slave %d\n",slaveId);
            }
            else
            {
                sfs.erase(slaveId);
            }
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
            set<int>::iterator it;
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
            printf("Session [%d] status\n",scp->GetSessionId());

            com::xvd::neuron::scp::State *state=NULL;
            com::xvd::neuron::scp::EventSeq *events=NULL;
            com::xvd::neuron::scp::MetricsSeq *metrics=NULL;

            set<int>::iterator it;

            for (it = sfs.begin(); it != sfs.end(); ++it)
            {
                printf("checking state for %d\n",*it);
                state = scp->GetState(*it);
                events = scp->GetEvents(*it);
                metrics = scp->GetMetrics(*it);

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
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_METRICS_UPDATE);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,SCP_EVENT_SESSION_EVENT);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_STATE_UPDATE);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_METRICS_UPDATE);
            AddHandleFunc(&ControllerEventHandler::MyEventHandler,ACP_EVENT_SESSION_EVENT);
            controller = _controller;
        }

        void Controller::ControllerEventHandler::MyEventHandler(Event *e)
        {
            switch (e->GetKind()) {
                case SCP_EVENT_SESSION_STATE_UPDATE:
                    state = (reinterpret_cast<SCPEventSessionStateUpdate*>(e))->GetData();
                    controller->RemoteSessionUpdate(state,(reinterpret_cast<SCPEventSessionStateUpdate*>(e))->GetSampleInfo());
                    break;
                case SCP_EVENT_SESSION_METRICS_UPDATE:
                    metrics = (reinterpret_cast<SCPEventSessionMetricsUpdate*>(e))->GetData();
                    printf("Metrics update: sf: %d, scp: %d, entityCount=%d,bytesSent=%d,bytesReceived=%d\n",metrics->srcId,metrics->sessionId,metrics->entityCount,metrics->bytesSent,metrics->bytesReceived);
                    break;
                case SCP_EVENT_SESSION_EVENT:
                    event = (reinterpret_cast<SCPEventSessionEvent*>(e))->GetData();
                    printf("Event update: sf: %d, scp: %d, eventCode=%d\n",event->srcId,event->sessionId,event->eventCode);
                    break;
                case ACP_EVENT_SESSION_STATE_UPDATE:
                    pACstate = (reinterpret_cast<ACPEventSessionStateUpdate*>(e))->GetData();
                    controller->RemoteSFUpdate(pACstate,(reinterpret_cast<ACPEventSessionStateUpdate*>(e))->GetSampleInfo());
                    break;
                case ACP_EVENT_SESSION_METRICS_UPDATE:
                    break;
                case ACP_EVENT_SESSION_EVENT:
                    break;
                default:
                    printf("Unknown event: %d\n",e->GetKind());
            }
        }

        void Controller::ControllerEventHandler::EventHandleLoop (void)
        {
        }

    //!
    //! \brief Main eventloop
    //!
    void* Controller::eventLoop(void *param)
    {
        ControllerEventHandler *ev = (ControllerEventHandler*)param;
        while (true)
        {
            usleep(100000);
            ev->HandleNextEvent();
        }
        return NULL;
    }

#ifndef ONLY_CONTROLLER_CLASS
//! \brief Main entry point
int
main(int argc, char **argv)
{
    int domainId = 67;
    int appId = 0;
	Controller *tester = NULL;
    int i;
    DDSDomainParticipantFactory *factory = DDSDomainParticipantFactory::get_instance();

    DDS_DomainParticipantFactoryQos fqos;

    factory->get_qos(fqos);
    fqos.resource_limits.max_objects_per_thread = 8192;
    factory->set_qos(fqos);

    for (i = 0; i < argc; ++i)
    {
        if (!strcmp("-appId",argv[i]))
        {
            ++i;
            if (i == argc)
            {
                printf("-appId <appId>\n");
                break;
            }
            appId = strtol(argv[i],NULL,0);
        }
        else if (!strcmp("-domainId",argv[i]))
        {
            ++i;
            if (i == argc)
            {
                printf("-domainId <domainId>\n");
                break;
            }
            domainId = strtol(argv[i],NULL,0);
        }
        /*else if (!strcmp("-appId",argv[i]))
        {
            ++i;
            if (i == argc)
            {
                printf("-appId <appId>\n");
                break;
            }
        }*/
    }

    if (i < argc) {
        return -1;
    }

    tester = new Controller(appId,domainId);

    if (tester == NULL)
    {
        return -1;
    }

    tester->run();

	return 0;
}
#endif

