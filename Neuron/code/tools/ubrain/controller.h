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

//! \class ControllerListener
//!
//! \brief Listener notifying on SF and Session changes. 
//!
//! Listeners can be installed by an application to listen to changes to
//! SF and Session objects. The listeners work at a slighty higher abstraction
//! than the controller, hiding some DDS concepts. The controller manages the
//! state of the obkjects, while the upper layer manages the state of the 
//! system. 
//!
//! A listener can only be installed by passing it to the constructor, thus it 
//! is immutable and non-chainable.
class Controller;
class ControllerListener {
	
public:
	
	//! \brief A new SF has been detected
	//!
	//! \param [in] srcId - srcId of the new SF
	//!
	//! \return 
	//!		- true if this SF is allowed into the system
	//!		- false if this SF is rejected from the system
	//!
	//! This listener is called when the controller has determined that a new
	//! has been detected. This callback is called <em> before </em> the 
	//! the controller starts to track the state of this SF. If the upper layer
	//! returns false on this callback, the controller will ignore all future
	//! events from the SF.
	virtual bool onSFDetect(Controller *c,int srcId) { return true; };
	
	//! \brief A state change has been detected on an SF
	//!
	//! \param [in] srcId - srcId of the SF
	//! \param [in] state - state of the SF
	//!
	//! \return 
	//!		- true if the listener has updated its state
	//!		- false if the listener failed to update its state	
	//! 
	//! This listener is called when the controller has detected a state-change
	//! on a known SF, that is a SF the controller already has detected 
	//! <em> and </em> the listener has accepted.
	//!
	//! If the listeners returns false, the controller will not update its 
	//! state, ensuring that the listener and the controller has a consistent 
	//! view. However, note that this does not mean it is a correct view since
	//! the state change already has occured. In general, this funtion should
	//! return true.
	virtual bool onSFStateChange(Controller *c,int srcId,com::xvd::neuron::acp::State *state) { return true; };
	
	//! \brief A SF has been succesfully deleted.
	//!
	//! \param [in] srcId - srcId of the SF
	//!
	//! \return 
	//!		- true if the listener has accepted the deletion
	//!		- false if the listener rejected the deletion
	//!
	//! This listener is called when the controller has detected that
	//! a known SF, that is a SF the controller already has detected 
	//! <em> and </em> the listener has accepted, has deleted itself after
	//! being instructed to do so via the controller.
	//!
	//! If the listeners returns false, the controller will not update its 
	//! state and will not remove the SF from its own data-structures, 
	//! ensuring that the listener and the controller has a consistent 
	//! view. However, note that this does not mean it is a correct view since
	//! the SF already has been deleted. In general, this funtion should
	//! return true.
	virtual bool onSFDelete(Controller *c,int srcId) { return true; };
	
	//! \brief A SF has terminated.
	//!
	//! \param [in] srcId - srcId of the SF
	//!
	//! \return 
	//!		- true if the listener has accepted the deletion
	//!		- false if the listener rejected the deletion
	//!
	//! This listener is called when the controller has detected that
	//! a known SF, that is a SF the controller already has detected 
	//! <em> and </em> the listener has accepted, has deleted itself 
	//! <b>without</b> being instructed to do so via the controller.
	//!
	//! If the listeners returns false, the controller will not update its 
	//! state and will not remove the SF from its own data-structures, 
	//! ensuring that the listener and the controller has a consistent 
	//! view. However, note that this does not mean it is a correct view since
	//! the SF already has been deleted. In general, this funtion should
	//! return true;
	virtual bool onSFTerminate(Controller *c,int srcId) { return true; };
	
	//! \brief A SF generated an event
	//!
	//! \param [in] srcId - srcId of the SF
	//!
	//! <em> TODO: Define the correct interface, add event? </em> 
	virtual void onSFEvent(Controller *c,int srcId,com::xvd::neuron::acp::Event *event) { return; };
	
	
	//! \brief A SF generated a metric update
	//!
	//! \param [in] srcId - srcId of the SF
	//!
	//! <em> TODO: Define the correct interface, add metrics? </em> 
	virtual void onSFMetricUpdate(Controller *c,int srcId,com::xvd::neuron::acp::Metrics *metrics) { return; };

	// ------------------------------------------------------------------------
	// -------------------- SESSION Listeners ---------------------------------
	// ------------------------------------------------------------------------

	//! \brief A new Session has been detected
	//!
	//! \param [in] srcId     - srcId of the SF creating the session
	//!
	//! \return 
	//!		- true if this Session created by this SF is allowed
	//!		- false if this Session from the SF is rejected
	//!
	//! This listener is called when the controller has determined that a new
	//! session from a SF has been detected. This callback is called <em> before 
	//! </em> the the controller starts to track the state of this SF. If the upper layer
	//! returns false on this callback, the controller will ignore all future
	//! events from the SF.
	virtual bool onSessionDetect(Controller *c,int sessionId) { return true; };

	//! \brief A  Session has been deleted
	//!
	//! \param [in] srcId     - srcId of the SF creating the session
	//
	//! \param [in] sessionId - sessionId. Id of the session
	//!
	//! \return 
	//!		- true if this Session created by this SF is allowed
	//!		- false if this Session from the SF is rejected
	//!
	virtual bool onSessionDelete(Controller *c,int sessionId) { return true; };
	
