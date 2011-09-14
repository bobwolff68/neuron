//
//  MyRecorderController.m
//  MyRecorder

#import "MyRecorderController.h"
#import "CoreVideo/CVPixelBuffer.h"

#import <CoreAudio/CoreAudioTypes.h>

#import "CameraInterrogation.h"

#define DESIRED_BITS_PER_SAMPLE 16

// For testing only - ability to clear a/v queues in Live555 source and monitor their lengths
bool bClearQueues = false;
bool bQuit = false;
int audioQueueLength=0;
int videoQueueLength=0;
int bitRate = 600;
bool bChangeBitrate = false;
int frameRate = 30;
bool bChangeFramerate = false;

#if 0
// Want to interrogate device attributes to see if we can set it's output to signed 16bit rather than convert later.
NSDictionary *pDict = [audioDevice deviceAttributes];
NSString *key;
for(key in pDict){
    NSLog(@"Key: %@, Value %@", key, [pDict objectForKey: key]);
}
#endif

// Helper function to convert QTTime value into raw micro-seconds.
#define QTT_US(Q) ((1000000*Q.timeValue)/Q.timeScale)

@implementation MyRecorderController

- (void)awakeFromNib
{    
    Converter = NULL;
    curDrops = 0;
    sendAudioType=2; // Using RAW-Data buffer mode by default now.
    
    outputWidth = 640;
    outputHeight = 360;
    
    captureWidth = 640;
    captureHeight = 480;
    
// Create the capture session
    
	mCaptureSession = [[QTCaptureSession alloc] init];
    
// Connect inputs and outputs to the session	
    
	BOOL success = NO;
	NSError *error;
	
// Find a video device  
    
    videoDevice = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
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
             [NSNumber numberWithDouble:captureWidth], (id)kCVPixelBufferWidthKey,
             [NSNumber numberWithDouble:captureHeight], (id)kCVPixelBufferHeightKey,
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

    // This allows me to crop the video preview image as needed for our desired aspect ratio.
    [mCaptureView setDelegate:self];

    // Prepping to capture and send frames to pCap.
    pCap = new QTKitCap(mCaptureSession, mCaptureDecompressedVideoOutput);

    // Now instantiate the 'connection' mechanism to the lower pipeline and call them to get the pipeline started.
    pTVC = new TVidCap(pCap);
    p_pipeline_runner = new RunPipeline(pTVC,outputWidth,outputHeight,"UYVY",true,true);
    
    // Always keep window on top of other windows.
//    [[mMainWindow window] setLevel:NSScreenSaverWindowLevel];
}

