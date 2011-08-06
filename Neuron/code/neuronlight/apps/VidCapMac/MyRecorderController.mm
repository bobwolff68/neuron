//
//  MyRecorderController.m
//  MyRecorder

/*
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
 Apple Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Inc. 
 may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2007-2008 Apple Inc. All Rights Reserved.  
 
 */

#import "MyRecorderController.h"
#import "CoreVideo/CVPixelBuffer.h"

@implementation MyRecorderController

- (void)awakeFromNib
{
    
    curDrops = 0;
    
// Create the capture session
    
	mCaptureSession = [[QTCaptureSession alloc] init];
    
// Connect inputs and outputs to the session	
    
	BOOL success = NO;
	NSError *error;
	
// Find a video device  
    
    QTCaptureDevice *videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
    success = [videoDevice open:&error];
    
    
// If a video input device can't be found or opened, try to find and open a muxed input device
    
	if (!success) {
		videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeMuxed];
		success = [videoDevice open:&error];
		
    }
    
    if (!success) {
        videoDevice = nil;
        // Handle error
        
    }
    
    if (videoDevice) {
//Add the video device to the session as a device input
		
		mCaptureVideoDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:videoDevice];
		success = [mCaptureSession addInput:mCaptureVideoDeviceInput error:&error];
		if (!success) {
			// Handle error
		}
        
// If the video device doesn't also supply audio, add an audio device input to the session
        
        if (![videoDevice hasMediaType:QTMediaTypeSound] && ![videoDevice hasMediaType:QTMediaTypeMuxed]) {
            
            QTCaptureDevice *audioDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeSound];
            success = [audioDevice open:&error];
            
            if (!success) {
                audioDevice = nil;
                // Handle error
            }
            
            if (audioDevice) {
                mCaptureAudioDeviceInput = [[QTCaptureDeviceInput alloc] initWithDevice:audioDevice];
                
                success = [mCaptureSession addInput:mCaptureAudioDeviceInput error:&error];
                if (!success) {
                    // Handle error
                }
            }
        }
        
#if 1
        mCaptureDecompressedVideoOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
        
        [mCaptureDecompressedVideoOutput setDelegate:self];
        
// Set output characteristics. Especially video pixel format data.
        [mCaptureDecompressedVideoOutput setAutomaticallyDropsLateVideoFrames:YES];
        [mCaptureDecompressedVideoOutput setMinimumVideoFrameInterval:1/(float)30];
        [mCaptureDecompressedVideoOutput setPixelBufferAttributes:
            [NSDictionary dictionaryWithObjectsAndKeys:
             [NSNumber numberWithDouble:640], (id)kCVPixelBufferWidthKey,
             [NSNumber numberWithDouble:360], (id)kCVPixelBufferHeightKey,
//             [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32ARGB/*kCVPixelFormatType_422YpCbCr10*/], (id)kCVPixelBufferPixelFormatTypeKey,
             nil]];

        // Finally add the output to the session.
        success = [mCaptureSession addOutput:mCaptureDecompressedVideoOutput error:&error];
        if (!success) {
            // Handle error
        }
#endif
	} 

// Associate the capture view in the UI with the session
        
    [mCaptureView setCaptureSession:mCaptureSession];

    pCap = new QTKitCap(mCaptureSession, mCaptureDecompressedVideoOutput);
//Not defaulting to capture -- let use press it.    pCap->start_capturing();    
//        [mCaptureSession startRunning];
    
    //TODO Here's where we inform Manjesh's code that we have a VidCap for him.
    // setQTKitCap(pCap);
        
}
    
// Handle window closing notifications for your device input

- (void)windowWillClose:(NSNotification *)notification
{
	
//	[mCaptureSession stopRunning];
    
	pCap->stop_capturing();
	pCap->clear();		// Force buffer cleared.

    if ([[mCaptureVideoDeviceInput device] isOpen])
        [[mCaptureVideoDeviceInput device] close];
    
    if ([[mCaptureAudioDeviceInput device] isOpen])
        [[mCaptureAudioDeviceInput device] close];
    
}

// Handle deallocation of memory for your capture objects

- (void)dealloc
{
    delete pCap;
    pCap = NULL;
    
	[mCaptureSession release];
	[mCaptureVideoDeviceInput release];
    [mCaptureAudioDeviceInput release];
	
	[super dealloc];
}

#pragma mark-

- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    static int storedWidth=0;
    static int storedHeight=0;
    int frameWidth;
    int frameHeight;
    OSType pixType;
    QTKitBufferInfo BI;

