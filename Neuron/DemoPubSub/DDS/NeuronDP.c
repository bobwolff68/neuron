#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "dds_dcps.h"
#include "NeuronDP.h"
#include "CheckStatus.h"
#include "crc.h"

DDS_InstanceHandle_t	frmMsgInstHdl;
NeuronDDS_Frame			frmMsg;

void NeuronPub_setup( NeuronDP *pNdp, const char *partition_name, char *name )
{
	DDS_ReturnCode_t	status;
	DDS_PublisherQos	*pubQos;

	// Adapt Publisher QOS to write to partition 'partition_name'
	printf( "Creating Publisher QOS Object..." );
	pubQos = DDS_PublisherQos__alloc();
	checkHandle( pubQos, "DDS_PublisherQos__alloc()" );
	printf( "Successful\nObtaining Default Publisher QOS Settings..." );
	status = DDS_DomainParticipant_get_default_publisher_qos( pNdp->dp, pubQos );
	checkStatus( status, "DDS_DomainParticipant_get_default_publisher_qos()" );
	printf( "Successful\nSetting Partition to %s...", partition_name );
	pubQos->partition.name._length = 1;
	pubQos->partition.name._maximum = 1;	
	pubQos->partition.name._buffer = DDS_StringSeq_allocbuf( 1 );
	checkHandle( pubQos->partition.name._buffer, "DDS_StringSeq_allocbuf()" );
	pubQos->partition.name._buffer[0] = DDS_string_alloc( strlen(partition_name) );
	checkHandle( pubQos->partition.name._buffer[0], "DDS_string_alloc()" );
	strcpy( pubQos->partition.name._buffer[0], partition_name );
	
	// Create Publisher
	printf( "Successful\nCreating Publisher Object..." );
	pNdp->npub.pub = DDS_DomainParticipant_create_publisher( 
															 pNdp->dp, pubQos, NULL, 
															 DDS_ANY_STATUS 
														   );
	checkHandle( pNdp->npub.pub, "DDS_DomainParticipant_create_publisher()" );
	
	// Create Data Writer for Frame Topic
	printf( "Successful\nCreating Data Writer for Frame Topic..." );
	pNdp->npub.frmWriter = DDS_Publisher_create_datawriter( 
															pNdp->npub.pub, pNdp->frmTopic,
															DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
															NULL, DDS_ANY_STATUS
														  ); 
	checkHandle( pNdp->npub.frmWriter, "DDS_Publisher_create_datawriter(FrameTopic)" );
	
	// Create Data Writer for SrcAdvert Topic
	printf( "Successful\nCreating Data Writer for SrcAdvert Topic..." );
	pNdp->npub.srcAdvertWriter = DDS_Publisher_create_datawriter( 
																  pNdp->npub.pub, 
																  pNdp->srcAdTopic,
																  DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
																  NULL, DDS_ANY_STATUS
														  		); 
	checkHandle( pNdp->npub.srcAdvertWriter, "DDS_Publisher_create_datawriter(SrcAdTopic)" );
	
	printf( "Successful\n" );
	NeuronPub_setup_throtmsg_sub( pNdp, partition_name );
	NeuronPub_write_srcadvert( pNdp, pNdp->dp_id, name );

	frmMsg.index = 0;
	
	// Deallocate resources
	DDS_free( pubQos );
	
	return; 
}

void NeuronPub_setup_throtmsg_sub( NeuronDP *pNdp, const char *partition_name )
{
	DDS_ReturnCode_t	status;
	DDS_SubscriberQos	*subQos;
	
	// Adapt Subscriber QOS to read from partition 'partition_name'
	printf( "Creating Subscriber QOS Object..." );
	subQos = DDS_SubscriberQos__alloc();
	checkHandle( subQos, "DDS_SubscriberQos__alloc()" );
	printf( "Successful\nObtaining Default Subscriber QOS Settings..." );
	status = DDS_DomainParticipant_get_default_subscriber_qos( pNdp->dp, subQos );
	checkStatus( status, "DDS_DomainParticipant_get_default_subscriber_qos()" );
	printf( "Successful\nSetting Partition to %s...", partition_name );
	subQos->partition.name._length = 1;
	subQos->partition.name._maximum = 1;	
	subQos->partition.name._buffer = DDS_StringSeq_allocbuf( 1 );
	checkHandle( subQos->partition.name._buffer, "DDS_StringSeq_allocbuf()" );
	subQos->partition.name._buffer[0] = DDS_string_alloc( strlen(partition_name) );
	checkHandle( subQos->partition.name._buffer[0], "DDS_string_alloc()" );
	strcpy( subQos->partition.name._buffer[0], partition_name );
	
	// Create Subscriber
	printf( "Successful\nCreating Subscriber Object..." );
	pNdp->npub.throtSub = DDS_DomainParticipant_create_subscriber( 
															  	   pNdp->dp, subQos, NULL, 
															  	   DDS_ANY_STATUS 
														    	 );
	checkHandle( pNdp->npub.throtSub, "DDS_DomainParticipant_create_subscriber()" );
	
	// Create Data Reader for ThrotMsg Topic
	printf( "Successful\nCreating Data Reader for ThrotMsg Topic..." );
	pNdp->npub.throtReader = DDS_Subscriber_create_datareader( 
															   pNdp->npub.throtSub, 
															   pNdp->throtMsgTopic,
															   DDS_DATAREADER_QOS_USE_TOPIC_QOS,
															   NULL, DDS_ANY_STATUS
														     ); 
	checkHandle( pNdp->npub.throtReader, "DDS_Subscriber_create_datareader(ThrotMsgTopic)" );

	// De-allocate resources
	DDS_free( subQos );
	
	return;
}

