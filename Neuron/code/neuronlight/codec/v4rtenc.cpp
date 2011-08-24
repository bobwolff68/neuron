#include <iostream>
#include "v4rtenc.h"

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
        for (int i=1; i<6; i++) 
        {
            frame.vp_frame.stride[i] = 0;
            frame.vp_frame.data[i] = NULL;
        }
    }
    else if(settings.input.colorspace==COLORSPACE_E_UYVY)
    {
        frame.vp_frame.colorspace = VP_UYVY;
        frame.vp_frame.stride[0] = settings.input.width<<1;
        for (int i=1; i<6; i++) 
        {
            frame.vp_frame.stride[i] = 0;
            frame.vp_frame.data[i] = NULL;
        }
    }

    return;
}

void v4_rtenc_t::SetRawFrameBuffers(unsigned char* p_frame_buf, int stride)
{

    if(settings.input.colorspace==COLORSPACE_E_YV12)
    {
        static const int v_offset = settings.input.width*settings.input.height;
        static const int u_offset = 5*v_offset/4;
        frame.vp_frame.data[0] = p_frame_buf;
        frame.vp_frame.data[1] = p_frame_buf + v_offset;
        frame.vp_frame.data[2] = p_frame_buf + u_offset;
    }
    else if(settings.input.colorspace==COLORSPACE_E_YUY2 ||
            settings.input.colorspace==COLORSPACE_E_UYVY)
    {
        frame.vp_frame.data[0] = p_frame_buf;
        for (int i=1; i<6; i++) 
        {
            frame.vp_frame.data[i] = NULL;
            frame.vp_frame.stride[i] = 0;
        }
    	frame.vp_frame.stride[0] = stride;
    }
    return;
}

#if (defined (__APPLE__) & defined (__MACH__))
v4_rtenc_t::v4_rtenc_t(const char* cfg_file,QTKitCapBuffer* _p_rtcap_buf,nl_aacrtbuf_t* _p_aac_rtbuf):
p_aac_rtbuf(_p_aac_rtbuf),
#else
v4_rtenc_t::v4_rtenc_t(const char* cfg_file,V4L2CapBuffer* _p_rtcap_buf):
#endif
p_rtcap_buf(_p_rtcap_buf)
{
    int err;
    p_handle = NULL;
    //timestamp_base = 0;
    //handle_mutex = PTHREAD_MUTEX_INITIALIZER;
    err = pthread_mutex_init(&handle_mutex, NULL);
    assert(err==0);
    settings.size = sizeof(v4e_settings_t);
    LOG_OUT("checkpoint: get default encoder settings");

    //Get default encoder settings
    if(v4e_default_settings(&settings)!=VSSH_OK)
    {
        LOG_ERR("v4e_default_settings() error");
        throw RTENC_RETCODE_ERR_DEFAULT_SETTINGS;
    }

    LOG_OUT("checkpoint: read config file");
    char cwd[500];
    getcwd(cwd, 499);
    cout << "Current working directory: " << cwd << endl;
    
    //Overwrite some settings from config file
    if(v4e_read_config_file(&settings,(char*)cfg_file)!=VSSH_OK)
    {
        LOG_ERR("v4e_read_config_file() error");
        throw RTENC_RETCODE_ERR_READ_CFG_FILE;
    }
}

v4_rtenc_t::~v4_rtenc_t()
{
    pthread_mutex_destroy(&handle_mutex);
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
            else if(it->second=="UYVY")
                settings.input.colorspace = COLORSPACE_E_UYVY;
        }
    }

    return;
}

