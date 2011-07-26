//!
//! \file V4L2Cap.cpp
//! 
//! \brief Linux version of video capture support. This utilizes the Video4Linux2 API and libraries.
//!	As a note, V4L2 captures in 3 distinct methods: fd read, memmapped, and user-pointer. Each has
//!	it's own pros and cons but memory mapped is preferred. Unfortunately any video capture driver
//! 	may support any one or more of these. So we must be able to suport each of them in as optimal
//!	a way possible since this is a major data-gathering and piping activity.
//! 
//! \author rwolff
//! \date Thu 30 Jun 2011 11:01:47 AM 
//! 

// When compiling from code/neuroncommon/
// testing-compilation: g++ -g3 V4L2Cap.cpp -I../include -I. -lpthread
#define TEST

#ifdef TEST
#define SKIP_DDS
#endif

#include "neuroncommon.h"
#include "V4L2Cap.h"

#include <iomanip>

bool V4L2CapBuffer::EmptyBufferRelease(V4L2BufferInfo& BI)
{

//! \todo Clean up the 'exit' strategy here. Make it at least alert and continue.
	if (-1 == xioctl (fd, VIDIOC_QBUF, &(BI.buf)))
		errno_exit ("VIDIOC_QBUF");
	
	return true;
}

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
   V4L2BufferInfo bi;
   
   bi.bFinalSample = true;
 
   FullBufferEnQ(bi);	
}

