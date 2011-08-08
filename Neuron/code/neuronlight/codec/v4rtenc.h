//!
//! \file v4rtenc.h
//! \brief Real-time encoder definition
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!
#ifndef V4RTENC_H_
#define V4RTENC_H_

#include <map>
#include <string>
#include "V4L2Cap.h"
#include "v4e_api.h"
#include "vp.h"
#include "ThreadSingle.h"

#define LOG_ERR(msg)    cerr << __FILE__ << "|"\
                             << __LINE__ << "|"\
                             << __func__ << "> "\
                             << msg << endl

#ifdef BUILD_DEBUG
    #define LOG_OUT(msg)    cout << __func__ << "> "\
                                 << msg << endl
#else
    #define LOG_OUT(msg)    ((void*)0)
#endif

//!
//! \enum RTEnc_ReturnCode_t
//! \brief Return codes of v4_rtenc_t member functions
//!
enum RTEnc_ReturnCode_t
{
    RTENC_RETCODE_OK = 0,
    RTENC_RETCODE_ERR_DEFAULT_SETTINGS,
    RTENC_RETCODE_ERR_READ_CFG_FILE,
    RTENC_RETCODE_ERR_OPEN,
    RTENC_RETCODE_ERR_CLOSE,
    RTENC_RETCODE_ERR_GET_SETTINGS,
    RTENC_RETCODE_ERR_WRITE_CFG_FILE,
    RTENC_RETCODE_ERR_LOCK_HANDLE,
    RTENC_RETCODE_ERR_UNLOCK_HANDLE
};

//!
//! \class v4_rtenc_t
//! \brief Real-time encoder class
//!
class v4_rtenc_t: public ThreadSingle
{
    private:

        //!
        //! \var p_rtcap_buf
        //! \brief Pointer to frame capture buffer
        //!
        V4L2CapBuffer* p_rtcap_buf;

        //!
        //! \var p_handle
        //! \brief Pointer to vsofts H.264 encoder handle
        //!
        void* p_handle;

        pthread_mutex_t handle_mutex;
        
        //!
        //! \var settings
        //! \brief Vsofts H.264 encoder handle settings
        //!
        v4e_settings_t settings;

        //!
        //! \var frame
        //! \brief Raw video frame structure
        //!
        vp_raw_frame_t frame;

        //!
        //! \brief Main worker routine (internal use only)
        //! \return 0 if successful, -1 if not
        //!
        virtual int workerBee(void);

        //!
        //! \brief Function to initialize raw frame settings
        //! \return void
        //!
        void InitRawFrameSettings(void);

        //!
        //! \brief Function to set y, u and v buffers of frame
        //! \param[in] p_frame_buf - Pointer to the capture buffer
        //! \return void
        //!
        void SetRawFrameBuffers(void* p_frame_buf);

    public:

        //!
        //! \brief Constructor
        //! \param[in] cfg_file - Path-appended name of the encoder config file
        //! \param[in] _p_rtcap_buf - Pointer to the real-time capture buffer
        //!
        v4_rtenc_t(const char* cfg_file,V4L2CapBuffer* _p_rtcap_buf);

        //!
        //! \brief Destructor
        //!
        virtual ~v4_rtenc_t();

        //!
        //! \brief Function to set encoder settings
        //! \param[in] nvpairs - Name value pairs of encoder settings
        //! \return void
        //!
        void SetEncSettings(std::map<std::string,std::string>& nvpairs);
        
        //!
        //!
        void* Handle(void) const;
        RTEnc_ReturnCode_t Open(void);
        RTEnc_ReturnCode_t Close(void);
        RTEnc_ReturnCode_t LockHandle(void);
        RTEnc_ReturnCode_t UnlockHandle(void);
};

#endif
