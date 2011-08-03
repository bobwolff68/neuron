#include <stdio.h>
#include "v4rtenc.h"
#include "logger.h"

using namespace std;

void v4_rtenc_t::InitRawFrameSettings(void)
{
    frame.sei_list = NULL;
    frame.reserved = 0;
    frame.modifier = NULL;
    frame.vp_frame.width = settings.input.width;
    frame.vp_frame.height = settings.input.height;
    frame.vp_frame.bytes_per_pel = settings.input.sample_size;
    frame.vp_frame.luma_bits = 8;
    frame.vp_frame.chroma_bits = 8;
    frame.vp_frame.valid_rows_count = settings.input.height;

    if(settings.input.colorspace==COLORSPACE_E_YV12)
    {
        frame.vp_frame.colorspace = VP_YV12;
        frame.vp_frame.stride[0] = settings.input.width;
        frame.vp_frame.stride[1] = settings.input.width>>1;
        frame.vp_frame.stride[2] = settings.input.width>>1;
    }
    else if(settings.input.colorspace==COLORSPACE_E_YUY2)
    {
        frame.vp_frame.colorspace = VP_YUY2;
        frame.vp_frame.stride[0] = settings.input.width<<1;
    }

    return;
}

void v4_rtenc_t::SetRawFrameBuffers(void* p_frame_buf)
{

    if(settings.input.colorspace==COLORSPACE_E_YV12)
    {
        static const int v_offset = settings.input.width*settings.input.height;
        static const int u_offset = 5*v_offset/4;
        frame.vp_frame.data[0] = p_frame_buf;
        frame.vp_frame.data[1] = p_frame_buf + v_offset;
        frame.vp_frame.data[2] = p_frame_buf + u_offset;
    }
    else if(settings.input.colorspace==COLORSPACE_E_YUY2)
        frame.vp_frame.data[0] = p_frame_buf;
    
    return;
}

v4_rtenc_t::v4_rtenc_t(const char* cfg_file,V4L2CapBuffer* _p_rtcap_buf):
p_rtcap_buf(_p_rtcap_buf)
{
    p_handle = NULL;
    handle_mutex = PTHREAD_MUTEX_INITIALIZER;
    settings.size = sizeof(v4e_settings_t);
    LOG_OUT("checkpoint: get default encoder settings");

    //Get default encoder settings
    if(v4e_default_settings(&settings)!=VSSH_OK)
    {
        LOG_ERR("v4e_default_settings() error");
        throw RTENC_RETCODE_ERR_DEFAULT_SETTINGS;
    }

    LOG_OUT("checkpoint: read config file");
    
    //Overwrite some settings from config file
    if(v4e_read_config_file(&settings,(char*)cfg_file)!=VSSH_OK)
    {
        LOG_ERR("v4e_read_config_file() error");
        throw RTENC_RETCODE_ERR_READ_CFG_FILE;
    }
}

v4_rtenc_t::~v4_rtenc_t()
{
}

void* v4_rtenc_t::Handle(void) const
{
    return p_handle;
}

RTEnc_ReturnCode_t v4_rtenc_t::Open(void)
{
    
    LOG_OUT("checkpoint: open encoder handle");
    
    //Open encoder handle
    if(v4e_open(&p_handle,&settings)!=VSSH_OK)
    {
        LOG_ERR("v4e_open() error");
        return RTENC_RETCODE_ERR_OPEN;
    }

    //Get actual encoder settings (v4e_open() probably modifies some params)
    if(v4e_get_current_settings(p_handle,&settings)!=VSSH_OK)
    {
        LOG_ERR("v4e_get_current_settings() error");
        return RTENC_RETCODE_ERR_GET_SETTINGS;
    }

    LOG_OUT("checkpoint: display encoder settings");

#if 0 //def BUILD_DEBUG
    if(v4e_write_config_file(&settings,NULL)!=VSSH_OK)
#else
    if(0)
#endif
    {
        LOG_ERR("v4e_write_config_file() error");
        return RTENC_RETCODE_ERR_WRITE_CFG_FILE;
    }

    LOG_OUT("checkpoint: init raw frame settings");

    //Init raw frame settings
    InitRawFrameSettings();

    return RTENC_RETCODE_OK;
}

RTEnc_ReturnCode_t v4_rtenc_t::Close(void)
{
    LOG_OUT("checkpoint: close encoder");

    if(v4e_close(p_handle)!=VSSH_OK)
    {
        LOG_ERR("v4e_close() error");
        return RTENC_RETCODE_ERR_CLOSE;
    }

    return RTENC_RETCODE_OK;
}

void v4_rtenc_t::SetEncSettings(map<string,string>& nvpairs)
{
    for(map<string,string>::iterator it = nvpairs.begin();
        it!=nvpairs.end();
        it++)
    {
#ifdef BUILD_DEBUG
        char msg[100];
        sprintf(msg,"checkpoint: set encoder setting '%s=%s'",
                it->first.c_str(),it->second.c_str());
#endif
        LOG_OUT(msg);

        if(it->first=="Width")
            sscanf(it->second.c_str(),"%d",&(settings.input.width));
        else if(it->first=="Height")
            sscanf(it->second.c_str(),"%d",&(settings.input.height));
        else if(it->first=="Colorspace")
        {
            if(it->second=="YUYV")
                settings.input.colorspace = COLORSPACE_E_YUY2;
            else if(it->second=="YV12")
                settings.input.colorspace = COLORSPACE_E_YV12;
        }
    }

    return;
}

int v4_rtenc_t::workerBee(void)
{
    while(1)
    {
        V4L2BufferInfo buf_info;

        if(p_rtcap_buf->FullBufferDQ(buf_info)==false)
        {
            LOG_ERR("capture buffer dequeue error");
            return -1;
        }

        if(buf_info.bFinalSample)
        {
            LOG_OUT("checkpoint: final sample");
            break;
        }

        LOG_OUT("checkpoint: set raw frame buffer");
        SetRawFrameBuffers(buf_info.pBuffer);
        frame.timestamp = buf_info.buf.timestamp.tv_sec*1000 + buf_info.buf.timestamp.tv_usec/1000;
        LOG_OUT("checkpoint: send raw frame to encoder");

        LockHandle();
        if(v4e_set_vp_frame(p_handle,&frame,1)!=VSSH_OK)
        {
            LOG_ERR("v4e_set_vp_frame() error");
            return -1;
        }
        UnlockHandle();

        if(p_rtcap_buf->EmptyBufferRelease(buf_info)==false)
        {
            LOG_ERR("empty capture buffer release error");
            return -1;
        }
    }

    LOG_OUT("checkpoint: flush encoder");

    LockHandle();
    if(v4e_set_flush(p_handle)!=VSSH_OK)
    {
        LOG_ERR("v4e_set_flush() error");
        return -1;
    }
    UnlockHandle();

    //sleep for some time so that output thread can
    //process all flushed frames
    usleep(30000);

    return 0;
}

RTEnc_ReturnCode_t v4_rtenc_t::LockHandle(void)
{
    if(pthread_mutex_lock(&handle_mutex)==-1)
    {
        LOG_ERR("pthread_mutex_lock() error");
        return RTENC_RETCODE_ERR_LOCK_HANDLE;
    }

    return RTENC_RETCODE_OK;
}

RTEnc_ReturnCode_t v4_rtenc_t::UnlockHandle(void)
{
    if(pthread_mutex_unlock(&handle_mutex)==-1)
    {
        LOG_ERR("pthread_mutex_unlock() error");
        return RTENC_RETCODE_ERR_UNLOCK_HANDLE;
    }

    return RTENC_RETCODE_OK;
}
