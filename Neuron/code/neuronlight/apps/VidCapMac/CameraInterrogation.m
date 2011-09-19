//
//  CameraInterrogation.m
//  
//
//  Created by Robert Wolff on 8/22/11.
//  Copyright 2011 XVD Technology LTD USA. All rights reserved.
//

#import "CameraInterrogation.h"

struct trial fullreq[] = {{80, 60}, {160,120}, {240,192}, {320,240}, {640,480}, {800,600}, {1024,768}, {1280,1024}, {1600,1200},                                 {160,90}, {240,136}, {320,180}, {640,360}, {800,448}, {1024,576}, {1280,720}, {1600,900}, {1920,1080}};

@implementation CameraInterrogation

- (id)init
{
    self = [super init];
    if (self) {
        // Initialization code here.
        [self reset];
        [self setupCapture];
    }
    
    return self;
}

- (void)dealloc
{
    if ([[mCaptureVideoDeviceInput device] isOpen])
        [[mCaptureVideoDeviceInput device] close];

	[mCaptureSession release];
	[mCaptureVideoDeviceInput release];
	
    [mCaptureDecompressedVideoOutput release];
    
	[super dealloc];
}

- (void)setupCapture
{
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
        
        mCaptureDecompressedVideoOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
        [mCaptureDecompressedVideoOutput setDelegate:self];
        
        // Set output characteristics. Especially video pixel format data.
        [mCaptureDecompressedVideoOutput setAutomaticallyDropsLateVideoFrames:YES];
        [mCaptureDecompressedVideoOutput setMinimumVideoFrameInterval:1/(float)60];
        [mCaptureDecompressedVideoOutput setPixelBufferAttributes:
         [NSDictionary dictionaryWithObjectsAndKeys:
          [NSNumber numberWithDouble:640], (id)kCVPixelBufferWidthKey,
          [NSNumber numberWithDouble:480], (id)kCVPixelBufferHeightKey,
          nil]];
        
        // Finally add the output to the session.
        success = [mCaptureSession addOutput:mCaptureDecompressedVideoOutput error:&error];
        if (!success) {
            // Handle error
        }
        
	} 

}

- (void)reset
{ 
    bSetSize=true;
    numResolutions = 0; 
    trialIteration = 0;
    bIsRunning = false;
}

- (void)startRunning
{
    assert(!bIsRunning);
    [mCaptureSession startRunning];
    bIsRunning = true;
}

- (void)stopRunning
{
    assert(bIsRunning);
    [mCaptureSession stopRunning];
    bIsRunning=false;
}

- (void)FullInternalInterrogation
{
    int i;
    
    for (i=0; i<sizeof(fullreq)/sizeof(struct trial); i++)
    {
        requested[i].width  = fullreq[i].width;
        requested[i].height = fullreq[i].height;
    }
    
    numResolutions = i-1;
    
    [self startRunning];
    
    while (bIsRunning) {
        usleep(250000);
    }
}

- (void)InterrogateCurrentResolutions
{
    [self startRunning];
}

- (bool)getBestCaptureResolutionForWidth :(int)w forHeight:(int)h forOutWidth:(int*)ow forOutHeight:(int*)oh
{
    struct trial candidates[MAX_REQUESTS];
    int numCandidates=0;
    int i;
    
    for(i=0;i<numResolutions;i++)
    {
        if (received[i].width >= w && received[i].height >= h)
        {
            candidates[numCandidates].width = received[i].width;
            candidates[numCandidates++].height = received[i].height;
        }
    }
    
    if (!numCandidates)
        return false;
    
    // Now pick the best candidate. Do this by finding the total # pixels in the candidates.
    // Find the smallest of the candidates and you've got the best one.
    int smallestpixels=0;
    int bestIndexSoFar=-1;
    int j;
    
    for (j=0;j<numCandidates;j++)
    {
        if (bestIndexSoFar==-1 || (candidates[j].width*candidates[j].height < smallestpixels))
        {
            smallestpixels = candidates[j].width*candidates[j].height;
            bestIndexSoFar = j;
        }
    }
    
    assert(smallestpixels);
    assert(bestIndexSoFar != -1);
    
    *ow = candidates[bestIndexSoFar].width;
    *oh = candidates[bestIndexSoFar].height;
    
    return true;
}

- (void)addResolutionWithWidth:(int)w withHeight:(int)h
{
    // Sanity.
    assert(w && h);
    assert(w <= 10000);
    assert(h <= 10000);
    
    requested[numResolutions].width = w;
    requested[numResolutions++].height = h;
    
    return;
}

- (void)captureOutput:(QTCaptureOutput *)captureOutput didOutputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    
    while (trialIteration < numResolutions)
    {
        if (bSetSize)
        {
            [mCaptureDecompressedVideoOutput setPixelBufferAttributes:
             [NSDictionary dictionaryWithObjectsAndKeys:
              [NSNumber numberWithDouble:requested[trialIteration].width], (id)kCVPixelBufferWidthKey,
              [NSNumber numberWithDouble:requested[trialIteration].height], (id)kCVPixelBufferHeightKey,
              nil]];
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
                    
                    received[trialIteration].width = w;
                    received[trialIteration].height = h;
                    NSLog(@"ON Iter#%d: Requested:%dx%d -- Received: %dx%d", trialIteration, requested[trialIteration].width, requested[trialIteration].height, w, h);
                }
                
            }
            
        }
        
        // Only iterate to the next resolution after the check has been made.
        if (!bSetSize)
            trialIteration++;
        
        if (trialIteration==numResolutions)
        {
            NSString *str=@"";
            int j;
            
            NSLog(@"Capture-Device Resolutions:");
            for (j=0;j<numResolutions;j++)
                [str stringByAppendingFormat:@"  %dx%d",received[j].width,received[j].height];
            NSLog(str);
        }
        
        // Toggle each frame - one go-thru sets a resolution and the next one tests it.
        bSetSize = !bSetSize;
        
        return;
    }

    // Once we get here, it's complete. Stop the session.
    [self stopRunning];
    trialIteration = 0;     // Reset for another set of trials.
}

@end
