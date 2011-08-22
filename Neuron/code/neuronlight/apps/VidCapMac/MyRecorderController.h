//
//  MyRecorderController.h
//  MyRecorder

#import <Cocoa/Cocoa.h>
#import <QTKit/QTkit.h>
#import <Foundation/Foundation.h>
#import <AudioToolbox/AudioToolbox.h>

#import "QTKitCap.h"
#import "cpp_main.h"


@interface MyRecorderController : NSObject {
    
    IBOutlet QTCaptureView *mCaptureView;
    IBOutlet NSView *mMainWindow;
    IBOutlet NSTextField *mDrops;
    int curDrops;
    
    QTCaptureDevice                  *videoDevice;
    QTCaptureSession                 *mCaptureSession;
    QTCaptureDecompressedVideoOutput *mCaptureDecompressedVideoOutput;
    QTCaptureDecompressedAudioOutput *mCaptureDecompressedAudioOutput;
    AudioConverterRef                Converter;
    QTCaptureDeviceInput             *mCaptureVideoDeviceInput;
    QTCaptureDeviceInput             *mCaptureAudioDeviceInput;
    
    RunPipeline* p_pipeline_runner;
    QTKitCap* pCap;
    TVidCap* pTVC;
    NSInteger sendAudioType;
    
    int outputWidth, outputHeight;      // Final image size for encoding.
    int captureWidth, captureHeight;    // The camera's current device capture value (actual)
}

- (IBAction)startRecording:(id)sender;
- (IBAction)stopRecording:(id)sender;
- (IBAction)captureMode:(id)sender;
- (IBAction)resetDrops:(id)sender;
- (IBAction)quitApplication:(id)sender;
- (IBAction)sendAudioRawData:(id)sender;
- (IBAction)sendAudioNoData:(id)sender;

@end
