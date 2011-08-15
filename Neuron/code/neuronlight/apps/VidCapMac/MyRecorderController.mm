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

// Helper function to convert QTTime value into raw micro-seconds.
#define QTT_US(Q) ((1000000*Q.timeValue)/Q.timeScale)

@implementation MyRecorderController

- (void)awakeFromNib
{    
    curDrops = 0;
    sendAudioType=2; // Using RAW-Data buffer mode by default now.
    
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
	} 

    // Setup for raw audio capture interleaved in the output.
    mCaptureDecompressedAudioOutput = [[QTCaptureDecompressedAudioOutput alloc] init];
    [mCaptureDecompressedAudioOutput setDelegate:self];
    
    // Finally add the AUDIO output to the session.
    //TODO - Must figure out how to change sample-rate - may not want 44.1kHz as default.
    
    // Finally add the output to the session.
    success = [mCaptureSession addOutput:mCaptureDecompressedAudioOutput error:&error];
    if (!success) {
        // Handle error
    }

    // Associate the capture view in the UI with the session
    
    [mCaptureView setCaptureSession:mCaptureSession];

    // Prepping to capture and send frames to pCap.
    pCap = new QTKitCap(mCaptureSession, mCaptureDecompressedVideoOutput);

    // Now instantiate the 'connection' mechanism to the lower pipeline and call them to get the pipeline started.
    pTVC = new TVidCap(pCap);
    p_pipeline_runner = new RunPipeline(pTVC,640,360,"UYVY");
    
    // Always keep window on top of other windows.
    [[mMainWindow window] setLevel:NSScreenSaverWindowLevel];
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
    assert(pTVC->bIsReleased);
    
    delete p_pipeline_runner;
    p_pipeline_runner = NULL;
    
    delete pTVC;
    pTVC = NULL;
    
    delete pCap;
    pCap = NULL;
    
	[mCaptureSession release];
	[mCaptureVideoDeviceInput release];
    [mCaptureAudioDeviceInput release];
	
    [mCaptureDecompressedVideoOutput release];
    [mCaptureDecompressedAudioOutput release];
    
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
    QTKitBufferInfo* pBI;
    void* pY;
    void* pCb;
    void* pCr;
    void* pBuff;

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

    pBI = new QTKitBufferInfo;
    assert(pBI);
    
    CVBufferRetain(videoFrame);
    CVReturn cvret = CVPixelBufferLockBaseAddress(videoFrame, 0); //LOCK_FLAGS);

    assert(cvret==0);

    pY = pCb = pCr = pBuff = NULL;

    pixType = CVPixelBufferGetPixelFormatType(videoFrame);

    if (pixType==kCVPixelFormatType_422YpCbCr8) // '2vuy' is active. Non-planar. Take base address and run.
        pBuff = CVPixelBufferGetBaseAddress(videoFrame);
    else if (pixType==kCVPixelFormatType_422YpCbCr8) 
    {
        pBuff = CVPixelBufferGetBaseAddressOfPlane(videoFrame, 0);
        pY  = CVPixelBufferGetBaseAddressOfPlane(videoFrame, 0);
        pCb = CVPixelBufferGetBaseAddressOfPlane(videoFrame, 1);
        pCr = CVPixelBufferGetBaseAddressOfPlane(videoFrame, 2);
    }
    else
        NSLog(@"pixType Unknown. '%c%c%c%c'", (char)(pixType>>24)&0xff, (char)(pixType>>16)&0xff, 
                                                (char)(pixType>>8)&0xff, (char)pixType&0xff);
    
    //void* baseAddress = CVPixelBufferGetBaseAddress(videoFrame);
//    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(videoFrame);
    frameWidth = CVPixelBufferGetWidth(videoFrame);
    frameHeight = CVPixelBufferGetHeight(videoFrame);

    CVPixelBufferUnlockBaseAddress(videoFrame, 0); // LOCK_FLAGS);

    assert(pixType==kCVPixelFormatType_422YpCbCr8); // Make sure '2vuy' is active. Eventually we'll allow for conversions etc, but for now we need AGREEMENT.
    
    NSDictionary *pDict;
    pDict = [mCaptureDecompressedVideoOutput pixelBufferAttributes];
    
    if (storedWidth != frameWidth || storedHeight != frameHeight)
    {
        storedWidth = frameWidth;
        storedHeight = frameHeight;
        
        NSLog(@"Capture CHANGE: WxH=%dx%d pixType=%c%c%c%c",frameWidth, frameHeight, (char)(pixType>>24)&0xff, (char)(pixType>>16)&0xff, (char)(pixType>>8)&0xff, (char)pixType&0xff);
    }
    
    // Now down to the business at hand. Enqueue the new frame.
    
    pBI->pVideoFrame =(void*) videoFrame;
    
    pBI->bIsVideo = true;
    
    pBI->pAudioSamples = NULL;

    pBI->pBuffer = pBuff;
    pBI->pY = pBI->pBuffer;
    pBI->pCb = pCb;
    pBI->pCr = pCr;
    
    pBI->timeStamp_uS = QTT_US([sampleBuffer presentationTime]);
    
    assert([sampleBuffer numberOfSamples]==1);

