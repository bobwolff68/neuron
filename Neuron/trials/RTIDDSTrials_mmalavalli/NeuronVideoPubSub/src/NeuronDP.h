/*
 * NeuronDP.h
 *
 *  Created on: Jul 30, 2010
 *      Author: manjesh
 */

#ifndef NEURONDP_H_
#define NEURONDP_H_

#define DEFAULT_DOMAIN_ID	0
#define CHECK_HANDLE(pHandle,errMsg) if(pHandle==NULL)\
                                       {\
                                           printf(errMsg);\
                                           exit(0);\
                                       }
#define CHECK_RETCODE(retCode,value,errMsg) if(retCode!=value)\
                                            {\
                                                printf(errMsg);\
                                                exit(0);\
                                            }

enum {
       TOPIC_FRAME = 0,
       N_TOPICS
     };

class DDSDPBuiltinListener : public DDSDataReaderListener
{
    public:
        virtual void    on_data_available   (DDSDataReader *pGenericReader);
};

class NeuronDP
{
	protected:
		const char			   *name;
		DDSDomainParticipant   *pDomainParticipant;
		DDSSubscriber		   *pBuiltinSub;
		DDSTopic			   *pTopic[N_TOPICS];

		void	startupDomainParticipant	(void);
		void	configParticipantDiscovery	(int fVidSrc,const char *vidStats);//Flag to inform base class that it is
																				//inherited by a video source
		void	registerAndCreateTopics		(void);
	public:
		NeuronDP(const char *nameParam,int fVidSrc,const char *vidStats);
		~NeuronDP();
};

#ifdef NEURONVS_H_
char	srcNameList[100][MAX_NAME_LEN];
char	srcVidStats[100][50];
int		srcNameListLen;
#else
extern char	srcNameList[100][MAX_NAME_LEN];
extern char	srcVidStats[100][50];
extern int	srcNameListLen;
//extern char DDS_Errors[13][50];
#endif

#endif /* NEURONDP_H_ */
