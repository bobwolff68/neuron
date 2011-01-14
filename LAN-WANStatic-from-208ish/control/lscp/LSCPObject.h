//!
//! \file LSCPObject.h
//!
//! \brief Defintion of base-class Session Control Object
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef LSCPOBJECT_H_
#define LSCPOBJECT_H_

#include "ndds_cpp.h"
#include "CPInterfaceT.h"
#include "LSCPInterface.h"
#include "LSCPInterfaceSupport.h"

class LSCPObject : public CPObject {
public:
    
    LSCPObject(int _sfId, int _sessionId);
    
    ~LSCPObject();
        
    int GetSessionId();

    int GetSrcId();

protected:
    int sessionId;
    int srcId;    
};

#endif

