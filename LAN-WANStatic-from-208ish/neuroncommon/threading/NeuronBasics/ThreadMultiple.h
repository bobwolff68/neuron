//!
//! \file ThreadMultiple.h
//!
//! \brief Flexible multi-threading base class.
//!
//! Very similar to ThreadSingle with the main exception that workerBee() is replaced with workerBeeSplitter(int tn).
//! The reason for this is so the parent class only has one function to act as the centralizing place where all
//! threads 'enter' the world. The user can use a switch(tn) to call different functions for many or all of the threads
//! or can creatively use the index 'tn' to identify itself and run.
//!
//! For showing single thread (one additional thread), simply execute the command with an argument of '1'
//!
//! \author Bob Wolff (rwolff)
//! \date Created on: Jun 10, 2010
//!
#ifndef THREADMULTIPLE_H_
#define THREADMULTIPLE_H_

#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>
using namespace std;

//!
//! \def REPORT_ERROR(a,b) is used to give feedback on true error conditions. Currently this is done via
//!      a simple printf(). In the future this needs to be made into a more formal throw of an exception.
//!
//! \todo REPORT_ERROR furthermore does not allow for flexible use of variable args lists currently. To change
//!       this would be quite helpful as well.
//!
#ifdef REPORT_ERROR
#undef REPORT_ERROR
#endif
#define REPORT_ERROR(a,b) printf(a,b)
//! Maximum allowed # of threads. This can be changed, but is a sane ceiling item for the moment.
#define MAXTHREADS 20

class ThreadMultiple;

//! \struct TInfo
//! \brief  Supports typedef ThreadInfo
//! \typedef ThreadInfo
//! \brief   Used by spawning process to track Thread pointer as well as thread number being spawned.
typedef struct TInfo
{
  //! pointer to the thread class instantiation which is being created/spawned.
  ThreadMultiple* pThread;

  //! This indicates which thread number is currently being spawned for workerBeeSplitter() sake.
  int threadNum;
} ThreadInfo;

//!
//! \class ThreadMultiple
//!
//! \brief Base class to be inherited for spawning workerBeeSplitter() pure virtual.
//!        Very similar to ThreadSingle with the added complication of the workerBee being indexed
//!        due to multiple threads.
//!
//! Details: Simply inherit this class and create your own workerBee() function in the parent.\n
//!   When instantiating, use the constructor's argument to specify the number of threads required.
//!   Then, use startAllThreads() to start the new threads which will call your workerBeeSplitter from each newly
//!   created thread. When it is time to shutdown, simply call stopAllThreads() and when it returns, you're done.
//!   \note It is important to note that your implementation of workerBeeSplitter() or the subsequent 'real workers'
//!         must be looking for the isStopRequested[tn] or for the more global bGlobalKillRequested kill signal to
//!         flag it to return() - otherwise the thread class will not end nicely and will crash upon
//!         a change of scope and stack.
//!   \par
//!   Also, there is a built in \b single mutex created at instantiation time. Parent may pthread_mutex_lock(&mutex) and
//!         then can pthread_mutex_unlock(&mutex) or even using signal/wait actions as well. If multiple threads need
//!         their own mutexes, it is up to them to create them so-as to not create multi-thread collision use of 'mutex'
//!         in this base class.
//!
//! \todo Clean up error reporting by using throw of a new type of exception - gui and console and logging automatically.
//!
class ThreadMultiple
{
public:
  //! Constructor with number of threads requested as its argument.
  //!
  //! \param[in] nt Number of threads to be launched when start* is called.
  ThreadMultiple(int nt);
  ~ThreadMultiple();
  //! Use to begin spawning all threads. This will kickoff every thread in order from 0-(nt-1).
  //! However, there is no guarantee that each workerBeeSplitter() will be called in order. This
  //! is up to the OS's implementation of pthreads + the scheduler. It is most likey they will go in
  //! order as they are called in order and queued in order.
  void startAllThreads();
  //! Use to finally send a stop request to all threads one at a time and wait for each of them one at a time.
  void stopAllThreads();

private:
  pthread_t thread[MAXTHREADS];
  volatile bool isRunning[MAXTHREADS];
  ThreadInfo tInfo[MAXTHREADS];

  static void* threadLaunchpoint(void* pData);

protected:
  //! Keeping only one mutex. Parent can create more if needed.
  pthread_mutex_t mutex;
  //! Global version of a kill-request flag. Can be easier for some usage scenarios to look at a non-indexed
  //! global kill request for ending a specific thread.
  volatile bool bGlobalKillRequested;
  //! A single flag for each thread involved to indicate that a kill has been requested.
  //! Worker bee functions must listen/watch for this to happen.
  volatile bool isStopRequested[MAXTHREADS];
  //!
  //! \var numThreads
  //! \brief Keeps track of how many threads have been requested in this instantiation.
  //! \b Not the current number of \e active threads.
  //!
  int numThreads;
  //!
  //! This is the most important area! Normally in ThreadSingle:: you'd simply put your routine in this function when
  //! you define it in the parent class. However, in this case, when workerBeeSplitter() gets call, it will also have
  //! along with it an indicator of WHICH thread this is. This gives the author an opportunity (the ONLY opportunity)
  //! to make a switch(tn) {} which allows for each thread to be split into separate function calls by the author.
  //!
  //! \b NOTE: If you are not familiar with threads, it may not be obvious ... this function will get called once for
  //!   each thread being spawned/forked with a unique 'tn' calling parameter. These are each of the new threads
  //!   just being executed. It is up to you to direct them to work items in the class.
  //!
  //! \param[in] tn The value of tn is the 'thread-number'. This gives the author the ability to have one worker
  //!               function which operates as a single function for all threads \e or it may use 'tn' in order
  //!               to distinguish each thread and make separate function called based upon the thread number.
  //!
  //! Example: in a ThreadMultiple(3) instantiation, you may have a reader thread, a writer thread, and a status thread.
  //! \code
  //! int MyParentThreadMultiple::workerBeeSplitter(int tn)
  //! {
  //!   if (tn >= 3) return -1; // Error - more than the orignal desired # of threads. Should never happen.
  //!   switch(tn)
  //!   {
  //!     case 0:        // The reader thread.
  //!       return readerThread();
  //!       break;
  //!     case 1:        // The writer thread.
  //!       return writerThread();
  //!       break;
  //!     case 2:        // The stats thread.
  //!       return statsThread();
  //!       break;
  //!     default:        // Should never happen. Return error.
  //!       assert(false);
  //!       return -1;
  //!       break;
  //!   }
  //!
  //!   return 0;
  //! }
  //! \endcode
   virtual int workerBeeSplitter(int tn)=0;
   //!
   //! Used internally. See startAllThreads() for user-based starting of multi-threads. It is, however, possible
   //! to have a parent class utilize startThread() and stopThread() individually. In this case, be sure to not
   //! make a repeat-call to either the start or the stop for a given thread. And do not use startAllThreads if
   //! you are using startThread() individually. And similarly do not use stopAllThreads if you are using
   //! stopThread() individually.
   //!
   void startThread(int tn);
   //!
   //! Used internally. See stopAllThreads() for user-based stopping of multi-threads. It is, however, possible
   //! to have a parent class utilize startThread() and stopThread() individually. In this case, be sure to not
   //! make a repeat-call to either the start or the stop for a given thread. And do not use startAllThreads if
   //! you are using startThread() individually. And similarly do not use stopAllThreads if you are using
   //! stopThread() individually.
   //!
   void stopThread(int tn);

};

#endif /* THREADMULTIPLE_H_ */