// On Data Available Callback
void on_throt_msg_available( void *listener_data, DDS_DataReader reader )
{	
	DDS_boolean						queryCondDelete = FALSE;
	DDS_sequence_NeuronDDS_ThrotMsg	*throtMsgSeq = NULL;
	DDS_SampleInfoSeq				*smpInfoSeq = NULL;
	DDS_StringSeq					*queryParams = NULL;
	NeuronDDS_ThrotMsg 				*throtMsg = NULL;
	DDS_ReturnCode_t				status;
	DDS_QueryCondition				queryCond;
	ThrotMsgListenerData			*ldata = (ThrotMsgListenerData *) listener_data;

	// Allocate memory for ThrotMsgSeq and smpInfoSeq
	throtMsgSeq = DDS_sequence_NeuronDDS_ThrotMsg__alloc();
	checkHandle( throtMsgSeq, "DDS_sequence_NeuronDDS_ThrotMsg__alloc()" );
	smpInfoSeq = DDS_SampleInfoSeq__alloc();
	checkHandle( smpInfoSeq, "DDS_SampleInfoSeq__alloc()" );
	
	if( strcmp( ldata->sink_id_string, "-1" ) )
	{
		// Create Query Condition to extract throttle msgs from sink having id sink_id
		queryParams = DDS_StringSeq__alloc();
		checkHandle( queryParams, "DDS_StringSeq__alloc(queryParams)" );
		queryParams->_length = queryParams->_maximum = 1;
		queryParams->_buffer = DDS_StringSeq_allocbuf( 1 );
		checkHandle( queryParams->_buffer, "DDS_StringSeq_allocbuf(queryParams)" );
		queryParams->_buffer[0] = DDS_string_alloc( strlen(ldata->sink_id_string)+1 );
		checkHandle( queryParams->_buffer[0], "DDS_string__alloc(queryParams->buffer[0])" );
		strcpy( queryParams->_buffer[0], ldata->sink_id_string );
		queryCond = DDS_DataReader_create_querycondition( 
														  ldata->pNdp->npub.throtReader, 
														  DDS_NOT_READ_SAMPLE_STATE,
														  DDS_ANY_VIEW_STATE,
														  DDS_ALIVE_INSTANCE_STATE,
														  ldata->take_query,
														  queryParams
														);
		checkHandle( queryCond, "DDS_DataReader_create_querycondition()" );
	
		status = NeuronDDS_ThrotMsgDataReader_take_w_condition( 
												 				ldata->pNdp->npub.throtReader, 
												 				throtMsgSeq, smpInfoSeq, 
												 				1, queryCond
											   				  );
		checkStatus( status, "NeuronDDS_FrameDataReader_take_w_condition()" );
		DDS_free( queryParams );
		queryCondDelete = TRUE;
	}
	else
	{
		status = NeuronDDS_ThrotMsgDataReader_take( 
												 	ldata->pNdp->npub.throtReader, throtMsgSeq, 
												 	smpInfoSeq, 1, DDS_NOT_READ_SAMPLE_STATE,
												 	DDS_ANY_VIEW_STATE, DDS_ALIVE_INSTANCE_STATE
											   	  );
		checkStatus( status, "NeuronDDS_FrameDataReader_take()" );
	}
	
	// If throttle msg available, set new throttle mode
	if( status != DDS_RETCODE_NO_DATA && smpInfoSeq->_buffer[0].valid_data )		
	{
		throtMsg = &(throtMsgSeq->_buffer[0]);
		//printf( "New throttle: %d\n", *(ldata->p_mux_throttle_mode) );
		if( !strcmp( ldata->sink_id_string, "-1" ) )
		{
			if( ldata->pNdp->dp_id==(throtMsg->srcID & 255) )
			{
				*(ldata->p_mux_throttle_mode) = (int) throtMsg->mode;
				sprintf( ldata->sink_id_string, "%d", (int) (throtMsg->srcID >> 8) );
				printf( "Transmission request from Client (ID: %s)...\n", ldata->sink_id_string );
			}
		}
		else
			*(ldata->p_mux_throttle_mode) = (int) throtMsg->mode;
	}
	// Sequence memory is borrowed from DDS, so return the loaned memory
	status = NeuronDDS_ThrotMsgDataReader_return_loan( 
													   ldata->pNdp->npub.throtReader, 
													   throtMsgSeq, smpInfoSeq 
													 );
	checkStatus( status, "NeuronDDS_ThrotMsgDataReader_return_loan()" );
	if( queryCondDelete )
	{
		status = DDS_DataReader_delete_readcondition( ldata->pNdp->npub.throtReader, queryCond );
		checkStatus( status, "DDS_DataReader_delete_readcondition(ThrotMsg)" );
	}	

	return;	
}

