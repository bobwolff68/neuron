#ifndef QSDDSLOGREADER_H_
#define QSDDSLOGREADER_H_ 

#include <string>
#include <stdint.h>
#include "ndds_cpp.h"
#include "qsddslog.h"
#include "qsddslogSupport.h"

class QSDDSLogReaderListener : public DDSDataReaderListener
{
    private:
        //QSPY parser input pipe
        int32_t qspyPipeSink;

    public:
        QSDDSLogReaderListener(int32_t qspyPipeSinkP);
        ~QSDDSLogReaderListener();
        void on_data_available(DDSDataReader* pReader);
};

class QSDDSLogReader
{
    private:
        //Singleton instance of QSDDSLogReader.
        static QSDDSLogReader *pTheInstance;

        //Private members of singleton instance.
        int32_t             qspyPipe[2];
        const uint32_t      domainId;
        const std::string   uniqueTopicName;
        
        //DDS members
        DDSDomainParticipant*   pParticipant;
        DDSTopic*               pTopic;
        DDSSubscriber*          pSubscriber;
        DDSDataReader*          pReader;

        //Private constructor to control instantiation.
        QSDDSLogReader(const uint32_t domainIdP,const std::string& uniqueTopicNameP); 
        ~QSDDSLogReader();

    public:
        static void CreateTheInstance(const uint32_t domainIdP,const char* uniqueTopicNameP);
        static QSDDSLogReader* TheInstance(void);
        static void DestroyTheInstance(void);

        //Public member functions
        uint8_t DDSStartup(void);
        void DDSTeardown(void);
        int32_t QspyPipeSource(void);
        bool QSTargetDone(void);
};

#endif //QSDDSLOGREADER_H_
