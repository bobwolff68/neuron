//
//  CameraInterrogation.h
//  
//
//  Created by Robert Wolff on 8/22/11.
//  Copyright 2011 XVD Technology LTD USA. All rights reserved.
//

#import <QTKit/QTkit.h>
#import <Foundation/Foundation.h>

#define MAX_REQUESTS 50

struct trial {
    int width;
    int height;
};

@interface CameraInterrogation : NSObject
{ 
    QTCaptureDevice                  *videoDevice;
    QTCaptureSession                 *mCaptureSession;
    QTCaptureDecompressedVideoOutput *mCaptureDecompressedVideoOutput;
    QTCaptureDeviceInput             *mCaptureVideoDeviceInput;
    
    // Try 4:3 resolutions first. Then 16:9
    // 4:3  -- 80x60, 160x120, 240x192, 320x240, 640x480, 800x600, 1024x768, 1280x1024, 1600x1200
    // 16:9 -- 160x90, 240x136, 320x180, 640x360, 800x448, 1024x576, 1280x720, 1600x900, 1920x1080
    struct trial requested[MAX_REQUESTS];
    struct trial received[MAX_REQUESTS];

    int numResolutions;
    bool bIsRunning;
    int trialIteration;
    bool bSetSize;
}

- (void)FullInternalInterrogation;
- (void)InterrogateCurrentResolutions;
- (void)setupCapture;
- (void)startRunning;
- (void)stopRunning;
- (void)addResolutionWithWidth:(int)w withHeight:(int)h;
- (void)reset;
- (bool)getBestCaptureResolutionForWidth:(int)w forHeight:(int)h
                     forOutWidth:(int*)ow forOutHeight:(int*)oh;

@end