//    unsigned char* p=(unsigned char*)pBuff;
//    NSLog(@"Prior to Enqueue: pBuffer=%p", pBuff);
//    NSLog(@"Data at pBuffer is: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
//          *p++, *p++, *p++, *p++, *p++, *p++, *p++, *p++, *p++, *p++, *p++, *p++);
    
    // Counting on FullBufferEnQ() to lock down the videoFrame for us.
    // Only did this for symmetry of responsibility between enque and dq
    if (!pCap->GetBufferPointer()->FullBufferEnQ(pBI))
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

///
/// \brief Audio sample buffers are sent here via another threading calling into this function with a set of samples are ready for processing.
///         When audio processing is enabled, the 'QTBufferSamples* sampleBuffer' is sent in the QTKitBufferInfo* as pAudioSamples. This is **NOT**
///         to be used by the consumer. This is for tracking objects and freeing them appropriately on the Mac.
///
///         There are 3 dynamic audio modes. These are delimited in this function by the member 'sendAudioType' values 0, 1, and 2.
///         0 - None - no processing is done on inbound samples. They are simply dropped.
///         1 - Samples - This mode sends a pointer to the AudioBufferList* in 'pBuffer' as a void*. This pointer requires the recipient
///                     to iterate through an array of structs to get pointers to each buffer, the config of each sample such as
///                     # channels, sample rate, etc. This allows the user to send individual buffers into an encoder. Hopefully
///                     this is unnecessary as there is a lot more iterative tracking to do. It is currently the case that audio
///                     samples are being placed here in 512 buffer lists. Lots of buffers. As the other side of processing the queue
///                     may not have access to Apple structures, there are mimick structures 'neuronAudioBufferList' and 'neuronAudioBuffer'.
///                     \note To distinguish this mode to the recipient, rawLength and rawNumSamples are set to a value of '-1'.
///         2 - Raw buffer - This method sends a single pointer to the raw audio chunk in memory via 'pBuffer'. The members of QTKitBufferInfo
///                     additionally used are: rawLength - the actual number of valid bytes in the pBuffer. And rawNumSamples - tells the recipient
///                     how many samples are contained in the pBuffer. This number should divide evenly and without remainder into the rawLength.
///
- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputAudioSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    
#if 1
    /* Get the sample buffer's AudioStreamBasicDescription, which will be used to set the input format of the effect audio unit and the ExtAudioFile. */
    QTFormatDescription *formatDescription = [sampleBuffer formatDescription];
    NSValue *sampleBufferASBDValue = [formatDescription attributeForKey:QTFormatDescriptionAudioStreamBasicDescriptionAttribute ];
    if (!sampleBufferASBDValue)
        return;
    
    AudioStreamBasicDescription sampleBufferASBD = {0};
    memset (&sampleBufferASBD, 0, sizeof (sampleBufferASBD));
    
    [sampleBufferASBDValue getValue:&sampleBufferASBD];
    
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mBytesPerFrame=%d\n", sampleBufferASBD.mBytesPerFrame);
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mSampleRate=%f\n", sampleBufferASBD.mSampleRate);
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mBitsPerChannel=%d\n", sampleBufferASBD.mBitsPerChannel);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFramesPerPacket=%d\n", sampleBufferASBD.mFramesPerPacket);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags=%x\n", sampleBufferASBD.mFormatFlags);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mBytesPerFrame=%x\n", sampleBufferASBD.mBytesPerFrame);
    
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kAudioFormatFlagIsFloat=%d\n", sampleBufferASBD.mFormatFlags & kAudioFormatFlagIsFloat);
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kAudioFormatFlagIsBigEndian=%d\n", sampleBufferASBD.mFormatFlags & kAudioFormatFlagIsBigEndian);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kLinearPCMFormatFlagIsNonMixable=%d \n", sampleBufferASBD.mFormatFlags & kLinearPCMFormatFlagIsNonMixable);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kLinearPCMFormatFlagIsAlignedHigh=%d \n", sampleBufferASBD.mFormatFlags & kLinearPCMFormatFlagIsAlignedHigh);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kLinearPCMFormatFlagIsPacked=%d\n", sampleBufferASBD.mFormatFlags & kLinearPCMFormatFlagIsPacked);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kLinearPCMFormatFlagsSampleFractionShift= %d\n", sampleBufferASBD.mFormatFlags & kLinearPCMFormatFlagsSampleFractionShift);	
    
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kAudioFormatFlagIsNonInterleaved=%d \n", sampleBufferASBD.mFormatFlags & kAudioFormatFlagIsNonInterleaved);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kAudioFormatFlagIsSignedInteger=%d\n", sampleBufferASBD.mFormatFlags & kAudioFormatFlagIsSignedInteger);	
    
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatID=%c\n", sampleBufferASBD.mFormatID);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatID=%c\n", sampleBufferASBD.mFormatID >> 8);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatID=%c\n", sampleBufferASBD.mFormatID >> 16);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatID=%c\n", sampleBufferASBD.mFormatID >> 24);	
    
    printf(" lengthForAllSamples=%d, numberOfSamples=%d\n",
           [sampleBuffer lengthForAllSamples],
           [sampleBuffer numberOfSamples]); 
    fflush(stdout);	
