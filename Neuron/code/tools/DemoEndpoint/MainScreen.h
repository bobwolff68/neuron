#ifndef MAINSCREEN_H_
#define MAINSCREEN_H_

#include <string>
#include "MultiStreamViewer.h"
#include "sessionfactory.h"

std::map<std::string,std::string> LocSrcList;

void OnSelLocSrcPressed(GtkWidget *pWidget,gpointer pData);
void OnSelRmtSrcPressed(GtkWidget *pWidget,gpointer pData);
void OnRmtSrcListPopupOrPopdown(GtkComboBox *pWidget,gpointer pData);
void OfferSourceToUBrain(SessionFactory *pSF,const char *srcName,int entityType,int resW,int resH);
void SelectRmtSource(SessionFactory *pSF,int gueid,int sinkType);
void IdleLoop(gpointer pData);

std::string ConvSrcNameForDropdownDisplay(const char *srcName)
{
    char                buf[100];
    std::string         SrcName;
    std::stringstream   sstream;

    //<srcname>~<resw>~<resh> ==> <srcname> (<resW>x<resh>)
    sstream << srcName;

    sstream.getline(buf,100,'~');
    SrcName = buf;
    SrcName = SrcName + " (";

    sstream.getline(buf,100,'~');
    SrcName = SrcName + buf + "x";

    sstream.getline(buf,100);
    SrcName = SrcName + buf + ")";

    return SrcName;
}

std::string ConvertDisplayNameToSrcListKey(const char *dispSrcName)
{
    int                 resW;
    int                 resH;
    char                buf[100];
    std::string         SrcNameKey;
    std::string         SrcRes;
    std::stringstream   sstream;

    //<srcname>~<resw>~<resh> <== <srcname> (<resW>x<resh>)
    sstream << dispSrcName;

    sstream >> SrcNameKey;
    SrcNameKey = SrcNameKey + "~";

    sstream >> SrcRes;
    sscanf(SrcRes.c_str(),"(%dx%d)",&resW,&resH);
    SrcNameKey = SrcNameKey + ToString<int>(resW) + "~" + ToString<int>(resH);

    return SrcNameKey;
}

class  GtkCallbackArgs
{
    public:

        MultiStreamViewer  *pMV;
        SessionFactory     *pSF;
        GtkWidget          *pRmtSrcList;
        GtkWidget          *pLocSrcList;
        std::string         UserName;
        int                *pNRmtSrcs;

         GtkCallbackArgs(MultiStreamViewer *pMVP,SessionFactory *pSFP,GtkWidget *pRmtSrcListP,
                         GtkWidget *pLocSrcListP,int *pNRmtSrcsP,std::string &UserNameP)
         {
             pMV = pMVP;
             pSF = pSFP;
             pRmtSrcList = pRmtSrcListP;
             pLocSrcList = pLocSrcListP;
             pNRmtSrcs = pNRmtSrcsP;
             UserName = UserNameP;
         }

         ~GtkCallbackArgs()
         {
         }
};

class MainScreen
{
    private:

        int                *pNRmtSrcs;
        GtkWidget          *pWindow;
        GtkWidget          *pFixedFrame;
        GtkWidget          *pRmtSrcList;
        GtkWidget          *pSelRmtSrcButton;
        GtkWidget          *pLocSrcList;
        GtkWidget          *pSelLocSrcButton;
        GtkCallbackArgs    *pArg;
        MultiStreamViewer  *pMultiViewer;

    public:

        MainScreen(SessionFactory *pSF,int widthP,int heightP,std::string &UserName)
        {
            std::string Title("Neuron Demo Endpoint (User: ");

            pNRmtSrcs = new int(1);
            Title = Title + UserName + ")";
            pMultiViewer = new MultiStreamViewer();

            //Hardwire local source list
            LocSrcList["UCLA Talk 1"] = "TalkingHead1~640~360";
            LocSrcList["UCLA Talk 2"] = "TalkingHead1~640~360";
            LocSrcList["UCLA Talk 3"] = "TalkingHead3~640~360";
            LocSrcList["Joe"] = "Joe~720~480";
            LocSrcList["Clip from 'Up'"] = "Up~1280~720";

            //Create main window
            pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_default_size(GTK_WINDOW(pWindow),widthP,heightP);
            gtk_window_set_title(GTK_WINDOW(pWindow),Title.c_str());
            g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(gtk_main_quit),NULL);