//    PlanarComponentInfo* pinfo;
//    struct PlanarComponentInfo {
//        SInt32              offset;
//        UInt32              rowBytes;
//    };
//    typedef struct PlanarComponentInfo      PlanarComponentInfo;
//    struct PlanarPixmapInfoYUV420 {
//        PlanarComponentInfo  componentInfoY;
//        PlanarComponentInfo  componentInfoCb;
//        PlanarComponentInfo  componentInfoCr;
//    };
//    typedef struct PlanarPixmapInfoYUV420   PlanarPixmapInfoYUV420;

    // get size of the frame
    CVPixelBufferLockBaseAddress(videoFrame, 0); //LOCK_FLAGS);

    BI.pBuffer = CVPixelBufferGetBaseAddressOfPlane(videoFrame, 0);
    //void* baseAddress = CVPixelBufferGetBaseAddress(videoFrame);
//    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(videoFrame);
    frameWidth = CVPixelBufferGetWidth(videoFrame);
    frameHeight = CVPixelBufferGetHeight(videoFrame);
    pixType = CVPixelBufferGetPixelFormatType(videoFrame);

    CVPixelBufferUnlockBaseAddress(videoFrame, 0); // LOCK_FLAGS);

    assert(pixType==kCVPixelFormatType_422YpCbCr8); // Make sure '2vuy' is active. Eventually we'll allow for conversions etc, but for now we need AGREEMENT.
    
    NSDictionary *pDict;
    pDict = [mCaptureDecompressedVideoOutput pixelBufferAttributes];
    
    if (storedWidth != frameWidth || storedHeight != frameHeight)
    {
        storedWidth = frameWidth;
        storedHeight = frameHeight;
        
        NSLog(@"Capture CHANGE: WxH=%dx%d pixType=%c%c%c%c",frameWidth, frameHeight, (pixType>>24)&0xff, (pixType>>16)&0xff, (pixType>>8)&0xff, pixType&0xff);
    }
    
    // Now down to the business at hand. Enqueue the new frame.
    
    BI.pFrame = videoFrame;

    // Counting on FullBufferEnQ() to lock down the videoFrame for us.
    // Only did this for symmetry of responsibility between enque and dq
    if (!pCap->GetBufferPointer()->FullBufferEnQ(BI))
    {
//        NSLog(@"Enqueue failed above. Marking as dropped.");
        
        curDrops++;
        [mDrops setIntValue:curDrops];
    }

}

- (void)captureOutput:(QTCaptureOutput *)captureOutput didDropVideoFrameWithSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    curDrops++;
    [mDrops setIntValue:curDrops];
}

// Add these start and stop recording actions, and specify the output destination for your recorded media. The output is a QuickTime movie.

- (IBAction)startRecording:(id)sender
{
//	[mCaptureSession startRunning];
    pCap->start_capturing();
}

- (IBAction)stopRecording:(id)sender
{
//    [mCaptureSession stopRunning];
    pCap->stop_capturing();
}

- (IBAction)captureMode:(id)sender {
    bool isHD;
    
    isHD = [sender state]==NSOnState;
    
    if (isHD)
    {
        [mCaptureDecompressedVideoOutput setPixelBufferAttributes:
         [NSDictionary dictionaryWithObjectsAndKeys:
                       [NSNumber numberWithDouble:1280], (id)kCVPixelBufferWidthKey,
                       [NSNumber numberWithDouble:720], (id)kCVPixelBufferHeightKey,
          //             [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32ARGB/*kCVPixelFormatType_422YpCbCr10*/], (id)kCVPixelBufferPixelFormatTypeKey,
          nil]];
        
        NSLog(@"Using HD mode 720p.");
    }
    else
    {
        [mCaptureDecompressedVideoOutput setPixelBufferAttributes:
         [NSDictionary dictionaryWithObjectsAndKeys:
                       [NSNumber numberWithDouble:640], (id)kCVPixelBufferWidthKey,
                       [NSNumber numberWithDouble:360], (id)kCVPixelBufferHeightKey,
          //             [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32ARGB/*kCVPixelFormatType_422YpCbCr10*/], (id)kCVPixelBufferPixelFormatTypeKey,
          nil]];

        NSLog(@"Using SD mode 360p.");
    }
}

- (IBAction)resetDrops:(id)sender {
    curDrops=0;
    [mDrops setIntValue:curDrops];
}

- (IBAction)quitApplication:(id)sender {
    [NSApp terminate:self];
}

@end
