#define MSG_SIZE_MAX_BYTES_STR		"65535"
#define MONITOR_DP_DOMAIN_ID_STR	"100"

int participant_qos_with_monitoring_enabled(DDS_DomainParticipantQos& partqos);
void DDS_DomainParticipantQos_setup_udpv4_message_size_max(DDS_DomainParticipantQos &dpQos,const char *msg_size_max_str);
DDS_DomainParticipantQos DPQos_with_UDPWAN(const char *stunLocator,int wanID,const char *stunLivePeriodStr,
										   const char *stunRetranIntvlStr, const char *stunNumRetransStr,
										   bool bEnableMonitor);
DDS_DomainParticipantQos DPQos_with_TCPLAN(int bindPort,bool bEnableMonitor);

