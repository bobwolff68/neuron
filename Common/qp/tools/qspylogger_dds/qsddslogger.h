//!
//! \file qsddslogger.h
//!
//! \brief Declaration of the QSPY DDS Logger.
//! \author Manjesh Malavalli (mmalavalli@xvdth.com)
//!

#ifndef QSDDSLOGGER_H_
#define QSDDSLOGGER_H_ 

#include <string>
#include <stdint.h>
#include "ndds_cpp.h"
#include "qsddslog.h"
#include "qsddslogSupport.h"

#ifndef QS_BUF_LEN
#define QS_BUF_LEN  1024  
#endif

//!
//! \class QSDDSLogger
//!
//! \brief Class for logging QSPY records over DDS.
//!
class QSDDSLogger
{
    private:

        //!
        //! \var pTheInstance
        //!
        //! \brief Pointer to the singleton instance of QSDDSLogger.
        //!
        static QSDDSLogger *pTheInstance;

        //!
        //! \var qsBuffer
        //!
        //! \brief Circular buffer used to store QSPY logs.
        //!
        uint8_t qsBuffer[QS_BUF_LEN];

        //!
        //! \var domainId
        //!
        //! \brief Domain ID for logging QSPY records.
        //!
        const uint32_t domainId;

        //!
        //! \var uniqueTopicName
        //!
        //! \brief Name of the topic under which QSPY records are logged.
        //!
        const std::string uniqueTopicName;
        
        //!
        //! \var pParticipant
        //!
        //! \brief Pointer to the DDS domain participant instance that enables logging over DDS.
        //!
        DDSDomainParticipant* pParticipant;

        //!
        //! \var pTopic
        //!
        //! \brief Pointer to the DDS topic instance used to log QSPY records over DDS.
        //!
        DDSTopic* pTopic;

        //!
        //! \var pPublisher
        //!
        //! \brief Pointer to the DDS publisher instance that logs QSPY records over DDS.
        //!
        DDSPublisher* pPublisher;

        //!
        //! \var pWriter
        //!
        //! \brief Pointer to the DDS data writer instance that logs QSPY records over DDS.
        //!
        DDSDataWriter* pWriter;

        //!
        //! \var pLogItemInstance
        //!
        //! \brief Pointer to the log item structure that contains QSPY data to be logged over DDS.
        //!
        com::xvd::neuron::qslogger::LogItem* pLogItemInstance;

        //!
        //! \brief Private constructor to prevent out-of-scope instantiation.
        //!
        //! \param [in] domainIdP - Value for member variable 'domainId'.
        //! \param [in] uniqueTopicNameP - Value for member variable 'uniqueTopicName'.
        //!
        QSDDSLogger(const uint32_t domainIdP,const std::string &uniqueTopicNameP): 
        domainId(domainIdP),uniqueTopicName(uniqueTopicNameP),done(false)
        { }

        //!
        //! \brief Private destructor to prevent out-of-scope deletion.
        //!
        ~QSDDSLogger()
        { }

    public:

        //!
        //! \var done
        //!
        //! \brief Boolean variable to indicate whether logging process is finished or not.
        //!
        //! Details: Usually logging is done in a different thread than instantiation/destruction.
        //! So, this variable is a way for the instantiation thread to signal to the logging thread
        //! that logging is over so that it flushes out any remaining logging data before the
        //! singleton instance of the logger is deleted.
        //!
        bool done;

        //!
        //! \brief Static function to create the singleton instance of QSDDSLogger.
        //!
        //! \param [in] domainIdP - Value for member variable 'domainId'.
        //! \param [in] uniqueTopicNameP - Value for member variable 'uniqueTopicName'.
        //!
        //! \return VOID.
        //!
        static void CreateTheInstance(const uint32_t domainIdP,const std::string &uniqueTopicName);

        //!
        //! \brief Static function to access the singleton instance of QSDDSLogger.
        //!
        //! \return Pointer to the singleton instance of QSDDSLogger.
        //!
        static QSDDSLogger* TheInstance(void);

        //!
        //! \brief Static function to destroy the singleton instance of QSDDSLogger.
        //!
        //! \return VOID.
        //!
        static void DestroyTheInstance(void);

        //!
        //! \brief Function to start-up the DDS publishing functionality of the logger.
        //!
        //! \return 1 on success, 0 on failure.
        //!
        uint8_t DDSStartup(void);

        //!
        //! \brief Function to tear down the DDS publishing functionality of the logger.
        //!
        //! \return VOID.
        //!
        void DDSTeardown(void);
        
        //!
        //! \brief Function to log QSPY data over DDS.
        //!
        //! \param [in] pLogItemBuf - Buffer storing QSPY log data.
        //! \param [in] bytes - Size in bytes of the QSPY data.
        //!
        //! \return TRUE if successful, FALSE if not.
        //!
        bool Log(uint8_t* pLogItemBuf,const uint32_t bytes);
        
        //!
        //! \brief Function to access the QS ring buffer.
        //!
        //! \return Pointer to the QS ring buffer.
        //!
        uint8_t* QSBuffer(void)
        {
            return qsBuffer;
        }
};

#endif //QSDDSLOGGER_H_
