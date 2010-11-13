//!
//! \file CPInterfaceT.h
//!
//! \brief Defintion of Control Plane (CP)
//!
//! \author Tron Kindseth (tron@rti.com)
//! \date Created on: Nov 1, 2010
//!
#ifndef CPINTERFACET_H_
#define CPINTERFACET_H_

#include "CPInterface.h"

template <
typename ControlTypeSupport,
typename EventTypeSupport,
typename StateTypeSupport,
typename MetricsTypeSupport> class CPInterfaceT : public CPInterface
{
public:
    CPInterfaceT(int domainId,const char *qosProfile = NULL) : 
    m_domainParticipant(NULL),
    m_publisher(NULL),
    m_subscriber(NULL) 
    {
        DDS_DomainParticipantQos dpQos;
        DDS_PublisherQos pubQos;
        DDS_SubscriberQos subQos;
        
        factory = DDSDomainParticipantFactory::get_instance();
        
        // TODO: Improve this.The basic idea is that instead of creating
        //       from a profile, get the profile and then modify the profile 
        //       based on other configurations parameters. This gives the best
        //       control
        factory->get_participant_qos_from_profile(dpQos, "NEURON", qosProfile);
        factory->get_publisher_qos_from_profile(pubQos, "NEURON", qosProfile);
        factory->get_subscriber_qos_from_profile(subQos, "NEURON", qosProfile);
        
        // TODO: Install listeners. The listeners will probably post on an 
        //       event queue
        m_domainParticipant = factory->create_participant(domainId, 
                                                          dpQos, 
                                                          NULL,
                                                          DDS_STATUS_MASK_NONE);
        if (m_domainParticipant == NULL) 
        {
            // Throw exception ??
            return;
        }
        
        // TODO: Install listeners
        m_publisher = m_domainParticipant->create_publisher(pubQos, 
                                                            NULL, 
                                                            DDS_STATUS_MASK_NONE);
        
        if (m_publisher == NULL) 
        {
            // Throw exception ??
            return;
        }
        
        // TODO: Install listeners
        m_subscriber = m_domainParticipant->create_subscriber(subQos, 
                                                              NULL, 
                                                              DDS_STATUS_MASK_NONE);
        
        if (m_subscriber == NULL) 
        {
            // Throw exception ??
            return;
        } 
        
        DDSTopic *topic;
        DDS_ReturnCode_t retcode;
        const char *type_name;
        
        type_name = ControlTypeSupport::get_type_name();        
        retcode = ControlTypeSupport::register_type(m_domainParticipant,
                                                    type_name);
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception ??
            return;
        }
        
        topic = m_domainParticipant->create_topic(type_name, 
                                                  type_name, 
                                                  DDS_TOPIC_QOS_DEFAULT, 
                                                  NULL, 
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL) 
        {
            // Throw exception ??
            return;
        }
        
        type_name = EventTypeSupport::get_type_name();        
        retcode = EventTypeSupport::register_type(m_domainParticipant,type_name);
        if (retcode != DDS_RETCODE_OK) {
            // Throw exception ??
            return;
        }
        
        topic = m_domainParticipant->create_topic(type_name, 
                                                  type_name, 
                                                  DDS_TOPIC_QOS_DEFAULT, 
                                                  NULL, 
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL) 
        {
            // Throw exception ??
            return;
        }
        
        type_name = StateTypeSupport::get_type_name();        
        retcode = StateTypeSupport::register_type(m_domainParticipant, type_name);
        if (retcode != DDS_RETCODE_OK) {
            // Throw exception ??
            return;
        }
        
        topic = m_domainParticipant->create_topic(type_name, 
                                                  type_name, 
                                                  DDS_TOPIC_QOS_DEFAULT, 
                                                  NULL, 
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL) 
        {
            // Throw exception ??
            return;
        }
        
        type_name = MetricsTypeSupport::get_type_name();        
        retcode = MetricsTypeSupport::register_type(m_domainParticipant, 
                                                    type_name);
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception ??
            return;
        }
        
        topic = m_domainParticipant->create_topic(type_name, 
                                                  type_name, 
                                                  DDS_TOPIC_QOS_DEFAULT, 
                                                  NULL, 
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL) 
        {
            // Throw exception ??
            return;
        }
    }
    
    CPInterfaceT() 
    {
        DDS_ReturnCode_t retcode;
        if (m_domainParticipant != NULL) 
        {
            retcode = m_domainParticipant->delete_contained_entities();
            if (retcode != DDS_RETCODE_OK) 
            {
                // Throw exception
            }
            
            retcode = DDSTheParticipantFactory->delete_participant(m_domainParticipant);
            if (retcode != DDS_RETCODE_OK) 
            {
                // Throw exception
            }
            
            retcode = DDSDomainParticipantFactory::finalize_instance();
            if (retcode != DDS_RETCODE_OK) 
            {
                // Throw exception
            }            
        }
    }
    
