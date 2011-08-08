#include "v4fifoout.h"

using namespace std;

v4_fifoout_t::v4_fifoout_t(const char* _fifo_out, v4_rtenc_t* _p_rtenc):
fifo_out(_fifo_out),p_rtenc(_p_rtenc)
{
    p_avcbs_buf = new v4_avcbsbuf_t();
}


v4_fifoout_t::~v4_fifoout_t()
{
    delete p_avcbs_buf;
}

int v4_fifoout_t::workerBee(void)
{
    int retcode;
   
    LOG_OUT("checkpoint: test if output fifo exists");

    if(access(fifo_out,F_OK)==-1)
    {
        if(mkfifo(fifo_out,S_IWUSR|S_IRUSR)==-1)
        {
            LOG_ERR("mkfifo() error");
            throw V4FIFOOUT_RETCODE_ERR_MKFIFO;
        }
    }

    LOG_OUT("checkpoint: open output fifo");
    fd_fifo_out = open(fifo_out,O_WRONLY|O_NONBLOCK);
    /*if((fd_fifo_out=open(fifo_out,O_WRONLY))==-1)
    {
        LOG_ERR("open() error");
        throw V4FIFOOUT_RETCODE_ERR_OPENFIFO;
    }*/

    while(1)
    {
        if(fd_fifo_out==-1)
            fd_fifo_out = open(fifo_out,O_WRONLY|O_NONBLOCK);
        
        p_rtenc->LockHandle();
        retcode = v4e_get_nal_ex((void*)p_rtenc->Handle(),&p_ms,1);
        p_rtenc->UnlockHandle();
        
        switch(retcode)
        {
            case VSSH_OK:
            {
                LOG_OUT("checkpoint: nalu_type="
                            << NALU_TYPE(((uint8_t*)(p_ms->data))[0])
                            << ", nalu_ridc="
                            << NALU_RIDC(((uint8_t*)(p_ms->data))[0])
                            << ", size="
                            << p_ms->used_size
                            << " (bytes)");

                p_avcbs_buf->copy_nalu(p_ms->data,p_ms->used_size);

                if(fd_fifo_out != -1)
                {
                    if(write(fd_fifo_out,
                          p_avcbs_buf->get_membuf_ptr(),
                          p_ms->used_size+4)==-1)
                    {
                        if(close(fd_fifo_out)==-1)
                        {
                            LOG_ERR("close() error");
                            throw V4FIFOOUT_RETCODE_ERR_CLOSEFIFO;
                        }
                        fd_fifo_out = -1;
                    }
                }
                v4_free_media_sample(p_ms);
                break;
            }

            case VSSH_WARN_EOS:
            {
                LOG_OUT("checkpoint: end of stream");                
                if(fd_fifo_out!=-1 && close(fd_fifo_out)==-1)
                {
                    LOG_ERR("close() error");
                    throw V4FIFOOUT_RETCODE_ERR_CLOSEFIFO;
                }
                return 0;
            }

            default:
            {
                usleep(15000);
                break;
            }
        }
    }

    return 0;
}