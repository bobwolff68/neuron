//!
//! \file ThreadMultiple.h
//!
//! \brief Flexible multi-threading base class.
//!
//! \author Bob Wolff (rwolff)
//! \date Created on: Jun 10, 2010
//!

#include "ThreadMultiple.h"

ThreadMultiple::ThreadMultiple(int nt)
{
  int ret;

  assert(nt < MAXTHREADS);
  numThreads = nt;

  if (numThreads >= MAXTHREADS || numThreads <= 0)
    throw;

  bGlobalKillRequested = false;

  for (int i=0; i<numThreads ; i++)
  {
    isRunning[i] = false;
    isStopRequested[i] = false;
  }

  if (pthread_mutex_init(&mutex, NULL))
  {
      assert(false);
      REPORT_ERROR("mutex init failed - err:%d", ret);
  }
}

ThreadMultiple::~ThreadMultiple()
{
  stopAllThreads();

  pthread_mutex_destroy(&mutex);
}

void ThreadMultiple::startThread(int tn)
{
  int ret;

  assert(tn < numThreads);

  if (tn >= numThreads)
    throw;

  assert(!isRunning[tn]);
  // Now set to running and create the thread -- and in an error condition, set to 'NOT running' again.

  isRunning[tn] = true;

  tInfo[tn].pThread = this;
  tInfo[tn].threadNum = tn;
  ret = pthread_create(&thread[tn], 0, &(ThreadMultiple::threadLaunchpoint), &tInfo[tn]);
  if (ret)
  {
    isRunning[tn] = false;
    assert(false);
    REPORT_ERROR("thread creation failed. Err:%d", ret);
  }
}

void ThreadMultiple::stopThread(int tn)
{
  int ret;

  assert(tn < numThreads);
  if (tn >= numThreads)
    throw;

  if (isRunning[tn])
  {
    // Set the stop requested flag, stop the thread, and then clear the isRunning flag.
    isStopRequested[tn] = true;
    ret = pthread_join(thread[tn], 0);
    if (ret)
    {
        assert(false);
        REPORT_ERROR("ending/joining thread failed. Err:%d", ret);
    }
    isRunning[tn] = false;
  }
}

void ThreadMultiple::stopAllThreads()
{
  bGlobalKillRequested = true;  //!< Set global kill flag upon stopAllThreads() call only.

  for (int i=0; i<numThreads; i++)
  {
    if (isRunning[i])
      stopThread(i);
  }
}

void ThreadMultiple::startAllThreads()
{
  for (int i=0; i<numThreads; i++)
    startThread(i);
}

void* ThreadMultiple::threadLaunchpoint(void* pData)
{
  ThreadInfo *pti;

  //
  // In this invocation, the spawning of a thread has data which not only points to the instantiation
  // of ThreadMultiple but also carries with it the 'thread number' for making the properly annotated
  // call to workerBeeSplitter()
  //
  pti = (ThreadInfo*)pData;

  pti->pThread->workerBeeSplitter(pti->threadNum);

  return (void*)NULL;
}
