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

// Empty base-class for a CP Master/Slave object
class CPInterface 
{
};

// Empty base-class for a CPObject
class CPObject {
};


template <typename ControlTypeSupport,
typename EventTypeSupport,
typename StateTypeSupport,
typename MetricsTypeSupport> class CPInterfaceT
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
typename ControlTypeSupport,
typename EventTypeSupport,
typename StateTypeSupport,
typename MetricsTypeSupport>
class CPMasterT : public CPInterfaceT<ControlTypeSupport,
                                      EventTypeSupport,
                                      StateTypeSupport,
                                      MetricsTypeSupport> {
public:
    CPMasterT(int domainId, int appId, const char *qosProfile) : CPInterfaceT<ControlTypeSupport,
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
    }
    
    ~CPMasterT()
    {
        // Delete DDS entities
    }
    
protected:
    DDSDataWriter  *m_controlWriter;
    DDSDataReader *m_stateReader;
    DDSDataReader *m_eventReader;
    DDSDataReader *m_metricsReader;
};

template <
typename ControlTypeSupport,
typename EventTypeSupport,
typename StateTypeSupport,
typename MetricsTypeSupport>
class CPSlaveT : public CPInterfaceT<ControlTypeSupport,EventTypeSupport,StateTypeSupport,MetricsTypeSupport> 
{
public:
    CPSlaveT(int domainId,int appId, const char *qosProfile) : CPInterfaceT<ControlTypeSupport,EventTypeSupport,StateTypeSupport,MetricsTypeSupport>(domainId,qosProfile),
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
        
    }
    
    ~CPSlaveT()
    {
        // Delete DDS entities
    }
    
protected:
    DDSDataReader  *m_controlReader;
    DDSDataWriter *m_stateWriter;
    DDSDataWriter *m_eventWriter;
    DDSDataWriter *m_metricsWriter;    
};
#endif

