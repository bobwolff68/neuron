#ifndef H264STREAMVIEWER_H_
#define H264STREAMVIEWER_H_

#include <time.h>
#include <sys/time.h>
//#include <gtk/gtk.h>
//#include <gdk/gdkx.h>
#include "H264DecoderObject.h"
#include "VideoDisplayObject.h"

class H264StreamViewer : public EventHandlerT<H264StreamViewer>
{
    int64_t GetTimeMicrosecs(void)
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
    }

    private:

        int64_t             curPtsMus;
        int64_t             curPtsDeltaMus;
        int64_t             curDispTimeMus;
        int64_t             prevDispTimeMus;
        H264DecoderObject  *pDecoderObj;
        VideoDisplayObject *pDispObj;

        void EventHandleLoop(void)
        {
            //Implemented by VideoRefreshLoop() to be called by gtk_main()
            SDL_Event   Event;

            while(1)
            {
                SDL_PollEvent(&Event);
                if(Event.type==SDL_QUIT)    break;
                HandleNextEvent();
            }
        }

        void HandleMediaInputEvent(Event *pEvent)
        {
            RawVideoFrame      *pRawFrame = reinterpret_cast<MediaInputEvent<RawVideoFrame*>*>(pEvent)->GetData();
            struct timespec     ttw;
            int64_t             dispIntvlMus;
            int64_t             offsetMus;

            if(!curPtsDeltaMus)
            {
                curPtsMus = GetTimeMicrosecs();
                prevDispTimeMus = GetTimeMicrosecs();
            }

            curDispTimeMus = GetTimeMicrosecs();
            dispIntvlMus = curDispTimeMus-prevDispTimeMus;
            offsetMus = (curPtsDeltaMus-dispIntvlMus);

            if(offsetMus>0)
            {
                ttw.tv_sec = offsetMus/1000000;
                ttw.tv_nsec = ((offsetMus)%1000000)*1000;
                nanosleep(&ttw,NULL);
            }

            curDispTimeMus = GetTimeMicrosecs();
            pDispObj->Write(pRawFrame->pFrame,pRawFrame->pixelFormat);
            prevDispTimeMus = curDispTimeMus;
            curPtsDeltaMus = pRawFrame->ptsDeltaMus;
            curPtsMus += curPtsDeltaMus;

            delete pRawFrame;
            return;
        }

    public:

        H264StreamViewer(const char *decInFifoNameP,int resWidthP,int resHeightP):
        EventHandlerT<H264StreamViewer>()
        {
            curPtsMus = 0;
            prevDispTimeMus = 0;
            curPtsDeltaMus = 0;
            pDecoderObj = new H264DecoderObject(this,decInFifoNameP,resWidthP,resHeightP);
            AddHandleFunc(&H264StreamViewer::HandleMediaInputEvent,MEDIA_INPUT_EVENT);
            pDispObj = new VideoDisplayObject(resWidthP,resHeightP,SDL_YV12_OVERLAY,PIX_FMT_YUV420P);

            pDecoderObj->startThread();
            EventHandleLoop();
        }

        ~H264StreamViewer()
        {
            std::cout << "Deleting Viewer" << std::endl;

            pDecoderObj->stopThread();

            delete pDecoderObj;
            delete pDispObj;
        }
};

/*inline void VideoRefreshLoop(gpointer pData)
{
    H264StreamViewer *pViewer = (H264StreamViewer *) pData;
    pViewer->HandleNextEvent();
    return;
}*/

/*class H264StreamWindow
{
    private:

        std::string         Title;
        GtkWidget          *pMainWin;
        GtkWidget          *pFixedFrame;
        GtkWidget          *pVideoWin;
        H264StreamViewer   *pViewer;

    public:

        H264StreamWindow(const char *decInFifoNameP,int resWidthP,int resHeightP)
        {
            std::string SDLGTKHack("SDL_WINDOWID=");
            char sdlGtkHackStr[50];

            Title = "Neuron Endpoint: ";

            //Init GTK window
            //gtk_init(NULL,NULL);
            pMainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_default_size(GTK_WINDOW(pMainWin),resWidthP+10,resHeightP+10);
            gtk_window_set_title(GTK_WINDOW(pMainWin),(Title+decInFifoNameP).c_str());

            //Add fixed frame to window
            pFixedFrame = gtk_fixed_new();
            gtk_container_add(GTK_CONTAINER(pMainWin),pFixedFrame);

            //Add video screen
            pVideoWin = gtk_drawing_area_new();
            gtk_widget_set_size_request(pVideoWin,resWidthP,resHeightP);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pVideoWin,5,5);
            g_signal_connect(G_OBJECT(pMainWin),"destroy",G_CALLBACK(gtk_main_quit),NULL);
            gtk_widget_show_all(pMainWin);

            //Hack for SDL window to be assigned to GTK drawable area
            SDLGTKHack = SDLGTKHack + ToString<int>(GDK_WINDOW_XWINDOW(pVideoWin->window));
            strcpy(sdlGtkHackStr,SDLGTKHack.c_str());
            putenv(sdlGtkHackStr);

            //Start stream viewer
            pViewer = new H264StreamViewer(decInFifoNameP,resWidthP,resHeightP);
            gtk_idle_add((GtkFunction)VideoRefreshLoop,pViewer);
            //gtk_main();
        }

        ~H264StreamWindow()
        {
            delete pViewer;
        }

};*/

#endif // H264STREAMVIEWER_H_
