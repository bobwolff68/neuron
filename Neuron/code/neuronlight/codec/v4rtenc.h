//!
//! \file v4rtenc.h
//! \brief Real-time encoder definition
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!
#ifndef V4RTENC_H_
#define V4RTENC_H_

#include <map>
#include <string>

#if (defined (__APPLE__) & defined (__MACH__))
    #include <RTBuffer.h>
    
/********** AAC AUDIO STREAM **********/
    extern "C" 
    {
        #include <libavcodec/avcodec.h>
        #include <libavformat/avformat.h>
    }
    #include <SafeBufferDeque.h>
/**************************************/
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <fcntl.h>
#else
    #include <V4L2Cap.h>
#endif

#include <v4e_api.h>
#include <vp.h>
#include <ThreadSingle.h>

//#define BUILD_DEBUG

#define LOG_ERR(msg)    cerr << __FILE__ << "|"\
                             << __LINE__ << "|"\
                             << __func__ << "> "\
                             << msg << endl

#ifdef BUILD_DEBUG
    #define LOG_OUT(msg)    cout << __func__ << "> "\
                                 << msg << endl
#else
    #define LOG_OUT(msg)    ((void)(msg))
#endif

#ifndef MANJESH_TIMESTAMPLOG_H_
//#define LOG_TIMESTAMPS
#ifdef LOG_TIMESTAMPS
#include <sys/time.h>

class TimestampsLog
{
private:
    int log_fd;
public:
    TimestampsLog(const char* log_file_name);
    ~TimestampsLog();
    void WriteEntry(struct timeval* p_tod,const int frm_num,const int64_t apple_ts,const int64_t rtp_ts);
};
#else
class TimestampsLog
{
public:
    TimestampsLog(const char* log_file_name)
    {
        (void)log_file_name;
    }
    ~TimestampsLog()
    {
        
    }
    void WriteEntry(struct timeval* p_tod,const int frm_num,const int64_t apple_ts,const int64_t rtp_ts)
    {
        (void)p_tod;
        (void)frm_num;
        (void)apple_ts;
        (void)rtp_ts;
    }
};
#endif
#endif

enum RTEnc_ReturnCode_t
{
    RTENC_RETCODE_OK = 0,
    RTENC_RETCODE_ERR_DEFAULT_SETTINGS,
    RTENC_RETCODE_ERR_READ_CFG_FILE,
    RTENC_RETCODE_ERR_OPEN,
    RTENC_RETCODE_ERR_CLOSE,
    RTENC_RETCODE_ERR_AOPEN,
    RTENC_RETCODE_ERR_ACLOSE,
    RTENC_RETCODE_ERR_GET_SETTINGS,
    RTENC_RETCODE_ERR_WRITE_CFG_FILE,
    RTENC_RETCODE_ERR_LOCK_HANDLE,
    RTENC_RETCODE_ERR_UNLOCK_HANDLE
};

class v4_rtenc_t: public ThreadSingle
{
    private:

        const bool b_video_on;
        const bool b_audio_on;
        TimestampsLog* p_tslog;
    
#if (defined (__APPLE__) & defined (__MACH__))
        QTKitCapBuffer* p_rtcap_buf;
        // audio codec
        AVCodec* p_acodec;
        AVCodecContext* p_acctx;
        AVDictionary* p_opts_dict;
        SafeBufferDeque* p_abs_dq;
        
#else
        V4L2CapBuffer* p_rtcap_buf;
#endif
    
        void* p_handle;
        pthread_mutex_t handle_mutex;
        v4e_settings_t settings;
        vp_raw_frame_t frame;
        virtual int workerBee(void);
        void InitRawFrameSettings(void);
        void SetRawFrameBuffers(unsigned char* p_frame_buf, int stride);
        std::string PrepareEncoderSettings(void);
        
    public:

#if (defined (__APPLE__) & defined (__MACH__))
        v4_rtenc_t(QTKitCapBuffer* _p_rtcap_buf,
                   SafeBufferDeque* _p_abs_dq,
                   const bool _b_video_on,
                   const bool _b_audio_on);
        RTEnc_ReturnCode_t OpenAudio(void);
        RTEnc_ReturnCode_t CloseAudio(void);
#else
        v4_rtenc_t(const char* cfg_file,V4L2CapBuffer* _p_rtcap_buf);
#endif
    
        virtual ~v4_rtenc_t();
        void SetEncSettings(std::map<std::string,std::string>& nvpairs);
        void* Handle(void) const;
        RTEnc_ReturnCode_t Open(void);
        RTEnc_ReturnCode_t Close(void);
        RTEnc_ReturnCode_t LockHandle(void);
        RTEnc_ReturnCode_t UnlockHandle(void);
        void ChangeTargetBitrate(int newBitrateKbps);
        void ChangeTargetFrameRate(int newFrameRateFps);
};

#endif