void NeuronPub_setup_throt_msg_listener( NeuronDP *pNdp, ThrotMsgListenerData *listenerData )
{
	DDS_ReturnCode_t	status;
	
	// Allocate memory for listener
	printf( "Creating ThrotMsg Reader Listener..." );
	pNdp->npub.throtMsgListener = DDS_DataReaderListener__alloc();
	checkHandle( pNdp->npub.throtMsgListener, "DDS_DataReaderListener__alloc()" );
	
	printf( "Successful\n" );
	
	// Set listener data pointer
	pNdp->npub.throtMsgListener->listener_data = (void *) listenerData;
	
	// Assign callback function pointer for 'on data available' event
	// Set other pointers to NULL
	pNdp->npub.throtMsgListener->on_data_available = on_throt_msg_available;
	pNdp->npub.throtMsgListener->on_requested_deadline_missed = NULL;
	pNdp->npub.throtMsgListener->on_requested_incompatible_qos = NULL;
	pNdp->npub.throtMsgListener->on_sample_rejected = NULL;
	pNdp->npub.throtMsgListener->on_liveliness_changed = NULL;
	pNdp->npub.throtMsgListener->on_subscription_matched = NULL;
	pNdp->npub.throtMsgListener->on_sample_lost = NULL;
	
	// Attach listener to throttle msg reader
	status = DDS_DataReader_set_listener( 
										  pNdp->npub.throtReader, pNdp->npub.throtMsgListener,
										  DDS_DATA_AVAILABLE_STATUS
										);
	checkStatus( status, "DDS_DataReader_set_listener()" );
	
	return;
}

void NeuronPub_destroy( NeuronDP *pNdp )
{
	DDS_ReturnCode_t	status;

	// Dispose and unregister Frame Topic instance
	printf( "Disposing Instance for Frame Topic for id: %ld...", pNdp->dp_id );
	status = NeuronDDS_FrameDataWriter_dispose( pNdp->npub.frmWriter, &frmMsg, frmMsgInstHdl );
	checkStatus( status, "NeuronDDS_FrameDataWriter_dispose()" );
	printf( "Successful\nUnregistering Instance for Frame Topic for id: %ld...", pNdp->dp_id );
	status = NeuronDDS_FrameDataWriter_unregister_instance( 
															pNdp->npub.frmWriter, &frmMsg, 
															frmMsgInstHdl
														  );
	checkStatus( status, "NeuronDDS_FrameDataWriter_unregister_instance()" );
	printf( "Successful\n" );
	
	// Delete ThrotMsg Publisher
	NeuronPub_destroy_throtmsg_sub(	pNdp );
	
	// Delete Data Writer for SrcAdvert Topic
	printf( "Deleting Data Writer for SrcAdvert Topic..." );
	status = DDS_Publisher_delete_datawriter( pNdp->npub.pub, pNdp->npub.srcAdvertWriter );
	checkStatus( status, "DDS_Publisher_delete_datawriter(SrcAdTopic)" );
	
	// Delete Data Writer for Frame Topic
	printf( "Successful\nDeleting Data Writer for Frame Topic..." );
	status = DDS_Publisher_delete_datawriter( pNdp->npub.pub, pNdp->npub.frmWriter );
	checkStatus( status, "DDS_Publisher_delete_datawriter(FrameTopic)" );
	
	// Delete the Publisher
	printf( "Successful\nDeleting the Publisher Object..." );
	status = DDS_DomainParticipant_delete_publisher( pNdp->dp, pNdp->npub.pub );
	checkStatus( status, "DDS_DomainParticipant_delete_publisher()" );
	printf( "Successful\n" );

	return;
}

void NeuronPub_destroy_throtmsg_sub( NeuronDP *pNdp )
{
	DDS_ReturnCode_t	status;
	
	DDS_free( pNdp->npub.throtMsgListener );
	// Delete Data Reader for ThrotMsg Topic
	printf( "Deleting Data Reader for ThrotMsg Topic..." );
	status = DDS_Subscriber_delete_datareader( pNdp->npub.throtSub, pNdp->npub.throtReader );
	checkStatus( status, "DDS_Subscriber_delete_datareader(ThrotMsg)" );
	
	// Delete the Subscriber
	printf( "Successful\nDeleting the ThrotMsg Subscriber Object..." );
	status = DDS_DomainParticipant_delete_subscriber( pNdp->dp, pNdp->npub.throtSub );
	checkStatus( status, "DDS_DomainParticipant_delete_subscriber(ThrotMsg)" );
	printf( "Successful\n" );
	
	return;
}

void NeuronPub_write_srcadvert( NeuronDP *pNdp, long src_id, char *src_name )
{
	DDS_ReturnCode_t	status;
	NeuronDDS_SrcAdvert	srcAdMsg;
	
	srcAdMsg.srcID = src_id;
	printf( "Writing Src Advert Message: (id=%lu,name=%s)...", src_id, src_name );
	srcAdMsg.srcName = DDS_string_alloc( NeuronDDS_MAX_NAMELEN+1 );
	checkHandle( srcAdMsg.srcName, "DDS_string_alloc()" );
	strncpy( srcAdMsg.srcName, src_name, NeuronDDS_MAX_NAMELEN+1 );
	status = NeuronDDS_SrcAdvertDataWriter_write( 
												  pNdp->npub.srcAdvertWriter, 
												  &srcAdMsg, DDS_HANDLE_NIL 
												);
	checkStatus( status, "NeuronDDS_SrcAdvertDataWriter_write()" );
	printf( "Successful\n" );
	
	// Deallocate Resources
	DDS_free( srcAdMsg.srcName );
	
	return;
}

