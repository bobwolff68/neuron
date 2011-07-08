//!
//! \file V4L2Cap.h
//! 
//! \brief 
//! 
//! \author rwolff
//! \date Thu 30 Jun 2011 11:28:25 AM 
//! 

#ifdef TEST
#define SKIP_DDS
#endif

#include "neuroncommon.h"

#include <string.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <semaphore.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>


