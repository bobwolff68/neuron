#include <iostream>
#include "nlvrtptransport.h"

using namespace std;

int nl_vrtptransport_t::workerBee(void)
{
    int retcode;
    uint32_t ts = 0;
    const char start_code[4] = {0,0,0,1};
    
    while(1)
    {
        p_rtenc->LockHandle();
        retcode = v4e_get_nal_ex(p_rtenc->Handle(), &p_ms, 1);
        p_rtenc->UnlockHandle();

        switch(retcode)
        {
            case VSSH_OK:
            {
                /*p_stream->dist_packet(start_code,4,40,0,ts);*/
                
                if(p_stream->dist_packet(
                      p_ms->data,
                      p_ms->used_size,
                      40,
                      p_ms->is_last_in_pict==LAST_IN_PICT,
                      ts++
                   ) != NL_RTPSTREAM_RETCODE_OK)
                {
                    LOG_ERR("dist_packet() error");
                }

                v4_free_media_sample(p_ms);
                break;
            }

            case VSSH_WARN_EOS:
            {
                LOG_OUT("checkpoint: end of stream");
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