void NeuronPub_write_frame( NeuronDP *pNdp, char *frm_buf, long frm_size, long frm_layer_type )
{
	int				i;
	unsigned short	crc_chksum;

	DDS_ReturnCode_t	status;
	
	//crcInit();
	
	//Assign the appropriate values to the constant fields of the Frame Message.
	frmMsg.srcID = pNdp->dp_id;
	frmMsg.size = (DDS_unsigned_long) frm_size;
	frmMsg.layerType = frm_layer_type;
	frmMsg.payload._length = frmMsg.payload._maximum = frmMsg.size;
	DDS_sequence_set_release( (void *) &(frmMsg.payload), FALSE );
	frmMsg.payload._buffer = frm_buf;

	//crc_chksum = crcFast( frmMsg.payload._buffer, frm_size );
		
	if( frmMsg.index==0 )
	{
		frmMsgInstHdl = NeuronDDS_FrameDataWriter_register_instance(  
																	 pNdp->npub.frmWriter,
																	 &frmMsg
																   );
	}
	
	status = NeuronDDS_FrameDataWriter_write( pNdp->npub.frmWriter, &frmMsg, frmMsgInstHdl );
	checkStatus( status, "NeuronDDS_FrameDataWriter_write()" );	
	//printf( "Index: %ld, Layer Type: %ld, Length: %ld\n", frmMsg.index, frmMsg.layerType, 
	//													   frmMsg.payload._length );	
	frmMsg.index++;
	
	return;
}

void NeuronSub_setup( NeuronDP *pNdp, const char *partition_name, char *src_id_str )
{
	DDS_ReturnCode_t	status;
	DDS_SubscriberQos	*subQos;

	// Adapt Subscriber QOS to read from partition 'partition_name'
	printf( "Creating Subscriber QOS Object..." );
	subQos = DDS_SubscriberQos__alloc();
	checkHandle( subQos, "DDS_SubscriberQos__alloc()" );
	printf( "Successful\nObtaining Default Subscriber QOS Settings..." );
	status = DDS_DomainParticipant_get_default_subscriber_qos( pNdp->dp, subQos );
	checkStatus( status, "DDS_DomainParticipant_get_default_subscriber_qos()" );
	printf( "Successful\nSetting Partition to %s...", partition_name );
	subQos->partition.name._length = 1;
	subQos->partition.name._maximum = 1;	
	subQos->partition.name._buffer = DDS_StringSeq_allocbuf( 1 );
	checkHandle( subQos->partition.name._buffer, "DDS_StringSeq_allocbuf()" );
	subQos->partition.name._buffer[0] = DDS_string_alloc( strlen(partition_name) );
	checkHandle( subQos->partition.name._buffer[0], "DDS_string_alloc()" );
	strcpy( subQos->partition.name._buffer[0], partition_name );
	
	// Create Subscriber
	printf( "Successful\nCreating Subscriber Object..." );
	pNdp->nsub.sub = DDS_DomainParticipant_create_subscriber( 
															  pNdp->dp, subQos, NULL, 
															  DDS_ANY_STATUS 
														    );
	checkHandle( pNdp->nsub.sub, "DDS_DomainParticipant_create_subscriber()" );
	
	// Create Data Reader for SrcAdvert Topic
	printf( "Successful\nCreating Data Reader for SrcAdvert Topic..." );
	pNdp->nsub.srcAdvertReader = DDS_Subscriber_create_datareader( 
																   pNdp->nsub.sub, 
																   pNdp->srcAdTopic,
																   DDS_DATAREADER_QOS_USE_TOPIC_QOS,
																   NULL, DDS_ANY_STATUS
														  		 ); 
	checkHandle( pNdp->nsub.srcAdvertReader, "DDS_Subscriber_create_datareader(SrcAdTopic)" );
	printf( "Successful\n" );
	NeuronSub_setup_throtmsg_pub( pNdp, partition_name );
	pNdp->nsub.n_srcs = 0;

	// Deallocate resources
	DDS_free( subQos );
	
	return;
}

void NeuronSub_setup_cftopic_and_reader( NeuronDP *pNdp, char *src_id_str )
{
	DDS_ReturnCode_t	status;
	DDS_StringSeq		*cfTopicParams;
	
	// Create Content Filtered Topic for receiving frames from a particular source
	printf( "Successful\nCreating Content Filtered Frame Topic..." );
	cfTopicParams = DDS_StringSeq__alloc();
	checkHandle( cfTopicParams, "DDS_StringSeq__alloc(cfTopicParams)" );
	cfTopicParams->_length = cfTopicParams->_maximum = 1;
	cfTopicParams->_buffer = DDS_StringSeq_allocbuf( 1 );
	checkHandle( cfTopicParams->_buffer, "DDS_StringSeq_allocbuf(cfTopicParams)" );
	cfTopicParams->_buffer[0] = DDS_string_alloc( strlen(src_id_str)+1 );
	checkHandle( cfTopicParams->_buffer[0], "DDS_string_alloc(cfTopicParams)" );
	strcpy( cfTopicParams->_buffer[0], src_id_str );
	pNdp->nsub.specSrcFrmTopic = DDS_DomainParticipant_create_contentfilteredtopic(
																				pNdp->dp,
																				"NeuronDDS_SSF",
																				pNdp->frmTopic,
																				CF_FRM_TOPIC_QUERY,
																				cfTopicParams
																				  );
	checkHandle( 
				 pNdp->nsub.specSrcFrmTopic, 
				 "DDS_DomainParticipant_create_contentfilteredtopic()"
			   );

	// Create Data Reader for Frame Topic
	printf( "Successful\nCreating Data Reader for Content Filtered Frame Topic..." );
	pNdp->nsub.frmReader = DDS_Subscriber_create_datareader( 
															 pNdp->nsub.sub, 
															 pNdp->nsub.specSrcFrmTopic,
															 DDS_DATAREADER_QOS_USE_TOPIC_QOS,
															 NULL, DDS_ANY_STATUS
														   ); 
	checkHandle( pNdp->nsub.frmReader, "DDS_Subscriber_create_datareader(Content FrameTopic)" );
	
	// Deallocate resources
	DDS_free( cfTopicParams );

	return; 
}

