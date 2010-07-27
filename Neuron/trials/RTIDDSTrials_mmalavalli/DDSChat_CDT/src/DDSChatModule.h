#ifndef DDSCHATMODULE_H_INCLUDED
#define DDSCHATMODULE_H_INCLUDED

#define DEFAULT_DOMAIN_ID     0
#define MAX_CMDLINE_LEN		100

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

class ChatMsgListener : public DDSDataReaderListener
{
    public:
        virtual void    on_data_available   (DDSDataReader *pGenericReader);
};

class DDSChatModule
{
    private:
        // Properties
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
        void	sendChatMessage				(char *cliName,char *msgContent);
        void	performTask					(void);
        void    startup     				(void);

    public:
        // Constructor
        DDSChatModule(pCChar paramName) : name(paramName)
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

            startup();
        }

        // Destructor
        ~DDSChatModule();
};

#endif // DDSCHATMODULE_H_INCLUDED