RTBuffer::RTBuffer(void) 
{ 
	pthread_mutex_init(&mutex, NULL);
	sem_init(&sem_numbuffers, 0, 0); 
	bReleased = false;
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

bool RTBuffer::FullBufferEnQ(V4L2BufferInfo& BI)
{
  int rc=0;
  
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

bool RTBuffer::FullBufferDQ(V4L2BufferInfo& BI)
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



//!
//! V4L2Cap::V4L2Cap
//! 
//! \brief Instantiate a Video4Linux2 device
//! 
//! \note 
//! 
//! \param const char* indev
//! \param int w
//! \param int h
//! \param const char* fmt
//! 
V4L2Cap::V4L2Cap(const char* indev, int w, int h, const char* fmt) 
{
	pRTBuffer = NULL;
	
	szErrors[OK] = "All ok.";
	szErrors[NO_DEVICE] = "No video devices found - expecting /dev/video* available.";
	szErrors[INVALID_DEVICE] = "Video device was invalid. Not a device.";
	szErrors[CANNOTOPEN_DEVICE] = "Cannot open device. Exists, but open() failed.";
	szErrors[NO_IMAGEFORMATS] = "Enumerating video image formats failed to produce results.";
	szErrors[FORMAT_REJECTED] = "Setting desired format failed.";
	
	szErrors[MAX_ERRTYPE] = "ACCESS ILLEGAL - MAX ENUM.";

	fd = -1;
	io = IO_METHOD_MMAP;	//! Hardwired until we break this into its subclass components. Then capabilities determine which object is to be new'd
	buffers = NULL;
	n_buffers = 0;
	
	
//! Setup true defaults in the case of no over-ridden values by the instantiator.
	def.device = "/dev/video0";
	def.width  = 640;
	def.height = 480;
	def.format = "YUYV";

//! Now use override values if they exist.	
	if (indev)
		def.device = indev;

	if (w)
		def.width  = w;
		
	if (h)
		def.height = h;
		
	if (fmt)
		def.format = fmt;
	
	errtype ret;
	
	ret = InitDevice();
	if (ret != OK)
	{
		cerr << "Error initializing device: " << szErrors[ret] << endl;
		return;
	}
	
	pRTBuffer = new V4L2CapBuffer(fd);
	assert(pRTBuffer);
	
};

V4L2Cap::~V4L2Cap(void) 
{
	DeInitDevice();
	
	delete pRTBuffer;
}

//!
//! V4L2Cap::InitDevice
//! 
//! \brief 
//! 
//! \note 
//! 
//! \todo Add support to automatically list all available /dev/video* devices
//! \return int - Success is '0' while errors are in the V4L2Cap.h file
//! 
//! 

inline V4L2Cap::errtype V4L2Cap::showerr(errtype err) {
	cerr << "V4L2Cap::Error: " << err << " (" << szErrors[err] << ")" << endl;
	return err;
}

V4L2Cap::errtype V4L2Cap::InitDevice()
{
	struct stat st;
	
        if (-1 == stat (def.device.c_str(), &st)) {
                cerr << "Cannot find valid device '" << def.device << 
                	"'. Errno: " << errno << "(" << strerror(errno) << ")" << endl;
                return showerr(NO_DEVICE);
        }

        if (!S_ISCHR (st.st_mode)) {
                cerr << def.device.c_str() << " is not a device" << endl;
                return showerr(INVALID_DEVICE);
        }

        fd = open (def.device.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
                cerr << "Cannot open device '" << def.device << 
                	"'. Errno: " << errno << "(" << strerror(errno) << ")" << endl;
                return showerr(CANNOTOPEN_DEVICE);
        }
        
        //! Now it's open....finalize initialization. This is where we'll instantiate the appropriate 'type/mode' later.
        
        
 {
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
	unsigned int min;

        if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s is no V4L2 device\n",
                                 def.device.c_str());
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf (stderr, "%s is no video capture device\n",
                         def.device.c_str());
                exit (EXIT_FAILURE);
        }

	switch (io) {
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			fprintf (stderr, "%s does not support read i/o\n",
				 def.device.c_str());
			exit (EXIT_FAILURE);
		}

		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			fprintf (stderr, "%s does not support streaming i/o\n",
				 def.device.c_str());
			exit (EXIT_FAILURE);
		}

		break;
	}


        /* Select video input, video standard and tune here. */


	CLEAR (cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
                        switch (errno) {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        } else {	
                /* Errors ignored. */
        }


        CLEAR (fmt);

        fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = def.width; 
        fmt.fmt.pix.height      = def.height;
        if (def.format=="YUYV")
	        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	else if (def.format=="YV12")
	        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YVU420;
	else
		assert(false && "Format was not what we expected in def.format");
	        
	        //!
	        //! \todo Figure out V4L2_FIELD_NONE versus _INTERLACED -- how to detect which is preferred and/or supported.
        fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

        if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
                errno_exit ("VIDIOC_S_FMT");

        /* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (io) {
	case IO_METHOD_READ:
	assert(false);
//		init_read (fmt.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		init_mmap ();
		break;

	case IO_METHOD_USERPTR:
	assert(false);
//		init_userp (fmt.fmt.pix.sizeimage);
		break;
	}
 }
        
        return OK;
}

void V4L2Cap::init_mmap(void)
{
	struct v4l2_requestbuffers req;

        CLEAR (req);

        req.count               = buffersRequested;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s does not support "
                                 "memory mapping\n", def.device.c_str());
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf (stderr, "Insufficient buffer memory on %s\n",
                         def.device.c_str());
                exit (EXIT_FAILURE);
        }
        
        if (req.count != buffersRequested)
        {
        	cerr << "Init_MMap - Requested " << buffersRequested << " frame buffers. Received only " << req.count 
        		<< ". Continuing." << endl;
        }

        buffers = new struct buffer[req.count];

        if (!buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR (buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit ("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap (NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit ("mmap");
        }
}

void V4L2Cap::DeInitDevice(void)
{
        unsigned int i;

	stop_capturing();
	
	switch (io)
	{
	case IO_METHOD_READ:
		if (buffers[0].start)
			delete (char*)buffers[0].start;
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
			if (-1 == munmap (buffers[i].start, buffers[i].length))
				errno_exit ("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i)
			if (buffers[0].start)
				delete (char*)buffers[i].start;
		break;
	}

	delete buffers;

//! Now close it.
	if (fd != -1)
		close(fd);
}

void V4L2Cap::stop_capturing                  (void)
{
        enum v4l2_buf_type type;

	// Now that we're done capturing...stop the workerBee thread.
	if (IsRunning())
		stopThread();

	pRTBuffer->Shutdown();
	// Give the shutdown a little time prior to the deletion.
	
	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
			errno_exit ("VIDIOC_STREAMOFF");

		break;
	}
	
	pRTBuffer->clear();
	pRTBuffer->ReleaseOutputSide();
}

void V4L2Cap::start_capturing                 (void)
{
        unsigned int i;
        enum v4l2_buf_type type;

	if (IsRunning())
	{
		cerr << "start_capturing: Thread is already running...hmm...skipping start..." << endl;
		return;
	}
		
	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i) {
            		struct v4l2_buffer buf;

        		CLEAR (buf);

        		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        		buf.memory      = V4L2_MEMORY_MMAP;
        		buf.index       = i;

        		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                    		errno_exit ("VIDIOC_QBUF");
		}
		
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
			errno_exit ("VIDIOC_STREAMON");

		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i) {
            		struct v4l2_buffer buf;

        		CLEAR (buf);

        		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        		buf.memory      = V4L2_MEMORY_USERPTR;
			buf.index       = i;
			buf.m.userptr	= (unsigned long) buffers[i].start;
			buf.length      = buffers[i].length;

			if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                    		errno_exit ("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
			errno_exit ("VIDIOC_STREAMON");

		break;
	}
	
	// Now that we're capturing...run the workerBee thread.
	startThread();
}

int V4L2Cap::read_frame			(void)
{
        struct v4l2_buffer buf;
	unsigned int i;
	V4L2BufferInfo bi;
	string st;

	assert(pRTBuffer);
	if (pRTBuffer->Qsize() >= 7)
	{
		cerr << "-S-" << flush;
		usleep(10000);
		return 0;
	}
	
		// This will remain dequeued from the capture driver until the encoder is DONE with it.

		//RMW:Test - re-enqueue after we're 4 deep so we never get to eight.
//		if (pRTBuffer->Qsize() <= 4)

	switch (io) {
	case IO_METHOD_READ:
	assert(false);
    		if (-1 == read (fd, buffers[0].start, buffers[0].length)) {
            		switch (errno) {
            		case EAGAIN:
                    		return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit ("read");
			}
		}
assert(false);
//    		process_image (buffers[0].start);

		break;

	case IO_METHOD_MMAP:
		CLEAR (buf);

            	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            	buf.memory = V4L2_MEMORY_MMAP;

    		if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
            		switch (errno) {
            		case EAGAIN:
            			cerr << "-burp-" << flush;
                    		return 0;

			case EINVAL:	// INVALID paramter(s)
				cerr << "-INV-" << flush;
				usleep(500000);
				return 0;
				
			case EIO:
				cerr << "-EIO-" << flush;
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit ("VIDIOC_DQBUF");
			}
		}

                assert (buf.index < n_buffers);

		bi.pBuffer = buffers[buf.index].start;
		memcpy(&bi.buf, &buf, sizeof(v4l2_buffer));
		
		assert(pRTBuffer);
cout << "." << pRTBuffer->Qsize() << flush;
		// This will remain dequeued from the capture driver until the encoder is DONE with it.

		//RMW:Test - re-enqueue after we're 4 deep so we never get to eight.
//		if (pRTBuffer->Qsize() <= 4)
			pRTBuffer->FullBufferEnQ(bi);
//		else
//			if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
//				errno_exit ("VIDIOC_QBUF");


		break;

	case IO_METHOD_USERPTR:
	assert(false);
		CLEAR (buf);

    		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit ("VIDIOC_DQBUF");
			}
		}

		for (i = 0; i < n_buffers; ++i)
			if (buf.m.userptr == (unsigned long) buffers[i].start
			    && buf.length == buffers[i].length)
				break;

		assert (i < n_buffers);

assert(false);
//    		process_image ((void *) buf.m.userptr);

		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
			errno_exit ("VIDIOC_QBUF");

		break;
	}

	return 1;
}

int V4L2Cap::workerBee(void)
{
	unsigned int count;

        count = 0;

        for (;;) {
                fd_set fds;
                struct timeval tv;
                int r;

		// Time to kill the thread?
		if (IsStopRequested())
			break;
			
                FD_ZERO (&fds);
                FD_SET (fd, &fds);

                /* Timeout. */
                tv.tv_sec = 2;
                tv.tv_usec = 0;

                r = select (fd + 1, &fds, NULL, NULL, &tv);

                if (-1 == r) {
                        if (EINTR == errno)
                                continue;

                        errno_exit ("select");
                }

                if (0 == r) {
                        fprintf (stderr, "select timeout\n");
                        exit (EXIT_FAILURE);
                }

		if (read_frame ())
		{
			// Got a frame...but read_frame should have enque'd it for encoding, right?
		}

		count++;
		/* EAGAIN - continue select loop. */
        }
}


