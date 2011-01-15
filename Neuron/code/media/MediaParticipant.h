#ifndef MEDIAPARTICIPANT_H_
#define MEDIAPARTICIPANT_H_

#include <map>
#include <string>
#include <sstream>
#include <stdio.h>
#include "media.h"
#include "mediaSupport.h"
#include "neuroncommon.h"

using namespace std;

class MediaParticipant
{
    class BuiltinListener : public DDSDataReaderListener
    {
        private:

            int sessionId;

        public:

            BuiltinListener(int sessionIdP)
            {
                sessionId = sessionIdP;
            }

            void on_data_available(DDSDataReader *pGenReader)
            {
                DDSParticipantBuiltinTopicDataDataReader   *pDiscReader = NULL;
                DDS_ParticipantBuiltinTopicDataSeq          seqDisc;
                DDS_SampleInfoSeq                           seqInfo;
                DDS_ReturnCode_t                            retCode;

                pDiscReader =  DDSParticipantBuiltinTopicDataDataReader::narrow(pGenReader);
                retCode = pDiscReader->take(seqDisc,seqInfo,DDS_LENGTH_UNLIMITED,DDS_ANY_SAMPLE_STATE,
                                            DDS_ANY_VIEW_STATE,DDS_ANY_INSTANCE_STATE);
                if(retCode!=DDS_RETCODE_NO_DATA)
                {
                    if(retCode!=DDS_RETCODE_OK)
                    {
                        cout << "BuiltinListener::on_data_available(): Can't read discovery data" << endl;
                        return;
                    }
                    else
                    {
                        for(int i=0; i<seqDisc.length(); i++)
                        {
                            if(seqInfo[i].valid_data)
                            {
                                int                     discSessionId;
                                DDS_Property_t         *pSessIdProperty = NULL;
                                DDSSubscriber          *pSubscriber = NULL;
                                DDSDomainParticipant   *pDomParticipant = NULL;

                                pSubscriber = pGenReader->get_subscriber();
                                pDomParticipant = pSubscriber->get_participant();

                                cout << "Discovered participant: " << seqDisc[i].participant_name.name << endl;

                                pSessIdProperty = DDSPropertyQosPolicyHelper::lookup_property(seqDisc[i].property,"sessionId");
                                if(pSessIdProperty!=NULL)
                                {
                                    cout << "Session ID: " << pSessIdProperty->value << ",Current: " << sessionId << endl;
                                    sscanf(pSessIdProperty->value,"%d",&discSessionId);
                                    if(discSessionId!=sessionId)
                                    {
                                        cout << "Different session from current one, ignoring..." << endl;
                                        retCode = pDomParticipant->ignore_participant(seqInfo[i].instance_handle);
                                        if(retCode!=DDS_RETCODE_OK)
                                        {
                                            cout << "BuiltinListener::on_data_available(): Ignore error" << endl;
                                            exit(0);
                                        }
                                    }
                                }
                                else
                                {
                                    //Ignore control plane participants
                                    stringstream    sstream;
                                    string          PartName;
                                    char            buf[100];

                                    sstream << seqDisc[i].participant_name.name;
                                    sstream.getline(buf,99,':');
                                    sstream.getline(buf,99,':');
                                    sstream.getline(buf,99,':');
                                    PartName = buf;

                                    if(PartName=="ACPMaster"||PartName=="ACPSlave"||PartName=="SCPMaster"||
                                       PartName=="SCPSlave"||PartName=="LSCPMaster"||PartName=="LSCPSlave"||
                                       PartName=="ACP")
                                    {
                                        cout << "Ignoring control plane participant "
                                             << seqDisc[i].participant_name.name << "..." << endl;
                                        retCode = pDomParticipant->ignore_participant(seqInfo[i].instance_handle);
                                        if(retCode!=DDS_RETCODE_OK)
                                        {
                                            cout << "BuiltinListener::on_data_available(): Ignore error" << endl;
                                            exit(0);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                pDiscReader->return_loan(seqDisc,seqInfo);

                return;
            }
    };

    private:

        int                             sessionId;
        DDSDomainParticipantFactory    *pPartFactory;
        DDSDomainParticipant           *pDomParticipant;
        map<string,DDSTopic*>           Topics;

        void SetParticipantFactoryAutoEnableEntities(DDS_Boolean val)
        {
            DDS_ReturnCode_t                retCode;
            DDS_DomainParticipantFactoryQos factQos;

            retCode = pPartFactory->get_qos(factQos);
            if(retCode!=DDS_RETCODE_OK)
            {
                cout << "MediaParticipant(): Can't get factory qos" << endl;
                exit(0);
            }
            factQos.entity_factory.autoenable_created_entities = val;
            retCode = pPartFactory->set_qos(factQos);
            if(retCode!=DDS_RETCODE_OK)
            {
                cout << "MediaParticipant(): Can't set factory qos" << endl;
                exit(0);
            }
        }

        void ConfigDiscovery(DDS_DomainParticipantQos& partQos)
        {
            DDS_ReturnCode_t    retCode;

            retCode = DDSPropertyQosPolicyHelper::add_property(partQos.property,"sessionId",
                                                               ToString<int>(sessionId).c_str(),
                                                               DDS_BOOLEAN_TRUE);
            if(retCode!=DDS_RETCODE_OK)
            {
                cout << "MediaParticipant(): Can't add property (sessionId,<id>)" << endl;
                exit(0);
            }

            return;
        }

    public:

        MediaParticipant(int domainId,int sessionId,const char *sessionName)
        {
            string                                      PartName(sessionName);
            const char                                 *typeName;
            BuiltinListener                            *pDiscListener = NULL;
            DDSSubscriber                              *pDiscSubscriber = NULL;
            DDSDataReader                              *pDiscGenReader = NULL;
            DDS_ReturnCode_t                            retCode;
            DDS_DomainParticipantQos                    partQos;
            DDSParticipantBuiltinTopicDataDataReader   *pDiscReader = NULL;

            this->sessionId = sessionId;
            pDomParticipant = NULL;
            PartName += "::Media";

            pPartFactory = DDSDomainParticipantFactory::get_instance();

            //Disable autenable of domain particioants
            SetParticipantFactoryAutoEnableEntities(DDS_BOOLEAN_FALSE);

            //Get domain participant qos
            retCode = pPartFactory->get_participant_qos_from_profile(partQos,"NEURON","MEDIA");
            if(retCode!=DDS_RETCODE_OK)
            {
                cout << "MediaParticipant(): Can't get default participant qos" << endl;
                exit(0);
            }

            //Set participant name
            partQos.participant_name.name = DDS_String_dup(PartName.c_str());

            //Discovery data
            ConfigDiscovery(partQos);

            //Create domain participant
            pDomParticipant = pPartFactory->create_participant(domainId,partQos,NULL,DDS_STATUS_MASK_NONE);
            if(pDomParticipant==NULL)
            {
                cout << "MediaParticipant(): Can't create participant" << endl;
                exit(0);
            }

            //Attach listener to builtin reader
            pDiscSubscriber = pDomParticipant->get_builtin_subscriber();
            pDiscGenReader = pDiscSubscriber->lookup_datareader(DDS_PARTICIPANT_TOPIC_NAME);
            pDiscReader = DDSParticipantBuiltinTopicDataDataReader::narrow(pDiscGenReader);
            pDiscListener = new BuiltinListener(sessionId);
            pDiscReader->set_listener(pDiscListener,DDS_DATA_AVAILABLE_STATUS);

            //Enable participant
            retCode = pDomParticipant->enable();
            if(retCode!=DDS_RETCODE_OK)
            {
                cout << "MediaParticipant(): Can't enable participant" << endl;
                exit(0);
            }

            //Register type name(s)
            typeName = com::xvd::neuron::media::DataUnitTypeSupport::get_type_name();
            retCode = com::xvd::neuron::media::DataUnitTypeSupport::register_type(pDomParticipant,typeName);
            if(retCode!=DDS_RETCODE_OK)
            {
                cout << "MediaParticipant(): Can't register type '" << typeName << "'" << endl;
                pPartFactory->delete_participant(pDomParticipant);
                exit(0);
            }
        }

        ~MediaParticipant()
        {
            DDS_ReturnCode_t    retCode;

            for(map<string,DDSTopic*>::iterator it=Topics.begin(); it!=Topics.end(); it++)
            {
                retCode = pDomParticipant->delete_topic(it->second);
                if(retCode!=DDS_RETCODE_OK)
                {
                    cout << "~MediaParticipant(): Can't delete topic of media type '" << it->first << "'" << endl;
                    exit(0);
                }
            }

            Topics.clear();
            retCode = pPartFactory->delete_participant(pDomParticipant);
            if(retCode!=DDS_RETCODE_OK)
            {
                cout << "~MediaParticipant(): Can't delete participant" << endl;
                exit(0);
            }
        }

        void AddTopic(const char *mediaType,const char *topicName)
        {
            string MediaType(mediaType);

            Topics[MediaType] = NULL;
            Topics[MediaType] = pDomParticipant->create_topic(topicName,
                                com::xvd::neuron::media::DataUnitTypeSupport::get_type_name(),
                                DDS_TOPIC_QOS_DEFAULT,NULL,DDS_STATUS_MASK_NONE);
            if(Topics[MediaType]==NULL)
            {
                cout << "MediaParticipant::AddTopic(): Can't add topic '" << topicName << "'" << endl;
                exit(0);
            }
        }

inline  DDSDomainParticipant *GetDomParticipant(void)
        {
            return pDomParticipant;
        }

inline  DDSTopic *GetTopic(const char *mediaType)
        {
            string MediaType(mediaType);

            if(Topics.find(MediaType)!=Topics.end())
                return Topics[MediaType];

            return NULL;
        }
};

#endif // MEDIAPARTICIPANT_H_
