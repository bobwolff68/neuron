
#include "RTBuffer.h"
#include <unistd.h>

//#define USE_COPY_BUFFERS

int RTBuffer::n_instances = 0;

RTBuffer::RTBuffer(void) 
{ 
    int pid;
    pid = getpid();
    stringstream semname;
    semname << "neuron_sem_" << pid << RTBuffer::n_instances++;
    
    cout << "Semaphore name is: " << semname.str() << endl;
    
	pthread_mutex_init(&mutex, NULL);
    p_sem_numbuffers = sem_open(semname.str().c_str(), O_CREAT, S_IRWXU, 0);
    string str;
    errnostr(str);
    assert(p_sem_numbuffers!=SEM_FAILED);
	
    bReleased = false;
    
    mFrameCount=0;
    mRefusedCount=0;
    
    bIsRunning=false;
};

RTBuffer::~RTBuffer(void) 
{ 
	if (bufferQ.size() && !bReleased)
	{
        // TODO - May want to iteratively flush just like 'clear()' is going to have to do.
		cerr << "~RTBuffer() is waiting on " << bufferQ.size() << " samples to be cleared before shutdown." << flush;
		while (bufferQ.size() && !bReleased)
		{
			cerr << "." << flush;
			usleep(100000);
		}
#ifdef USE_COPY_BUFFERS
        //TODO - need to flush the copy buffer set if there is one.
#endif
	}
	
	pthread_mutex_destroy(&mutex);
	sem_close(p_sem_numbuffers);
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
    QTKitBufferInfo* pbi = new QTKitBufferInfo;
    
    pbi->bFinalSample = true;
    
    FullBufferEnQ(pbi);	
    
    pauseRunning();
    
    mFrameCount=0;
    mRefusedCount=0;
}

bool RTBuffer::FullBufferEnQ(RTBufferInfoBase* pBI)
{
    int rc=0;
    
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
        
        // Special case -- when enQ fails, who deletes the pointer? We do internally.
#ifdef COPY_QTKIT_CAP_BUFFERS
        this->EmptyBufferRelease(pBI, NULL);
#else
        this->EmptyBufferRelease(pBI);
#endif
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

        // Special case -- when enQ fails, who deletes the pointer? We do internally.
#ifdef COPY_QTKIT_CAP_BUFFERS
        this->EmptyBufferRelease(pBI, NULL);
#else
        this->EmptyBufferRelease(pBI);
#endif
        
        return false;
    }
    
    // Enqueue the item sent in.
    bufferQ.push_back(pBI);

#ifdef COPY_QTKIT_CAP_BUFFERS
    void* pb=NULL;
    posix_memalign(&pb, 16, 640*360*2);

    assert(pb);
    
    memcpy(pb, pBI->pBuffer, 640*360*2);

    pBuff_copied.push_back(pb);
#endif
    
    sem_post(p_sem_numbuffers);	// Release the semaphore by incrementing by one.
    
    mFrameCount++;
    
    rc = pthread_mutex_unlock(&mutex);
    if (rc)
    {
        cerr << "UnLock failed with returncode = " << rc << endl;
        assert(false && "Unlock Failed");

        // We do **NOT** delete the inbound pointer on this failure as it's already bee pushed onto the queue.
        
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

#ifdef COPY_QTKIT_CAP_BUFFERS
bool RTBuffer::FullBufferDQ(RTBufferInfoBase** ppBI, void**ppb)
#else
bool RTBuffer::FullBufferDQ(RTBufferInfoBase** ppBI)
#endif
{
    int rc=0;
    bool ret = true;
    
    sem_wait(p_sem_numbuffers);	// Waits in blocked mode until there is a frame ready.
    // Note that this occurs BEFORE the mutex is locked. Else the post and wait will
    // deadlock each other.
    
    // There is an odd case where the RTBuffer may be getting flushed after the sem_wait has released.
    // So, all flush operations must have their own mutex.
//TODO    Need to do a mutex lock-try. If we fail to get it, this oddity has happened. So dont dequeue. Just fail.
//TODO        If we do get the lock, then release it inside the queue mutex.
    
    rc = pthread_mutex_lock(&mutex);
    if (rc)
    {
        cerr << "Lock failed with returncode = " << rc << endl;
        assert(false && "Lock Failed");
        return false;
    }
    
    if (bufferQ.size()==0 && !bReleased)
    {
        // Must have been cleared out from under us. Return false.
        cout << "Hello world. It is raining." << endl;
        ret = false;
    }
    else
    {  
        *ppBI = bufferQ.front();
        // Deletion of this pointer is handled by the RELEASE -- not by dequeuing it.
        // It is always the responsibility of the DQ person to call release on every buffer it receives.
        
        // DeQueue the item at the front.
        bufferQ.pop_front();
     
#ifdef COPY_QTKIT_CAP_BUFFERS
        *ppb = pBuff_copied.front();
        pBuff_copied.pop_front();
#endif
        
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

