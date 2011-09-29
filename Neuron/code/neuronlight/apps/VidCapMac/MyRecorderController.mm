//
//  MyRecorderController.m
//  MyRecorder

#import "MyRecorderController.h"
#import "CoreVideo/CVPixelBuffer.h"

#import <CoreAudio/CoreAudioTypes.h>
#import <AudioToolbox/AudioConverter.h>

#import "CameraInterrogation.h"

#define DESIRED_BITS_PER_SAMPLE 16
#define DESIRED_AUDIO_FREQUENCY 32000
#define DESIRED_NUM_CHANNELS_OUT    2
//Manjesh - #define DESIRED_AUDIO_FREQUENCY 32000
#define DESIRED_CONVERTEDOUTPUT_IN_TIME_MS 10

#define MODULO_REQUIRED 4

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


///
/// This is the callback which will be used in the complex audio converter
/// It's job is to call the real callback within the MyRecorderController instance
/// via using the UserData field (for 'this')
///
OSStatus iConverter (
                     AudioConverterRef             inAudioConverter,
                     UInt32                        *ioNumberDataPackets,
                     AudioBufferList               *ioData,
                     AudioStreamPacketDescription  **outDataPacketDescription,
                     void                          *inUserData
                     )
{
    MyRecorderController * pMc = (MyRecorderController*)inUserData;
    
    return [pMc iConvertSupplyData: ioData withNumPackets: ioNumberDataPackets];
}



@interface NSString (IntCompare)

- (NSComparisonResult) intCompare: (NSString *) aString;

@end

@implementation NSString (IntCompare)

- (NSComparisonResult) intCompare: (NSString *) aString
{
    NSComparisonResult result;

    result = [[NSNumber numberWithInt:[self intValue]] compare: [NSNumber numberWithInt:[aString intValue]]];
    
    return result;
}

@end

@implementation MyRecorderController

- (void)awakeFromNib
{    
    Converter = NULL;
    curDrops = 0;
    sendAudioType=2; // Using RAW-Data buffer mode by default now.
    
    outputWidth   = 640;
    outputHeight  = 360;
    
    captureWidth  = 640;
    captureHeight = 480;
    
    pCameraResolutions = [NSMutableArray arrayWithObjects:@"160 x 120", @"176 x 144",
                          @"320 x 240", @"640 x 480", @"960 x 540", 
                          @"1024 x 576", @"1280 x 720", nil];

    [pCameraResolutions sortUsingSelector:@selector(intCompare:)];
    [pCameraResolutions retain];
    
    // A few UI components to setup.
    [captureResBox setStringValue:[NSString stringWithFormat:@"%d x %d", outputWidth, outputHeight]];
    [captureButton setEnabled:true];
    [stopCaptureButton setEnabled:false];
    
    // Audio conversion support.
    pAudioInputQueue = new SafeBufferDeque(10);
    mAudioInputNumChannels = -1; // Uninitialized
    // Setup audio callback output buffer.
    audioInputOutBufSize = 2048;
    pAudioInputOutBuf = (unsigned char*)malloc(audioInputOutBufSize);    // Reasonably sized buffer for output. Will auto-resize if needed at runtime.
    // We can pre-calculate our desired size of an output buffer.
    // If we CHOOSE to ask for 10ms of data, we calculate how many samples and how many bytes this is.
    audioInputPostConversionNumPackets = DESIRED_AUDIO_FREQUENCY / (1000 / DESIRED_CONVERTEDOUTPUT_IN_TIME_MS);
    // TODO change to ms version and figure out the conversion offset problem.
//    audioInputPostConversionNumPackets = 256;
    
    // The rest of the initialization variables must be done after the first sample of audio is received.
    // See comment [FINISH_AUDIO_INITIALIZATION]

    // Must start the Complex Converter thread for feeding the converter.
    pExtractor = new ExtractConverted(self);
    
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

    pCap->init_capturing();
    
    // Always keep window on top of other windows.
//    [[mMainWindow window] setLevel:NSScreenSaverWindowLevel];
    
    // Now - set a timer for updating the UI components for real-time data-lookups etc.
    // This was inititally placed in service to track the audio and video queue lengths.
    timerUIUpdate = [NSTimer scheduledTimerWithTimeInterval:0.25
                                                  target:self
                                                selector:@selector(updateUINow:)
                                                userInfo:nil
                                                 repeats:YES];
}