            //Add fixed frame to window
            pFixedFrame = gtk_fixed_new();
            gtk_container_add(GTK_CONTAINER(pWindow),pFixedFrame);

            //Add remote source dropdown box to frame
            pRmtSrcList = gtk_combo_box_new_text();
            gtk_combo_box_append_text(GTK_COMBO_BOX(pRmtSrcList),"Subscribe to remote source...");
            gtk_combo_box_set_active(GTK_COMBO_BOX(pRmtSrcList),0);
            gtk_widget_set_size_request(pRmtSrcList,widthP/2,30);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pRmtSrcList,widthP/4,10);

            //Add offer source dropdown box to frame
            pLocSrcList = gtk_combo_box_new_text();
            gtk_widget_set_size_request(pLocSrcList,widthP/2,30);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pLocSrcList,widthP/4,100);

            //Populate local src dropdown
            gtk_combo_box_append_text(GTK_COMBO_BOX(pLocSrcList),"Publish local source...");
            gtk_combo_box_append_text(GTK_COMBO_BOX(pLocSrcList),"Video Source 1");
            gtk_combo_box_append_text(GTK_COMBO_BOX(pLocSrcList),"Video Source 2");
            gtk_combo_box_append_text(GTK_COMBO_BOX(pLocSrcList),"Video Source 3");
            gtk_combo_box_append_text(GTK_COMBO_BOX(pLocSrcList),"Video Source 4");
            gtk_combo_box_set_active(GTK_COMBO_BOX(pLocSrcList),0);

            pArg = new GtkCallbackArgs(pMultiViewer,pSF,pRmtSrcList,pLocSrcList,pNRmtSrcs,UserName);
            g_signal_connect(G_OBJECT(pRmtSrcList),"popdown",G_CALLBACK(OnRmtSrcListPopupOrPopdown),pArg);
            g_signal_connect(G_OBJECT(pRmtSrcList),"popup",G_CALLBACK(OnRmtSrcListPopupOrPopdown),pArg);

            //Add rmt src add button to frame
            pSelRmtSrcButton = gtk_button_new_with_label("Subscribe");
            gtk_widget_set_size_request(pSelRmtSrcButton,100,30);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pSelRmtSrcButton,widthP/2-50,45);
            g_signal_connect(G_OBJECT(pSelRmtSrcButton),"clicked",G_CALLBACK(OnSelRmtSrcPressed),pArg);

            //Add loc src add button to frame
            pSelLocSrcButton = gtk_button_new_with_label("Publish");
            gtk_widget_set_size_request(pSelLocSrcButton,100,30);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pSelLocSrcButton,widthP/2-50,135);
            g_signal_connect(G_OBJECT(pSelLocSrcButton),"clicked",G_CALLBACK(OnSelLocSrcPressed),pArg);

            //Display window and start main loop
            gtk_widget_show_all(pWindow);
            gtk_idle_add((GtkFunction)IdleLoop,this);
            gtk_main();
        }

        ~MainScreen()
        {
            delete pNRmtSrcs;
            delete pArg;
            delete pMultiViewer;
        }

        void UpdateRPChainLengths()
        {
            pMultiViewer->UpdateRPChainLengths(pArg->pSF);
            return;
        }
};

