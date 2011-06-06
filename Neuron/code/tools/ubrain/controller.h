//!
//! \file controller.h
//!
//! \brief An example controller - class definition.
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
//! This code is provided "as is", without warranty, expressed or implied.
//!

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/// If we are going to only use the class, we don't use the 'run()' or readline/history.
#ifndef ONLY_CONTROLLER_CLASS
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "ndds_cpp.h"
#include "neuroncommon.h"
#include "controlplane.h"

#include <set>
#include <map>
#include <list>

class Controller;

#include "testjigsupport.h"

//! \class Controller
//!
//! \brief Trial manager of a network
//!
//! This class not only sets up the DDS planes but manages factories,
//! Sessions, events, and statefulness of the network.
class Controller : public TestJigSupport
{

	class Session;

public:
    
    Controller(int appId,int domaindId,map<string,string> &nvPairs);
    ~Controller();
    void RemoteSessionUpdate(com::xvd::neuron::scp::State *state,DDS_SampleInfo *info);
    void RemoteSFUpdate(com::xvd::neuron::acp::State *state,DDS_SampleInfo *info);
    void CmdHelp();
    void CmdACP(int *argc,int max_argc,char **argv);
    void CmdSCP(int *argc,int max_argc,char **argv);
    void CmdSF(int *argc,int max_argc,char **argv);
    void ListRemoteSF();
    void ListSF();
    void ShutdownRemoteSF(int sfId);
    void CreateOrUpdateSession(int sessionId, set<int> *_sfs,const char *script,bool update);
    void ListSession();
    Session* GetSession(int sessionId);
    void DeleteSession(int sId);
    bool runSingleCommand(const char*cmdIn);
    void SetCallback(CallbackBase* pc) { pCallback = pc; };
    bool AddSCPMasterPeer(const char* descriptor);
    bool AddACPMasterPeer(const char* descriptor);
#ifndef ONLY_CONTROLLER_CLASS
    void run();
#endif
//! \brief Callback to Lua is defined in terms of the controlling singleton TestInstance
    bool MakeLuaCallback(const char* fnName, int id, const char* txt)
		{ if (pCallback) return pCallback->MakeLuaCallback(fnName, (int)id, "junk"); 
		  else return false; };


private:
    //! \class RemoteSessionFactory
    //!
    //! \brief Manage sessions
    //!
    //! This class represents a session from a controllers point of view
    //! Each session object creates a SCPMasterObject that manages one
    //! session. Note that a MasterObect can send to multiple SF.
    class Session 
    {
    public:
        
        // \brief The constructor takes the SCP master object and scp ID as input. The
        // SCP Master attaches to the SCP plane and can manipulate any scp. 
        Session(SCPMaster *_sm,int _sid);
        ~Session();
        void AddSlave(int slaveId,const char *script);
        bool UpdateSF(int slaveId,const char *script);
        void RemoveSlave(int slaveId);
        bool SlavesAreReady(set<int> *pending);
        bool SendCommand(const char *script,int slaveId);
        void ReportSessionStatus(void);
    private:
        SCPMaster *sm;
        SCPMasterObject *scp;
        com::xvd::neuron::scp::Control *control;
        std::set<int> sfs;
    };

    //! \class RemoteSessionFactory
    //!
    //! \brief Manage remote session factories
    //!
    //! This class represents the controllers view of the state of 
    //! SFs. A session factory is managed via the ACP control
    //! plane. Each RemoteSessionFactory entity create a ACPMasterObject, and
    //! this object is used to manage exactly one SF. This class must handle
    //! cases where an SF may already exist (created manually) or forked by
    //! the controller. It must also handle the case where the controller
    //! itself is restarted. Note the very little information is kept in this
    //! object, all the state is accessed via the MasterObject, and the
    //! state itself is stored in DDS.
    //!
    class RemoteSessionFactory
    {
    public:
        RemoteSessionFactory(int appId,DDS_Long _rank,ACPMaster *_pACPMaster);
        ~RemoteSessionFactory();
        void PrintInfo();
        bool IsAvailable();
        ACPMasterObject* GetACPMasterObject();
        void Send(char *cmd);
        void Shutdown();
        bool isShutdown();
        void enable();
        void ping();
        bool isSame(DDS_Long _rank);
    private:
        enum {
            INIT,
            DETECTED,            
            READY
        } localState;
        
        int m_appId;
        bool shutDown;
        ACPMasterObject *pACPMasterObject;
        ACPMaster *pACPMaster;
        com::xvd::neuron::acp::Control *control;
        DDS_Long rank;
    };
    
    //! \class ControllerEventHandler
    //!
    //! \brief Handle ACP and SCP Events
    //!    
    class ControllerEventHandler : public EventHandlerT<ControllerEventHandler>
    {
    public:
        com::xvd::neuron::scp::State *state;
        com::xvd::neuron::scp::Event *event;
        com::xvd::neuron::scp::Metrics *metrics;
        com::xvd::neuron::acp::State *pACstate;
        com::xvd::neuron::acp::Event *pACevent;
        com::xvd::neuron::acp::Metrics *pACetrics;        
        Controller *controller;
        ControllerEventHandler(Controller *_controller);
        void MyEventHandler(Event *e);
        virtual void EventHandleLoop (void);
    };
    
    static void* eventLoop(void *param);

    typedef std::map<int,RemoteSessionFactory *> RemoteSessionFactoryMap;
    typedef std::map<int,Session *> SessionMap;

    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    std::map<int,pid_t> SFList;

    //! \var pSCPMaster 
    //! \brief Attachment to the SCP plane
    SCPMaster *pSCPMaster;
    
    //! \var pACPMaster 
    //! \brief Attachment to the ACP plane
    ACPMaster *pACPMaster;
    
    //! \var pACPMasterObject
    //! \brief Objects used to manage SFs. Only one is needed since there is 
    //!        only a single management "session"
    ACPMasterObject *pACPMasterObject;
    
    //! \var sfs
    //! \brief Track all SessionFactories. Key is the ID of the SF
    RemoteSessionFactoryMap remoteSF;

    //! \var sessions
    //! \brief Track all Sessions. Key is the ID of the session
    SessionMap sessions;
    
    ControllerEventHandler *pSessionEventHandler;
    
    int m_domainId;
    
    int m_appId;

    CallbackBase* pCallback;
    RTIOsapiThread *eventThread;
};
#endif