- (void)updateUINow:(NSTimer*) timer {
//    [VLen setStringValue:[NSString stringWithFormat:@"V:%d",videoQueueLength]];
//    [ALen setStringValue:[NSString stringWithFormat:@"A:%d",audioQueueLength]];
}

// Handle window closing notifications for your device input
- (void)windowWillClose:(NSNotification *)notification
{
	
    if ([[mCaptureVideoDeviceInput device] isOpen])
        [[mCaptureVideoDeviceInput device] close];
    
    if ([[mCaptureAudioDeviceInput device] isOpen])
        [[mCaptureAudioDeviceInput device] close];
    
}

// Handle deallocation of memory for your capture objects
- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    delete p_pipeline_runner;
    p_pipeline_runner = NULL;

    if(pTVC)
        assert(pTVC->bIsReleased);
    
    if (Converter)
        AudioConverterDispose(Converter);
    
    delete pTVC;
    pTVC = NULL;
    
    delete pExtractor;
    pExtractor = NULL;
    
    delete pCap;
    pCap = NULL;
        
	[mCaptureSession release];
	[mCaptureVideoDeviceInput release];
    [mCaptureAudioDeviceInput release];
	
    [mCaptureDecompressedVideoOutput release];
    [mCaptureDecompressedAudioOutput release];
    
    if (pCameraResolutions)
        [pCameraResolutions release];
    
    free(pAudioInputOutBuf);
    delete pAudioInputPostConversionData;
    delete pAudioInputQueue;
}

