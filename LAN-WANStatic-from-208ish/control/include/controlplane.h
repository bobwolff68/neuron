#ifndef CONTROL_PLANE_H
#define CONTROL_PLANE_H

#include "CPInterface.h"
#include "CPInterfaceT.h"
#include "idl/CPInterfaceIDL.h"
#include "idl/CPInterfaceIDLSupport.h"

#include "scp/idl/SCPInterface.h"
#include "scp/idl/SCPInterfaceSupport.h"
#include "scp/SCPEvent.h"
#include "scp/SCPMaster.h"
#include "scp/SCPSlave.h"
#define SCP_MASTER_NAME		"SCPMaster"
#define	SCP_SLAVE_NAME		"SCPSlave"

#include "lscp/idl/LSCPInterface.h"
#include "lscp/idl/LSCPInterfaceSupport.h"
#include "lscp/LSCPEvent.h"
#include "lscp/LSCPMaster.h"
#include "lscp/LSCPSlave.h"
#define LSCP_MASTER_NAME	"LSCPMaster"
#define LSCP_SLAVE_NAME		"LSCPSlave"

#include "ecp/idl/ECPInterface.h"
#include "ecp/idl/ECPInterfaceSupport.h"
#include "ecp/ECPEvent.h"
#include "ecp/ECPMaster.h"
#include "ecp/ECPSlave.h"

#include "acp/idl/ACPInterface.h"
#include "acp/idl/ACPInterfaceSupport.h"
#include "acp/ACPEvent.h"
#include "acp/ACPMaster.h"
#include "acp/ACPSlave.h"
#define ACP_MASTER_NAME	"ACPMaster"
#define ACP_SLAVE_NAME	"ACPSlave"

#define	GEN_CP_INTERFACE_NAME(name,ownerName,CPInterfaceTypeName)\
		{\
			strcpy(name,ownerName);\
			strcat(name,"::");\
			strcat(name,CPInterfaceTypeName);\
		}

#endif