int v4_rtenc_t::workerBee(void)
{    
/*********** AAC AUDIO STREAM **********/
    uint8_t* p_aacbuf = NULL;
    const int aacbuf_size = FF_MIN_BUFFER_SIZE*10;
    int aac_outbytes = 0;
    
//    float t = 0;
//    float t_incr = 2*M_PI*440.0/p_acctx->sample_rate;
    const int exp_samples = p_acctx->frame_size;
    int in_samples = 0;
    short* p_samples = (short*) malloc((exp_samples<<1)*p_acctx->channels);
/***************************************/
    
    while(1)
    {
#if (defined (__APPLE__) & defined (__MACH__))
        QTKitBufferInfo* p_bi;
        RTBufferInfoBase* p_bib;
#else
        V4L2BufferInfo buf_info;
#endif
        
#ifdef COPY_QTKIT_CAP_BUFFERS
        void* pb;
        if(p_rtcap_buf->FullBufferDQ(&p_bib, &pb)==false)
#else
        if(p_rtcap_buf->FullBufferDQ(&p_bib)==false)
#endif
        {
            cout << "OUCH - Bad Deque" << endl;
            LOG_ERR("capture buffer dequeue error");
            return -1;
        }

        if(p_bib->bFinalSample)
        {
#ifdef COPY_QTKIT_CAP_BUFFERS
            if(p_rtcap_buf->EmptyBufferRelease(p_bib, pb)==false)
#else
            if(p_rtcap_buf->EmptyBufferRelease(p_bib)==false)
#endif
            {
                LOG_ERR("empty capture buffer release error");
                return -1;
            }
            LOG_OUT("checkpoint: final sample");
            break;
        }

#if (defined (__APPLE__) & defined (__MACH__))
        p_bi = static_cast<QTKitBufferInfo*>(p_bib);
        if(p_bi->bIsVideo)
        {
#ifdef COPY_QTKIT_CAP_BUFFERS
            if (pb)
                SetRawFrameBuffers((unsigned char*)pb, p_bi->captureStride);
            else
#endif
            SetRawFrameBuffers((unsigned char*)(p_bi->pBuffer), p_bi->captureStride);
            
            /*if (timestamp_base == 0) 
            {
                timestamp_base = p_bi->timeStamp_uS;
            }
            frame.timestamp = p_bi->timeStamp_uS - timestamp_base;*/
            
            LockHandle();
                
            if(v4e_set_vp_frame(p_handle,&frame,1)!=VSSH_OK)
            {
                LOG_ERR("v4e_set_vp_frame() error");
                return -1;
            }
            
            UnlockHandle();
        }
        /********************* AAC AUDIO STREAM *******************/
        else
        {
            assert(p_bi->rawLength!=-1);
            
            if((exp_samples-in_samples) >= p_bi->rawNumSamples)
            {
                cout << "Expected samples remaining: " << (exp_samples-in_samples) << endl;
                memcpy(p_samples+in_samples*p_acctx->channels, p_bi->pBuffer, p_bi->rawLength);
                in_samples += p_bi->rawNumSamples;
            }
            else
            {
                cout << "Expected samples remaining: " << (exp_samples-in_samples) << endl;
                memcpy(p_samples+in_samples*p_acctx->channels, p_bi->pBuffer, ((exp_samples-in_samples)<<1)*p_acctx->channels);
                //in_samples = exp_samples;

            cout << "Got " << exp_samples << " samples, encoding..." << endl;
            p_aacbuf = (uint8_t*) malloc(aacbuf_size*sizeof(uint8_t));
            assert(p_aacbuf!=NULL);
            
/*            for (int i=0; i<p_acctx->frame_size; i++) 
            {
                p_samples[(i<<1)] = (int)(sin(t)*10000);
                p_samples[(i<<1)+1] = p_samples[(i<<1)];
                t += t_incr;
            }
*/            
            //assume raw data mode
            aac_outbytes = avcodec_encode_audio(
                                p_acctx,
                                p_aacbuf,
                                aacbuf_size,
                                //(const short*)(p_bi->pBuffer)
                                p_samples
                           );
            
            if(aac_outbytes < 0)
            {
                LOG_ERR("Unable to encode sample...");
            }
            else if(aac_outbytes > 0)
            {
                /*cout << "************************************" << endl;
                cout << "Frame size: " << p_acctx->frame_size << endl;
                cout << "Channels: " << p_acctx->channels << endl;
                cout << "Out bytes: " << aac_outbytes << endl;*/
                
                nl_aacbufinfo_t* pBI = new nl_aacbufinfo_t();
                pBI->bytes = aac_outbytes;
                pBI->pBuffer = p_aacbuf;
                pBI->bFinalSample = false;
                p_aac_rtbuf->FullBufferEnQ(pBI);
            }
                
                in_samples = p_bi->rawNumSamples - (exp_samples-in_samples);
                memcpy(p_samples, p_bi->pBuffer, ((exp_samples-in_samples)<<1)*p_acctx->channels);
            }
        }
        /*********************************************************************************/
#else
        SetRawFrameBuffers((unsigned char*)(buf_info.pBuffer));
        frame.timestamp = buf_info.buf.timestamp.tv_sec*1000 + buf_info.buf.timestamp.tv_usec/1000;

        
        LOG_OUT("checkpoint: send raw frame to encoder");

        LockHandle();
        if(v4e_set_vp_frame(p_handle,&frame,1)!=VSSH_OK)
        {
            LOG_ERR("v4e_set_vp_frame() error");
            return -1;
        }
        UnlockHandle();
#endif
        
        //cout << "Before releasing" << endl;
#ifdef COPY_QTKIT_CAP_BUFFERS
        if(p_rtcap_buf->EmptyBufferRelease(p_bib, pb)==false)
#else
        if(p_rtcap_buf->EmptyBufferRelease(p_bib)==false)
#endif
        {
            LOG_ERR("empty capture buffer release error");
            return -1;
        }
        //cout << "After releasing" << endl;
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

#if (defined(__APPLE__) && defined(__MACH__))
RTEnc_ReturnCode_t v4_rtenc_t::OpenAudio(void)
{
    avcodec_register_all();

    p_acodec = avcodec_find_encoder(CODEC_ID_MP2);
    if(p_acodec == NULL)
    {
        LOG_ERR("aac codec not found...");
        throw RTENC_RETCODE_ERR_AOPEN;
    }
    
    p_acctx = avcodec_alloc_context3(p_acodec);
    
    //encoder parameters
    p_acctx->bit_rate = 128000;
    p_acctx->sample_rate = 44100;
    p_acctx->channels = 2;
    p_acctx->sample_fmt = AV_SAMPLE_FMT_S16;
    p_acctx->profile = FF_PROFILE_AAC_LOW;
    p_acctx->time_base = (AVRational){1,p_acctx->sample_rate};
    
    //Test if sample format is supported
    LOG_OUT("Test if flt sample format is supported...");
    
    int i;
    for(i=0; p_acodec->sample_fmts[i]!=AV_SAMPLE_FMT_NONE; i++)
    {
        if (p_acctx->sample_fmt == p_acodec->sample_fmts[i]) 
        {
            LOG_OUT("Specified sample format is indeed supported...");
            break;
        }
    }
    
    if (p_acodec->sample_fmts[i] == AV_SAMPLE_FMT_NONE) 
    {
        LOG_ERR("Specified sample format is not supported");
        throw RTENC_RETCODE_ERR_AOPEN;
    }
    
    //open encoder
    //if(avcodec_open2(p_acctx, p_acodec, &p_opts_dict) < 0)
    if(avcodec_open(p_acctx, p_acodec) < 0)
    {
        LOG_ERR("unable to open aac codec...");
        return RTENC_RETCODE_ERR_AOPEN;
    }
    
    return RTENC_RETCODE_OK;
}
    
RTEnc_ReturnCode_t v4_rtenc_t::CloseAudio(void)
{
    avcodec_close(p_acctx);
    av_free(p_acctx);
    return RTENC_RETCODE_OK;
}

#endif
