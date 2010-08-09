#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_NAME_LEN	256
extern char	srcNameList[100][MAX_NAME_LEN];
extern int	srcNameListLen;

//APIs
void	NVPStartup			(const char *name);
void	NVPSetThrotModePtr	(int *pThrotMode);
void	NVPSetVDPPartition	(void);
void	NVPPublishFrame		(unsigned char *pFrameBuf,int bufLen,int lType);
void	NVPDestroy			(void);
void	NVSStartup			(const char *name);
int		NVSGetVidSrcList	(const char ***pppVidSrcList);
void	NVSSetTMPPartition	(const char *partitionName);
void	NVSPublishThrotMsg	(int modeVal);
void	NVSGetFrame			(unsigned char **ppFrameBuf,int *pBufLen,char fpsChoice);
void	NVPDestroy			(void);

#ifdef __cplusplus
}
#endif

