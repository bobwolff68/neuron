#ifndef MULTISTREAMVIEWER_H_
#define MULTISTREAMVIEWER_H_

#include <sys/wait.h>
#include <sys/stat.h>
#include "H264StreamViewer.h"
#include "sessionfactory.h"

void OnDestroy(GtkWidget *pWidget,gpointer pData);

class FrameRateButtonCallbackArgs
{
    public:

        int             srcEntId;
        GtkWidget      *pCurFramerateLabel;
        SessionFactory *pSF;

        FrameRateButtonCallbackArgs(SessionFactory *pSFP,GtkWidget *pCurFramerateLabelP,int srcEntIdP):
        pSF(pSFP),srcEntId(srcEntIdP),pCurFramerateLabel(pCurFramerateLabelP)
        {
        }

        ~FrameRateButtonCallbackArgs()
        {
        }
};

void OnFramerateButtonPressed(GtkWidget *pWidget,gpointer pData)
{
    std::map<std::string,int>           LayerULimitMap;
    std::map<std::string,std::string>   CurFrmrtLblTextMap;
    std::string                         ButtonLabel = gtk_button_get_label((GtkButton *)pWidget);
    FrameRateButtonCallbackArgs        *pArg = (FrameRateButtonCallbackArgs *) pData;

    LayerULimitMap["Full Framerate"] = 2;
    LayerULimitMap["Half Framerate"] = 1;
    LayerULimitMap["Quarter Framerate"] = 0;

    CurFrmrtLblTextMap["Full Framerate"] = "Currently playing\nat full frame rate";
    CurFrmrtLblTextMap["Half Framerate"] = "Currently playing\nat half frame rate";
    CurFrmrtLblTextMap["Quarter Framerate"] = "Currently playing at\nquarter frame rate";

    pArg->pSF->SetH264DecoderSinkSubLayerULimit(1001,pArg->srcEntId,LayerULimitMap[ButtonLabel]);
    gtk_label_set_text(GTK_LABEL(pArg->pCurFramerateLabel),CurFrmrtLblTextMap[ButtonLabel].c_str());

    return;
}

class H264StreamWindow
{
    private:

        bool                            closed;
        pid_t                           viewerPid;
        int                             sigPipeFd[2];
        int                             srcEntId;
        std::string                     Title;
        GtkWidget                      *pMainWin;
        GtkWidget                      *pFixedFrame;
        GtkWidget                      *pVideoWin;
        GtkWidget                      *pTempScalButtons[3];
        GtkWidget                      *pCurFramerateLabel;
        GtkWidget                      *pRPChainLengthLabel;
        H264StreamViewer               *pViewer;
        FrameRateButtonCallbackArgs    *pArg;

    public:

        H264StreamWindow(SessionFactory *pSF,const char *decInFifoNameP,const char *srcNameP,int resWidthP,int resHeightP)
        {
            int             decInFifoExt;
            char            sdlGtkHackStr[50];
            std::string     SDLGTKHack("SDL_WINDOWID=");
            const char     *buttonTexts[3] = {"Full Framerate","Half Framerate","Quarter Framerate"};

            closed = false;
            Title = "Neuron Demo: ";

            sscanf(decInFifoNameP,"%d.%d",&srcEntId,&decInFifoExt);

            //Init GTK window
            pMainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_default_size(GTK_WINDOW(pMainWin),resWidthP+150,resHeightP+10);
            gtk_window_set_title(GTK_WINDOW(pMainWin),(Title+srcNameP).c_str());

            //Add fixed frame to window
            pFixedFrame = gtk_fixed_new();
            gtk_container_add(GTK_CONTAINER(pMainWin),pFixedFrame);

            //Add video screen
            pVideoWin = gtk_drawing_area_new();
            gtk_widget_set_size_request(pVideoWin,resWidthP,resHeightP);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pVideoWin,5,5);
            g_signal_connect(G_OBJECT(pMainWin),"destroy",G_CALLBACK(OnDestroy),this);

