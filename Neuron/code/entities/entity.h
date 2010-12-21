#ifndef ENTITY_H_
#define ENTITY_H_

#include "ndds_cpp.h"
#include "neuroncommon.h"

#define ENTITY_KIND_NATNUMSRC       0
#define ENTITY_KIND_STDOUTSINK      1
#define ENTITY_KIND_RELAYPROXY      2
#define ENTITY_KIND_H264FILESRC     3
#define ENTITY_KIND_H264DECODERSINK 4

#define MEDIA_TOPIC_NAME(topicStr,prefix,sessionId)\
        {\
            sprintf(topicStr,"%s",prefix);\
            strcat(topicStr,ToString<int>(sessionId).c_str());\
        }

class SessionEntity
{
    protected:

        int id;
        int ownerId;
        int sessionId;
        int kind;
        DDSDomainParticipant *pOwnerDP;

    public:

        SessionEntity(DDSDomainParticipant *pOwnerDPP,int idP,int ownerIdP,int sessionIdP,int kindP)
        {
            id = idP;
            pOwnerDP = pOwnerDPP;
            ownerId = ownerIdP;
            sessionId = sessionIdP;
            kind = kindP;
        }

        ~SessionEntity()
        {
        }

        bool AddPeer(const char *peerIP[],int srcEntityId)
        {
            DDS_ReturnCode_t retCode;

            if(id==srcEntityId)
            {
                retCode = pOwnerDP->add_peer(peerIP[1]);
                if(retCode!=DDS_RETCODE_OK)
                {
                    std::cout << "Cannot add to peer list: " << peerIP[1] << std::endl;
                    return false;
                }
            }
            else
            {
                retCode = pOwnerDP->add_peer(peerIP[0]);
                if(retCode!=DDS_RETCODE_OK)
                {
                    std::cout << "Cannot add to peer list: " << peerIP[0] << std::endl;
                    return false;
                }
            }

            return true;
        }

        int GetKind(void)
        {
            return kind;
        }

        int GetId(void)
        {
            return id;
        }
};

#endif // ENTITY_H_
