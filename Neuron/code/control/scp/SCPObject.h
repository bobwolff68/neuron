//!
//! \file SCPObject.h
//!
//! \brief Defintion of base-class Session Control Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef SCPOBJECT_H_
#define SCPOBJECT_H_

#include "ndds_cpp.h"
#include "CPInterfaceT.h"
#include "SCPInterface.h"
#include "SCPInterfaceSupport.h"

class SCPObject : public CPObject {
public:
    
    SCPObject(int _sfId, int _sessionId);
    
    ~SCPObject();
        
    int GetSessionId();

    int GetSrcId();

protected:
    int sessionId;
    int srcId;
    
    // These objects go on the bus, depending on which Method is used.
    com::xvd::neuron::session::Control *control;
};

#endif