void NeuronSub_setup_throtmsg_pub( NeuronDP *pNdp, const char *partition_name )
{
	DDS_ReturnCode_t	status;
	DDS_PublisherQos	*pubQos;

	// Adapt Publisher QOS to write to partition 'partition_name'
	printf( "Creating Publisher QOS Object..." );
	pubQos = DDS_PublisherQos__alloc();
	checkHandle( pubQos, "DDS_PublisherQos__alloc()" );
	printf( "Successful\nObtaining Default Publisher QOS Settings..." );
	status = DDS_DomainParticipant_get_default_publisher_qos( pNdp->dp, pubQos );
	checkStatus( status, "DDS_DomainParticipant_get_default_publisher_qos()" );
	printf( "Successful\nSetting Partition to %s...", partition_name );
	pubQos->partition.name._length = 1;
	pubQos->partition.name._maximum = 1;	
	pubQos->partition.name._buffer = DDS_StringSeq_allocbuf( 1 );
	checkHandle( pubQos->partition.name._buffer, "DDS_StringSeq_allocbuf()" );
	pubQos->partition.name._buffer[0] = DDS_string_alloc( strlen(partition_name) );
	checkHandle( pubQos->partition.name._buffer[0], "DDS_string_alloc()" );
	strcpy( pubQos->partition.name._buffer[0], partition_name );
	
	// Create Publisher
	printf( "Successful\nCreating Publisher Object..." );
	pNdp->nsub.throtPub = DDS_DomainParticipant_create_publisher( 
															 	  pNdp->dp, pubQos, NULL, 
															 	  DDS_ANY_STATUS 
														   		);
	checkHandle( pNdp->nsub.throtPub, "DDS_DomainParticipant_create_publisher()" );
	
	// Create Data Writer for ThrotMsg Topic
	printf( "Successful\nCreating Data Writer for Frame Topic..." );
	pNdp->nsub.throtWriter = DDS_Publisher_create_datawriter( 
															  pNdp->nsub.throtPub, 
															  pNdp->throtMsgTopic,
															  DDS_DATAWRITER_QOS_USE_TOPIC_QOS,
															  NULL, DDS_ANY_STATUS
														  	); 
	checkHandle( pNdp->nsub.throtWriter, "DDS_Publisher_create_datawriter(ThrotMsg)" );
	
	// Deallocate resources
	DDS_free( pubQos );
	
	return; 
}

void NeuronSub_destroy( NeuronDP *pNdp )
{
	DDS_ReturnCode_t	status;
	
	// Destroy ThrotMsg Publisher
	NeuronSub_destroy_throtmsg_pub( pNdp );
	
	// Delete Data Reader for SrcAdvert Topic
	printf( "Deleting Data Reader for SrcAdvert Topic..." );
	status = DDS_Subscriber_delete_datareader( pNdp->nsub.sub, pNdp->nsub.srcAdvertReader );
	checkStatus( status, "DDS_Subscriber_delete_datareader(SrcAdTopic)" );
		
	// Delete the Subscriber
	printf( "Successful\nDeleting the Subscriber Object..." );
	status = DDS_DomainParticipant_delete_subscriber( pNdp->dp, pNdp->nsub.sub );
	checkStatus( status, "DDS_DomainParticipant_delete_subscriber()" );
	printf( "Successful\n" );

	return;
}

void NeuronSub_destroy_throtmsg_pub( NeuronDP *pNdp )
{
	DDS_ReturnCode_t	status;
	
	// Delete Data Writer for ThrotMsg Topic
	printf( "Deleting Data Writer for ThrotMsg Topic..." );
	status = DDS_Publisher_delete_datawriter( pNdp->nsub.throtPub, pNdp->nsub.throtWriter );
	checkStatus( status, "DDS_Subscriber_delete_datawriter(ThrotMsg)" );
	
	// Delete the Subscriber
	printf( "Successful\nDeleting the ThrotMsg Publisher Object..." );
	status = DDS_DomainParticipant_delete_publisher( pNdp->dp, pNdp->nsub.throtPub );
	checkStatus( status, "DDS_DomainParticipant_delete_publisherer(ThrotMsg)" );
	printf( "Successful\n" );
	
	return;
}

void NeuronSub_destroy_cftopic_and_reader( NeuronDP *pNdp )
{
	DDS_ReturnCode_t	status;
	
	// Delete Data Reader for Content Filtered Frame Topic
	printf( "Successful\nDeleting Data Reader for Content Filtered Frame Topic..." );
	status = DDS_Subscriber_delete_datareader( pNdp->nsub.sub, pNdp->nsub.frmReader );
	checkStatus( status, "DDS_Subscriber_delete_datareader(FrameTopic)" );
	
	// Delete Content Filtered Frame Topic
	printf( "Successful\nDeleting Content Filtered Frame Topic..." );
	status = DDS_DomainParticipant_delete_contentfilteredtopic( 
																pNdp->dp, 
																pNdp->nsub.specSrcFrmTopic 
															  );
	checkStatus( status, "DDS_DomainParticipant_delete_contentfilteredtopic(Frame)" );

	return;
}

