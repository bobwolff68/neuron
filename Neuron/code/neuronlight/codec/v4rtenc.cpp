//#import <Cocoa/Cocoa.h>
//#import <Foundation/Foundation.h>

#include <iostream>
#include "v4rtenc.h"

#include <pwd.h>

using namespace std;

#ifdef LOG_TIMESTAMPS
TimestampsLog::TimestampsLog(const char* log_file_name)
{
    log_fd = open(log_file_name, O_WRONLY|O_CREAT);
    if(log_fd == -1)
    {
        LOG_ERR("Unable to open log file " << log_file_name);
        throw -1;
    }
}

TimestampsLog::~TimestampsLog()
{
    close(log_fd);
}

void TimestampsLog::WriteEntry(struct timeval *p_tod, const int frm_num, const int64_t apple_ts, const int64_t rtp_ts)
{
    char entry[4][100];
    
    if(p_tod != NULL)
    {
        sprintf(entry[0], "%ld.%06d,",p_tod->tv_sec,p_tod->tv_usec);
    }
    else
    {
        sprintf(entry[0], " ,");
    }
    
    if (frm_num >= 0) 
    {
        sprintf(entry[1], "%d,", frm_num);
    }
    else
    {
        sprintf(entry[1], " ,");        
    }
    
    if (apple_ts > 0) 
    {
        sprintf(entry[2], "%lld,", apple_ts);
    }
    else
    {
        sprintf(entry[2], " ,");        
    }
    
    if (rtp_ts > 0) 
    {
        sprintf(entry[3], "%lld\n", rtp_ts);
    }
    else
    {
        sprintf(entry[3], " \n");        
    }
    
    for (int i=0; i<4; i++) 
    {
        write(log_fd, entry[i], strlen(entry[i]));
    }
}
#endif


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
v4_rtenc_t::v4_rtenc_t(QTKitCapBuffer* _p_rtcap_buf,
                       SafeBufferDeque* _p_abs_dq,
                       bool _b_video_on,
                       bool _b_audio_on):
b_video_on(_b_video_on),
b_audio_on(_b_audio_on),
p_abs_dq(_p_abs_dq),
#else
v4_rtenc_t::v4_rtenc_t(const char* cfg_file,V4L2CapBuffer* _p_rtcap_buf):
#endif
p_rtcap_buf(_p_rtcap_buf)
{
    int err;
    p_handle = NULL;
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
    string cfg_file_path = PrepareEncoderSettings();
    
    //Overwrite some settings from config file
    if(v4e_read_config_file(&settings,(char*)cfg_file_path.c_str())!=VSSH_OK)
    {
        LOG_ERR("v4e_read_config_file() error");
        throw RTENC_RETCODE_ERR_READ_CFG_FILE;
    }
    
    p_tslog = new TimestampsLog("ts_preenc.log");
}

v4_rtenc_t::~v4_rtenc_t()
{
    delete p_tslog;
    pthread_mutex_destroy(&handle_mutex);
    cout << "Deleting rtenc instance... " << endl;
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
        LOG_OUT(msg);
#endif

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
    int frm_num = 0;
    struct timeval tod;
    uint8_t* p_aacbuf = NULL;
    const int aacbuf_size = FF_MIN_BUFFER_SIZE*10;
    int aac_outbytes = 0;
    
    const int exp_samples = p_acctx->frame_size;
    int in_samples = 0;
    short* p_samples = (short*) malloc((exp_samples<<1)*p_acctx->channels);
    
    p_aacbuf = (uint8_t*) malloc(aacbuf_size*sizeof(uint8_t)+7);
    assert(p_aacbuf!=NULL);
    
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
            continue;
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
            
            if(p_aacbuf != NULL)
                free(p_aacbuf);
            
            if(p_samples != NULL)
                free(p_samples);
            
            break;
        }

