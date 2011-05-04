// Entity Information Discovery Test
// Manjesh Malavalli
// XVDTH, USA

#include <assert.h>
#include "testcommon.h"
#include "entity_info_discovery_test.h"

using namespace std;

Entity_Info_Discovery_Test::Entity_Info_Discovery_Test(int entityId,int trueSourceId,int strongestSourceId,int mode)
{
    const char         *typeName = NULL;
    DDS_ReturnCode_t    retCode = DDS_RETCODE_OK;
    
    ASSERT_NOT_EQUAL(entityId,TEST_INTERMED_RP_ID);
    pParticipant = DDSTheParticipantFactory->create_participant_with_profile(TEST_DOMAIN_ID,
                                                                             "NEURON","MEDIALAN",
                                                                             NULL,DDS_STATUS_MASK_NONE);
    ASSERT_NOT_NULL(pParticipant);
    typeName = com::xvd::neuron::media::DataUnitTypeSupport::get_type_name();
    ASSERT_NOT_NULL(typeName);
    ASSERT_DDS_RETCODE_OK(com::xvd::neuron::media::DataUnitTypeSupport::register_type(pParticipant,typeName));
    pTopic = pParticipant->create_topic(TEST_VIDEO_TOPIC_NAME,typeName,DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
    ASSERT_NOT_NULL(pTopic);
    
    typeName = com::xvd::neuron::media::EntityInfoTypeSupport::get_type_name();
    ASSERT_NOT_NULL(typeName);
    ASSERT_DDS_RETCODE_OK(com::xvd::neuron::media::EntityInfoTypeSupport::register_type(pParticipant,typeName));
    pEntInfoTopic = pParticipant->create_topic(TEST_ENTINFO_TOPIC_NAME,typeName,DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
    ASSERT_NOT_NULL(pEntInfoTopic);
    
    if(mode==RELAYPROXY_TEST_MODE)
    {
        pDecoderSink = NULL;
        pRelay = new RelayProxy(entityId,trueSourceId,strongestSourceId,0,TEST_SESSION_ID,pParticipant,pTopic,TEST_NUM_LAYERS,"*");
        pRelay->startThread();
    }
    else
    {
        pRelay = NULL;
        pDecoderSink = new H264DecoderSink(entityId,trueSourceId,strongestSourceId,0,TEST_SESSION_ID,"entity_info_test.264",pParticipant,pTopic,"*");
        pDecoderSink->startThread();
    }
}

Entity_Info_Discovery_Test::~Entity_Info_Discovery_Test()
{
    if(pRelay)
    {
        pRelay->stopThread();
        delete pRelay;
    }
    else
    {
        pDecoderSink->stopThread();
        delete pDecoderSink;
    }
    
    ASSERT_DDS_RETCODE_OK(pParticipant->delete_topic(pEntInfoTopic));
    ASSERT_DDS_RETCODE_OK(pParticipant->delete_topic(pTopic));
    ASSERT_DDS_RETCODE_OK(DDSTheParticipantFactory->delete_participant(pParticipant));
}

bool Entity_Info_Discovery_Test::ExecuteSSH(const char *targetHost,const char *identityFile,const char *command)
{
    stringstream    sshCmd;
    
    sshCmd << "ssh";
    if(identityFile)
        sshCmd << " -i " << identityFile;
        
    sshCmd << " " << targetHost << " \"" << command << "\"";
    cout << "SSH Command: " << sshCmd.str() << endl;
    return ((bool)(system(sshCmd.str().c_str())!=-1));
}

bool Entity_Info_Discovery_Test::ExecuteSCP(const char *identityFile,const char *sourcePath, const char *destPath)
{
    stringstream    scpCmd;
    
    scpCmd << "scp";
    if(identityFile)
        scpCmd << " -i " << identityFile;
        
    scpCmd << " " << sourcePath << " " << destPath;
    cout << "SCP Command: " << scpCmd.str() << endl;
    return ((bool)(system(scpCmd.str().c_str())!=-1));
}

bool Entity_Info_Discovery_Test::Test_Source_Connect(const char *sourceHost,const char *identityFile)
{
    string          destPath = sourceHost;
    stringstream    sshRemoteCmd;
    bool            comparisons[4] = {false,false,false,false};
    
    destPath += ":";
    ASSERT_EQUAL(ExecuteSSH(sourceHost,identityFile,"killall h264source_standalone"),true);
    ASSERT_EQUAL(ExecuteSCP(identityFile,"~/h264source_standalone",destPath.c_str()),true);
    ASSERT_EQUAL(ExecuteSCP(identityFile,"USER_QOS_PROFILES.xml",destPath.c_str()),true);
    
    sshRemoteCmd << "~/h264source_standalone " << TEST_H264_SRC_ID 
                 << " </dev/null >>/tmp/h264source_standalone_out.log 2>&1 &";
    ASSERT_EQUAL(ExecuteSSH(sourceHost,identityFile,sshRemoteCmd.str().c_str()),true);
    
    cout << endl << "Running Test_Source_Connect()..." << endl << endl;
    usleep(10000000);
    
    SessionEntity *pEntity = (pRelay?(SessionEntity *)pRelay:(SessionEntity *)pDecoderSink);
    comparisons[0] = (bool)(pEntity->GetUplineEntityInfo().entityId==TEST_H264_SRC_ID);
    cout << "Condition 'pEntity->GetUplineEntityInfo().entityId==TEST_H264_SRC_ID': " << (comparisons[0]?"PASSED":"FAILED") << endl;
    comparisons[1] = (bool)(pEntity->GetUplineEntityInfo().uplineSourceId==-1);
    cout << "Condition 'pEntity->GetUplineEntityInfo().uplineSourceId==-1': " << (comparisons[1]?"PASSED":"FAILED") << endl;
    comparisons[2] = (bool)(pEntity->GetUplineEntityInfo().hopsFromTrueSource==0);    
    cout << "Condition 'pEntity->GetUplineEntityInfo().hopsFromTrueSource==0': " << (comparisons[2]?"PASSED":"FAILED") << endl;
    comparisons[3] = (bool)(pEntity->GetUplineEntityInfo().trueSourceId==pEntity->GetUplineEntityInfo().entityId);    
    cout << "Condition 'pEntity->GetUplineEntityInfo().trueSourceId==pEntity->GetUplineEntityInfo().entityId': " 
         << (comparisons[3]?"PASSED":"FAILED") << endl;
    
    cout << "Test_Source_Connect(): " << ((comparisons[0]&&comparisons[1]&&comparisons[2]&&comparisons[3])?"PASSED":"FAILED") << endl;
    return ((bool)(comparisons[0]&&comparisons[1]&&comparisons[2]&&comparisons[3]));
}

bool Entity_Info_Discovery_Test::Test_Intermed_RP_Connect(const char *rpHost,const char *identityFile)
{
    string          destPath = rpHost;
    stringstream    sshRemoteCmd;
    bool            comparisons[4] = {false,false,false,false};

    pRelay?pRelay->UpdateVideoSource(TEST_INTERMED_RP_ID):pDecoderSink->UpdateVideoSource(TEST_INTERMED_RP_ID);
    
    destPath += ":";
    ASSERT_EQUAL(ExecuteSSH(rpHost,identityFile,"killall rp_standalone"),true);
    ASSERT_EQUAL(ExecuteSCP(identityFile,"~/rp_standalone",destPath.c_str()),true);
    ASSERT_EQUAL(ExecuteSCP(identityFile,"USER_QOS_PROFILES.xml",destPath.c_str()),true);
    
    sshRemoteCmd << "killall rp_standalone;~/rp_standalone " << TEST_INTERMED_RP_ID << " " << TEST_H264_SRC_ID << " " << TEST_H264_SRC_ID 
                 << " </dev/null >>/tmp/rp_standalone_out.log 2>&1 &";
    ASSERT_EQUAL(ExecuteSSH(rpHost,identityFile,sshRemoteCmd.str().c_str()),true);
    
    cout << endl << "Running Test_Intermed_RP_Connect()..." << endl << endl;
    usleep(5000000);
    
    SessionEntity *pEntity = (pRelay?(SessionEntity *)pRelay:(SessionEntity *)pDecoderSink);
    comparisons[0] = (bool)(pEntity->GetUplineEntityInfo().entityId==TEST_INTERMED_RP_ID);
    cout << "Condition 'pEntity->GetUplineEntityInfo().entityId==TEST_INTERMED_RP_ID': " << (comparisons[0]?"PASSED":"FAILED") << endl;
    comparisons[1] = (bool)(pEntity->GetUplineEntityInfo().uplineSourceId==TEST_H264_SRC_ID);
    cout << "Condition 'pEntity->GetUplineEntityInfo().uplineSourceId==TEST_H264_SRC_ID': " << (comparisons[1]?"PASSED":"FAILED") << endl;
    comparisons[2] = (bool)(pEntity->GetUplineEntityInfo().hopsFromTrueSource==1);    
    cout << "Condition 'pEntity->GetUplineEntityInfo().hopsFromTrueSource==1': " << (comparisons[2]?"PASSED":"FAILED") << endl;
    comparisons[3] = (bool)(pEntity->GetUplineEntityInfo().trueSourceId==TEST_H264_SRC_ID);    
    cout << "Condition 'pEntity->GetUplineEntityInfo().trueSourceId==TEST_H264_SRC_ID': " << (comparisons[3]?"PASSED":"FAILED") << endl;
  
    cout << "Test_Intermed_RP_Connect(): " << ((comparisons[0]&&comparisons[1]&&comparisons[2]&&comparisons[3])?"PASSED":"FAILED") << endl;
    return ((bool)(comparisons[0]&&comparisons[1]&&comparisons[2]&&comparisons[3]));
}

bool Entity_Info_Discovery_Test::Test_Intermed_RP_Disconnect(void)
{
    bool    comparisons[4];
    
    cout << endl << "Running Test_Intermed_RP_Disconnect()... (waiting for intermediate RP to shut down)" << endl << endl;
    usleep(20000000);
    
    SessionEntity *pEntity = (pRelay?(SessionEntity *)pRelay:(SessionEntity *)pDecoderSink);
    comparisons[0] = (bool)(pEntity->GetUplineEntityInfo().entityId==TEST_H264_SRC_ID);
    cout << "Condition 'pEntity->GetUplineEntityInfo().entityId==TEST_H264_SRC_ID': " << (comparisons[0]?"PASSED":"FAILED") << endl;
    comparisons[1] = (bool)(pEntity->GetUplineEntityInfo().uplineSourceId==-1);
    cout << "Condition 'pEntity->GetUplineEntityInfo().uplineSourceId==-1': " << (comparisons[1]?"PASSED":"FAILED") << endl;
    comparisons[2] = (bool)(pEntity->GetUplineEntityInfo().hopsFromTrueSource==0);    
    cout << "Condition 'pEntity->GetUplineEntityInfo().hopsFromTrueSource==0': " << (comparisons[2]?"PASSED":"FAILED") << endl;
    comparisons[3] = (bool)(pEntity->GetUplineEntityInfo().trueSourceId==pEntity->GetUplineEntityInfo().entityId);    
    cout << "Condition 'pEntity->GetUplineEntityInfo().trueSourceId==pEntity->GetUplineEntityInfo().entityId': " 
         << (comparisons[3]?"PASSED":"FAILED") << endl;
    
    cout << "Test_Source_Connect(): " << ((comparisons[0]&&comparisons[1]&&comparisons[2]&&comparisons[3])?"PASSED":"FAILED") << endl;
    return ((bool)(comparisons[0]&&comparisons[1]&&comparisons[2]&&comparisons[3]));
}