void NeuronSub_read_srcadverts( NeuronDP *pNdp )
{
	DDS_sequence_NeuronDDS_SrcAdvert	*srcAdSeq;
	DDS_SampleInfoSeq					*smpInfoSeq;
	DDS_ReturnCode_t					status;
	DDS_unsigned_long					i;
	
	// Allocate memory for srcAdSeq and smpInfoSeq
	srcAdSeq = DDS_sequence_NeuronDDS_SrcAdvert__alloc();
	checkHandle( srcAdSeq, "DDS_sequence_NeuronDDS_SrcAdvert__alloc()" );
	smpInfoSeq = DDS_SampleInfoSeq__alloc();
	checkHandle( smpInfoSeq, "DDS_SampleInfoSeq__alloc()" );
	
	status = NeuronDDS_SrcAdvertDataReader_read( 
												 pNdp->nsub.srcAdvertReader,
												 srcAdSeq, smpInfoSeq, DDS_LENGTH_UNLIMITED,
												 DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE,
												 DDS_ANY_INSTANCE_STATE
											   );
	checkStatus( status, "NeuronDDS_SrcAdvertDataReader_take()" );

	pNdp->nsub.n_srcs = (int) srcAdSeq->_length;
	for( i=0; i<srcAdSeq->_length; i++ )
	{
		NeuronDDS_SrcAdvert *pSrcAdMsg = &(srcAdSeq->_buffer[i]);
		strcpy( pNdp->nsub.srcNameList[i], pSrcAdMsg->srcName );
		sprintf( pNdp->nsub.srcIdList[i], "%d", (int) pSrcAdMsg->srcID );	
	}

	// Sequence memory is borrowed from DDS, so return the loaned memory
	status = NeuronDDS_SrcAdvertDataReader_return_loan( 
														pNdp->nsub.srcAdvertReader,
														srcAdSeq, smpInfoSeq
													  );
	checkStatus( status, "NeuronDDS_SrcAdvertDataReader_return_loan()" );	

	return;
}

void NeuronSub_read_frame( NeuronDP *pNdp, char **frm_buf, int *buf_size, int *frm_size, 
						   int *frm_layer_type, const char *query_str, char query_param_list[][10] )
{
	unsigned short	crc_chksum;
	
	DDS_boolean						valid_data = FALSE;
	DDS_sequence_NeuronDDS_Frame	*frmSeq = NULL;
	DDS_SampleInfoSeq				*smpInfoSeq = NULL;
	DDS_StringSeq					*queryParams = NULL;
	NeuronDDS_Frame 				*frmMsg = NULL;
	DDS_ReturnCode_t				status;
	DDS_QueryCondition				queryCond;
	DDS_WaitSet						newFrmWaitSet;
	DDS_ConditionSeq				*trigCondList = NULL;
	DDS_Duration_t					timeout = DDS_DURATION_INFINITE;
	
	//crcInit();
	
	// Create WaitSet to wait for new frame samples or source dead
	queryParams = DDS_StringSeq__alloc();
	checkHandle( queryParams, "DDS_StringSeq__alloc(queryParams)" );
	queryParams->_length = queryParams->_maximum = 2;
	queryParams->_buffer = DDS_StringSeq_allocbuf( 2 );
	checkHandle( queryParams->_buffer, "DDS_StringSeq_allocbuf(queryParams)" );
	queryParams->_buffer[0] = DDS_string_alloc( strlen(query_param_list[0])+1 );
	checkHandle( queryParams->_buffer[0], "DDS_string__alloc(queryParams->buffer[0])" );
	queryParams->_buffer[1] = DDS_string_alloc( strlen(query_param_list[1])+1 );
	checkHandle( queryParams->_buffer[1], "DDS_string__alloc(queryParams->buffer[1])" );
	strcpy( queryParams->_buffer[0], query_param_list[0] );
	strcpy( queryParams->_buffer[1], query_param_list[1] );
	queryCond = DDS_DataReader_create_querycondition( 
													  pNdp->nsub.frmReader, 
													  DDS_NOT_READ_SAMPLE_STATE,
													  DDS_ANY_VIEW_STATE,
													  DDS_ALIVE_INSTANCE_STATE,
													  query_str,
													  queryParams
													);
	checkHandle( queryCond, "DDS_DataReader_create_querycondition()" );
	
	newFrmWaitSet = DDS_WaitSet__alloc();
	checkHandle( newFrmWaitSet, "DDS_WaitSet__alloc()" );
	status = DDS_WaitSet_attach_condition( newFrmWaitSet, queryCond );
	checkStatus( status, "DDS_WaitSet_attach_condition()" );
	trigCondList = DDS_ConditionSeq__alloc();
	checkHandle( trigCondList, "DDS_ConditionSeq__alloc()" );
	trigCondList->_maximum = trigCondList->_length = 1;
	trigCondList->_buffer = DDS_ConditionSeq_allocbuf( trigCondList->_length );
	
	// Allocate memory for srcAdSeq and smpInfoSeq
	frmSeq = DDS_sequence_NeuronDDS_Frame__alloc();
	checkHandle( frmSeq, "DDS_sequence_NeuronDDS_Frame__alloc()" );
	smpInfoSeq = DDS_SampleInfoSeq__alloc();
	checkHandle( smpInfoSeq, "DDS_SampleInfoSeq__alloc()" );
	
	do
	{
		status = DDS_WaitSet_wait( newFrmWaitSet, trigCondList, &timeout );
		checkStatus( status, "DDS_WaitSet_wait()" );
		
		status = NeuronDDS_FrameDataReader_take_w_condition( 
															 pNdp->nsub.frmReader, frmSeq, 
															 smpInfoSeq, 1, queryCond
										   				   );
		checkStatus( status, "NeuronDDS_FrameDataReader_take_w_condition()" );
	
		if( status!=DDS_RETCODE_NO_DATA && smpInfoSeq->_buffer[0].valid_data )
		{		
			frmMsg = &(frmSeq->_buffer[0]);
			//crc_chksum = crcFast( frmMsg->payload._buffer, frmMsg->payload._length );
			//printf( "Index: %ld, Layer Type: %ld, Length: %ld\n", 
			//	frmMsg->index, frmMsg->layerType, frmMsg->payload._length );
	
			*frm_size = (int) (frmMsg->payload._length);
			*frm_layer_type = (int) (frmMsg->layerType);
			if( *buf_size<(*frm_size) )
			{
				*frm_buf = (char *) realloc( *frm_buf, *frm_size );
				if( *frm_buf==NULL )
				{
					fprintf( stderr, "Subscribe realloc error\n" );
					exit(0);
				}
				*buf_size = *frm_size;
			}
			memcpy( *frm_buf, frmMsg->payload._buffer, *frm_size );
			valid_data = TRUE;
		}
	
		// Sequence memory is borrowed from DDS, so return the loaned memory
		status = NeuronDDS_FrameDataReader_return_loan( pNdp->nsub.frmReader, frmSeq, smpInfoSeq );
		checkStatus( status, "NeuronDDS_FrameDataReader_return_loan()" );
	} while( !valid_data );
	
	status = DDS_WaitSet_detach_condition( newFrmWaitSet, queryCond );
	checkStatus( status, "DDS_WaitSet_detach_condition()" );
	status = DDS_DataReader_delete_readcondition( pNdp->nsub.frmReader, queryCond );
	checkStatus( status, "DDS_DataReader_delete_readcondition()" );
	
	DDS_free( queryParams );
	DDS_free( newFrmWaitSet );
	DDS_free( trigCondList );
	
	return;
}

