//!
//! \file ACPObject.h
//!
//! \brief Defintion of base-class Session Control Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef ACPOBJECT_H_
#define ACPOBJECT_H_

#include "ndds_cpp.h"
#include "CPInterfaceT.h"
#include "ACPInterface.h"
#include "ACPInterfaceSupport.h"

class ACPObject : public CPObject {
public:
    
    ACPObject(int _sfId, int _sessionId);
    
    ~ACPObject();
        
    int GetSessionId();

    int GetSrcId();

protected:
    int sessionId;
    int srcId;    
};

#endif

