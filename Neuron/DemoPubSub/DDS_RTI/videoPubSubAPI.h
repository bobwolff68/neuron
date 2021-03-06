#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_NAME_LEN	256
extern char	srcNameList[100][MAX_NAME_LEN];
extern char	srcVidStats[100][50];
extern int	srcNameListLen;

//APIs
void	NVPStartup				(const char *name, const char *vidStats);
void	NVPPublishFrame			(unsigned char *pFrameBuf,int bufLen,int lType);
void	NVPDestroy				(void);
void	NVSStartup				(const char *name, bool bMulticast);
void	NVSSetupFrameListener	(void);
void	NVSSetVDSPartition		(const char *partitionName);
void	NVSChangeVDSPartition	(char fpsChoice);
void	NVSDestroy				(void);

#ifdef __cplusplus
}
#endif

