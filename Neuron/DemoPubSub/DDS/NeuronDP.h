#include "NeuronDDS.h"

#define	PUB_CHOICE	0
#define	SUB_CHOICE	1

#define PARTITION_NAME		"NeuronDDS"
#define CF_FRM_TOPIC_QUERY	"srcID = %0"

typedef struct
{
	DDS_Publisher					pub;
	DDS_Subscriber					throtSub;
	NeuronDDS_SrcAdvertDataWriter	srcAdvertWriter;
	NeuronDDS_FrameDataWriter		frmWriter;
	NeuronDDS_ThrotMsgDataReader	throtReader;
	struct DDS_DataReaderListener	*throtMsgListener;
} NeuronPub;

typedef struct
{
	char							srcNameList[10][50];
	char							srcIdList[10][10];
	int								n_srcs;
	DDS_Subscriber					sub;
	DDS_Publisher					throtPub;
	DDS_ContentFilteredTopic		specSrcFrmTopic;
	NeuronDDS_SrcAdvertDataReader	srcAdvertReader;
	NeuronDDS_FrameDataReader		frmReader;
	NeuronDDS_ThrotMsgDataWriter	throtWriter;
} NeuronSub;

typedef struct
{
	long	dp_id;
	
	DDS_DomainParticipantFactory	dpf;
	DDS_DomainParticipant			dp;
	DDS_Topic						srcAdTopic;
	DDS_Topic						frmTopic;
	DDS_Topic						throtMsgTopic;
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
void	NeuronPub_write_frame( NeuronDP *, char *, long, long );

void	NeuronSub_setup( NeuronDP *, const char *, char * );
void	NeuronSub_setup_cftopic_and_reader( NeuronDP *, char * );
void	NeuronSub_setup_throtmsg_pub( NeuronDP *, const char * );
void	NeuronSub_destroy( NeuronDP * );
void	NeuronSub_destroy_throtmsg_pub( NeuronDP * );
void	NeuronSub_destroy_cftopic_and_reader( NeuronDP * );
void	NeuronSub_read_srcadverts( NeuronDP * );
void	NeuronSub_read_frame( NeuronDP *, char **, int *, int *, int *, const char *, char [][10] );
void	NeuronSub_write_throtmsg( NeuronDP *, char );

void	NeuronDP_create_dp_factory( NeuronDP * );
void	NeuronDP_setup( NeuronDP *, int, char * );
void	NeuronDP_destroy( NeuronDP *, int );