void NeuronSub_write_throtmsg( NeuronDP *pNdp, char throt_signal )
{
	NeuronDDS_ThrotMsg	throtMsg;
	DDS_ReturnCode_t	status;

	throtMsg.srcID = pNdp->dp_id;
	throtMsg.mode = (long) (throt_signal-'0');
	
	status = NeuronDDS_ThrotMsgDataWriter_write( 
												 pNdp->nsub.throtWriter, 
												 &throtMsg, DDS_HANDLE_NIL 
											   );
	checkStatus( status, "NeuronDDS_ThrotMsgDataWriter_write()" );
	//printf( "New throttle(%d): %d\n", (int) throtMsg.srcID, (int) throtMsg.mode );
	
	return;
}

void NeuronDP_create_dp_factory( NeuronDP *pNdp )
{
	// Create Domain Participant Factory.
	printf( "Creating Domain Participant Factory..." );
	pNdp->dpf = DDS_DomainParticipantFactory_get_instance();
	checkHandle( pNdp->dpf, "DDS_DomainParticipantFactory_get_instance()" );

	return;	
}

void NeuronDP_setup( NeuronDP *pNdp, int pub_or_sub, char *src_id_str )
{
	char	*sa_type_name;
	char	*frm_type_name;
	char	*throtmsg_type_name;
	
	DDS_ReturnCode_t				status;
	//DDS_Duration_t					inf_block_time = DDS_DURATION_INFINITE;
	//DDS_Duration_t					serv_clnup_dly = { 0, 10e-8 };
	NeuronDDS_SrcAdvertTypeSupport	sats;
	NeuronDDS_FrameTypeSupport		fts;
	NeuronDDS_ThrotMsgTypeSupport	tmts;
	DDS_TopicQos					*qos;
	
	// Create Domain Participant.
	printf( " Successful\nCreating Domain Participant..." );
	pNdp->dp = DDS_DomainParticipantFactory_create_participant( 
																pNdp->dpf, NULL, 
    															DDS_PARTICIPANT_QOS_DEFAULT, 
    															NULL, DDS_ANY_STATUS
														      );
	checkHandle( pNdp->dp, "DDS_DomainParticipantFactory_create_participant()" );
	
	// Create Type Supports for 'SrcAdvert', 'Frame' and 'ThrotMsg' Topics and register them.
	printf( "Successful\nCreating Type Support for SrcAdvert..." );
	sats = NeuronDDS_SrcAdvertTypeSupport__alloc();
	checkHandle( sats, "NeuronDDS_SrcAdvertTypeSupport__alloc(SrcAdvert)" );
	printf( "Successful\n" );
	sa_type_name = NeuronDDS_SrcAdvertTypeSupport_get_type_name( sats );
	printf( "Registering Data Type %s...", sa_type_name );
	status = NeuronDDS_SrcAdvertTypeSupport_register_type( sats, pNdp->dp, sa_type_name );
	checkStatus( status, "NeuronDDS_SrcAdvertTypeSupport_register_type(SrcAdvert)" );

	printf( "Successful\nCreating Type Support for Frame..." );
	fts = NeuronDDS_FrameTypeSupport__alloc();
	checkHandle( fts, "NeuronDDS_FrameTypeSupport__alloc(Frame)" );
	printf( "Successful\n" );
	frm_type_name = NeuronDDS_FrameTypeSupport_get_type_name( fts );
	printf( "Registering Data Type %s...", frm_type_name );
	status = NeuronDDS_FrameTypeSupport_register_type( fts, pNdp->dp, frm_type_name );
	checkStatus( status, "NeuronDDS_FrameTypeSupport_register_type(Frame)" );
	
	printf( "Successful\nCreating Type Support for ThrotMsg..." );
	tmts = NeuronDDS_ThrotMsgTypeSupport__alloc();
	checkHandle( tmts, "NeuronDDS_ThrotMsgTypeSupport__alloc(ThrotMsg)" );
	printf( "Successful\n" );
	throtmsg_type_name = NeuronDDS_ThrotMsgTypeSupport_get_type_name( tmts );
	printf( "Registering Data Type %s...", throtmsg_type_name );
	status = NeuronDDS_ThrotMsgTypeSupport_register_type( tmts, pNdp->dp, throtmsg_type_name );
	checkStatus( status, "NeuronDDS_ThrotMsgTypeSupport_register_type(ThrotMsg)" );


	// Create Topic Objects with appropriate QOS settings
	printf( "Successful\nCreating Qos Object..." );
	qos = DDS_TopicQos__alloc();
	checkHandle( qos, "DDS_TopicQos__alloc()" );
	printf( "Successful\nObtaining Default Topic QOS..." );
	status = DDS_DomainParticipant_get_default_topic_qos( pNdp->dp, qos );
	checkStatus( status, "DDS_DomainParticipant_get_default_topic_qos()" );

    printf( "Successful\nCreating Frame Topic..." );
    qos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    qos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    qos->history.depth = 2;
    pNdp->frmTopic = DDS_DomainParticipant_create_topic( 
    													 pNdp->dp, "NeuronDDS_Frame", 
    													 frm_type_name, qos, NULL, DDS_ANY_STATUS
												 	   );
	checkHandle( pNdp->frmTopic, "DDS_DomainParticipant_create_topic(Frame)" );

    printf( "Successful\nCreating SrcAdvert Topic..." );
	status = DDS_DomainParticipant_get_default_topic_qos( pNdp->dp, qos );
	checkStatus( status, "DDS_DomainParticipant_get_default_topic_qos()" );    
	qos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;
    pNdp->srcAdTopic = DDS_DomainParticipant_create_topic( 
    											  			pNdp->dp, "NeuronDDS_SrcAdvert", 
    											  			sa_type_name, qos, NULL, DDS_ANY_STATUS
														 );
	checkHandle( pNdp->srcAdTopic, "DDS_DomainParticipant_create_topic(SrcAdvert)" );

    printf( "Successful\nCreating ThrotMsg Topic..." );
	qos->durability.kind = DDS_VOLATILE_DURABILITY_QOS;
    pNdp->throtMsgTopic = DDS_DomainParticipant_create_topic( 
    											  			  pNdp->dp, "NeuronDDS_ThrotMsg", 
    											  			  throtmsg_type_name, qos, NULL, 
    											  			  DDS_ANY_STATUS
														 	);
	checkHandle( pNdp->throtMsgTopic, "DDS_DomainParticipant_create_topic(ThrotMsg)" );
	printf( "Successful\n" );
	
	// If Domain Participant is configured as Publisher, setup the Publisher Object.
	if( pub_or_sub==PUB_CHOICE )
		NeuronPub_setup( pNdp, PARTITION_NAME, src_id_str );
	else
		NeuronSub_setup( pNdp, PARTITION_NAME, src_id_str );
	
	// Deallocate resources
	DDS_free( qos );
	DDS_free( fts );
	DDS_free( frm_type_name );
	DDS_free( sats );
	DDS_free( sa_type_name );
	DDS_free( tmts );
	DDS_free( throtmsg_type_name );
	
	return;	
}

