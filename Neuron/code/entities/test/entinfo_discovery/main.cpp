#include "testcommon.h"
#include "entity_info_discovery_test.h"
#include "cppunit/config/SourcePrefix.h"
#include "TestingPeers.h"

using namespace std;

int testMode;
Entity_Info_Discovery_Test *pTestClass = NULL;
CPPUNIT_TEST_SUITE_REGISTRATION(TestingPeers);
extern int RunTests();

void TestingPeers::setUp(void)
{
    cout << "Init test class..." << endl;
    pTestClass = new Entity_Info_Discovery_Test(400,TEST_H264_SRC_ID,TEST_H264_SRC_ID,testMode);
}

void TestingPeers::tearDown(void)
{
    delete pTestClass;
}

void TestingPeers::Run_Entity_Info_Discovery_Test(void)
{
    cout << "Running Test_Source_Connect()..." << endl;
    CPPUNIT_ASSERT(pTestClass->Test_Source_Connect("manjesh@192.168.46.69",NULL));
    cout << "Running Test_Intermed_RP_Connect()..." << endl;
    CPPUNIT_ASSERT(pTestClass->Test_Intermed_RP_Connect("rwolff@192.168.46.30",NULL));
    cout << "Running Test_Intermed_RP_Disconnect()..." << endl;
    CPPUNIT_ASSERT(pTestClass->Test_Intermed_RP_Disconnect());
    
    return;
}

int main(int argc, char *argv[])
{
    testMode = (argc>1)?DECODERSINK_TEST_MODE:RELAYPROXY_TEST_MODE;
    return RunTests();
}

