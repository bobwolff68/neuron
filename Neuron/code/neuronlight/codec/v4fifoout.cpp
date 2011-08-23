#include "v4fifoout.h"

using namespace std;

v4_fifoout_t::v4_fifoout_t(const char* _stream_name,v4_rtenc_t* _p_rtenc)://,nl_aacrtbuf_t* _p_aac_rtbuf):
stream_name(_stream_name),
p_rtenc(_p_rtenc)
{
    for (int i=0; i<N_STREAMS; i++) 
    {
        char snum[2];
        sprintf(snum, "%d", i);
        p_fs[i] = new nl_fifostream_t((stream_name+snum).c_str());
    }
    
    //p_aac_fifoout = new nl_aacfifoout_t(_p_aac_rtbuf, p_fs);
}


v4_fifoout_t::~v4_fifoout_t()
{
    delete p_aac_fifoout;
    
    for (int i=0; i<N_STREAMS; i++) 
    {
        delete p_fs[i];
    }
}

int v4_fifoout_t::workerBee(void)
{
    int retcode;
    
    //p_aac_fifoout->startThread();
    while(1)
    {
        p_rtenc->LockHandle();
        retcode = v4e_get_nal_ex((void*)p_rtenc->Handle(),&p_ms,1);
        p_rtenc->UnlockHandle();
        
        switch(retcode)
        {
            case VSSH_OK:
            {
                for (int i=0; i<N_STREAMS; i++) 
                {
                    if (p_fs[i]->write_to_substream((uint8_t*)p_ms->data,
                                                    p_ms->used_size, 
                                                    true) != NLFS_RETCODE_OK) 
                    {
                        throw V4FIFOOUT_RETCODE_ERR_CLOSEFIFO;
                    }
                }

                v4_free_media_sample(p_ms);
                break;
            }

            case VSSH_WARN_EOS:
            {
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

int nl_aacfifoout_t::workerBee(void)
{
    RTBufferInfoBase* p_bib = NULL;
    nl_aacbufinfo_t* p_bi = NULL;
    
    while(1)
    {
        cout << "Before aac dequeue" << endl;
        if(p_aac_rtbuf->FullBufferDQ(&p_bib)==false)
        {
            LOG_ERR("Bad dequeue(aac)");
            throw NL_AACBUFINFO_EXCEPTION_BAD_DEQUEUE;
        }
        cout << "After aac dequeue" << endl;
        
        assert(p_bib != NULL);
        p_bi = static_cast<nl_aacbufinfo_t*>(p_bib);
        
        if (p_bib->bFinalSample) 
        {
            break;
        }
        
        for (int i=0; i<N_STREAMS; i++) 
        {
            p_afs[i]->write_to_substream((uint8_t*)p_bi->pBuffer, p_bi->bytes, false);
        }
        
        p_aac_rtbuf->EmptyBufferRelease(p_bib);
    }
    
    return 0;
}