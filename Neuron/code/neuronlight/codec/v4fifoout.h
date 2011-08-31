#ifndef V4FIFOOUT_H_
#define V4FIFOOUT_H_

#include "v4rtenc.h"
#include "nlfifostream.h"
#include "nlaacrtbuf.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <v4e_api.h>
#include <v4_nalu.h>
#include <ThreadSingle.h>

#define N_STREAMS               9
#define MAX_WRITE_FIFO_ATTEMPTS 100

typedef enum
{
    NL_AACBUFINFO_EXCEPTION_BAD_DEQUEUE = 0
} nl_aacbufinfo_exception_t;

typedef enum
{
    V4FIFOOUT_RETCODE_OK = 0,
    V4FIFOOUT_RETCODE_ERR_MKFIFO,
    V4FIFOOUT_RETCODE_ERR_OPENFIFO,
    V4FIFOOUT_RETCODE_ERR_CLOSEFIFO,
} v4fifoout_retcode_t;

class nl_aacfifoout_t : public ThreadSingle
{
private:
    nl_aacrtbuf_t* p_aac_rtbuf;
    nl_fifostream_t* p_afs[N_STREAMS];
    virtual int workerBee(void);
    
public:
    nl_aacfifoout_t(nl_aacrtbuf_t* _p_aac_rtbuf,nl_fifostream_t* _p_afs[]):
    p_aac_rtbuf(_p_aac_rtbuf)
    {
        for (int i=0; i<N_STREAMS; i++) 
        {
            p_afs[i] = _p_afs[i];
        }
    }
    
    virtual ~nl_aacfifoout_t()
    {
    }
};

class v4_fifoout_t : public ThreadSingle
{
private:
    TimestampsLog* p_tslog;
    v4_rtenc_t* p_rtenc;
    nl_aacfifoout_t* p_aac_fifoout;
    nl_fifostream_t* p_fs[N_STREAMS];
    const std::string stream_name;
    media_sample_t* p_ms;
    virtual int workerBee(void);

public:
    v4_fifoout_t(const char* _stream_name,v4_rtenc_t* _p_rtenc,nl_aacrtbuf_t* _p_aac_rtbuf);
    void start_aac_fifoout(void);
    virtual ~v4_fifoout_t();
};

#endif