void OnSelRmtSrcPressed(GtkWidget *pWidget,gpointer pData)
{
    GtkCallbackArgs *pArg = (GtkCallbackArgs *)pData;
    gchar *srcName = gtk_combo_box_get_active_text((GtkComboBox *)(pArg->pRmtSrcList));

    if((*(pArg->pNRmtSrcs))>0 && strcmp(srcName,"Subscribe to remote source..."))
    {
        std::string SrcNameKey = ConvertDisplayNameToSrcListKey(srcName);
        int         srcEntId = pArg->pSF->GetEntityIdForRmtSrc(1001,SrcNameKey.c_str());

        if(srcEntId>-1)
        {
            int                 resW;
            int                 resH;
            char                buf[100];
            std::stringstream   sstream;

            sstream << SrcNameKey;
            sstream.getline(buf,100,'~');
            sstream.getline(buf,100);
            sscanf(buf,"%d~%d",&resW,&resH);
            std::cout << "Res: " << resW << "x" << resH << std::endl;

            SelectRmtSource(pArg->pSF,srcEntId,ENTITY_KIND_H264DECODERSINK);
            pArg->pMV->RemoveClosedWins();
            pArg->pMV->AddStream(pArg->pSF,srcEntId,srcName,resW,resH);
        }
    }

    gtk_combo_box_set_active((GtkComboBox *)(pArg->pRmtSrcList),0);
    return;
}

void OnRmtSrcListPopupOrPopdown(GtkComboBox *pWidget,gpointer pData)
{
    GtkCallbackArgs            *pArg = (GtkCallbackArgs *)pData;
    std::map<std::string,int>   RmtSrcList;

    if(pArg->pSF->GetRmtSrcEntIdList(1001,RmtSrcList))
    {
        while((*(pArg->pNRmtSrcs))>1)
        {
            std::cout << "n: " << (*(pArg->pNRmtSrcs)) << std::endl;
            gtk_combo_box_remove_text((GtkComboBox *)(pArg->pRmtSrcList),(*(pArg->pNRmtSrcs))-1);
            (*(pArg->pNRmtSrcs))--;
        }

        std::cout << "nNew: " << *(pArg->pNRmtSrcs) << std::endl;
        for(std::map<std::string,int>::iterator it=RmtSrcList.begin(); it!=RmtSrcList.end(); it++)
        {
            char                rmtSrcUserName[100];
            std::string         DispSrcName = ConvSrcNameForDropdownDisplay(it->first.c_str());
            std::stringstream   sstream;

            sstream << DispSrcName;
            sstream.getline(rmtSrcUserName,99,'/');
            if(strcmp(rmtSrcUserName,pArg->UserName.c_str()))
            {
                gtk_combo_box_append_text((GtkComboBox *)(pArg->pRmtSrcList),DispSrcName.c_str());
                (*(pArg->pNRmtSrcs))++;
            }
        }
    }

    return;
}

void OnSelLocSrcPressed(GtkWidget *pWidget,gpointer pData)
{
    GtkCallbackArgs            *pArg = (GtkCallbackArgs *)pData;
    gchar                      *locSrcDispName = gtk_combo_box_get_active_text((GtkComboBox *)(pArg->pLocSrcList));
    int                         resW;
    int                         resH;
    char                        buf[100];
    std::string                 LocSrcName;
    std::stringstream           sstream;

    if(strcmp(locSrcDispName,"Publish local source..."))
    {
        sstream << LocSrcList[locSrcDispName];
        sstream.getline(buf,100,'~');
        LocSrcName = buf;

        sstream.getline(buf,100,'~');
        sscanf(buf,"%d",&resW);

        sstream.getline(buf,100);
        sscanf(buf,"%d",&resH);
        OfferSourceToUBrain(pArg->pSF,LocSrcName.c_str(),ENTITY_KIND_H264FILESRC,resW,resH);
    }

    gtk_combo_box_set_active((GtkComboBox *)(pArg->pLocSrcList),0);
    return;
}

void IdleLoop(gpointer pData)
{
    MainScreen *pMainScreen = (MainScreen *) pData;

    usleep(20000);
    pMainScreen->UpdateRPChainLengths();
    return;
}

#endif // MAINSCREEN_H_