//
//TODO - Find out why this dealloc function is ***NOT*** being called during
//       the termiantion sequence. Shutdown seems to be taking a lot of time too.
//
- (void) dealloc
{
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
/// This is the class-version of the callback which will be used in the complex audio converter
/// It's job is to supply data to the converter when the converter calls it.
/// If no input is available, that's ok too.
///
- (OSStatus) iConvertSupplyData:  (AudioBufferList *)data withNumPackets:(UInt32 *)numberDataPackets
{
    int bytes_to_copy_total;
    int bytes_copied_so_far = 0;    // can be used for pAudioInputOutBuf offset in bytes.
    int numInboundBuffers = data->mNumberBuffers;
    
    // If/when we allow mono capture, this will wind up being '1' but the copy loops must be looked over carefully at that point.
    // They are implemented to handle it but could be deficient in some way.
//    assert(numInboundBuffers == 2);
    
//    std::cerr << "Supply input data to Converter - in callback." << endl;
/*    AudioBuffer* pbuff0 = &data->mBuffers[0];
    AudioBuffer* pbuff1 = &data->mBuffers[1];
    assert(data->mNumberBuffers == 2);
//    *numberDataPackets = 100;
    pAudioInputOutBuf = (unsigned char*)malloc(32768);
    data->mBuffers[0].mNumberChannels = 1;
    data->mBuffers[0].mData = pAudioInputOutBuf;
    data->mBuffers[0].mDataByteSize = *numberDataPackets * mAudioInputIndividualSampleSize;
    data->mBuffers[1].mNumberChannels = 1;
    data->mBuffers[1].mData = pAudioInputOutBuf;
    data->mBuffers[1].mDataByteSize = *numberDataPackets * mAudioInputIndividualSampleSize;
    return 0;
*/
    
    // We may bail out if there is no residual and there is no further frames available.
/*    if (pAudioInputQueue->qsize()==0)
    {
        *numberDataPackets = 0;
        return 2;
    }
*/
    
    // Remember, this is INPUT data we're pushing now. As of Sep 2011, this means float32 samples (4-bytes)
    //
    // ALSO - since 'regular' input data is float32 and NON-interleaved, we'll get 2 inbound buffers so grand total of bytes is doubled.
    bytes_to_copy_total = *numberDataPackets * mAudioInputIndividualSampleSize * numInboundBuffers;
    
    // Flexibly grow the audio outbuf when needed.
    if (bytes_to_copy_total > audioInputOutBufSize)
    {
        audioInputOutBufSize = bytes_to_copy_total;
        pAudioInputOutBuf = (unsigned char*)realloc(pAudioInputOutBuf, audioInputOutBufSize);
    }

    assert(pAudioInputOutBuf);
    
    int wait_iterations=0;
    ///
    /// Keep grabbing items off the queue until we've copied FULLY into the buffer area -  unless capturing stops.
    ///
    while (bytes_copied_so_far < bytes_to_copy_total && [mCaptureSession isRunning])
    {
        int to_copy_now;
        
        // Let's assume the best - we get to copy all that's left to be copied.
        // And to_copy_now is the actual memcpy amount so adjust for 2 inbound buffers.
        to_copy_now = (bytes_to_copy_total - bytes_copied_so_far) / numInboundBuffers;
        
        // Copy at most '*numberDataPackets' into '*data' (as AudioBufferList of 1) . We will wind up with residual data on each iteration.
        // Initially, see if we need a residual buffer or already have residual to process.
        
        // Now, we either still have residual or it's NULL.
        // If it's NULL, then let's grab the next item and process it.
        if (!pAudioInputResidualData)
        {
            // Get the new residual buffer for copying now and potentially later.
            audioInputResidualBytesProcessed = 0;
            pAudioInputQueue->RemoveItem(&pAudioInputResidualData, &audioInputResidualTotalLength, &audioInputResidualTV);
//            if (pAudioInputQueue->qsize())
//                std::cerr << "After removing an item for input, AudioInputQueue length==" << pAudioInputQueue->qsize() << endl;
            
            // There was no data ready. Cope by sleeping for a few and going around again.
            if (!pAudioInputResidualData)
            {
                ///
                /// This will only function properly if the inbound samples are on a separate thread as the FillComplexAudioBuffer() call.
                /// Initially this was not true and so this would iterate forever.
                ///
                
                // When we get here, we need to know it the first time it happens. Then decide what to do on implementation.
                // It may be just fine to sleep and continue depending on threading model in AudioConverter. See alternative below.
//                std::cerr << "Iter:" << wait_iterations++ << " Delay in converter callback. Requested length=" << bytes_to_copy_total << ". Only copied " << bytes_copied_so_far << " at this point." << endl;
                usleep(8*1000);
                continue;
            }
            
            // In this case, we have data. Let 'er ride.
        }
        
        // Now process the data.
        if (pAudioInputResidualData)
        {
            // If we don't have enough data in the residual buffer, then only copy that amount for now.
            if ((audioInputResidualTotalLength - audioInputResidualBytesProcessed)/numInboundBuffers < to_copy_now)
                to_copy_now = (audioInputResidualTotalLength - audioInputResidualBytesProcessed)/numInboundBuffers;     // Artificially shorten.
            
            ///
            /// Now copy data and then copy again for 2nd buffer if there is one. 
            /// NOTE: Decided against generic for() loop due to byte-tracking complication. 2 channels of input should be fine for a LONG time.
            ///
            assert(numInboundBuffers <= 2);
            
            /// Do 0th entry. 0th channel of data goes into 1st half of output buffer memory area.
            int outoffset=bytes_copied_so_far/numInboundBuffers;
            int inoffset=audioInputResidualBytesProcessed/numInboundBuffers;

            memcpy(&pAudioInputOutBuf[bytes_copied_so_far/numInboundBuffers], &pAudioInputResidualData[audioInputResidualBytesProcessed/numInboundBuffers], to_copy_now);

            /// Now the 2nd half if required. Note the offset into each region for left/right channels.
            if (numInboundBuffers == 2)
            {
                outoffset = bytes_copied_so_far/numInboundBuffers + audioInputOutBufSize/numInboundBuffers;
                inoffset = audioInputResidualBytesProcessed/numInboundBuffers + audioInputResidualTotalLength/numInboundBuffers;

                memcpy(&pAudioInputOutBuf[bytes_copied_so_far/numInboundBuffers + audioInputOutBufSize/numInboundBuffers], 
                       &pAudioInputResidualData[audioInputResidualBytesProcessed/numInboundBuffers + audioInputResidualTotalLength/numInboundBuffers], to_copy_now);
            }

            // Advance pointers/counters
            audioInputResidualBytesProcessed += to_copy_now * numInboundBuffers;
            bytes_copied_so_far += to_copy_now * numInboundBuffers;
            
            // Are we completely done with the residual buffer yet?
            if (audioInputResidualBytesProcessed == audioInputResidualTotalLength)
            {
                audioInputResidualBytesProcessed = 0;
                audioInputResidualTotalLength = 0;
                
                // Must delete as this pointer came from a SafeBufferDeque.RemoveItem()
                delete pAudioInputResidualData;
                pAudioInputResidualData = NULL;
            }
            
            // Should never happen unless math was wrong above.
            assert(audioInputResidualBytesProcessed <= audioInputResidualTotalLength);
        }

    }
    
    *numberDataPackets = bytes_copied_so_far / mAudioInputIndividualSampleSize / numInboundBuffers;
    
    // Fill out the output structures
    for (int i=0 ; i < numInboundBuffers ; i++)
    {
        // This is supplied to us on inbound.        data->mBuffers[0].mNumberChannels = mAudioInputNumChannels;
        data->mBuffers[i].mData = &pAudioInputOutBuf[i*(audioInputOutBufSize/numInboundBuffers)];
        data->mBuffers[i].mDataByteSize = bytes_copied_so_far / numInboundBuffers;
    }
    
    if (![mCaptureSession isRunning])
        return 1;
    else
        return noErr;
}

///
/// \brief Audio sample buffers are sent here via another threading calling into this function with a set of samples are ready for processing.
///         When audio processing is enabled, the 'QTBufferSamples* sampleBuffer' is sent in the QTKitBufferInfo* as pAudioSamples. This is **NOT**
///         to be used by the consumer. This is for tracking objects and freeing them appropriately on the Mac.
///
///         There are 3 dynamic audio modes. These are delimited in this function by the member 'sendAudioType' values 0, 1, and 2.
///         0 - None - no processing is done on inbound samples. They are simply dropped.
///         1 - Samples - Now illegal.
/// [OLD]        2 - Raw buffer - This method sends a single pointer to the raw audio chunk in memory via 'pBuffer'. The members of QTKitBufferInfo
///                     additionally used are: rawLength - the actual number of valid bytes in the pBuffer. And rawNumSamples - tells the recipient
///                     how many samples are contained in the pBuffer. This number should divide evenly and without remainder into the rawLength.
///         2 - AudioConvert - a) Takes inbound samples and copies them into a SafeBufferDeque so that the audio converter callback can pull from the
///             deque for conversion when the converter is ready for new input data. b) Make a call to see if there is any converted/final data, and
///             if so, send this into the RTBuffer for later encoding.
///
- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputAudioSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    
    ///
    /// Skip out if audio is disabled.
    ///
    if (sendAudioType==0)
    {
        //        NSLog(@"Skipping audio samples.");
        return;
    }
    
    if (mAudioInputNumChannels==-1)
    {
        // Setup input channels based upon this sample.
        /* Get the sample buffer's AudioStreamBasicDescription, which will be used to set the input format of the effect audio unit and the ExtAudioFile. */
        QTFormatDescription *formatDescription = [sampleBuffer formatDescription];
        NSValue *sampleBufferASBDValue = [formatDescription attributeForKey:QTFormatDescriptionAudioStreamBasicDescriptionAttribute ];
        if (!sampleBufferASBDValue)
        {
            assert(false);
            return;
        }
        
        AudioStreamBasicDescription sampleBufferASBD = {0};
        memset (&sampleBufferASBD, 0, sizeof (sampleBufferASBD));
        
        [sampleBufferASBDValue getValue:&sampleBufferASBD];
        
        mAudioInputNumChannels = sampleBufferASBD.mChannelsPerFrame;
        mAudioInputIndividualSampleSize = sampleBufferASBD.mBitsPerChannel / 8;    // Float is float32.
        
        // Now we can finalize... [FINISH_AUDIO_INITIALIZATION] (tag from awakeFromNib)
        // Time to prep our output-converted data area and ask for data.
        audioInputPostConversionSize = DESIRED_NUM_CHANNELS_OUT * audioInputPostConversionNumPackets * (DESIRED_BITS_PER_SAMPLE / 8);
        pAudioInputPostConversionData = new char[audioInputPostConversionSize];
        assert(pAudioInputPostConversionData);
        
        audioInputPostConversionBufferList.mNumberBuffers = 1;
        audioInputPostConversionBufferList.mBuffers->mNumberChannels = DESIRED_NUM_CHANNELS_OUT;
        audioInputPostConversionBufferList.mBuffers->mDataByteSize = audioInputPostConversionSize;
        audioInputPostConversionBufferList.mBuffers->mData = pAudioInputPostConversionData;
    }
    
    ///
    /// Get audio stream description. This will dictate how we convert data and setup the converter
    ///
    
    // If the conversion unit is not running already, start it up now that we have an ASBD for the input.
    if (!Converter)
    {
        AudioStreamBasicDescription outputBufferASBD = {0};
        
        /* Get the sample buffer's AudioStreamBasicDescription, which will be used to set the input format of the effect audio unit and the ExtAudioFile. */
        QTFormatDescription *formatDescription = [sampleBuffer formatDescription];
        NSValue *sampleBufferASBDValue = [formatDescription attributeForKey:QTFormatDescriptionAudioStreamBasicDescriptionAttribute ];
        if (!sampleBufferASBDValue)
            return;
        
        AudioStreamBasicDescription sampleBufferASBD = {0};
        memset (&sampleBufferASBD, 0, sizeof (sampleBufferASBD));
        
        [sampleBufferASBDValue getValue:&sampleBufferASBD];
        
        // Output is similar to input so a starting point is nice.
        outputBufferASBD = sampleBufferASBD;
        /* - FYI - flags list from the header file in CoreAudio
         
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

        FillOutASBDForLPCM(outputBufferASBD, DESIRED_AUDIO_FREQUENCY, DESIRED_NUM_CHANNELS_OUT, DESIRED_BITS_PER_SAMPLE, DESIRED_BITS_PER_SAMPLE, false, false, false);
        
        // Open the audio converter
        OSStatus stat = AudioConverterNew(&sampleBufferASBD, &outputBufferASBD, &Converter);
        assert(stat==0);
    } // end if (!Converter)

    assert(Converter);

    
    
    ///
    /// Part a) -- get inbound data and place it in a SafeBufferDeque for later use by the converter on its callback.
    ///
    bool bOk;
    
    bOk = pAudioInputQueue->AddItem((unsigned char*)[sampleBuffer bytesForAllSamples], [sampleBuffer lengthForAllSamples]);
    if (!bOk)
        std::cerr << "Audio Convertsion - Could not add incoming sample ot audioInputQueue. qSize=" << pAudioInputQueue->qsize() << endl;
    assert(bOk);
    
    

    ///
    /// Part b) -- If there is data ready from the converter, send it into RTBuffer for encoding.
    ///         -- NOTE: This is handled by the pFeedConverter thread now.
    ///

}

// Add these start and stop recording actions, and specify the output destination for your recorded media. The output is a QuickTime movie.

- (IBAction)startRecording:(id)sender
{
    // No longer allowed to change resolution due to inflexible encoder situation currently so have UI reflect this by disabling the res-change
    [captureResBox setEnabled:false];

    [captureButton setEnabled:false];
    [stopCaptureButton setEnabled:true];

    // Pick off the UI elements for bitrate and framerate.
    bitRate = [Vkbps intValue];
    bChangeBitrate=true;
    frameRate = [Vfps intValue];
    bChangeFramerate=true;

    // Now instantiate the 'connection' mechanism to the lower pipeline and call them to get the pipeline started.
    if (!pTVC)
        pTVC = new TVidCap(pCap);
    
    if (!p_pipeline_runner)
        p_pipeline_runner = new RunPipeline(pTVC,outputWidth,outputHeight,"UYVY",true,true);

    pCap->start_capturing();
}

- (IBAction)stopRecording:(id)sender
{
    pCap->stop_capturing();
    
    [captureButton setEnabled:true];
    [stopCaptureButton setEnabled:false];
    [captureResBox setEnabled:false];
}

- (IBAction)quitApplication:(id)sender {
    bQuit = true;
    
    sleep(0);   // Yield for other thread to pickup the 'bQuit'
    
    pCap->quit_capturing();
    
    [NSApp terminate:self];
}

- (IBAction)sendAudioRawData:(id)sender {
    sendAudioType = 2;
}

- (IBAction)sendAudioNoData:(id)sender {
    sendAudioType = 0;
}

- (IBAction)videoBitrateChanged:(id)sender {
    bitRate = [Vkbps intValue];
    bChangeBitrate=true;
}

- (IBAction)videoFramerateChanged:(id)sender {
    frameRate = [Vfps intValue];
    bChangeFramerate=true;

    [mCaptureDecompressedVideoOutput setMinimumVideoFrameInterval:1/(float)frameRate];
}


- (int)parseWidthHeight:(NSString*)pStr widthOut:(int*)pWidth heightOut:(int*)pHeight
{
    NSArray* pResStr;
    
    pResStr = [pStr componentsSeparatedByString: @"x"];
    
    // Give a try at a comma-separated version like '640,360'
    if ([pResStr count] != 2)
        pResStr = [pStr componentsSeparatedByString: @","];
    
    // Give a try at a space-separated version like '640 360'
    if ([pResStr count] != 2)
        pResStr = [pStr componentsSeparatedByString: @" "];
    
    if ([pResStr count] != 2)
    {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Bad format for capture resolution"];
        [alert setInformativeText:@"Must be of the form 'width x height'.\nFor example '640 x 360' is valid."];
        [alert runModal];
        [alert release];
        
        return -1;
    }
    
    *pWidth = [[pResStr objectAtIndex:0] intValue];
    *pHeight = [[pResStr objectAtIndex:1] intValue];

    return 0;
}

- (IBAction)captureResolution:(id)sender {
    int width, height;
    int result;
    
    result = [self parseWidthHeight:[captureResBox stringValue] widthOut:&width heightOut:&height];
    
    if (result)
        return;
    
    if (width < 0 || width > 4096 || height < 0 || height > 3072)
    {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Bad format for capture resolution"];
        [alert setInformativeText:@"You must use the form 'width x height'.\nFor example '640 x 360' is valid."];
        [alert runModal];
        [alert release];
        
        return;
    }
    
    if (width % MODULO_REQUIRED || height % MODULO_REQUIRED)
    {
        // We must modify the width and height to become mod-4. Inform the user.
        // Encoders generally won't allow odd numbers and sometimes not even all evens
        // and need modulo 4 values.
        int nonmodWidth, nonmodHeight;
        
        nonmodWidth = width;
        nonmodHeight = height;

        if (width % MODULO_REQUIRED)
            width = width + (MODULO_REQUIRED - (width % MODULO_REQUIRED));
        
        if (height % MODULO_REQUIRED)
            height = height + (MODULO_REQUIRED - (height % MODULO_REQUIRED));
        
        // Now inform the user.
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Desired capture resolution modified."];
        [alert setInformativeText:[NSString stringWithFormat:@"Requested resolution of (%d x %d)\nwas modified to become (%d x %d).\nVideo encoder requires modulo-%d values.", nonmodWidth, nonmodHeight, width, height, MODULO_REQUIRED]];
        [alert runModal];
        [alert release];
    }
    
    // Set capture resolution based on desired output resolution in width/height
    [self setDisplayResolutionWidth:width withHeight:height];
        
}

- (int) CaptureMatchForDesiredWidth:(int)desWidth forDesiredHeight:(int)desHeight
        CaptureWidth:(int *)pCaptureWidth CaptureHeight:(int *)pCaptureHeight
{
    int curWidth, curHeight;
    int lastCandidateWidth, lastCandidateHeight;
    
    // No candidates yet...
    lastCandidateWidth = lastCandidateHeight = -1;

    // Map a desired width and height
    // pCameraResolutions is sorted by the width-field at this time.
    for (id e in pCameraResolutions){
//        std::cerr << [(NSString*)e intValue] << " -- " << [(NSString*)e cString] << std::endl;

        if ([self parseWidthHeight:e widthOut:&curWidth heightOut:&curHeight] == 0)
        {
            if (desWidth <= curWidth && desHeight <= curHeight) {
                // We have a potential winner.
                // If we have no last candidate, set the 'lastCandidate'
                // Or if the delta between the current height/desired height is less
                //    than the lastCandidate delta, then we have a BETTER match - so set
                //    a new lastCandidate.
                if (lastCandidateWidth==-1 || 
                    ((curWidth-desWidth) + (curHeight-desHeight) < 
                        ((lastCandidateWidth-desWidth) + (lastCandidateHeight-desHeight))))
                    {
                        lastCandidateWidth = curWidth;
                        lastCandidateHeight = curHeight;
                    }
            }
        }
        else
            assert(false); // Shouldn't have a parse error in our own formatted data.
    }
    
    if (lastCandidateWidth != -1)
    {
        *pCaptureWidth = lastCandidateWidth;
        *pCaptureHeight = lastCandidateHeight;
        return 0;
    }
/*
    if (desWidth==640 && desHeight==360)
    {
        *pCaptureWidth = 640;
        *pCaptureHeight = 480;
        return 0;
    }
    
    if (desWidth==1280 && desHeight==720)
    {
        *pCaptureWidth = 1280;
        *pCaptureHeight = 720;
        return 0;
    }
  */  
    return -1;
}

- (int)setDisplayResolutionWidth:(int)displayWidth withHeight:(int)displayHeight
{
    int captureWidthResult, captureHeightResult;
    int result;
    
    result = [self CaptureMatchForDesiredWidth:displayWidth
                                forDesiredHeight:displayHeight
                                CaptureWidth:&captureWidthResult
                                CaptureHeight:&captureHeightResult];
    
    if (result != 0)
    {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Desired capture resolution was not available."];
        [alert setInformativeText:[NSString stringWithFormat:@"Requested resolution of (%d x %d)\nis not available.", displayWidth, displayHeight]];
        [alert runModal];
        [alert release];
        
        [captureResBox setStringValue:[NSString stringWithFormat:@"%d x %d", outputWidth, outputHeight]];

        return -99;
    }
    else
    {
        // Reflect new resolutions in class variables then set it up.
        outputWidth = displayWidth;
        outputHeight = displayHeight;
        
        captureWidth = captureWidthResult;
        captureHeight = captureHeightResult;
        
        // Setup the capture change.
        [mCaptureDecompressedVideoOutput setPixelBufferAttributes:
         [NSDictionary dictionaryWithObjectsAndKeys:
          [NSNumber numberWithDouble:captureWidth], (id)kCVPixelBufferWidthKey,
          [NSNumber numberWithDouble:captureHeight], (id)kCVPixelBufferHeightKey,
          //             [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32ARGB/*kCVPixelFormatType_422YpCbCr10*/], (id)kCVPixelBufferPixelFormatTypeKey,
          nil]];

        // Re-update the UI. If the user chose a non-modulo-4 value, the text box
        // will not be reflecting the proper value yet.
        [captureResBox setStringValue:[NSString stringWithFormat:@"%d x %d", outputWidth, outputHeight]];
    }
    
    return 0;
}

- (void)ExtractConvertedData
{
    UInt32 sampleSizeExpected = audioInputPostConversionNumPackets;
    
    // Must wait until the converter is created upon first sample reception.
    if (!Converter)
    {
        usleep(20 * 1000);
        return;
    }
    
    if ([mCaptureSession isRunning])
    {
        // Get Converted samples if any are available.
        OSStatus stat = AudioConverterFillComplexBuffer(Converter, iConverter, (void*)self, &sampleSizeExpected, &audioInputPostConversionBufferList, NULL);
        assert(stat==noErr || stat==1 || stat==2);
        
        if (stat == 1 || sampleSizeExpected==0)  // We're quitting. So bail out on this attempt.
            return;
        
        if (stat == 2)  // There was no available data. Skip conversion this go-round.
        {
            usleep(DESIRED_CONVERTEDOUTPUT_IN_TIME_MS * 1000);
            return;
        }
    }
    else
    {
        // Paused...so clear things out and sleep a bit.
        //
        // Clear out any residual and start fresh.
        //
        if (pAudioInputResidualData)
        {
            audioInputResidualBytesProcessed = 0;
            audioInputResidualTotalLength = 0;
            
            // Must delete as this pointer came from a SafeBufferDeque.RemoveItem()
            if (pAudioInputResidualData)
            {
                delete pAudioInputResidualData;
                pAudioInputResidualData = NULL;
            }
        }
        
        usleep(20 * 1000);
        
        // Skip as we're no longer running.
        return;
    }
    
    // Otherwise we have data. It should be the right size, right?
    assert(sampleSizeExpected == audioInputPostConversionNumPackets);
    
    QTKitBufferInfo* pBI;
//    QTTime qtt;
    
    pBI = new QTKitBufferInfo;
    assert(pBI);
    
    pBI->bIsVideo = false;
    
    pBI->pVideoFrame=NULL;
    pBI->pAudioSamples = NULL;  // Not using the sampleBuffer in RTBuffer when AudioConvert is utilized.
    
    pBI->pY = NULL;
    pBI->pCb = NULL;
    pBI->pCr = NULL;
    
//    qtt = [sampleBuffer presentationTime];
    pBI->timeStamp_uS = 0; // QTT_US(qtt);
    
    // Must 'delete' this array in Release via pBuffer
    pBI->pBuffer = new char[audioInputPostConversionSize];
    assert(pBI->pBuffer);
    memcpy(pBI->pBuffer, pAudioInputPostConversionData, audioInputPostConversionSize);
    pBI->pBufferLength = audioInputPostConversionSize;
    
    pBI->rawLength = audioInputPostConversionSize;
    pBI->rawNumSamples = audioInputPostConversionNumPackets;
    
    if (!pCap->GetBufferPointer()->FullBufferEnQ(pBI))
        NSLog(@"Audio Enqueue failed. No way to mark audio drops at this time.");
}

@end

int ExtractConverted::workerBee(void)
{
    while (!isStopRequested)
        [pController ExtractConvertedData];
    
    return 0;
}
