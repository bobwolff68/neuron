
#include "RTBuffer.h"

RTBuffer::RTBuffer(void) 
{ 
	pthread_mutex_init(&mutex, NULL);
	sem_init(&sem_numbuffers, 0, 0); 
	bReleased = false;
    
    mFrameCount=0;
    mRefusedCount=0;
    
    bIsRunning=false;
};

RTBuffer::~RTBuffer(void) 
{ 
	if (bufferQ.size() && !bReleased)
	{
		cerr << "~RTBuffer() is waiting on " << bufferQ.size() << " samples to be cleared before shutdown." << flush;
		while (bufferQ.size() && !bReleased)
		{
			cerr << "." << flush;
			usleep(100000);
		}
	}
	
	pthread_mutex_destroy(&mutex);
	sem_destroy(&sem_numbuffers);
};

//!
//! RTBuffer::Shutdown
//! 
//! \brief Purpose is to send one more (null-sample) sample through the buffer so that
//!		any 'FullBufferDQ()' people are alerted that this is a final non-usable sample.
//! 
//! \note If the consumer of samples doesn't adhere to this, they will become hopelessly deadlocked in a sem_wait()
//!		while the object is being deleted. Not a good situation.
//! 
//! \param void
//! \return void
//! 
//! 
void RTBuffer::Shutdown(void)
{
    QTKitBufferInfo bi;
    
    bi.bFinalSample = true;
    
    FullBufferEnQ(bi);	
    
    pauseRunning();
    
    mFrameCount=0;
    mRefusedCount=0;
}

bool RTBuffer::FullBufferEnQ(RTBufferInfoBase& BI)
{
    int rc=0;
    
    //TODO WARNING: inbound BI must be copied to a new allocated BI
    // prior to enqueuing as these were stack objects inbound.
    
    if (!bIsRunning)
    {
//        NSLog(@"RTBuffer is not running. Dropping enqueue.");
        mRefusedCount++;
        return false;
    }
    
    if (Qsize() >= MAX_QUEUE_SIZE)
    {
        //      NSLog(@"RTBuffer full: Enqueue refused.");
        mRefusedCount++;
        return false;
    }
    
    rc = pthread_mutex_lock(&mutex);
    if (rc)
    {
        string st;
        cerr << "Lock failed with returncode = " << rc << endl;
        cerr << "errno=" << errno << " at this moment." << endl;
        cerr << "Equates to: " << errnostr(st) << endl;
        assert(false && "Lock Failed");
        return false;
    }
    
    // Enqueue the item sent in.
    bufferQ.push_back(BI);
    
    sem_post(&sem_numbuffers);	// Release the semaphore by incrementing by one.
    
    mFrameCount++;
    
    rc = pthread_mutex_unlock(&mutex);
    if (rc)
    {
        cerr << "UnLock failed with returncode = " << rc << endl;
        assert(false && "Unlock Failed");
        return false;
    }
    
    return true;
}

//!
//! RTBuffer::FullBufferDQ
//! 
//! \brief Remove one full buffer from the queue - ready for processing.
//! 
//! \note This is a mildly blocking function. Will not return until the 
//!		mutex is locked or fails-trying. Will return early if size() is zero.
//! 
//! \param V4L2BufferInfo& BI
//! \return bool
//! 
//! 

bool RTBuffer::FullBufferDQ(RTBufferInfoBase& BI)
{
    int rc=0;
    int val;
    bool ret = true;
    assert(sem_getvalue(&sem_numbuffers, &val)==0 && val==bufferQ.size());
    
    if (bReleased)
    {
        BI.bFinalSample=true;
        return true;
    }
    
    sem_wait(&sem_numbuffers);	// Waits in blocked mode until there is a frame ready.
    // Note that this occurs BEFORE the mutex is locked. Else the post and wait will
    // deadlock each other.
    
    rc = pthread_mutex_lock(&mutex);
    if (rc)
    {
        cerr << "Lock failed with returncode = " << rc << endl;
        assert(false && "Lock Failed");
        return false;
    }
    
    if (bufferQ.size()==0)
    {
        // Must have been cleared out from under us. Return false.
        ret = false;
    }
    else
    {  
        BI = bufferQ.front();  
        // TODO WARNING: on dequeue, must deallocate BI that pops from the fifo
        
        // DeQueue the item at the front.
        bufferQ.pop_front();
        ret = true;
    }
    
    rc = pthread_mutex_unlock(&mutex);
    if (rc)
    {
        cerr << "UnLock failed with returncode = " << rc << endl;
        assert(false && "Unlock Failed");
        return false;
    }
    
    return ret;
}

