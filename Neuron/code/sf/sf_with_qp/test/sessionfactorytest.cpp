#include <iostream>
#include <string>
#include <cppunit/config/SourcePrefix.h>
#include "TestingPeers.h"
#include "sessionfactory.h"

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( TestingPeers );

void TestingPeers::setUp()
{
}

void TestingPeers::tearDown()
{
}

void TestingPeers::TestCase_SF_CreateTheInstance(void)
{
    uint32_t sfId = 100;
    string SFName = "testsf@localhost";
    bool bIsEndpoint = true;
    CPPUNIT_ASSERT_MESSAGE("SessionFactory::CreateTheInstance() 1st Call test failed...",
                           SessionFactory::CreateTheInstance(sfId,SFName,bIsEndpoint));
    CPPUNIT_ASSERT_MESSAGE("SessionFactory::CreateTheInstance() 2nd Call test failed...",
                           !SessionFactory::CreateTheInstance(sfId,SFName,bIsEndpoint));
    CPPUNIT_ASSERT_MESSAGE("sf->sfId!=100",sfId==SessionFactory::TheInstance()->Guid());
    CPPUNIT_ASSERT_MESSAGE("sf->sfname!=testsf@localhost",SFName==SessionFactory::TheInstance()->Name());
    CPPUNIT_ASSERT_MESSAGE("sf->bIsEndpoint!=true",bIsEndpoint==SessionFactory::TheInstance()->IsEndpoint());
                           
    SessionFactory::DestroyTheInstance();
    return;
}

void TestingPeers::TestCase_SF_DestroyTheInstance(void)
{
    string SFName = "testsf@localhost";
    SessionFactory::CreateTheInstance(100,SFName,false);
    CPPUNIT_ASSERT_MESSAGE("SessionFactory::DestroyTheInstance() 1st Call test failed...",
                           SessionFactory::DestroyTheInstance());
    CPPUNIT_ASSERT_MESSAGE("SessionFactory::DestroyTheInstance() 2nd Call test failed...",
                           !SessionFactory::DestroyTheInstance());    
    return;
}

extern int RunTests(void);

int main()
{
    return RunTests();
}
