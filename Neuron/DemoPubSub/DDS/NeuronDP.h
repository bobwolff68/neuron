#include "NeuronDDS.h"
//RMW: for RTI - must hard-wire include this.
#ifdef RTI_STYLE
#include "NeuronDDSSupport.h"
#endif
//RMW: Needed?? #include "NeuronDDSPlugin.h"

#define	PUB_CHOICE	0
#define	SUB_CHOICE	1

#define PARTITION_NAME		"NeuronDDS"
#define CF_FRM_TOPIC_QUERY	"srcID = %0"

// Depricated - use -DRTI_STYLE on commandline instead.
//#define RTI_STYLE

#ifdef RTI_STYLE
#define DDS_boolean 			DDS_Boolean
#define DDS_long			DDS_Long
#define DDS_string_alloc 			DDS_String_alloc
#define DDS_WaitSet__alloc 		DDS_WaitSet_new
#define DDS_WaitSet_finalize		DDS_WaitSet_delete
#define DDS_ANY_STATUS		DDS_STATUS_MASK_ALL
#else
#define DDS_BOOLEAN_TRUE		TRUE
#define DDS_BOOLEAN_FALSE		FALSE
#define DDS_Topic_as_topicdescription(a)        (a)
#define DDS_QueryCondition_as_readcondition(q)	(q)
#endif
//RMW:#define NeuronDDS_SrcAdvertDataWriter long

typedef struct
{
#ifdef RTI_STYLE
	DDS_Publisher					*pub;
	DDS_Subscriber					*throtSub;
	
	NeuronDDS_SrcAdvertDataWriter	*srcAdvertWriter;
	DDS_DataWriter				*generic_srcAdvertWriter;
	
	NeuronDDS_FrameDataWriter		*frmWriter;
	DDS_DataWriter				*generic_frmWriter;
	
	NeuronDDS_ThrotMsgDataReader	*throtReader;
	DDS_DataReader				*generic_throtReader;
#else
	DDS_Publisher					pub;
	DDS_Subscriber					throtSub;
	NeuronDDS_SrcAdvertDataWriter	srcAdvertWriter;
	NeuronDDS_FrameDataWriter		frmWriter;
	NeuronDDS_ThrotMsgDataReader	throtReader;
#endif
	struct DDS_DataReaderListener	*throtMsgListener;
} NeuronPub;

typedef struct
{
        char                                   srcNameList[10][50];
        char                                   srcIdList[10][10];
        int                                    n_srcs;

#ifdef RTI_STYLE
	DDS_Subscriber					*sub;
	DDS_Publisher					*throtPub;
	
	NeuronDDS_SrcAdvertDataReader	*srcAdvertReader;
	DDS_DataReader				*generic_srcAdvertReader;

	NeuronDDS_FrameDataReader		*frmReader;
	DDS_DataReader				*generic_frmReader;

	NeuronDDS_ThrotMsgDataWriter	*throtWriter;
	DDS_DataWriter				*generic_throtWriter;
	
	DDS_ContentFilteredTopic		*specSrcFrmTopic;
#else
	DDS_Subscriber					sub;
	DDS_Publisher					throtPub;
	NeuronDDS_SrcAdvertDataReader	srcAdvertReader;
	NeuronDDS_FrameDataReader		frmReader;
	NeuronDDS_ThrotMsgDataWriter	throtWriter;
	DDS_ContentFilteredTopic		specSrcFrmTopic;
#endif
} NeuronSub;

typedef struct
{
	long	dp_id;
	
#ifdef RTI_STYLE
	DDS_DomainParticipantFactory	*dpf;
	DDS_DomainParticipant			*dp;
	DDS_Topic				*srcAdTopic;
	DDS_Topic				*frmTopic;
	DDS_Topic				*throtMsgTopic;
#else
	DDS_DomainParticipantFactory	dpf;
	DDS_DomainParticipant			dp;
	DDS_Topic						srcAdTopic;
	DDS_Topic						frmTopic;
	DDS_Topic						throtMsgTopic;
#endif
	NeuronPub						npub;
	NeuronSub						nsub;
} NeuronDP;

typedef struct
{
	NeuronDP	*pNdp;
	int			*p_mux_throttle_mode;
	char		*sink_id_string;
	char		*take_query;
} ThrotMsgListenerData;

void	NeuronPub_setup( NeuronDP *, const char *, char * );
void	NeuronPub_setup_throtmsg_sub( NeuronDP *, const char * );
void	NeuronPub_setup_throt_msg_listener( NeuronDP *, ThrotMsgListenerData * );
void	NeuronPub_destroy( NeuronDP * );
void	NeuronPub_destroy_throtmsg_sub( NeuronDP * );
void	NeuronPub_write_srcadvert( NeuronDP *, long, char * );
void	NeuronPub_write_frame( NeuronDP *, unsigned char *, long, long );

void	NeuronSub_setup( NeuronDP *, const char *, char * );
void    NeuronSub_setup_cftopic_and_reader( NeuronDP *, char * );
void	NeuronSub_setup_throtmsg_pub( NeuronDP *, const char * );
void	NeuronSub_destroy( NeuronDP * );
void	NeuronSub_destroy_throtmsg_pub( NeuronDP * );
void    NeuronSub_destroy_cftopic_and_reader( NeuronDP * );
void	NeuronSub_read_srcadverts( NeuronDP * );
void	NeuronSub_read_frame( NeuronDP *, unsigned char **, int *, int *, int *, const char *, char [][10] );
void	NeuronSub_write_throtmsg( NeuronDP *, char );

void	NeuronDP_create_dp_factory( NeuronDP * );
void	NeuronDP_setup( NeuronDP *, int, char * );
void	NeuronDP_destroy( NeuronDP *, int );