	//! \brief A state change has been detected on an session
	//!
	//! \param [in] sessionId - Id of the session
	//! \param [in] srcId     - srcId of the SF reporting the state
	//! \param [in] state     - state of the session
	//!
	//! \return 
	//!		- true if the listener has updated its state
	//!		- false if the listener failed to update its state	
	//! 
	//! This listener is called when the controller has detected a state-change
	//! on a known session at a known SF, that is a SF the controller already 
	//! has detected <em> and </em> the listener has accepted.
	//!
	//! If the listeners returns false, the controller will not update its 
	//! state, ensuring that the listener and the controller has a consistent 
	//! view. However, note that this does not mean it is a correct view since
	//! the state change already has occured. In general, this funtion should
	//! return true.
	virtual bool onSFSessionStateChange(Controller *c,
									  int sessionId, 
									  int srcId,
									  com::xvd::neuron::scp::State *state) { return true; };
	
	//! \brief A session has been succesfully deleted on a SF
	//!
	//! \param [in] sessionId - sessionId of the SF
	//!
	//! \param [in] srcId     - srcId of the SF
	//!
	//! \return 
	//!		- true if the listener has accepted the deletion
	//!		- false if the listener rejected the deletion
	//!
	//! This listener is called when the controller has detected that
	//! a known SF, that is a SF the controller already has detected 
	//! <em> and </em> the listener has accepted, has deleted itself after
	//! being instructed to do so via the controller.
	//!
	//! If the listeners returns false, the controller will not update its 
	//! state and will not remove the SF from its own data-structures, 
	//! ensuring that the listener and the controller has a consistent 
	//! view. However, note that this does not mean it is a correct view since
	//! the SF already has been deleted. In general, this funtion should
	//! return true.
	virtual bool onSFSessionDelete(Controller *c,int sessionId,int srcId) { return true; };
	
	//! \brief A SF has terminated.
	//!
	//! \param [in] srcId - srcId of the SF
	//!
	//! \return 
	//!		- true if the listener has accepted the deletion
	//!		- false if the listener rejected the deletion
	//!
	//! This listener is called when the controller has detected that
	//! a known SF, that is a SF the controller already has detected 
	//! <em> and </em> the listener has accepted, has deleted itself 
	//! <b>without</b> being instructed to do so via the controller.
	//!
	//! If the listeners returns false, the controller will not update its 
	//! state and will not remove the SF from its own data-structures, 
	//! ensuring that the listener and the controller has a consistent 
	//! view. However, note that this does not mean it is a correct view since
	//! the SF already has been deleted. In general, this funtion should
	//! return true;
	virtual bool onSFSessionTerminate(Controller *c,int sessionId,int srcId) { return true; };
	
	//! \brief A SF generated an event
	//!
	//! \param [in] srcId - srcId of the SF
	//!
	//! <em> TODO: Define the correct interface, add event? </em> 
	virtual void onSFSessionEvent(Controller *c,int sessionId,int srcId,com::xvd::neuron::scp::Event *event) { return; };
	
	
	//! \brief A SF generated a metric update
	//!
	//! \param [in] srcId - srcId of the SF
	//!
	//! <em> TODO: Define the correct interface, add metrics? </em> 
	virtual void onSFSessionMetricUpdate(Controller *c,int sessionId,int srcId,com::xvd::neuron::scp::Metrics *metrics) { return; };	
};

//! \class Controller
//!
//! \brief Trial manager of a network
//!
//! This class not only sets up the DDS planes but manages factories,
//! Sessions, events, and statefulness of the network.
class Controller
{

	class Session;

public:
    
    Controller(int appId,int domaindId,map<string,string> &nvPairs,ControllerListener *listener);
    ~Controller();
    void RemoteSessionStateUpdate(com::xvd::neuron::scp::State *state,DDS_SampleInfo *info);
	void RemoteSessionDeleted(int srcId,int sessionId);
    void RemoteSFUpdate(com::xvd::neuron::acp::State *state,DDS_SampleInfo *info);
	void RemoteSFDeleted(int srcId);
    void CmdHelp();
    bool CmdACP(int *argc,int max_argc,char **argv);
    bool CmdSCP(int *argc,int max_argc,char **argv);
    bool CmdSF(int *argc,int max_argc,char **argv);
    void ListRemoteSF();
    void ListSF();
    void ShutdownRemoteSF(int sfId);
    void CreateOrUpdateSession(int sessionId, set<int> *_sfs,const char *script,bool update);
    void ListSession();
    Session* GetSession(int sessionId);
    void DeleteSession(int sId,set<int>);
    bool runSingleCommand(const char*cmdIn);
    bool AddSCPMasterPeer(const char* descriptor);
    bool AddACPMasterPeer(const char* descriptor);
#ifndef ONLY_CONTROLLER_CLASS
    void run();
#endif

private:

	//! \class Session
    //!
    //! \brief Manage sessions
    //!
    //! This class represents a session from a controllers point of view
    //! Each session object creates a SCPMasterObject that manages one
    //! session. Note that a MasterObect can send to multiple SF.
    class RemoteSessionFactorySession
    {
	public:
		RemoteSessionFactorySession() {}
		com::xvd::neuron::ObjectState _state;
		com::xvd::neuron::ObjectState _expectedState;
    };
	
    //! \class Session
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
		void Delete();
		void DeleteSF(int sfId);
		bool IsDeleted();
		bool IsSFDeleted(int sfId);
		void UpdateSFSessionState(int srcId,com::xvd::neuron::scp::State *state);
		com::xvd::neuron::ObjectState localState;
    private:
        SCPMaster *sm;
        SCPMasterObject *scp;
        com::xvd::neuron::scp::Control *control;
		//std::set<int> sfs;
		std::map<int,RemoteSessionFactorySession*> sfs;
		bool locallyDeleted;
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
	typedef std::map<int,RemoteSessionFactorySession*> RemoteSessionFactorySessionMap;
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

    ControllerListener* pListener;
    RTIOsapiThread *eventThread;
	pthread_mutex_t lock;
};
