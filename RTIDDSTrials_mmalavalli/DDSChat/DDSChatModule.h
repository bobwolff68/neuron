#ifndef DDSCHATMODULE_H_INCLUDED
#define DDSCHATMODULE_H_INCLUDED

#define DEFAULT_DOMAIN_ID   0

typedef const char *pCChar;
enum {
       TOPIC_MSG        = 0,
       TOPIC_DISCOVERY,
       MAX_TOPICS
     };

class DDSDPBuiltinListener : public DDSDataReaderListener
{
    public:
        virtual void    on_data_available   (DDSDataReader *pGenericReader);
};

class DDSChatModule
{
    private:
        // Properties
        long    id;
        pCChar  name;
        // Domain participant and relevant topics
        DDSDomainParticipant    *pDomainParticipant;
        DDSTopic                *pTopic[MAX_TOPICS];
        // Publisher
        DDSPublisher    *pPub;
        DDSDataWriter   *pWriter[MAX_TOPICS];
        // Subscriber
        DDSSubscriber   *pSub;
        DDSSubscriber   *pBuiltinSub;
        DDSDataReader   *pReader[MAX_TOPICS];

        // Member functions not called by main()
        void    startupDomainParticipant    (int domainId);
        void    startupPublisher            (void);
        void    startupSubscriber           (void);

    public:
        // Constructor
        DDSChatModule(long paramId,pCChar paramName) : id(paramId),name(paramName)
        {
            pDomainParticipant = NULL;
            pBuiltinSub = NULL;
            pPub = NULL;
            pSub = NULL;

            for(int iTopic=0; iTopic<MAX_TOPICS; iTopic++)
            {
                pTopic[iTopic] = NULL;
                pWriter[iTopic] = NULL;
                pReader[iTopic] = NULL;
            }
        }

        // Destructor
        ~DDSChatModule();

        // Member functions
        void    startup     (void);
};

#endif // DDSCHATMODULE_H_INCLUDED
