//
//  MyRecorderController.h
//  MyRecorder

#import <Cocoa/Cocoa.h>
#import <QTKit/QTkit.h>
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

#import "QTKitCap.h"
#import "cpp_main.h"
#import "SafeBufferDeque.h"

// Our httpd daemon is partially controlled and utilized by MyRecorderController among others.
#import "sessionmanager.h"

OSStatus iConverter (
                     AudioConverterRef             inAudioConverter,
                     UInt32                        *ioNumberDataPackets,
                     AudioBufferList               *ioData,
                     AudioStreamPacketDescription  **outDataPacketDescription,
                     void                          *inUserData
                     );

class ExtractConverted;

@interface MyRecorderController : NSObject {
    
    IBOutlet QTCaptureView *mCaptureView;
    IBOutlet NSView *mMainWindow;
    
    IBOutlet NSTextField *Vkbps;
    IBOutlet NSTextField *Vfps;
    
    IBOutlet NSComboBox *captureResBox;
    IBOutlet NSButton *captureButton;
    IBOutlet NSButton *stopCaptureButton;
    
    IBOutlet NSTextField *mDrops;
    int curDrops;
    
    QTCaptureDevice                  *videoDevice;
    QTCaptureSession                 *mCaptureSession;
    QTCaptureDecompressedVideoOutput *mCaptureDecompressedVideoOutput;
    QTCaptureDecompressedAudioOutput *mCaptureDecompressedAudioOutput;
    AudioConverterRef                Converter;
    QTCaptureDeviceInput             *mCaptureVideoDeviceInput;
    QTCaptureDeviceInput             *mCaptureAudioDeviceInput;
    
    // Conversion support items.
    SafeBufferDeque* pAudioInputQueue;
    int mAudioInputNumChannels;
    int mAudioInputIndividualSampleSize;
    int audioInputOutBufSize;
    unsigned char* pAudioInputOutBuf;
//
    AudioBufferList audioInputPostConversionBufferList;
    struct timeval audioInputResidualTV;
    unsigned char* pAudioInputResidualData;
    int audioInputResidualBytesProcessed;
    int audioInputResidualTotalLength;
//
    ExtractConverted *pExtractor;
    
    char* pAudioInputPostConversionData;
    int audioInputPostConversionSize;
    int audioInputPostConversionNumPackets;
    
    SessionManager* sm;
    AudioDeviceID m_InputDeviceId;
    bool bIsMicMuted;

    bool bIsCapturing;
    RunPipeline* p_pipeline_runner;
    QTKitCap* pCap;
    TVidCap* pTVC;
    NSInteger sendAudioType;
    
    NSTimer* timerUIUpdate;
    
    int outputWidth, outputHeight;      // Final image size for encoding.
    int captureWidth, captureHeight;    // The camera's current device capture value (actual)
    NSTextField *videoFramerateChanged;
    
    NSMutableArray* pCameraResolutions;
}

- (void)applicationWillTerminate:(NSNotification *)aNotification;

- (OSStatus) iConvertSupplyData:  (AudioBufferList *)data withNumPackets:(UInt32 *)numberDataPackets;
- (int)parseWidthHeight:(NSString*)pStr widthOut:(int*)pWidth heightOut:(int*)pHeight;
- (int)setDisplayResolutionWidth:(int)dWidth withHeight:(int)dHeight;
- (int) CaptureMatchForDesiredWidth:(int)desWidth forDesiredHeight:(int)desHeight
                      CaptureWidth:(int *)pCaptureWidth CaptureHeight:(int *)pCaptureHeight;
- (void)ExtractConvertedData;

- (void) updateUINow:(NSTimer*)timer;
- (void) smCallback:(SessionManager*) psm;
- (void) openDefaultMicrophoneInput;
- (void) setMicVolume:(int) vol100;
- (int)  getMicVolume;
- (void) setMicMuteToggle;
- (bool) getIsMicMuted;

- (IBAction)startRecording:(id)sender;
- (IBAction)stopRecording:(id)sender;
- (IBAction)quitApplication:(id)sender;
- (IBAction)sendAudioRawData:(id)sender;
- (IBAction)sendAudioNoData:(id)sender;
- (IBAction)videoBitrateChanged:(id)sender;
- (IBAction)videoFramerateChanged:(id)sender;
- (IBAction)captureResolution:(id)sender;

@end

void smCallbackGlobal(void* pData, SessionManager* psm);

class ExtractConverted : public ThreadSingle {
public:
    ExtractConverted(MyRecorderController* pCtrl)
    {
        assert(pCtrl);
        pController = pCtrl;
        
        bConvert = false;
        
        startThread();
    };
    ~ExtractConverted(void) { pause(); };
    void start() { bConvert = true; };
    void pause() { bConvert = false; };
protected:
    int workerBee(void);
    MyRecorderController* pController;
    bool bConvert;
    AudioBufferList audioInputPostConversionBufferList;
    struct timeval audioInputResidualTV;
    unsigned char* pAudioInputResidualData;
    int audioInputResidualBytesProcessed;
    int audioInputResidualTotalLength;
};
