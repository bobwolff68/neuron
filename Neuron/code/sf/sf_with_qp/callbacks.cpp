#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#include "qp_port.h"

#ifdef Q_SPY
    #include "qsddslogger.h"
#endif //Q_SPY

using namespace std;

Q_DEFINE_THIS_FILE

extern "C" void *IdleThreadRoutine(void* pThreadData)
{
    cout << "ENTERED IDLE THREAD" << endl;
    for(;;)
    {
        struct timeval timeout = {0};
        fd_set con;
        FD_ZERO(&con);
        FD_SET(0,&con);
        timeout.tv_usec = 8000;
        select(1,&con,0,0,&timeout);
        
#ifdef Q_SPY
        uint16_t blockSize = 1000;
        uint8_t const* pBlock = (uint8_t*)0;

        if(QSDDSLogger::TheInstance()->done)
        {
            for(;;)
            {
                QF_INT_LOCK(ignore);
                pBlock = QS::getBlock(&blockSize);
                QF_INT_UNLOCK(ignore);
                if(pBlock!=(uint8_t*)0)
                    QSDDSLogger::TheInstance()->Log((uint8_t*)pBlock,blockSize);
                else
                    break;
            }
            
            break;
        }
        else
        {
            QF_INT_LOCK(ignore);
            pBlock = QS::getBlock(&blockSize);
            QF_INT_UNLOCK(ignore);
            if(pBlock!=(uint8_t*)0)
                QSDDSLogger::TheInstance()->Log((uint8_t*)pBlock,blockSize);
        }
#endif           
    }

    cout << "EXITING IDLE THREAD" << endl;
    return (void*)0;
}

void QF::onStartup(void)
{
    // SCHED_FIFO corresponds to real-time preemptive priority-based scheduler
    // NOTE: This scheduling policy requires the superuser priviledges
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

    struct sched_param param;
    param.sched_priority = sched_get_priority_min(SCHED_FIFO);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_t idle;
    if (pthread_create(&idle, &attr, &IdleThreadRoutine, 0) != 0) {
               // Creating the p-thread with the SCHED_FIFO policy failed.
               // Most probably this application has no superuser privileges,
               // so we just fall back to the default SCHED_OTHER policy
               // and priority 0.
        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        param.sched_priority = 0;
        pthread_attr_setschedparam(&attr, &param);
        Q_ALLEGE(pthread_create(&idle, &attr, &IdleThreadRoutine, 0) == 0);
    }
    pthread_attr_destroy(&attr);
    return;
}

void QF::onCleanup(void)
{
    QS_EXIT();
    return;
}

void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line)
{
    cerr << "@ " << file << "(line " << line << "): ASSERTION FAILED" << endl;
    QF::stop();
    return;
}

#ifdef Q_SPY

uint8_t QS::onStartup(void const *arg) 
{
    uint32_t domainId;
    char     topicName[50];

    sscanf((char const*)arg,"%u:%s",&domainId,topicName);
    cout << "QSDDSLOGGER: domainId=" << domainId << ",topicName='" << topicName << "'" << endl; 
    QSDDSLogger::CreateTheInstance(domainId,topicName);
    initBuf(QSDDSLogger::TheInstance()->QSBuffer(),QS_BUF_LEN);
    return QSDDSLogger::TheInstance()->DDSStartup();
}

void QS::onCleanup(void) 
{
    QSDDSLogger::TheInstance()->done = true;
    cout << "Waiting for QSDDSLogger to complete logging..." << endl;
    sleep(5);
    QSDDSLogger::TheInstance()->DDSTeardown();
    QSDDSLogger::DestroyTheInstance();
}

QSTimeCtr QS::onGetTime(void) 
{
    return (QSTimeCtr)clock();
}

void QS::onFlush(void) 
{
    uint16_t nBytes = 1000;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) 
    {
        QSDDSLogger::TheInstance()->Log((uint8_t*)block,nBytes);
        nBytes = 1000;
    }
}

#endif
