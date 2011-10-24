#include "v4fifoout.h"

using namespace std;

v4_fifoout_t::v4_fifoout_t(SafeBufferDeque* _p_bsdq,v4_rtenc_t* _p_rtenc):
p_bsdq(_p_bsdq),
p_rtenc(_p_rtenc)
{
}

v4_fifoout_t::~v4_fifoout_t()
{
    cout << "Deleting fifoout instance..." << endl;
}

int v4_fifoout_t::workerBee(void)
{
    int retcode;
    int frm_num = 0;
    
    cout << "fifoout workerBee() started." << endl;

    while(1)
    {
        p_rtenc->LockHandle();
        retcode = v4e_get_nal_ex((void*)p_rtenc->Handle(),&p_ms,1);
        p_rtenc->UnlockHandle();
        
        switch(retcode)
        {
            case VSSH_OK:
            {
                //Log timestamps
                if(NALU_TYPE(((char*)(p_ms->data))[0]) == NALU_TYPE_SLICE)
                {
                    struct timeval tod;
                    gettimeofday(&tod, NULL);
                    p_tslog->WriteEntry(&tod, frm_num++, 0, 0);
                }
                
//                cout << "v4fifoout - Encoded video received - Adding to p_ms->data" << endl;
                
                p_bsdq->AddItem((unsigned char*)p_ms->data, p_ms->used_size);
                v4_free_media_sample(p_ms);
                break;
            }

            case VSSH_WARN_EOS:
            {
                p_bsdq->SetDefunct();
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