            //Add cur framerate label
            pCurFramerateLabel = gtk_label_new("Currently playing\nat full frame rate");
            gtk_widget_set_size_request(pCurFramerateLabel,135,resHeightP/8);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pCurFramerateLabel,resWidthP+10,5+3*resHeightP/4);

            //Add rp chain length label
            pRPChainLengthLabel = gtk_label_new("Current relay proxy\nchain length: 0");
            gtk_widget_set_size_request(pRPChainLengthLabel,135,resHeightP/8);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pRPChainLengthLabel,resWidthP+10,5+7*resHeightP/8);

            //Add scalability buttons
            pArg = new FrameRateButtonCallbackArgs(pSF,pCurFramerateLabel,srcEntId);
            for(int i=0; i<3; i++)
            {
                pTempScalButtons[i] = gtk_button_new_with_label(buttonTexts[i]);
                gtk_widget_set_size_request(pTempScalButtons[i],135,resHeightP/8);
                gtk_fixed_put(GTK_FIXED(pFixedFrame),pTempScalButtons[i],resWidthP+10,5+i*resHeightP/4);
                g_signal_connect(G_OBJECT(pTempScalButtons[i]),"clicked",G_CALLBACK(OnFramerateButtonPressed),pArg);
            }

            gtk_widget_show_all(pMainWin);

            //Hack for SDL window to be assigned to GTK drawable area
            SDLGTKHack = SDLGTKHack + ToString<int>(GDK_WINDOW_XWINDOW(pVideoWin->window));
            strcpy(sdlGtkHackStr,SDLGTKHack.c_str());
            putenv(sdlGtkHackStr);

            if(pipe(sigPipeFd)==-1)
            {
                std::cout << "Pipe failure" << std::endl;
                exit(0);
            }

            //Start stream viewer
            if((viewerPid=fork())==0)
            {
                close(sigPipeFd[1]);
                pViewer = new H264StreamViewer(decInFifoNameP,resWidthP,resHeightP,sigPipeFd[0]);
                exit(0);
            }
            else if(viewerPid>0)
                close(sigPipeFd[0]);
            else
            {
                std::cout << "Fork fail" << std::endl;
                exit(0);
            }
        }

        ~H264StreamWindow()
        {
            delete pArg;
        }

        void TellViewerToDie()
        {
            int viewerStatus;

            //Signal viewer thread to terminate and wait until it does
            write(sigPipeFd[1],"1",1);
            do
            {
                waitpid(viewerPid,&viewerStatus,WNOHANG|WUNTRACED);
            } while(!WIFEXITED(viewerStatus));

            close(sigPipeFd[1]);
            closed = true;

            return;
        }

        void UpdateRPChainLength(SessionFactory *pSF)
        {
            int         timesParsed = pSF->GetTimesParsed(1001,srcEntId);
            std::string RPChainLenLblText = "Current relay proxy\nchain length: ";

            RPChainLenLblText = RPChainLenLblText + ToString<int>(timesParsed);
            if(timesParsed>=3)  RPChainLenLblText = RPChainLenLblText + "(or more)";

            gtk_label_set_text(GTK_LABEL(pRPChainLengthLabel),RPChainLenLblText.c_str());

            return;
        }

        bool Closed(void)
        {
            return closed;
        }

        void RemoveH264DecoderSink(void)
        {
            pArg->pSF->RemoveH264DecoderSink(1001,srcEntId);
            return;
        }
};

void OnDestroy(GtkWidget *pWidget,gpointer pData)
{
    std::cout << "Exiting..." << std::endl;
    H264StreamWindow *pWin = (H264StreamWindow *)pData;
    gtk_widget_hide(pWidget);
    pWin->TellViewerToDie();
    pWin->RemoveH264DecoderSink();
    std::cout << "Exiting Done..." << std::endl;
    return;
}

class MultiStreamViewer
{
    private:

        std::map<int,H264StreamWindow*> WinList;

    public:

        MultiStreamViewer()
        {
        }

        ~MultiStreamViewer()
        {
            for(std::map<int,H264StreamWindow*>::iterator it=WinList.begin(); it!=WinList.end(); it++)
                delete it->second;
            WinList.clear();
        }

        bool AddStream(SessionFactory *pSF,int srcEntId,const char *srcName,int resWidth,int resHeight)
        {
            bool retVal = false;

            if(WinList.find(srcEntId)==WinList.end())
            {
                retVal = true;
                WinList[srcEntId] = new H264StreamWindow(pSF,(ToString<int>(srcEntId)+".264").c_str(),srcName,resWidth,resHeight);
            }

            return retVal;
        }

        void RemoveClosedWins(void)
        {
            bool winListTraversed;

            do
            {
                winListTraversed = true;
                for(std::map<int,H264StreamWindow*>::iterator it=WinList.begin(); it!=WinList.end(); it++)
                {
                    if(it->second->Closed())
                    {
                        delete it->second;
                        WinList.erase(it->first);
                        winListTraversed = false;
                        break;
                    }
                }
            } while(!winListTraversed);

            return;
        }

        void UpdateRPChainLengths(SessionFactory *pSF)
        {
            for(std::map<int,H264StreamWindow*>::iterator it=WinList.begin(); it!=WinList.end(); it++)
                if(!it->second->Closed())
                    it->second->UpdateRPChainLength(pSF);

            return;
        }
};

#endif // MULTISTREAMVIEWER_H_
