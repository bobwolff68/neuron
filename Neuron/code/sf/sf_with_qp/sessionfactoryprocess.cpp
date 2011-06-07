#include "sessionfactoryprocess.h"

Q_DEFINE_THIS_FILE

bool SessionFactoryProcess::Init(int argc,char *argv[])
{
    char const* qsInitArg = "67:QSPYLOG";
    
    if(argc!=4 && argc!=5)
    {
        cout << "Usage: bin/sf <sfid> <sfname> <minibrain-ip> [<isEndpoint>]" << endl;
        return false;
    }

    //Scan session factory arguments
    sscanf(argv[1],"%u",&sfGuid);
    sfName = argv[2];
    mBrainIP = argv[3];

    //Init QSPY logger
    if(!QS_INIT(qsInitArg))
        return false;

    //Init QF real-time framework
    QF::init();
    
    //Init event pub-sub mechanism
    QF::psInit(evtSubList,Q_DIM(evtSubList));
    
    //Init event memory pool
    QF::poolInit(evtPool,sizeof(evtPool),sizeof(evtPool[0]));

    //Turn on QSPY filter for all records
    QS_FILTER_ON(QS_ALL_RECORDS);
    
    //Turn of unnecessary records
    QS_FILTER_OFF(QS_QF_INT_LOCK);
    QS_FILTER_OFF(QS_QF_INT_UNLOCK);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);
    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QK_SCHEDULE);
    
    //Assign dictionary name to event pool
    QS_OBJ_DICTIONARY(evtPool);
    
    //Create the session factory singleton instance
    return SessionFactory::CreateTheInstance(sfGuid,sfName,(argc==5)?true:false);
}

void SessionFactoryProcess::Run(void)
{
    //Send mini-brain info to registration state machine through an init event
    strcpy(initEvt.mBrainIP,mBrainIP.c_str());
    initEvt.httpPort = DEFAULT_MINIBRAIN_REG_PORT;

    //Add dictionary names to the registration state machine,
    //its timeout event and its event queue
    QS_OBJ_DICTIONARY(&sfsmReg);
    QS_OBJ_DICTIONARY(&sfsmReg.regTimeoutEvt);
    QS_OBJ_DICTIONARY(sfsmReg.evtQ);
    
    
    //Start execution of the registration state machine
    sfsmReg.Start(1,SF_EVENT(&initEvt));
    
    //Run the QF real-time framework
    QF::run();
    
    return;
}

void SessionFactoryProcess::Teardown(void)
{
    SessionFactory::DestroyTheInstance();
    return;
}
