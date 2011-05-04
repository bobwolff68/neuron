#ifndef ENTITY_INFO_DISCOVERY_H_
#define ENTITY_INFO_DISCOVERY_H_

#include "H264DecoderSink.h"
#include "relayproxy.h"

#define RELAYPROXY_TEST_MODE    0
#define DECODERSINK_TEST_MODE   1

#define ASSERT_DDS_RETCODE_OK(retCode)  assert((retCode)==DDS_RETCODE_OK)
#define ASSERT_NOT_NULL(pObject)        assert((pObject)!=NULL)   
#define ASSERT_NOT_EQUAL(lvalue,rvalue) assert((lvalue)!=(rvalue))
#define ASSERT_EQUAL(lvalue,rvalue) assert((lvalue)==(rvalue))

class Entity_Info_Discovery_Test
{
    private:

	    DDSDomainParticipant   *pParticipant;
	    DDSTopic		       *pTopic;
	    DDSTopic               *pEntInfoTopic;
	    RelayProxy 		       *pRelay;
	    H264DecoderSink        *pDecoderSink;

        bool ExecuteSSH(const char *targetHost,const char *identityFile,const char *command);
        bool ExecuteSCP(const char *identityFile,const char *sourcePath, const char *destPath);
        
    public:

	    Entity_Info_Discovery_Test(int entityId,int trueSourceId,int strongestSourceId,int mode);
	    ~Entity_Info_Discovery_Test();
	    
	    bool Test_Source_Connect(const char *sourceHost,const char *identityFile);
	    bool Test_Intermed_RP_Connect(const char *rpHost, const char *identityFile);
	    bool Test_Intermed_RP_Disconnect(void);
};

#endif //ENTITY_INFO_DISCOVERY_H_
