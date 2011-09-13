#ifndef V4FIFOOUT_H_
#define V4FIFOOUT_H_

#include <v4rtenc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <v4e_api.h>
#include <v4_nalu.h>
#include <ThreadSingle.h>
#include <SafeBufferDeque.h>

typedef enum
{
    V4FIFOOUT_RETCODE_OK = 0,
    V4FIFOOUT_RETCODE_ERR_MKFIFO,
    V4FIFOOUT_RETCODE_ERR_OPENFIFO,
    V4FIFOOUT_RETCODE_ERR_CLOSEFIFO,
} v4fifoout_retcode_t;

class v4_fifoout_t : public ThreadSingle
{
private:
    TimestampsLog* p_tslog;
    v4_rtenc_t* p_rtenc;
    SafeBufferDeque* p_bsdq;
    media_sample_t* p_ms;
    virtual int workerBee(void);

public:
    v4_fifoout_t(SafeBufferDeque* _p_bsdq,v4_rtenc_t* _p_rtenc);
    virtual ~v4_fifoout_t();
};

#endif