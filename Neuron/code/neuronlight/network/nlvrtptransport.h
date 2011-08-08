#ifndef NLVRTPTRANSPORT_H_
#define NLVRTPTRANSPORT_H_

#include <v4rtenc.h>
#include "nlrtpstream.h"

typedef enum
{
    NL_VRTPTRANSPORT_RETCODE_OK = 0,
    NL_VRTPTRANSPORT_RETCODE_ERR_RTPSTREAM
} nl_vrtptransport_retcode_t;

class nl_vrtptransport_t : public ThreadSingle
{
private:
    v4_rtenc_t* p_rtenc;
    media_sample_t* p_ms;
    nl_rtpstream_t* p_stream;
    virtual int workerBee(void);

public:
    nl_vrtptransport_t(const char* dest_ta, v4_rtenc_t* _p_rtenc):
    p_rtenc(_p_rtenc)
    {
        try
        {
            p_stream = new nl_rtpstream_t(0xdeadbeef);
            p_stream->add_dest(dest_ta);
        }
        catch(nl_rtpstream_retcode_t& err_code)
        {
            LOG_ERR("nl_vrtptransport_t() error");
            throw NL_VRTPTRANSPORT_RETCODE_ERR_RTPSTREAM;
        }
    }

    ~nl_vrtptransport_t()
    {
        delete p_stream;
    }
};

#endif