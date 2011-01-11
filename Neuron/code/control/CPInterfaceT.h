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


// TODO: Move this to a command log-directory
#define CONTROL_LOG_INFO    1
#define CONTROL_LOG_WARNING 2
#define CONTROL_LOG_ERROR   3

template <
typename ControlTypeSupport,
typename EventTypeSupport,
typename StateTypeSupport,
typename MetricsTypeSupport> class CPInterfaceT : public CPInterface
{
public:
    CPInterfaceT(int domainId,const char *name, const char *qosProfile = NULL) :
    pDomainParticipant(NULL),
    pPublisher(NULL),
    pSubscriber(NULL)
    {
        DDS_DomainParticipantFactoryQos fQos;
        DDS_DomainParticipantQos dpQos;
        //DDS_PublisherQos pubQos;
        //DDS_SubscriberQos subQos;
        DDS_ReturnCode_t retcode;
        //DDSTopic *topic;
        //const char *type_name;

        pFactory = DDSDomainParticipantFactory::get_instance();

        retcode = pFactory->get_qos(fQos);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("CP: Failed to get default participant factory profile\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }
        fQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
        retcode = pFactory->set_qos(fQos);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("CP: Failed to set default participant factory profile\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        // TODO: Improve this.The basic idea is that instead of creating
        //       from a profile, get the profile and then modify the profile
        //       based on other configurations parameters. This gives the best
        //       control
        retcode = pFactory->get_participant_qos_from_profile(dpQos, "NEURON", qosProfile);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to get NEURON::%s participant profile\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }
        // Ensure all DPs have a name
        //snprintf(dpQos.participant_name.name,255,"%s",name);
	dpQos.participant_name.name = DDS_String_dup(name);

        /*retcode = pFactory->get_publisher_qos_from_profile(pubQos, "NEURON", qosProfile);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to get NEURON::%s publisher profile\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }
        // Disable autostart on the pFactory to give better control in instantiations
        // of this class.
        pubQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;

        retcode = pFactory->get_subscriber_qos_from_profile(subQos, "NEURON", qosProfile);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to get NEURON::%s subscriber profile\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }
        subQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;*/

        // NOTE: Install listeners if needed. Currently all listeners are handled at the
        //       data-reader and data-writer level
        pDomainParticipant = pFactory->create_participant(domainId,
                                                          dpQos,
                                                          NULL,
                                                          DDS_STATUS_MASK_NONE);
        if (pDomainParticipant == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create domain participant with profile %s\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        /*// TODO: Install listeners if required. Currently all are handlded at
        //       the data-writer entity level
        pPublisher = pDomainParticipant->create_publisher(pubQos,
                                                            NULL,
                                                            DDS_STATUS_MASK_NONE);

        if (pPublisher == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create publisher with profile %s\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        // TODO: Install listeners
        pSubscriber = pDomainParticipant->create_subscriber(subQos,
                                                              NULL,
                                                              DDS_STATUS_MASK_NONE);

        if (pSubscriber == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create subscriber with profile %s\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        // Register all types using the canonical name, no real benefit of using a different
        // one
        type_name = ControlTypeSupport::get_type_name();
        retcode = ControlTypeSupport::register_type(pDomainParticipant,
                                                    type_name);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to register type %s\n",type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        // Use the type name as the topic name.
        // NOTE: There is a 256 char limit on each, so if the names are
        // longer this must be changed.
        topic = pDomainParticipant->create_topic(type_name,
                                                  type_name,
                                                  DDS_TOPIC_QOS_DEFAULT,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create topic %s of type %s\n",type_name,type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        type_name = EventTypeSupport::get_type_name();
        retcode = EventTypeSupport::register_type(pDomainParticipant,type_name);
        if (retcode != DDS_RETCODE_OK) {
            //TODO: Replace with real error logging
            ControlLogError("Failed to register type %s\n",type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = pDomainParticipant->create_topic(type_name,
                                                  type_name,
                                                  DDS_TOPIC_QOS_DEFAULT,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create topic %s of type %s\n",type_name,type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        type_name = StateTypeSupport::get_type_name();
        retcode = StateTypeSupport::register_type(pDomainParticipant, type_name);
        if (retcode != DDS_RETCODE_OK) {
            //TODO: Replace with real error logging
            ControlLogError("Failed to register type %s\n",type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = pDomainParticipant->create_topic(type_name,
                                                  type_name,
                                                  DDS_TOPIC_QOS_DEFAULT,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create topic %s of type %s\n",type_name,type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        type_name = MetricsTypeSupport::get_type_name();
        retcode = MetricsTypeSupport::register_type(pDomainParticipant,
                                                    type_name);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to register type %s\n",type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = pDomainParticipant->create_topic(type_name,
                                                  type_name,
                                                  DDS_TOPIC_QOS_DEFAULT,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create topic %s of type %s\n",type_name,type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }*/
    }

    bool AddPeer(const char *peer)
    {
        return (pDomainParticipant->add_peer(peer) == DDS_RETCODE_OK);
    }

    void Enable(const char *qosProfile)
    {
        DDS_PublisherQos pubQos;
        DDS_SubscriberQos subQos;
        DDS_ReturnCode_t retcode;
        DDSTopic *topic;
        const char *type_name;

        retcode = pDomainParticipant->enable();
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("CPInterfaceT::Enable(): Failed\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = pFactory->get_publisher_qos_from_profile(pubQos, "NEURON", qosProfile);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to get NEURON::%s publisher profile\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }
        // Disable autostart on the pFactory to give better control in instantiations
        // of this class.
        pubQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;

        retcode = pFactory->get_subscriber_qos_from_profile(subQos, "NEURON", qosProfile);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to get NEURON::%s subscriber profile\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }
        subQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;

        // TODO: Install listeners if required. Currently all are handlded at
        //       the data-writer entity level
        pPublisher = pDomainParticipant->create_publisher(pubQos,
                                                            NULL,
                                                            DDS_STATUS_MASK_NONE);

        if (pPublisher == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create publisher with profile %s\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        // TODO: Install listeners
        pSubscriber = pDomainParticipant->create_subscriber(subQos,
                                                              NULL,
                                                              DDS_STATUS_MASK_NONE);

        if (pSubscriber == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create subscriber with profile %s\n",qosProfile);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        // Register all types using the canonical name, no real benefit of using a different
        // one
        type_name = ControlTypeSupport::get_type_name();
        retcode = ControlTypeSupport::register_type(pDomainParticipant,
                                                    type_name);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to register type %s\n",type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        // Use the type name as the topic name.
        // NOTE: There is a 256 char limit on each, so if the names are
        // longer this must be changed.
        topic = pDomainParticipant->create_topic(type_name,
                                                  type_name,
                                                  DDS_TOPIC_QOS_DEFAULT,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create topic %s of type %s\n",type_name,type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        type_name = EventTypeSupport::get_type_name();
        retcode = EventTypeSupport::register_type(pDomainParticipant,type_name);
        if (retcode != DDS_RETCODE_OK) {
            //TODO: Replace with real error logging
            ControlLogError("Failed to register type %s\n",type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = pDomainParticipant->create_topic(type_name,
                                                  type_name,
                                                  DDS_TOPIC_QOS_DEFAULT,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create topic %s of type %s\n",type_name,type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        type_name = StateTypeSupport::get_type_name();
        retcode = StateTypeSupport::register_type(pDomainParticipant, type_name);
        if (retcode != DDS_RETCODE_OK) {
            //TODO: Replace with real error logging
            ControlLogError("Failed to register type %s\n",type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = pDomainParticipant->create_topic(type_name,
                                                  type_name,
                                                  DDS_TOPIC_QOS_DEFAULT,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create topic %s of type %s\n",type_name,type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        type_name = MetricsTypeSupport::get_type_name();
        retcode = MetricsTypeSupport::register_type(pDomainParticipant,
                                                    type_name);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to register type %s\n",type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = pDomainParticipant->create_topic(type_name,
                                                  type_name,
                                                  DDS_TOPIC_QOS_DEFAULT,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create topic %s of type %s\n",type_name,type_name);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        return;
    }

    void AddProperty(const char *name, const char *value)
    {
        DDS_ReturnCode_t retcode;
        DDS_DomainParticipantQos partQos;

        retcode = pDomainParticipant->get_qos(partQos);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("CP::AddProperty(): Failed to get participant profile\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = DDSPropertyQosPolicyHelper::add_property(partQos.property,name,value,DDS_BOOLEAN_FALSE);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("CP::AddProperty(): Failed to set participant property(%s,%s)\n",name,value);
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = pDomainParticipant->set_qos(partQos);
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("CP::AddProperty(): Failed to set participant profile\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        return;
    }

    ~CPInterfaceT()
    {
        DDS_ReturnCode_t retcode;
        if (pDomainParticipant != NULL)
        {
            retcode = pDomainParticipant->delete_contained_entities();
            if (retcode != DDS_RETCODE_OK)
            {
                ControlLogError("Failed to delete the participants contained entitiees\n");
                throw DDS_RETCODE_BAD_PARAMETER;
            }

            retcode = DDSTheParticipantFactory->delete_participant(pDomainParticipant);
            if (retcode != DDS_RETCODE_OK)
            {
                ControlLogError("Failed to delete the participant\n");
                throw DDS_RETCODE_BAD_PARAMETER;
            }
            // NOTE: We cannot finalize the DomainParticipantFacgtory instance since
            //       other participant may still be running
        }
    }

protected:
    DDSDomainParticipantFactory *pFactory;
    DDSDomainParticipant *pDomainParticipant;
    DDSPublisher *pPublisher;
    DDSSubscriber *pSubscriber;
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
    //CPMasterT(EventHandler *eh,int srcId,int domainId, const char *name,int appId, const char *qosProfile) : CPInterfaceT<ControlTypeSupport,
    CPMasterT(EventHandler *eh,int _srcId,int domainId, const char *name,int appId, const char *qosProfile) : CPInterfaceT<ControlTypeSupport,
                                    EventTypeSupport,
                                    StateTypeSupport,
                                    MetricsTypeSupport>(domainId,name,qosProfile),
                                    controlWriter(NULL),
                                    stateReader(NULL),
                                    eventReader(NULL),
                                    metricsReader(NULL)
    {
        DDSTopic *topic;
        DDS_ReturnCode_t retcode;
        DDSDataWriter *pDW;
        DDSDataReader *pDR;
        DDSTopicDescription *td;

        td = this->pDomainParticipant->lookup_topicdescription(ControlTypeSupport::get_type_name());

        if (td == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = DDSTopic::narrow(td);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow topic %s\n",td->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        //Need one profile for each Topic
        DDS_DataWriterQos dw_qos;
        retcode = this->pFactory->get_datawriter_qos_from_profile_w_topic_name(
                                            dw_qos,
                                            "NEURON",
                                            qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup data-writer profile %s for topic %s\n",
                            qosProfile,topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }
        // update any application specfic QoS here
        pDW = this->pPublisher->create_datawriter(topic,
                                            dw_qos,
                                            NULL,
                                            DDS_STATUS_MASK_NONE);


        if (pDW == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create data-writer for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        controlWriter = ControlDataWriter::narrow(pDW);
        if (controlWriter == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow data-writer for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }


        DDS_DataReaderQos dr_qos;
        td = this->pDomainParticipant->lookup_topicdescription(StateTypeSupport::get_type_name());

        if (td == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = DDSTopic::narrow(td);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow topic %s\n",td->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = this->pFactory->get_datareader_qos_from_profile_w_topic_name(
                                                dr_qos,
                                                "NEURON",
                                                qosProfile,
                                                topic->get_name());
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup data-reader profile %s for topic %s\n",
                            qosProfile,topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        pDR = this->pSubscriber->create_datareader(topic,
                                                   dr_qos,
                                                   NULL,
                                                   DDS_STATUS_MASK_NONE);
        if (pDR == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create data-reader for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        stateReader = StateDataReader::narrow(pDR);
        if (stateReader == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow data reader for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        td = this->pDomainParticipant->lookup_topicdescription(EventTypeSupport::get_type_name());

        if (td == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = DDSTopic::narrow(td);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow topic %s\n",td->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = this->pFactory->get_datareader_qos_from_profile_w_topic_name(
                                            dr_qos,
                                            "NEURON",
                                            qosProfile,
                                            topic->get_name());
        if (retcode != DDS_RETCODE_OK)
        {
            ControlLogError("Failed to lookup data-reader profile %s for topic %s\n",
                            qosProfile,topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        pDR = this->pSubscriber->create_datareader(topic,
                                                   dr_qos,
                                                   NULL,
                                                   DDS_STATUS_MASK_NONE);
        if (pDR == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create data-reader for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        eventReader = EventDataReader::narrow(pDR);
        if (eventReader == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow data reader for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        td = this->pDomainParticipant->lookup_topicdescription(MetricsTypeSupport::get_type_name());

        if (td == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = DDSTopic::narrow(td);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow topic %s\n",td->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = this->pFactory->get_datareader_qos_from_profile_w_topic_name(
                                                    dr_qos,
                                                    "NEURON",
                                                    qosProfile,
                                                    topic->get_name());
        if (retcode != DDS_RETCODE_OK)
        {
            ControlLogError("Failed to lookup data-reader profile %s for topic %s\n",
                            qosProfile,topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        pDR = this->pSubscriber->create_datareader(topic,
                                                   dr_qos,
                                                   NULL,
                                                   DDS_STATUS_MASK_NONE);
        if (pDR == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create data-reader for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        metricsReader = MetricsDataReader::narrow(pDR);
        if (metricsReader == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow data reader for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        state = StateTypeSupport::create_data();

        if (state == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed create state sample\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        event = EventTypeSupport::create_data();
        if (event == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed create event sample\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        metrics = MetricsTypeSupport::create_data();
        if (metrics == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed create metrics sample\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        control = ControlTypeSupport::create_data();
        if (control == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed create control sample\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        //srcId = srcId;
        srcId = _srcId;
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
            ControlLogError("Send control failed with error-code: \n",retcode);
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
            // TODO: Error handling
            ControlLogError("read_instance failed with error-code: %d\n",retcode);
            return false;
        }

        if (retcode == DDS_RETCODE_NO_DATA)
        {
            // TODO: Not a real error
            return false;
        }

        if ((data_seq.length() != 1) || !info_seq[0].valid_data)
        {
            ControlLogError("invalid dag-state %d/%d",data_seq.length(),!info_seq[0].valid_data);
            return false;
        }

        retcode = StateTypeSupport::copy_data(out_state,&data_seq[0]);
        if (retcode != DDS_RETCODE_OK)
        {
            // TODO: Add error handling
            ControlLogError("Failed to copy data %d",retcode);
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
            // TODO: Error handling
            ControlLogError("GetMasterObjectEvents read_instance failed with error-code: %d\n",retcode);
            return false;
        }

        if (retcode == DDS_RETCODE_NO_DATA)
        {
            return false;
        }

        // Copy the data
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
            // TODO: Error handling
            ControlLogError("GetMasterObjectMetrics failed with error-code: %d\n",retcode);
            return false;
        }

        if (retcode == DDS_RETCODE_NO_DATA)
        {
          return true;
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
    CPSlaveT(EventHandler *q,int _srcId,int domainId, const char *name, int appId, const char *qosProfile) : CPInterfaceT<ControlTypeSupport,EventTypeSupport,StateTypeSupport,MetricsTypeSupport>(domainId,name,qosProfile),
    controlReader(NULL),
    stateWriter(NULL),
    eventWriter(NULL),
    metricsWriter(NULL)
    {
        DDSTopic *topic;
        DDS_ReturnCode_t retcode;
        DDSContentFilteredTopic *cft;
        char expression[1024];
        DDS_StringSeq params;
        DDSDataWriter *pDW;
        DDSDataReader *pDR;
        DDSTopicDescription *td;

        // TODO: Register Types with DP
        td = this->pDomainParticipant->lookup_topicdescription(ControlTypeSupport::get_type_name());

        if (td == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = DDSTopic::narrow(td);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        //Need one profile for each Topic
        DDS_DataReaderQos dr_qos;
        retcode = this->pFactory->get_datareader_qos_from_profile_w_topic_name(
                                                dr_qos,
                                                "NEURON",
                                                qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup data-reader profile %s for topic %s\n",
                            qosProfile,topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        // update any application specfic QoS here
        sprintf(expression,"dstId = %d",appId);
        cft = this->pDomainParticipant->create_contentfilteredtopic(expression,
                                                                    topic,
                                                                    expression,
                                                                    params);
        if (cft == NULL) {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create content-filtered topic %s with expression [%s]\n",
                            expression,expression);
            throw DDS_RETCODE_BAD_PARAMETER;
        }
        pDR = this->pSubscriber->create_datareader(cft,
                                                   dr_qos,
                                                   NULL,
                                                   DDS_STATUS_MASK_NONE);


        controlReader = ControlDataReader::narrow(pDR);
        if (controlReader == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create controlReader\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        DDS_DataWriterQos dw_qos;
        td = this->pDomainParticipant->lookup_topicdescription(StateTypeSupport::get_type_name());

        if (td == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = DDSTopic::narrow(td);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = this->pFactory->get_datawriter_qos_from_profile_w_topic_name(
                                                        dw_qos,
                                                        "NEURON",
                                                        qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup data-writer profile %s for topic %s\n",
                            qosProfile,topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        pDW = this->pPublisher->create_datawriter(topic,
                                                  dw_qos,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (pDW == NULL)
        {
            ControlLogError("Failed to create data-writer for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        stateWriter = StateDataWriter::narrow(pDW);
        if (stateWriter == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow data-writer for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        td = this->pDomainParticipant->lookup_topicdescription(EventTypeSupport::get_type_name());

        if (td == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = DDSTopic::narrow(td);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = this->pFactory->get_datawriter_qos_from_profile_w_topic_name(dw_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup data-writer profile %s for topic %s\n",
                            qosProfile,topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        pDW = this->pPublisher->create_datawriter(topic, dw_qos, NULL, DDS_STATUS_MASK_NONE);
        if (pDW == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create data-writer for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        eventWriter = EventDataWriter::narrow(pDW);
        if (eventWriter == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow data-writer for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        td = this->pDomainParticipant->lookup_topicdescription(MetricsTypeSupport::get_type_name());

        if (td == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        topic = DDSTopic::narrow(td);
        if (topic == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        retcode = this->pFactory->get_datawriter_qos_from_profile_w_topic_name(dw_qos, "NEURON", qosProfile,topic->get_name());
        if (retcode != DDS_RETCODE_OK)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to lookup data-writer profile %s for topic %s\n",
                            qosProfile,topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        pDW = this->pPublisher->create_datawriter(topic,
                                                  dw_qos,
                                                  NULL,
                                                  DDS_STATUS_MASK_NONE);
        if (pDW == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to create data-writer for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        metricsWriter = MetricsDataWriter::narrow(pDW);
        if (metricsWriter == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed to narrow data-writer for topic %s\n",topic->get_name());
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        state = StateTypeSupport::create_data();
        if (state == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed create state sample\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        event = EventTypeSupport::create_data();
        if (event == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed create event sample\n");
            throw DDS_RETCODE_BAD_PARAMETER;
        }

        metrics = MetricsTypeSupport::create_data();
        if (metrics == NULL)
        {
            //TODO: Replace with real error logging
            ControlLogError("Failed metrics state sample\n");
            throw DDS_RETCODE_BAD_PARAMETER;
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
            // TODO: Error handling
            ControlLogError("Send state failed with error-code: \n",retcode);
            return false;
        }
        return true;
    }

    bool Send(EventT *event, DDS_InstanceHandle_t ih)
    {
        DDS_ReturnCode_t retcode;
        retcode = eventWriter->write(*event, ih);
        if (retcode != DDS_RETCODE_OK)
        {
            ControlLogError("Send event failed with error-code: \n",retcode);
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
            // TODO: Error handling
            ControlLogError("Send metrics failed with error-code: \n",retcode);
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

