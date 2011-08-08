#ifndef V4FIFOOUT_H_
#define V4FIFOOUT_H_

#include "v4rtenc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <v4e_api.h>
#include <v4_nalu.h>
#include <ThreadSingle.h>

#define MAX_WRITE_FIFO_ATTEMPTS 100

typedef enum
{
    V4FIFOOUT_RETCODE_OK = 0,
    V4FIFOOUT_RETCODE_ERR_MKFIFO,
    V4FIFOOUT_RETCODE_ERR_OPENFIFO,
    V4FIFOOUT_RETCODE_ERR_CLOSEFIFO
} v4fifoout_retcode_t;

class v4_avcbsbuf_t
{
private:
    unsigned char* p_membuf;
    int size;
    
public:
    v4_avcbsbuf_t()
    {
        size = 4;
        p_membuf = (unsigned char*) malloc(size);
        p_membuf[0] = 0;
        p_membuf[1] = 0;
        p_membuf[2] = 0;
        p_membuf[3] = 1;
    }

    ~v4_avcbsbuf_t()
    {
        free(p_membuf);
    }

    void copy_nalu(const void* p_nalu_buf,const int nalu_size)
    {
        if(size < (nalu_size+4))
        {
            size = nalu_size + 4;
            p_membuf = (unsigned char*) realloc(p_membuf,size);
        }

        memcpy(p_membuf+4,p_nalu_buf,nalu_size);
    }

    unsigned char* get_membuf_ptr(void) const
    {
        return p_membuf;
    }
};

class v4_fifoout_t : public ThreadSingle
{
private:
    v4_rtenc_t* p_rtenc;
    v4_avcbsbuf_t* p_avcbs_buf;
    int fd_fifo_out;
    const char* fifo_out;
    media_sample_t* p_ms;
    virtual int workerBee(void);

public:
    v4_fifoout_t(const char* fifo_out, v4_rtenc_t* _p_rtenc);
    virtual ~v4_fifoout_t();
};

#endif