#ifdef TEST
int main(int argc, char** argv)
{
	V4L2Cap *pcap;
	
	char ch;
	
	pcap = new V4L2Cap("/dev/video0", 1280, 720, "YUYV");
	
	pcap->start_capturing();

	cout << "Press a LETTER followed by <enter> to quit capturing...." << endl;	
	usleep(150000);
	
	// Test1
	// Show realtime buffer filling in tight loop.
	int count = 0;
	while(true)
	{
		int i, f;
		
		f = pcap->GetBufferPointer()->Qsize();
		for (i=0;i<f;i++)
		  cout << "=";
		  
		cout << "> (" << f << ")" << endl;
		
		usleep(10000);	// attempt a 10ms delay and allow for an effective yeild()
		if (count++ > 10)
			break;
	}
	

	cout << endl << "Giving back some buffers...." << endl;
	count = 0;
	long start=0;
	
	long last=0;
	long cur=0;
	while(true)
	{
		int i, f;
		int w = cout.width();
		
		// Take a sample off the queue.
		V4L2BufferInfo bi;
		bool tr = pcap->GetBufferPointer()->FullBufferDQ(bi);
		assert(tr && "Dequeue of full buffer failed.");

		// Init state.
		if (start==0)
			start = bi.buf.timestamp.tv_sec;
			
		if (last==0)
			last = (bi.buf.timestamp.tv_sec - start) * 1000 + (bi.buf.timestamp.tv_usec/1000);
		
		cur = (bi.buf.timestamp.tv_sec - start) * 1000 + (bi.buf.timestamp.tv_usec/1000);
#if 0
		cout << endl << "Giving buffer back to driver. Timestamp(s.us)=" << 
				bi.buf.timestamp.tv_sec-start << "." << setw(6) << bi.buf.timestamp.tv_usec << setw(w) <<
				"/" << (bi.buf.timestamp.tv_usec/1000) << "ms" << endl;
#endif
		cout << endl << "Giving buffer back to driver. Timestamp(s.ms)=" << bi.buf.timestamp.tv_sec-start << "." << 
				setfill('0') << setw(3) << (bi.buf.timestamp.tv_usec/1000) << setw(w) << setfill(' ') <<
				" diff(" << setw(3) << cur-last << setw(w) <<"ms)   ";
	
		last = cur;
		
		tr = pcap->GetBufferPointer()->EmptyBufferRelease(bi);
		assert(tr && "Giving buffer back to Driver failed.");


		f = pcap->GetBufferPointer()->Qsize();
		for (i=0;i<f;i++)
		  cout << "=";
		  
		cout << "> (" << f << ")" << endl;
		
		usleep(30000);	// attempt a 10ms delay and allow for an effective yeild()
		if (count++ > 15)
			break;
	}

	
#if 0
	// Test 2
	// wait until a frame is ready, and then read one every 33ms for 5 seconds.
	time_t end;
	end = time() + 5
	
	while (time() < end)
	{
		
	}
#endif
	
//	while (!kb_hit());
	cin >> ch;
	
	cout << "Stopping the Capture..." << endl;
	
	pcap->stop_capturing();
	pcap->clear();		// Force buffer cleared.
	
	cout << "Stopped." << endl;
	
	delete pcap;
	
	//cap.ListResolutions();
	
	return 0;
}
#endif