void NeuronDP_destroy( NeuronDP *pNdp, int pub_or_sub )
{
	DDS_ReturnCode_t	status;
	
	if( pub_or_sub==PUB_CHOICE )
		NeuronPub_destroy( pNdp );
	else
		NeuronSub_destroy( pNdp );

    printf( "Deleting Topic Frame..." );
    status = DDS_DomainParticipant_delete_topic( pNdp->dp, pNdp->frmTopic );
	checkStatus( status, "DDS_DomainParticipant_delete_topic(Frame)" );

    printf( "Successful\nDeleting Topic SrcAdvert..." );
    status = DDS_DomainParticipant_delete_topic( pNdp->dp, pNdp->srcAdTopic );
	checkStatus( status, "DDS_DomainParticipant_delete_topic(SrcAdvert)" );

    printf( "Successful\nDeleting Topic ThrotMsg..." );
    status = DDS_DomainParticipant_delete_topic( pNdp->dp, pNdp->throtMsgTopic );
	checkStatus( status, "DDS_DomainParticipant_delete_topic(ThrotMsg)" );
	
	printf( "Successful\nDeleting Domain Participant..." );
	status = DDS_DomainParticipantFactory_delete_participant( pNdp->dpf, pNdp->dp );
	checkStatus( status, "DDS_DomainParticipantFactory_delete_participant()" );
    printf( "Successful\n" );
    
    return;
}