// Handle window closing notifications for your device input
- (void)windowWillClose:(NSNotification *)notification
{
	
//	[mCaptureSession stopRunning];
    
	pCap->stop_capturing();
    
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
    
    if (Converter)
        AudioConverterDispose(Converter);
    
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

- (CIImage *)view:(QTCaptureView *)view willDisplayImage:(CIImage *)image {
    //mirror image across x axis
    //    return [image imageByApplyingTransform:CGAffineTransformMakeScale(-1, 1)];

    // Crop image to 640x360
//    return [image imageByCroppingToRect:CGRectMake(0, 60, 640, 360)];

    // Crop and mirror at the same time.
    return [[image imageByCroppingToRect:CGRectMake((captureWidth-outputWidth)/2, (captureHeight-outputHeight)/2, 
                                                            outputWidth, outputHeight)] 
            imageByApplyingTransform:CGAffineTransformMakeScale(-1, 1)];
    
//    return image;
}

#pragma mark-

- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    static int storedWidth=0;
    static int storedHeight=0;
//    int frameWidth;
//    int frameHeight;
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

#ifdef INTERROGATE_CAMERA
    // Final entry is the one we'll be left with, so for now we'll wire this as 640x480 so we can play with sending
    // the cropped-down version for 640x360
    struct trial {
        int width;
        int height;
    };
    // Try 4:3 resolutions first. Then 16:9
    // 4:3  -- 80x60, 160x120, 240x192, 320x240, 640x480, 800x600, 1024x768, 1280x1024, 1600x1200
    // 16:9 -- 160x90, 240x136, 320x180, 640x360, 800x448, 1024x576, 1280x720, 1600x900, 1920x1080
    static struct trial requested[] = {{80,60}, {160,120}, {240,192}, {320,240}, {640,480}, {800,600}, {1024,768}, {1280,1024}, {1600,1200},
        {160,90}, {240,136}, {320,180}, {640,360}, {800,448}, {1024,576}, {1280,720}, {1600,900}, {1920,1080},
        // Final one will be the one that 'sticks'. We'll do this a better way later.
        {640,480}};
    static int tstIter=0;
    static bool bSetSize=true;   // Set

    int max = sizeof(requested)/sizeof(struct trial);
    static struct trial received[40];  // Proper way is to have this and requested[] be class members along with bSetSize etc.
    
    while (tstIter < max)
    {

        if (bSetSize)
        {
            [mCaptureDecompressedVideoOutput setPixelBufferAttributes:
             [NSDictionary dictionaryWithObjectsAndKeys:
              [NSNumber numberWithDouble:requested[tstIter].width], (id)kCVPixelBufferWidthKey,
              [NSNumber numberWithDouble:requested[tstIter].height], (id)kCVPixelBufferHeightKey,
              //             [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32ARGB/*kCVPixelFormatType_422YpCbCr10*/], (id)kCVPixelBufferPixelFormatTypeKey,
              nil]];
            
            // Yes - sleep for 1 MICRO-SECOND -- just a minimal sleep to switch contexts and yield to the system.
//            usleep(1);
        }
        else
        {
            NSArray* pArr = [videoDevice formatDescriptions];
            if (pArr)
            {
                int count = [pArr count];
                if (count)
                {
                    NSValue *psz;
                    psz = [[[videoDevice formatDescriptions]  objectAtIndex:0] attributeForKey:@"videoEncodedPixelsSize"];
                          
                    int w = (int)[psz sizeValue].width;
                    int h = (int)[psz sizeValue].height;
                    
                    received[tstIter].width = w;
                    received[tstIter].height = h;
                    NSLog(@"ON Iter#%d: Requested:%dx%d -- Received: %dx%d", tstIter, requested[tstIter].width, requested[tstIter].height, w, h);
                }
                
// Full interrogation of formatDescriptionAttributes
#if 0
                for (int i=0; i<count; i++)
                {
                    NSDictionary* pDict;
//                    pDict = [[pArr objectAtIndex:i] formatDescriptionAttributes];
                    pDict = [[[videoDevice formatDescriptions] objectAtIndex:i] formatDescriptionAttributes];
                    NSString *key;
                    for(key in pDict){
                        NSLog(@"Key: %@, Value %@", key, [pDict objectForKey: key]);
                    }
                }
#endif
            }
            
        }
        
        // Only iterate to the next resolution after the check has been made.
        if (!bSetSize)
            tstIter++;
        
        if (tstIter==max)
        {
            NSLog(@"Capture-Device Resolutions:");
            for (int j=0;j<max;j++)
                cout << "  " << received[j].width << "x" << received[j].height;
            
            cout << endl;
        }
        
        // Toggle each frame - one go-thru sets a resolution and the next one tests it.
        bSetSize = !bSetSize;
        
        return;
    }
//    static bool firstTime=true;
//    if (firstTime)
//    {
//        firstTime = false;
//        // Figure out what modes the camera captures in natively.
//        // Then map our desired sizes to larger capture settings in prep for cropping games.
//        [self validCaptureModes];
//    }
#endif

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
//    frameWidth = CVPixelBufferGetWidth(videoFrame);
//    frameHeight = CVPixelBufferGetHeight(videoFrame);

    CVPixelBufferUnlockBaseAddress(videoFrame, 0); // LOCK_FLAGS);

    assert(pixType==kCVPixelFormatType_422YpCbCr8); // Make sure '2vuy' is active. Eventually we'll allow for conversions etc, but for now we need AGREEMENT.
    
    NSDictionary *pDict;
    pDict = [mCaptureDecompressedVideoOutput pixelBufferAttributes];
    
    if (storedWidth != outputWidth || storedHeight != outputHeight)
    {
        storedWidth = outputWidth;
        storedHeight = outputHeight;
        
        NSLog(@"Capture CHANGE: WxH=%dx%d pixType=%c%c%c%c",outputWidth, outputHeight, (char)(pixType>>24)&0xff, (char)(pixType>>16)&0xff, (char)(pixType>>8)&0xff, (char)pixType&0xff);
    }
    
    // Now down to the business at hand. Enqueue the new frame.
    
    pBI->pVideoFrame =(void*) videoFrame;
    
    pBI->bIsVideo = true;
    
    pBI->pAudioSamples = NULL;

    // Grab the current device capture resolution as this will determine cropping and strides to be given downline.
    NSValue *psz;
    psz = [[[videoDevice formatDescriptions]  objectAtIndex:0] attributeForKey:@"videoEncodedPixelsSize"];
    
    captureWidth = (int)[psz sizeValue].width;
    captureHeight = (int)[psz sizeValue].height;
    int captureStride;
    
    if (pixType==kCVPixelFormatType_422YpCbCr8) // '2vuy' is active. Non-planar. Take base address and run.
    {
        int leftOffset, topOffset;
        
        assert(outputWidth <= captureWidth);
        assert(outputHeight <= captureHeight);

        captureStride = captureWidth * 2;   // 4:2:2 is always 2*width for stride and 2*w*h for buffer size.

        // Always centering our cropping (if any) horizontally and vertically.
        leftOffset = (captureWidth - outputWidth)/2; // Example: capture 640, only want 500, (640-500)/2=70 crop left side
        topOffset  = (captureHeight - outputHeight)/2;
        
        // Effective starting buffer location with top-crop and left-crop taken into account. Stride will take care of the rest.
        pBI->pBuffer = (void*) ( (unsigned char*)pBuff + (topOffset * captureStride) + (leftOffset * 2));
        pBI->pBufferLength = outputHeight * captureStride;
        pBI->captureStride = captureStride;
        
    }
    else if (pixType==kCVPixelFormatType_422YpCbCr8) 
    {
        // Cope with Y, Cr, and Cb offsets and lengths separately.
        
        // Will have to have lengthY, lengthCr, and lengthCb
        // Will also have to have strideY, strideCr, strideCb.
        pBI->pY = pBI->pBuffer;
        pBI->pCb = pCb;
        pBI->pCr = pCr;
        
    }
    else
    {
        NSLog(@"Unsupported pixType at this time.");
        assert(false);
    }

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
    
    if (sendAudioType==0)
    {
        //        NSLog(@"Skipping audio samples.");
        return;
    }
    
    /* Get the sample buffer's AudioStreamBasicDescription, which will be used to set the input format of the effect audio unit and the ExtAudioFile. */
    QTFormatDescription *formatDescription = [sampleBuffer formatDescription];
    NSValue *sampleBufferASBDValue = [formatDescription attributeForKey:QTFormatDescriptionAudioStreamBasicDescriptionAttribute ];
    if (!sampleBufferASBDValue)
        return;
    
    AudioStreamBasicDescription sampleBufferASBD = {0};
    memset (&sampleBufferASBD, 0, sizeof (sampleBufferASBD));
    
    [sampleBufferASBDValue getValue:&sampleBufferASBD];
#if 0
    
/*    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mBytesPerFrame=%lu\n", sampleBufferASBD.mBytesPerFrame);
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mBytesPerPacket=%lu\n", sampleBufferASBD.mBytesPerPacket);
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mSampleRate=%f\n", sampleBufferASBD.mSampleRate);
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mBitsPerChannel=%lu\n", sampleBufferASBD.mBitsPerChannel);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mChannelsPerFrame=%lu\n", sampleBufferASBD.mChannelsPerFrame);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFramesPerPacket=%lu\n", sampleBufferASBD.mFramesPerPacket);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags=0x%lx\n", sampleBufferASBD.mFormatFlags);	
    
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kAudioFormatFlagIsFloat=%lu\n", sampleBufferASBD.mFormatFlags & kAudioFormatFlagIsFloat);
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kAudioFormatFlagIsBigEndian=%lu\n", sampleBufferASBD.mFormatFlags & kAudioFormatFlagIsBigEndian);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kLinearPCMFormatFlagIsNonMixable=%lu \n", sampleBufferASBD.mFormatFlags & kLinearPCMFormatFlagIsNonMixable);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kLinearPCMFormatFlagIsAlignedHigh=%lu \n", sampleBufferASBD.mFormatFlags & kLinearPCMFormatFlagIsAlignedHigh);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kLinearPCMFormatFlagIsPacked=%lu\n", sampleBufferASBD.mFormatFlags & kLinearPCMFormatFlagIsPacked);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kLinearPCMFormatFlagsSampleFractionShift= %lu\n", sampleBufferASBD.mFormatFlags & kLinearPCMFormatFlagsSampleFractionShift);	
    
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kAudioFormatFlagIsNonInterleaved=%lu \n", sampleBufferASBD.mFormatFlags & kAudioFormatFlagIsNonInterleaved);	
    printf( "!!!UncompressedVideoOutput::outputAudioSampleBuffer(), sampleBufferASBD.mFormatFlags kAudioFormatFlagIsSignedInteger=%lu\n", sampleBufferASBD.mFormatFlags & kAudioFormatFlagIsSignedInteger);	
    
    printf(" lengthForAllSamples=%d, numberOfSamples=%d\n",
           [sampleBuffer lengthForAllSamples],
           [sampleBuffer numberOfSamples]); 
    fflush(stdout);	*/
#endif

#if 0
    // If the conversion unit is not running already, start it up now that we have an ASBD for the input.
    if (!Converter)
    {
        AudioStreamBasicDescription outputBufferASBD = {0};
        
        // Output is similar to input so a starting point is nice.
        outputBufferASBD = sampleBufferASBD;
/*
 kAudioFormatFlagIsFloat                     = (1 << 0),     // 0x1
 kAudioFormatFlagIsBigEndian                 = (1 << 1),     // 0x2
 kAudioFormatFlagIsSignedInteger             = (1 << 2),     // 0x4
 kAudioFormatFlagIsPacked                    = (1 << 3),     // 0x8
 kAudioFormatFlagIsAlignedHigh               = (1 << 4),     // 0x10
 kAudioFormatFlagIsNonInterleaved            = (1 << 5),     // 0x20
 kAudioFormatFlagIsNonMixable                = (1 << 6),     // 0x40
 kAudioFormatFlagsAreAllClear                = (1 << 31),

 */
        // Now modify the output to become:
        // 16-bit signed
        // Convert to interleaved.
        // Endian-ness uncertain.

#if 0
        FillOutASBDForLPCM(outputBufferASBD, 44100.00, 2, 16, 16, false, true);
        
#else
        outputBufferASBD.mBitsPerChannel = DESIRED_BITS_PER_SAMPLE;
        outputBufferASBD.mFramesPerPacket = 1;  // Always true for LPCM.

        outputBufferASBD.mFormatFlags &= ~kAudioFormatFlagIsFloat;
        outputBufferASBD.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
#if 1   // Interleaved
        outputBufferASBD.mFormatFlags &= ~kAudioFormatFlagIsNonInterleaved;
#endif
        
        outputBufferASBD.mBytesPerFrame = (outputBufferASBD.mBitsPerChannel / 8) * outputBufferASBD.mChannelsPerFrame;
//#else
//        outputBufferASBD.mBytesPerFrame = (outputBufferASBD.mBitsPerChannel / 8) * 1;
//#endif
        
        outputBufferASBD.mBytesPerPacket = outputBufferASBD.mBytesPerFrame * outputBufferASBD.mFramesPerPacket;
#endif
        
        // Open the audio converter
        OSStatus stat = AudioConverterNew(&sampleBufferASBD, &outputBufferASBD, &Converter);
        assert(stat==0);
    }

    assert(Converter);
#endif
    
    QTKitBufferInfo* pBI;
    QTTime qtt;

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

    // Only allowing NONE or RAW-Data format now.
    assert(sendAudioType==2 || sendAudioType==0);
#if 0    
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
#endif
    if (sendAudioType==2)
    {
        assert(sendAudioType==2);   // Raw bucket of data plus # samples thrown in 'other'
// Old method of sending float32 direct.        pBI->pBuffer = [sampleBuffer bytesForAllSamples];
        // Time to convert.
        UInt32 outSize = [sampleBuffer numberOfSamples] * (DESIRED_BITS_PER_SAMPLE / 8) * sampleBufferASBD.mChannelsPerFrame;
//TODO - reset to PROPER amount. *4 is only for illegal access debug
        void* pConvData = new char[outSize];
        assert(pConvData);
        
        // Must 'delete' this array in Release via pBuffer
        pBI->pBuffer = pConvData;
        pBI->pBufferLength = outSize;
        
//        unsigned char* test = (unsigned char*)pBI->pBuffer;
        
//        for (int i=0;i<[sampleBuffer lengthForAllSamples];i++,test++)
//            *test = *test;
        Float32* inbuf = (Float32*)[sampleBuffer bytesForAllSamples];
#if 0        
        
        // Convert
        OSStatus stat = AudioConverterConvertBuffer(Converter, [sampleBuffer lengthForAllSamples], (const void*)[sampleBuffer bytesForAllSamples], &outSize, pBI->pBuffer);
        assert(stat==0);
#endif
#if 1
        SInt16* outbuf = (SInt16*)pConvData;
        
        int offset = [sampleBuffer numberOfSamples];
        int max = INT16_MAX;
        Float32 fl1, fl2;
        
//        NSLog(@"Sample  in-L(f)    in-R(f)       out-L     out-R");
        
        // Bob's po-man float32->sint16 converter and interleaver.
        for (int sample=0; sample < [sampleBuffer numberOfSamples]; sample++)
        {
            // On input, must take sample left and sample right and compensate for offset of non-interleaved.
            //assert(inbuf[sample] <= 1.0 && inbuf[sample] >= -1.0);
            //assert(inbuf[sample+offset] <= 1.0 && inbuf[sample+offset] >= -1.0);
            
            fl1 = inbuf[sample] * (Float32)max;
            outbuf[sample*2] = (SInt16)(fl1);
            //outbuf[sample*2] = EndianS16_NtoB(outbuf[sample*2]);
                                //Lower Byte shifted up         //Upper byte shifted low
            //outbuf[sample*2] = ((outbuf[sample*2]&0xff)<<8) | ((outbuf[sample*2]&0xff00)>>8);
            //EndianS16_NtoB(outbuf[sample*2]);
            
            fl2 = inbuf[sample + offset] * (Float32)max;
            outbuf[sample*2 + 1] = (SInt16)(fl2);
            //outbuf[sample*2 + 1] = EndianS16_NtoB(outbuf[sample*2]+1);


            
            //outbuf[sample*2+1] = ((outbuf[sample*2+1]&0xff)<<8) | ((outbuf[sample*2+1]&0xff00)>>8);
            
//            NSLog(@"[%03d]:  % 1.4f    % 1.4f    %6hi      %6hi", sample, inbuf[sample], inbuf[sample+offset], outbuf[sample*2], outbuf[sample*2+1]);
        }
        
#ifdef DUMP_RAW_LPCM
        static FILE* fpFloat=NULL;
        static FILE* fpSint=NULL;
        static FILE* fpSintBigEndian=NULL;
        static int chunksWritten=0;
        
        if (!fpFloat)
        {
            fpFloat = fopen("FloatRaw.lpcm", "wb");
            assert(fpFloat);
        }
        if (!fpSint)
        {
            fpSint = fopen("SInt16RawNative.lpcm", "wb");
            assert(fpSint);
        }
        if (!fpSintBigEndian)
        {
            fpSintBigEndian = fopen("SInt16RawBigEndian.lpcm", "wb");
            assert(fpSintBigEndian);
        }

        //Write out the data. First we write outbuf as this is Sint16 interleaved and prepared.
        int out;
        out = fwrite(outbuf, sizeof(SInt16), [sampleBuffer numberOfSamples]*2, fpSint);
        assert(out == [sampleBuffer numberOfSamples]*2);
        
        // Now convert in-place to Big endian.
        for (int sample=0; sample < [sampleBuffer numberOfSamples]; sample++)
        {
            outbuf[sample*2] = EndianS16_NtoB(outbuf[sample*2]);
            outbuf[sample*2 + 1] = EndianS16_NtoB(outbuf[sample*2 + 1]);
        }
        out = fwrite(outbuf, sizeof(SInt16), [sampleBuffer numberOfSamples]*2, fpSintBigEndian);
        assert(out == [sampleBuffer numberOfSamples]*2);
        
       // Now take the original float32 values and simply interleave them into outbuf[] and write it out.
        Float32* pFOut = new Float32[[sampleBuffer numberOfSamples] * 2];
        out = sizeof(pFOut);        // should be 4096, right?
        for (int sample=0; sample < [sampleBuffer numberOfSamples]; sample++)
        {
            pFOut[sample*2] = inbuf[sample];
            pFOut[sample*2 + 1] = inbuf[sample + offset];
       }
        out = fwrite(pFOut, sizeof(Float32), [sampleBuffer numberOfSamples]*2, fpFloat);
        assert(out == [sampleBuffer numberOfSamples]*2);
        
        delete pFOut;
        
        // Artificial way to limit the number of samples and ensure the files get closed properly.
        if (chunksWritten++ > 200)
        {
            fclose(fpSint);
            fclose(fpSintBigEndian);
            fclose(fpFloat);
            assert(false);
        }
#endif  // DUMP_RAW_LPCM

#endif  // 1/0 in/out
        
        // Now decrement the sample count on the original input buffer.
        [sampleBuffer decrementSampleUseCount];
        
        // outSize was modified by the conversion routine -- so this is the actual amount used. We want to make sure it is always
        // less than our calculated outSize initially however.
        assert(outSize < [sampleBuffer lengthForAllSamples]);
               
        pBI->rawLength = outSize;
        
        // Are samples the same? How do we find out?
        // Would be nice to assert this fact.
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
        
        outputWidth = 1280;
        outputHeight = 720;
        
        NSLog(@"Using HD mode 720p.");
    }
    else
    {
        [mCaptureDecompressedVideoOutput setPixelBufferAttributes:
         [NSDictionary dictionaryWithObjectsAndKeys:
                       [NSNumber numberWithDouble:640], (id)kCVPixelBufferWidthKey,
                       [NSNumber numberWithDouble:480], (id)kCVPixelBufferHeightKey,
          //             [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32ARGB/*kCVPixelFormatType_422YpCbCr10*/], (id)kCVPixelBufferPixelFormatTypeKey,
          nil]];

        outputWidth = 640;
        outputHeight = 360;
        
        NSLog(@"Using SD mode 360p.");
    }
}

- (IBAction)resetDrops:(id)sender {
    curDrops=0;
    [mDrops setIntValue:curDrops];
}

- (IBAction)quitApplication:(id)sender {
    bQuit = true;
    [NSApp terminate:self];
}

- (IBAction)sendAudioRawData:(id)sender {
    sendAudioType = 2;
}

- (IBAction)sendAudioNoData:(id)sender {
    sendAudioType = 0;
}

- (IBAction)clearQueues:(id)sender {
    bClearQueues = true;
}

@end