protected:
    DDSDomainParticipantFactory *factory;
    DDSDomainParticipant *m_domainParticipant;
    DDSPublisher *m_publisher;
    DDSSubscriber *m_subscriber;
};

template <
typename CPMasterObjectT,
typename ControlDataWriter,
typename StateDataReader,
typename EventDataReader,
typename MetricsDataReader,
typename EventSeq,
typename StateSeq,
typename MetricsSeq,
typename Metrics,
typename EventT,
typename State,
typename Control,
typename ControlTypeSupport,
typename EventTypeSupport,
typename StateTypeSupport,
typename MetricsTypeSupport>
class CPMasterT : public CPInterfaceT<ControlTypeSupport,
                                      EventTypeSupport,
                                      StateTypeSupport,
                                      MetricsTypeSupport> {
public:
    CPMasterT(EventHandler *eh,int srcId,int domainId, int appId, const char *qosProfile) : CPInterfaceT<ControlTypeSupport,
                                    EventTypeSupport,
                                    StateTypeSupport,
                                    MetricsTypeSupport>(domainId,qosProfile),
                                    m_controlWriter(NULL),
                                    m_stateReader(NULL),
                                    m_eventReader(NULL),
                                    m_metricsReader(NULL)
    {
        DDSTopic *topic;
        DDS_ReturnCode_t retcode;
        
        // TODO: Register Types with DP
        topic = this->m_domainParticipant->find_topic(ControlTypeSupport::get_type_name(), DDS_DURATION_INFINITE);
        
        if (topic == NULL) 
        {
            // Throw exception
            return;
        }
        
        //Need one profile for each Topic
        DDS_DataWriterQos dw_qos;
        retcode = this->factory->get_datawriter_qos_from_profile_w_topic_name(dw_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception
            return;
        }
        // update any application specfic QoS here
        m_controlWriter = this->m_publisher->create_datawriter(topic, 
                                                               dw_qos, 
                                                               NULL, 
                                                               DDS_STATUS_MASK_NONE);
        
        DDS_DataReaderQos dr_qos;
        topic = this->m_domainParticipant->find_topic(StateTypeSupport::get_type_name(), DDS_DURATION_INFINITE);
        
        if (topic == NULL) 
        {
            // Throw exception
            return;
        }
        retcode = this->factory->get_datareader_qos_from_profile_w_topic_name(dr_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception
            return;
        }
        
        m_stateReader = this->m_subscriber->create_datareader(topic, dr_qos, NULL, DDS_STATUS_MASK_NONE);
        if (m_stateReader == NULL) 
        {
            // Throw exception
            return;
        }
        
        topic = this->m_domainParticipant->find_topic(EventTypeSupport::get_type_name(), DDS_DURATION_INFINITE);
        
        if (topic == NULL) 
        {
            // Throw exception
            return;
        }
        retcode = this->factory->get_datareader_qos_from_profile_w_topic_name(dr_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception
            return;
        }
        
        m_eventReader = this->m_subscriber->create_datareader(topic, dr_qos, NULL, DDS_STATUS_MASK_NONE);
        if (m_eventReader == NULL) 
        {
            // Throw exception
            return;
        }
        
        topic = this->m_domainParticipant->find_topic(MetricsTypeSupport::get_type_name(), DDS_DURATION_INFINITE);
        
        if (topic == NULL) 
        {
            // Throw exception
            return;
        }
        retcode = this->factory->get_datareader_qos_from_profile_w_topic_name(dr_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception
            return;
        }
        
        m_metricsReader = this->m_subscriber->create_datareader(topic, dr_qos, NULL, DDS_STATUS_MASK_NONE);
        if (m_metricsReader == NULL) 
        {
            // Throw exception
            return;
        }   
        controlWriter = ControlDataWriter::narrow(m_controlWriter);
        if (controlWriter == NULL)
        {
            // TODO: Error handling
            return;
        }
        
        stateReader = StateDataReader::narrow(m_stateReader);
        if (stateReader == NULL) 
        {
            // TODO: Error handling
            return;
        }
                
        eventReader = EventDataReader::narrow(m_eventReader);
        if (eventReader == NULL) 
        {
            // TODO: Error handling
            return;
        }
        
        metricsReader = MetricsDataReader::narrow(m_metricsReader);
        if (metricsReader == NULL) 
        {
            // TODO: Error handling
            return;
        }
        
        state = StateTypeSupport::create_data();
        
        if (state == NULL)
        {
            //TODO: Error handling
            return;
        }
        
        event = EventTypeSupport::create_data();
        
        if (event == NULL)
        {
            //TODO: Error handling
            return;
        }
        
        metrics = MetricsTypeSupport::create_data();    
        
        if (metrics == NULL)
        {
            //TODO: Error handling
            return;
        }
        
        control = ControlTypeSupport::create_data();
        
        if (control == NULL) 
        {
            //TODO: Error handling
            return;
        }
        
        srcId = srcId;
        upper = eh;        
    }
    
    ~CPMasterT()
    {
        if (state != NULL)
        {
            StateTypeSupport::delete_data(state);
        }
        
        if (event != NULL)
        {
            EventTypeSupport::delete_data(event);
        }
        
        if (control != NULL) 
        {
            ControlTypeSupport::delete_data(control);
        }
        
        if (metrics != NULL)
        {
            MetricsTypeSupport::delete_data(metrics);
        }
    }
    
    //! \param[in] sessionId   Session ID
    virtual CPMasterObjectT* CreateMasterObject(int sessionId) = 0;
                                    
    //! Delete a Session object created with CreateMasterObject
    //!
    //! \param[in] so Session object to delete
    virtual bool DeleteMasterObject(CPMasterObjectT* so)=0;
                       
    //! Send a control topic
    //!
    //! \param[in] control control data to send on the SCP
    //!
    //! \todo. This method should not be exposed to applications
    bool Send(Control *c,DDS_InstanceHandle_t ih)
    {
        DDS_ReturnCode_t retcode;
        retcode = controlWriter->write(*c, ih);
        if (retcode != DDS_RETCODE_OK)
        {
            // TODO: Error handling
            return false;
        }
        return true;
    }
                                          
    //! Return the current state for a particular session
    //!
    //! \param[in] instance Session instance to retrieve state for
    //! \param[out] state Contains state on succcessful return
    //! \return true on success, false on failure
    //!
    //! \todo. This method should not be exposed to applications    
    bool GetMasterObjectState(DDS_InstanceHandle_t instance,State *out_state)
    {
        StateSeq data_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;

        data_seq.ensure_length(1,1);
        info_seq.ensure_length(1,1);
        retcode = stateReader->read_instance(data_seq,
                                           info_seq,
                                           1,
                                           instance,
                                           DDS_READ_SAMPLE_STATE,
                                           DDS_NOT_NEW_VIEW_STATE,
                                           DDS_ALIVE_INSTANCE_STATE);
        if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) 
        {
          //TODO: Error handling
          return false;
        }

        if (retcode == DDS_RETCODE_NO_DATA)
        {
          //TODO: Error handling
            return false;
        }

        if ((data_seq.length() != 1) || !info_seq[0].valid_data)
        {
          //TODO: Error handling
          return false;
        }

        retcode = StateTypeSupport::copy_data(out_state,&data_seq[0]);
        if (retcode != DDS_RETCODE_OK)
        {
          //TODO: Error handling
          return false;
        }

        return true;
    }
                                          
    //! Return the current state for a particular session
    //!
    //! \param[in] instance Session instance to retrieve Events for
    //! \param[out] events Contains events on succcessful return
    //! \return true on success, false on failure
    //!
    //! \todo. This method should not be exposed to applications        
    bool GetMasterObjectEvents(DDS_InstanceHandle_t instance,EventSeq* eventSeq)
    {
        EventSeq result_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;

        retcode = eventReader->read_instance(result_seq,
                                           info_seq,
                                           DDS_LENGTH_UNLIMITED,
                                           instance,
                                           DDS_READ_SAMPLE_STATE,
                                           DDS_NOT_NEW_VIEW_STATE,
                                           DDS_ALIVE_INSTANCE_STATE);

        if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) 
        {
          //TODO: Error handling
          return false;
        }

        if (retcode == DDS_RETCODE_NO_DATA)
        {
          //TODO: Error handling
          return false;
        }

        *eventSeq = result_seq;

        eventReader->return_loan(result_seq,info_seq);

        return true;
    }

    //! Return the current state for a particular session
    //!
    //! \param[in] instance Session instance to retrieve Metrics for
    //! \param[out] metrics Contains events on succcessful return
    //! \return true on success, false on failure
    //!
    //! \todo. This method should not be exposed to applications            
    bool GetMasterObjectMetrics(DDS_InstanceHandle_t instance,MetricsSeq *metricsSeq)
    {
        MetricsSeq result_seq;
        DDS_SampleInfoSeq info_seq;
        DDS_ReturnCode_t retcode;

        retcode = metricsReader->read_instance(result_seq,
                                             info_seq,
                                             DDS_LENGTH_UNLIMITED,
                                             instance,
                                             DDS_READ_SAMPLE_STATE,
                                             DDS_NOT_NEW_VIEW_STATE,
                                             DDS_ALIVE_INSTANCE_STATE);

        if ((retcode != DDS_RETCODE_OK) && (retcode != DDS_RETCODE_NO_DATA)) 
        {
          //TODO: Error handling
          return false;
        }

        if (retcode == DDS_RETCODE_NO_DATA)
        {
          //TODO: Error handling
          return false;
        }

        *metricsSeq = result_seq;

        metricsReader->return_loan(result_seq,info_seq);

        return true;    
    }
                                          
    virtual DDS_InstanceHandle_t GetMasterObjectStateHandle(int dstId,int sid)
    {
      DDS_InstanceHandle_t ih = DDS_HANDLE_NIL;
      
      state->srcId = dstId;
      
      ih = stateReader->lookup_instance(*state);
      
      return ih;
    }

    virtual DDS_InstanceHandle_t GetMasterObjectEventHandle(int dstId, int sid)
    {
      DDS_InstanceHandle_t ih = DDS_HANDLE_NIL;
      
      event->srcId = dstId;
      
      ih = eventReader->lookup_instance(*event);
      
      return ih;
    }

    virtual DDS_InstanceHandle_t GetMasterObjectMetricsHandle(int dstId,int sid)
    {
      DDS_InstanceHandle_t ih = DDS_HANDLE_NIL;
      
      metrics->srcId = dstId;
      
      ih = metricsReader->lookup_instance(*metrics);
      
      return ih;
    }

    //! Return the current state for a particular session
    //!
    //! \param[in] ev new event
    //!
    //! \todo. This method should not be exposed to applications                
    virtual bool PostEvent(Event *ev)
    {
        upper->SignalEvent(ev);
        return true;
    }
                                          
                                          