#if (defined (__APPLE__) & defined (__MACH__))
        p_bi = static_cast<QTKitBufferInfo*>(p_bib);
        if(p_bi->bIsVideo && b_video_on)
        {
#ifdef COPY_QTKIT_CAP_BUFFERS
            if (pb)
                SetRawFrameBuffers((unsigned char*)pb, p_bi->captureStride);
            else
#endif
            SetRawFrameBuffers((unsigned char*)(p_bi->pBuffer), p_bi->captureStride);
            frame.timestamp = p_bi->timeStamp_uS;
            
            //Log timestamp info
            gettimeofday(&tod, NULL);
            p_tslog->WriteEntry(&tod, frm_num++, p_bi->timeStamp_uS, 0);
            
            
            LockHandle();
            if(v4e_set_vp_frame(p_handle,&frame,1)!=VSSH_OK)
            {
                LOG_ERR("v4e_set_vp_frame() error");
                return -1;
            }
            UnlockHandle();
        }
        else if(!p_bi->bIsVideo && b_audio_on)
        {
            assert(p_bi->rawLength!=-1);
            
            if((exp_samples-in_samples) >= p_bi->rawNumSamples)
            {
                memcpy(p_samples+in_samples*p_acctx->channels, p_bi->pBuffer, p_bi->rawLength);
                in_samples += p_bi->rawNumSamples;
            }
            else
            {
                memcpy(p_samples+in_samples*p_acctx->channels, p_bi->pBuffer, ((exp_samples-in_samples)<<1)*p_acctx->channels);
            
                //assume raw data mode
                aac_outbytes = avcodec_encode_audio(
                                    p_acctx,
                                    p_aacbuf+7,
                                    aacbuf_size,
                                    p_samples
                           );
            
                if(aac_outbytes < 0)
                {
                    LOG_ERR("Unable to encode sample...");
                }
                else if(aac_outbytes > 0)
                {
                    //aac frame header
                    p_aacbuf[0] = 0xFF;
                    p_aacbuf[1] = 0xF1;

                    //profile
                    p_aacbuf[2] = 0 | (p_acctx->profile<<6);

                    //sampling frequency index
                    //TODO: remove hardcoded index and replace with map of sampling frequencies and indices
                    p_aacbuf[2] |= (5<<2);
                    p_aacbuf[2] |= (p_acctx->channels>>2);
                    p_aacbuf[3] = 0 | ((p_acctx->channels&0x03)<<6);

                    //frame length
                    uint16_t frame_length = aac_outbytes + 7;
                    p_aacbuf[3] |= ((frame_length>>11) & 0x03);
                    p_aacbuf[4] = 0 | ((frame_length>>3) & 0xFF);
                    p_aacbuf[5] = 0 | ((frame_length&0x07) << 5);
                    
                    //buffer fullness
                    p_aacbuf[5] |= 0x1F;
                    
                    //buffer fullness + no of aac frames-1
                    p_aacbuf[6] = 0xFC | 0;
                    
                    p_abs_dq->AddItem(p_aacbuf, frame_length);
                }

                int offset = ((exp_samples-in_samples)<<1)*p_acctx->channels;
                int wrlen = ((p_bi->rawNumSamples - (exp_samples-in_samples))<<1)*p_acctx->channels;
                
                memcpy(p_samples, (void*)((unsigned char*)p_bi->pBuffer + offset), wrlen);
                
                in_samples = p_bi->rawNumSamples - (exp_samples-in_samples);
            }
        }
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
        
#ifdef COPY_QTKIT_CAP_BUFFERS
        if(p_rtcap_buf->EmptyBufferRelease(p_bib, pb)==false)
#else
        if(p_rtcap_buf->EmptyBufferRelease(p_bib)==false)
#endif
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

#if (defined(__APPLE__) && defined(__MACH__))
RTEnc_ReturnCode_t v4_rtenc_t::OpenAudio(void)
{
    avcodec_register_all();

    p_acodec = avcodec_find_encoder(CODEC_ID_AAC);
    if(p_acodec == NULL)
    {
        LOG_ERR("aac codec not found...");
        throw RTENC_RETCODE_ERR_AOPEN;
    }
    
    p_acctx = avcodec_alloc_context3(p_acodec);
    
    //encoder parameters
    //TODO: remove hardcoded values
    p_acctx->bit_rate = 64000;
    p_acctx->sample_rate = 32000;
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
    if(avcodec_open2(p_acctx, p_acodec, NULL) < 0)
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

void v4_rtenc_t::ChangeTargetBitrate(int newBitrateKbps)
{
    int retcode = v4e_change_bitrate(Handle(), newBitrateKbps);
    assert(retcode == VSSH_OK);
}

void v4_rtenc_t::ChangeTargetFrameRate(int newFrameRateFps)
{
    int retcode = v4e_change_bitrate_and_framerate(Handle(), 0, newFrameRateFps*2000, 1000);
    assert(retcode == VSSH_OK);
}

string v4_rtenc_t::PrepareEncoderSettings(void)
{
    string encSettings(
        "svc.num_layers=0\n"                     
        "svc.multistream_mode=0\n" 
        "profile_idc=77\n"
        "level_idc=32\n"
        "sym_mode=1\n"
        "gpu_acceleration=0\n"
        "gop.idr_period=1\n"
        "gop.keyframes=150\n"
        "gop.bframes=3\n"
        "gop.emulate_b=2\n"
        "gop.num_units=1000\n"
        "gop.time_scale=60000\n"
        "rc.type=2\n"
        "rc.kbps=600\n"
        "rc.auto_qp=1\n"
        "speed.i=8\n"
        "speed.p=8\n"
        "speed.b=8\n"
        "speed.automatic=0\n"
    );
    
#if 1
    char* sHomeDir = getenv("HOME");
    if (!sHomeDir)
    {
        struct passwd* pwd = getpwuid(getuid());
        if (pwd)
            sHomeDir = pwd->pw_dir;
        else
            cerr << "getenv() failed followed by getpwuid() failing. Failed to find $HOME environment." << endl;
    }
#else
    // Must rename this file to .mm or make a new .mm file which does '#import "v4rtenc.cpp"'
    NSString* pNSHome = NSHomeDirectory();
    char sHomeDir[100];
    [pNSHome getCString:sHomeDir maxLength:99 encoding:NSASCIIStringEncoding];
#endif
    
    cout << "Default encoder settings:\n" << encSettings << endl;
    cout << "Home Directory: " << sHomeDir << endl;
    string sSettingsFilePath = sHomeDir;
    sSettingsFilePath += "/Library/Preferences/neuronlite_vrtencoder_settings.cfg";
    
    //Check if settings file is present
    if(access(sSettingsFilePath.c_str(), F_OK) == -1)
    {
        FILE* fpEncSettings = fopen(sSettingsFilePath.c_str(), "w");
        assert(fpEncSettings != NULL);
        fprintf(fpEncSettings, "%s", encSettings.c_str());
        fclose(fpEncSettings);
    }
    
    return sSettingsFilePath;
}

#endif
