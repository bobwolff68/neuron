//!
//! \file sessionfactoryprocess.h
//!
//! \brief Definition of the session factory process namespace.
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#ifndef SESSIONFACTORYPROCESS_H_
#define SESSIONFACTORYPROCESS_H_

#include "sfsmregistration.h"

//!
//! \namespace SessionFactoryProcess
//!
//! \brief Namespace encapsulating the variables and the methods used to run a session factory.
//!
namespace SessionFactoryProcess
{
    //!
    //! \var sfGuid
    //!
    //! \brief Globally unique id of the session factory instance.
    //!
    static uint32_t sfGuid = 0;
    
    //!
    //! \var sfName
    //!
    //! \brief Globally unique human-readable name of the session factory instance.
    //!
    static string sfName = "sf@localhost";
    
    //!
    //! \var mBrainIP
    //!
    //! \brief IP address of the mini-brain handling this session factory instance.
    //!
    static string mBrainIP = "127.0.0.1";
    
    //!
    //! \var initEvt
    //!
    //! \brief Event instance containing mini-brain info for initializing registration client.
    //!
    static MinibrainInfoEvent initEvt;
    
    //!
    //! \var sfsmReg
    //!
    //! \brief Instance of the session factory registration state machine.
    //!
    static SfsmRegistration sfsmReg;
    
    //!
    //! \brief Function to initialize the session factory process.
    //!
    //! \param [in] argc - Command line rgument count.
    //! \param [in] argv - Command line argument list.
    //!
    //! \return VOID.
    //!
    bool Init(int argc, char *argv[]);
    
    //!
    //! \brief Function to run the session factory process.
    //!
    //! \return VOID.
    //!
    void Run(void);
    
    //!
    //! \brief Function to terminate the session factory process.
    //!
    //! \return VOID.
    //!
    void Teardown(void);
};

#endif //SESSIONFACTORYPROCESS_H_