protected:
    DDSDataWriter  *m_controlWriter;
    DDSDataReader *m_stateReader;
    DDSDataReader *m_eventReader;
    DDSDataReader *m_metricsReader;
    int srcId;
  
//! \var upper
//! \brief Upper-layer event handle
EventHandler *upper;
  
  //! \var controlWriter
  //! \brief DDS writer for control data
  ControlDataWriter *controlWriter;
  
  //! \var stateReader
  //! \brief DDS reader for state data
  StateDataReader *stateReader;
  
  //! \var eventReader
  //! \brief DDS reader for event data
  EventDataReader *eventReader;
  
  //! \var metricsReader
  //! \brief DDS reader for metrics data
  MetricsDataReader *metricsReader;
  
  //! \var metrics
  //! \brief scratch to lookup instance handles
  Metrics *metrics;
  
  //! \var event
  //! \brief scratch to lookup instance handles
  EventT *event;
  
  //! \var state
  //! \brief scratch to lookup instance handles
  State *state;
  
  //! \var control
  //! \brief scratch to lookup instance handles
  Control *control;
};

template <
typename SlaveObjectT,
typename ControlDataReader,
typename StateDataWriter,
typename EventDataWriter,
typename MetricsDataWriter,
typename State,
typename EventT,
typename Metrics,
typename ControlTypeSupport,
typename EventTypeSupport,
typename StateTypeSupport,
typename MetricsTypeSupport>
class CPSlaveT : public CPInterfaceT<ControlTypeSupport,EventTypeSupport,StateTypeSupport,MetricsTypeSupport> {
public:
    CPSlaveT(EventHandler *q,int _srcId,int domainId,int appId, const char *qosProfile) : CPInterfaceT<ControlTypeSupport,EventTypeSupport,StateTypeSupport,MetricsTypeSupport>(domainId,qosProfile),
    m_controlReader(NULL),
    m_stateWriter(NULL),
    m_eventWriter(NULL),
    m_metricsWriter(NULL)
    {
        DDSTopic *topic;
        DDS_ReturnCode_t retcode;
        DDSContentFilteredTopic *cft;
        char expression[1024];
        DDS_StringSeq params;
        
        // TODO: Register Types with DP
        topic = this->m_domainParticipant->find_topic(ControlTypeSupport::get_type_name(), DDS_DURATION_INFINITE);
        
        if (topic == NULL) 
        {
            // Throw exception
            return;
        }
        
        //Need one profile for each Topic
        DDS_DataReaderQos dr_qos;
        retcode = this->factory->get_datareader_qos_from_profile_w_topic_name(dr_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception
            return;
        }
        
        // update any application specfic QoS here
        sprintf(expression,"dstId = %d",appId);
        cft = this->m_domainParticipant->create_contentfilteredtopic(expression,topic, expression, params);
        if (cft == NULL) {
            // Throw exception
            return;
        }
        m_controlReader = this->m_subscriber->create_datareader(cft, 
                                                    dr_qos, 
                                                    NULL, 
                                                    DDS_STATUS_MASK_NONE);
        
        DDS_DataWriterQos dw_qos;
        topic = this->m_domainParticipant->find_topic(StateTypeSupport::get_type_name(), DDS_DURATION_INFINITE);
        
        if (topic == NULL) 
        {
            // Throw exception
            return;
        }
        
        retcode = this->factory->get_datawriter_qos_from_profile_w_topic_name(dw_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception
            return;
        }
        
        m_stateWriter = this->m_publisher->create_datawriter(topic, dw_qos, NULL, DDS_STATUS_MASK_NONE);
        if (m_stateWriter == NULL) 
        {
            // Throw exception
            return;
        }
        
        topic = this->m_domainParticipant->find_topic(EventTypeSupport::get_type_name(), DDS_DURATION_INFINITE);
        
        if (topic == NULL) 
        {
            // Throw exception
            return;
        }
        retcode = this->factory->get_datawriter_qos_from_profile_w_topic_name(dw_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception
            return;
        }
        
        m_eventWriter = this->m_publisher->create_datawriter(topic, dw_qos, NULL, DDS_STATUS_MASK_NONE);
        if (m_eventWriter == NULL) 
        {
            // Throw exception
            return;
        }
        
        topic = this->m_domainParticipant->find_topic(MetricsTypeSupport::get_type_name(), DDS_DURATION_INFINITE);
        
        if (topic == NULL) 
        {
            // Throw exception
            return;
        }
        retcode = this->factory->get_datawriter_qos_from_profile_w_topic_name(dw_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK) 
        {
            // Throw exception
            return;
        }
        
        m_metricsWriter = this->m_publisher->create_datawriter(topic, dw_qos, NULL, DDS_STATUS_MASK_NONE);
        if (m_metricsWriter == NULL) 
        {
            // Throw exception
            return;
        }
        
        controlReader = ControlDataReader::narrow(m_controlReader);
        if (controlReader == NULL)
        {
            return;
        }
        
        stateWriter = StateDataWriter::narrow(m_stateWriter);
        if (stateWriter == NULL) 
        {
            return;
        }
        
        eventWriter = EventDataWriter::narrow(m_eventWriter);
        if (eventWriter == NULL) 
        {
            return;
        }
        
        metricsWriter = MetricsDataWriter::narrow(m_metricsWriter);
        if (metricsWriter == NULL) 
        {
            return;
        }
        
        state = StateTypeSupport::create_data();
        if (state == NULL)
        {
            //TODO: Error log
            return;
        }
        
        event = EventTypeSupport::create_data();
        if (event == NULL) 
        {
            //TODO: Error log
            return;
        }
        
        metrics = MetricsTypeSupport::create_data();
        if (metrics == NULL)
        {
            //TODO: Error log
            return;
        }
        
        srcId = _srcId;
        upper = q;        
    }
    