#endif
    
    QTKitBufferInfo* pBI;
    QTTime qtt;

    if (sendAudioType==0)
    {
//        NSLog(@"Skipping audio samples.");
        return;
    }
    
    pBI = new QTKitBufferInfo;
    assert(pBI);
    
    pBI->bIsVideo = false;
    
    qtt = [sampleBuffer presentationTime];
    
//    NSLog(@"Audio sample count pre-increment: %d", [sampleBuffer sampleUseCount]);
    
    // Add a hold on this sample
    [sampleBuffer incrementSampleUseCount];
    [sampleBuffer retain];
    
//    NSLog(@"Audio sample count post-increment: %d", [sampleBuffer sampleUseCount]);
//    uint8_t* pData;
//    pData =(uint8_t*) [sampleBuffer bytesForAllSamples];
//    pBI->pY = pData;
//    NSLog(@"Databytes @ 0x%p: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",pData,
//          pData[0],pData[1],pData[2],pData[3],
//          pData[4],pData[5],pData[6],pData[7],
//          pData[8],pData[9],pData[10],pData[11]);

//    pData = (uint8_t*)sampleBuffer;
//    NSLog(@"sampleBuffer @ 0x%p: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",pData,
//          pData[0],pData[1],pData[2],pData[3],
//          pData[4],pData[5],pData[6],pData[7],
//          pData[8],pData[9],pData[10],pData[11]);
    
    pBI->pVideoFrame=NULL;
    pBI->pAudioSamples = sampleBuffer;

    pBI->pY = NULL;
    pBI->pCb = NULL;
    pBI->pCr = NULL;
    
    pBI->timeStamp_uS = QTT_US(qtt);
    
//    NSLog(@"Audio sampleBuffer ptr=0x%p", sampleBuffer);
//    NSLog(@"Audio: %d samples, %d bytes-total, timestamp: %lld, scale: %ld Ticks/sec, reported_uS:%lld", 
//          [sampleBuffer numberOfSamples], [sampleBuffer lengthForAllSamples], qtt.timeValue, qtt.timeScale, pBI->timeStamp_uS);
    
    NSDictionary *pd;
    pd = [sampleBuffer sampleBufferAttributes];

    // We know this should be incoming with LPCM audio samples, but we're gonna make sure.
    QTFormatDescription* pfd;
    pfd = [sampleBuffer formatDescription];
    uint32 fourb;
    fourb=[pfd formatType];
    assert(fourb==kAudioFormatLinearPCM);

    AudioBufferList* pAbufflist;
    pAbufflist = [sampleBuffer audioBufferListWithOptions:0];
    
    pBI->pBuffer = (void*)pAbufflist;
    if (sendAudioType==1)       // Send list of samplebuffers.
    {
        AudioBufferList* pAbufflist;
        pAbufflist = [sampleBuffer audioBufferListWithOptions:0];
        
        pBI->pBuffer = (void*)pAbufflist;
        
        pBI->rawLength = -1;
        pBI->rawNumSamples = -1;
    }
    else
    {
        assert(sendAudioType==2);   // Raw bucket of data plus # samples thrown in 'other'
        pBI->pBuffer = [sampleBuffer bytesForAllSamples];
        
        pBI->rawLength = [sampleBuffer lengthForAllSamples];
        pBI->rawNumSamples = [sampleBuffer numberOfSamples];
        assert(pBI->rawLength % pBI->rawNumSamples == 0);
    }

    // Counting on FullBufferEnQ() to lock down the videoFrame for us.
    // Only did this for symmetry of responsibility between enque and dq
    if (!pCap->GetBufferPointer()->FullBufferEnQ(pBI))
    {
        //        NSLog(@"Enqueue failed above. Marking as dropped.");
        
        curDrops++;
        [mDrops setIntValue:curDrops];
    }
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

- (IBAction)sendAudioOfType:(id)sender {
    sendAudioType = [sender indexOfSelectedItem];
    
    //        case 0: // Send no audio.
    //        case 1: // Send the array of structs (list of QTSampleBuffers
    //        case 2: // Send the whole block of data raw (with #samples alongside)
    
    if (sendAudioType < 0 || sendAudioType > 2)
        assert(false && "Invalid selection in button-menu.");
}

@end
