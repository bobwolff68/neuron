//!
//! \file testthreads.cpp
//!
//! \brief Simple test jig to show off single- or multi- threading using two different base classes.
//!
//! Usage of either single or multiple threads is done via the commandline. The default is to show
//!     multi-threading. In this case, 3 concurrent threads which each print information to stdout/cout
//!     while the main thread waits for a period of time before sending a 'bKillRequest' which is a single
//!     flag listened to universally by all threads. This implementation could have just as easily set
//!     isStopRequested[0]=true; isStopRequested[1]=true; isStopRequested[2]=true; and the workerBeeSplitter()
//!     could have listened for its individual thread stop request. This would be nice in the case where
//!     individual control over each thread's 'halt' is desired or required.
//!
//! For showing single thread (one additional thread), simply execute the command with an argument of '1'
//!
//! \author Bob Wolff (rwolff)
//! \date Created on: Jun 10, 2010
//!
#include <iostream>
using namespace std;

#include "ThreadSingle.h"
#include "ThreadMultiple.h"

#include <string.h>
#include <stdlib.h>

//!
//! \class MyThread
//!
//! \brief Inherits ThreadSingle for a sample of creating a worker-thread.
//!
//! Used only in 'single' mode by passing '1' as the only argument to the commandline.
//!
class MyThread : public ThreadSingle {
public:
  MyThread()
  {
  }
  ~MyThread()
  {
  }
private:
  int workerBee()
  {
    cout << "Worker Bee says hello." << endl;

    for (int i=0; i<15; i++)
    {
      cout << "Worker Bee: Round #" << i << endl;
      sleep(1);

      if (isStopRequested)
      {
        cout << "Worker had early termination request. Exiting EARLY.\n" << flush;
        return 1;               // Exit worker bee early.
      }
    }

    return 0;
  }
};

//! Simple static storage area for '*' characters for setting-off messages in cout.
char accent[15][80];

//!
//! \class MyThreadTriple
//!
//! \brief Inherits ThreadMultiple for a sample of creating a set of (3) worker-threads.
//!
class MyThreadTriple : public ThreadMultiple {
public:
  //!
  //! \fn MyThreadTriple(int nt)
  //! \brief Instantiation of base plus some custom work for demo printout statements.
  //!
  //! \param[in] nt Number of threads to create.
  //!
  MyThreadTriple(int nt) : ThreadMultiple(nt)
  {
    const char *stars = "********************";
    for (int i=0; i<15; i++)
    {
#if 1
      strncpy(accent[i], stars, i+1);
#else
      strcpy(accent[i], "*");

      for (int j=0; j<i; j++)
        strcat(accent[i], "*");
#endif
    }
  }
  ~MyThreadTriple()
  {

  }
private:
  int workerBeeSplitter(int tn)
  {
    if (tn >= 0 && tn < numThreads)
      return mywork(tn);
    else
      return -1;
  }

  int mywork(int tn)
  {
    cout << "Worker Bee #" << accent[tn] << tn << accent[tn] << " says hello." << endl;

    for (int i=0; i<100; i++)
    {
#if 0
      int r;

      r = 250000 * drand48();
      cout << "Worker Bee #" << accent[tn] << tn << accent[tn] << "      : Round #" << i << " r=" << r << endl;
#else
      cout << "Worker Bee #" << accent[tn] << tn << accent[tn] << "      : Round #" << i << endl;
      usleep(250000 - (i>10 ? i%10 : i)*20000);           // Sleep .25 seconds minus a factor for speedup along the way.
#endif

      if (bGlobalKillRequested)  //! This could also be done via isStopRequested[tn] - either way globally or individually.
      {
        cout << "Worker " << accent[tn] << tn << accent[tn] << " received an early termination request. Exiting EARLY.\n" << flush;
        return 1;               // Exit worker bee early.
      }
    }

    return 0;
  }
};

//! Main function - start of program.
int main(int argc, char** argv)
{
  if (argc==2 && !strcmp(argv[1], "1"))
  {
    MyThread thr;
    cout << "Hello World." << endl;
    thr.startThread();

    cout << "Main thread beginning 5 second wait...\n";
    sleep(5);

    cout << "Requested early termination of the child thread. Should end suddenly now." << endl;
    thr.stopThread();

    sleep(1);
  }
  else
  {
    // Now instantiate the multiple threaded beast.
    MyThreadTriple* pThr;

    pThr = new MyThreadTriple(3);
    cout << "Hello World." << endl;
    pThr->startAllThreads();

    cout << "Main thread beginning 5 second wait...\n";
    sleep(5);

    cout << "Main thread back awake and heading for goodbye." << endl;
    pThr->stopAllThreads();

    delete pThr;

    sleep(1);
  }

  return 0;
}
