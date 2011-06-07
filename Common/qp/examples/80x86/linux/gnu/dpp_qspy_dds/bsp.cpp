//////////////////////////////////////////////////////////////////////////////
// Product: DPP example
// Last Updated for Version: 4.0.00
// Date of the Last Update:  Apr 07, 2008
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"
#include "dpp.h"
#include "bsp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>                               // for memcpy() and memset()
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#ifdef Q_SPY
    #include "qsddslogger.h"

    enum AppRecords {                    // application-specific trace records
        PHILO_STAT = QS_USER
    };
#endif

Q_DEFINE_THIS_FILE

// Local objects -------------------------------------------------------------
static struct termios l_tsav;      // structure with saved terminal attributes
static uint32_t l_delay;          // limit for the loop counter in busyDelay()

//............................................................................
extern "C" void *idleThread(void *me) {     // the expected P-Thread signature
    char ch;

    for(;;) {
        struct timeval timeout = { 0 };                // timeout for select()
        fd_set con;                         // FD set representing the console
        FD_ZERO(&con);
        FD_SET(0, &con);
        timeout.tv_usec = 8000;

           // sleep for the full tick or until a console input arrives, NOTE01
        if (0 != select(1, &con, 0, 0, &timeout)) {     // any descriptor set?
            read(0, &ch, 1);
            if (ch == '\33') {                                 // ESC pressed?
                QF::publish(Q_NEW(QEvent, TERMINATE_SIG));
            }
        }

#ifdef Q_SPY
        {                                                 // perform QS output
            uint16_t nBytes = 1000;
            uint8_t const *block;
            QF_INT_LOCK(ignore);
            block = QS::getBlock(&nBytes);
            QF_INT_UNLOCK(ignore);
            if (block != (uint8_t *)0 && ch!='\33') {
                QSDDSLogger::TheInstance()->Log((uint8_t*)block,nBytes);
            }
        }
#endif
    }

    return (void *)0;                                        // return success
}
//............................................................................
void BSP_init(int argc, char *argv[]) {
    if (argc > 1) {
        l_delay = atol(argv[1]);            // set the delay from the argument
    }

    char const *domIdAndTopic = "67:QSPYLOG";
    if (argc > 2) {                                         // port specified?
        domIdAndTopic = argv[2];
    }
    if (!QS_INIT(domIdAndTopic)) {
        printf("\nUnable to init QS DDS Logger\n");
        QF::stop();
    }
    printf("Dining Philosopher Problem example"
           "\nQEP %s\nQF  %s\n"
           "Press ESC to quit...\n",
           QEP::getVersion(),
           QF::getVersion());
}
//............................................................................
void QF::onStartup(void) {                                 // startup callback

    struct termios tio;                        // modified terminal attributes
    tcgetattr(0, &l_tsav);             // save the current terminal attributes
    tcgetattr(0, &tio);              // obtain the current terminal attributes
    tio.c_lflag &= ~(ICANON | ECHO);      // disable the canonical mode & echo
    tcsetattr(0, TCSANOW, &tio);                     // set the new attributes

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
    if (pthread_create(&idle, &attr, &idleThread, 0) != 0) {
               // Creating the p-thread with the SCHED_FIFO policy failed.
               // Most probably this application has no superuser privileges,
               // so we just fall back to the default SCHED_OTHER policy
               // and priority 0.
        pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
        param.sched_priority = 0;
        pthread_attr_setschedparam(&attr, &param);
        Q_ALLEGE(pthread_create(&idle, &attr, &idleThread, 0) == 0);
    }
    pthread_attr_destroy(&attr);
}
//............................................................................
void QF::onCleanup(void) {                                 // cleanup callback
    printf("\nBye! Bye!\n");
    tcsetattr(0, TCSANOW, &l_tsav);   // restore the saved terminal attributes
    QS_EXIT();                                       // perfomr the QS cleanup
}
//............................................................................
void BSP_displyPhilStat(uint8_t n, char const *stat) {
    printf("Philosopher %2d is %s\n", (int)n, stat);

    QS_BEGIN(PHILO_STAT, AO_Philo[n])     // application-specific record begin
        QS_U8(1, n);                                     // Philosopher number
        QS_STR(stat);                                    // Philosopher status
    QS_END()
}
//............................................................................
void BSP_busyDelay(void) {
    uint32_t volatile i = l_delay;
    while (i-- > 0UL) {
    }
}
//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    fprintf(stderr, "Assertion failed in %s, line %d", file, line);
    QF::stop();
}

//----------------------------------------------------------------------------
#ifdef Q_SPY                                            // define QS callbacks

enum QSQHsmTstRecords {
    QS_DPP_DISPLAY = QS_USER
};

//............................................................................
uint8_t QS::onStartup(void const *arg) {
    uint32_t domainId;
    char     topicName[50];
    bool     ret;

    sscanf((char const*)arg,"%u:%s",&domainId,topicName);
    printf("DOMAIN ID: %u, TOPIC: %s\n",domainId,topicName);

    QSDDSLogger::CreateTheInstance(domainId,topicName);
    initBuf(QSDDSLogger::TheInstance()->QSBuffer(),QS_BUF_LEN);
    return QSDDSLogger::TheInstance()->DDSStartup();
}
//............................................................................
void QS::onCleanup(void) {
    QSDDSLogger::TheInstance()->done = true;
    QSDDSLogger::TheInstance()->DDSTeardown();
    QSDDSLogger::DestroyTheInstance();
}
//............................................................................
QSTimeCtr QS::onGetTime(void) {             // invoked within critical section
    return (QSTimeCtr)clock();
}
//............................................................................
void QS::onFlush(void) {
    uint16_t nBytes = 1000;
    uint8_t const *block;
    while ((block = getBlock(&nBytes)) != (uint8_t *)0) {
        QSDDSLogger::TheInstance()->Log((uint8_t*)block,nBytes);
        nBytes = 1000;
    }
}
#endif                                                                // Q_SPY
//----------------------------------------------------------------------------
