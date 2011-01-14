#ifndef VIDEODISPLAYOBJECT_H_
#define VIDEODISPLAYOBJECT_H_

#include <iostream>
#include <SDL/SDL.h>

extern "C"
{
#include <libswscale/swscale.h>
}

class SDLDisplay
{
    private:

       	int	            overlayType;
        int	            resWidth;
        int             resHeight;
        PixelFormat     pixelFormat;
        SDL_Rect	    DispRect;
        SDL_Surface    *pScreen;
        SDL_Overlay	   *pOverlay;

        void ConvertToOverlayPixelFormat(AVPicture *pNewPic,AVPicture *pOldPic,PixelFormat oldPixelFormat)
        {
            static struct SwsContext *pImgConvCtx = NULL;

            if(pImgConvCtx==NULL)
            {
                pImgConvCtx = sws_getContext(resWidth,resHeight,oldPixelFormat,resWidth,resHeight,
                                             pixelFormat,SWS_BICUBIC,NULL,NULL,NULL);
                if(pImgConvCtx==NULL)
                {
                    std::cout << "Cannot create conversion context" << std::endl;
                    exit(0);
                }
            }

            sws_scale(pImgConvCtx,pOldPic->data,pOldPic->linesize,0,resHeight,pNewPic->data,pNewPic->linesize);
            return;
        }

    public:

        SDLDisplay(int resWidthP,int resHeightP,int overlayTypeP,PixelFormat pixelFormatP):
        resWidth(resWidthP),resHeight(resHeightP),overlayType(overlayTypeP),pixelFormat(pixelFormatP)
        {
           	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER))
            {
                std::cout << "Could not initialize SDL display: " << SDL_GetError() << std::endl;
                exit(0);
            }

            pScreen = SDL_SetVideoMode(resWidth,resHeight,0,SDL_HWSURFACE);
            if(pScreen==NULL)
            {
                std::cout << "Could not set video mode" << std::endl;
                exit(0);
            }

            pOverlay = SDL_CreateYUVOverlay(resWidth,resHeight,overlayType,pScreen);
            DispRect.x = DispRect.y = 0;
            DispRect.w = resWidth;
            DispRect.h = resHeight;
        }

        ~SDLDisplay()
        {
            SDL_Quit();
        }

        void WriteFrame(AVFrame *pFrame,PixelFormat framePixelFormat)
        {
            AVPicture	DispPic;

            // Load converted frame onto overlay
            if(pOverlay!=NULL)
            {
                SDL_LockYUVOverlay(pOverlay);
                DispPic.data[0] = pOverlay->pixels[0];
                DispPic.data[1] = pOverlay->pixels[2];
                DispPic.data[2] = pOverlay->pixels[1];
                DispPic.linesize[0] = pOverlay->pitches[0];
                DispPic.linesize[1] = pOverlay->pitches[2];
                DispPic.linesize[2] = pOverlay->pitches[1];
                ConvertToOverlayPixelFormat(&DispPic,(AVPicture *)pFrame,framePixelFormat);
                SDL_UnlockYUVOverlay(pOverlay);

                // Display overlaid frame
                SDL_DisplayYUVOverlay(pOverlay,&DispRect);
            }

            return;
        }
};

class VideoDisplayObject
{
    private:

        SDLDisplay *pDisp;

    public:

        VideoDisplayObject(int resWidthP,int resHeightP,int overlayTypeP,PixelFormat pixelFormatP)
        {
            pDisp = new SDLDisplay(resWidthP,resHeightP,overlayTypeP,pixelFormatP);
        }

        ~VideoDisplayObject()
        {
            std::cout << "Deleting display" << std::endl;
            delete pDisp;
        }

        void Write(AVFrame *pFrame,PixelFormat framePixelFormat)
        {
            pDisp->WriteFrame(pFrame,framePixelFormat);
            return;
        }
};

#endif // VIDEODISPLAYOBJECT_H_