    ~CPSlaveT()
    {
        if (state != NULL)
        {
            StateTypeSupport::delete_data(state);
        }
        
        if (event != NULL)
        {
            EventTypeSupport::delete_data(event);
        }
        
        if (metrics != NULL)
        {
            MetricsTypeSupport::delete_data(metrics);
        }    
    }
     
    virtual SlaveObjectT* CreateSlaveObject(int sid)=0;
    
    virtual bool DeleteSlaveObject(SlaveObjectT* aSession)=0;

    bool Send(State *state, DDS_InstanceHandle_t ih)
    {
        DDS_ReturnCode_t retcode;
        retcode = stateWriter->write(*state, ih);
        if (retcode != DDS_RETCODE_OK)
        {
            printf("error sending\n");
            return false;
        }
        printf("ok sending\n");
        return true;
    }        
    
    bool Send(EventT *event, DDS_InstanceHandle_t ih)
    {
        DDS_ReturnCode_t retcode;
        retcode = eventWriter->write(*event, ih);
        if (retcode != DDS_RETCODE_OK)
        {
            return false;
        }
        return true;        
    }
    
    bool Send(Metrics *metrics, DDS_InstanceHandle_t ih)
    {
        DDS_ReturnCode_t retcode;
        retcode = metricsWriter->write(*metrics, ih);
        if (retcode != DDS_RETCODE_OK)
        {
            return false;
        }
        return true;
    }
    
    virtual bool PostEvent(Event *ev)
    {
        upper->SignalEvent(ev);
        return true;
    }
    
protected:
    DDSDataReader  *m_controlReader;
    DDSDataWriter *m_stateWriter;
    DDSDataWriter *m_eventWriter;
    DDSDataWriter *m_metricsWriter;
    
    /* Experiments */
    ControlDataReader *controlReader;
    StateDataWriter *stateWriter;
    EventDataWriter *eventWriter;
    MetricsDataWriter *metricsWriter;
    State *state;
    EventT *event;
    Metrics *metrics;
    
    int srcId;
    EventHandler *upper;
};

#endif

