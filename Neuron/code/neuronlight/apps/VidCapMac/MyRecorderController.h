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

OSStatus iConverter (
                     AudioConverterRef             inAudioConverter,
                     UInt32                        *ioNumberDataPackets,
                     AudioBufferList               *ioData,
                     AudioStreamPacketDescription  **outDataPacketDescription,
                     void                          *inUserData
                     );

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
    unsigned char* pAudioInputResidualData;
    int audioInputResidualBytesProcessed;
    int audioInputResidualTotalLength;
    
    char* pAudioInputPostConversionData;
    int audioInputPostConversionSize;
    int audioInputPostConversionNumPackets;
    AudioBufferList audioInputPostConversionBufferList;
    struct timeval audioInputResidualTV;
    
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

- (void)updateUINow:(NSTimer*)timer;

- (IBAction)startRecording:(id)sender;
- (IBAction)stopRecording:(id)sender;
- (IBAction)quitApplication:(id)sender;
- (IBAction)sendAudioRawData:(id)sender;
- (IBAction)sendAudioNoData:(id)sender;
- (IBAction)videoBitrateChanged:(id)sender;
- (IBAction)videoFramerateChanged:(id)sender;
- (IBAction)captureResolution:(id)sender;

@end
