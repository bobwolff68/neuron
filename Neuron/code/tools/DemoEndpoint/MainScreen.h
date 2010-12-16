#ifndef MAINSCREEN_H_
#define MAINSCREEN_H_

#include <string>
#include <gtk/gtk.h>
#include "sessionfactory.h"

void OnSelRmtSrcPressed(GtkWidget *pWidget,gpointer pData);
void OnRefreshRmtSrcListPressed(GtkWidget *pWidget,gpointer pData);
void OfferSourceToUBrain(SessionFactory *pSF,const char *srcName,int entityType,int resW,int resH);
void SelectRmtSource(SessionFactory *pSF,int gueid,int sinkType);

class  GtkCallbackArgs
{
    public:

        SessionFactory *pSF;
        GtkWidget      *pRmtSrcList;
        int            *pNRmtSrcs;

         GtkCallbackArgs(SessionFactory *pSFP,GtkWidget *pRmtSrcListP,int *pNRmtSrcsP)
         {
             pSF = pSFP;
             pRmtSrcList = pRmtSrcListP;
             pNRmtSrcs = pNRmtSrcsP;
         }

         ~GtkCallbackArgs()
         {
         }
};

class MainScreen
{
    private:

        int                *pNRmtSrcs;
        GList              *pRmtSrcItems;
        GtkWidget          *pWindow;
        GtkWidget          *pFixedFrame;
        GtkWidget          *pRmtSrcListLabel;
        GtkWidget          *pRmtSrcList;
        GtkWidget          *pSelRmtSrcButton;
        GtkWidget          *pRefreshRmtSrcListButton;
        GtkCallbackArgs    *pArg;

    public:

        MainScreen(SessionFactory *pSF,int widthP,int heightP,std::string UserName)
        {
            std::string Title("Neuron Demo Endpoint (User: ");

            pNRmtSrcs = new int(0);
            Title = Title + UserName + ")";

            //Create main window
            pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_default_size(GTK_WINDOW(pWindow),widthP,heightP);
            gtk_window_set_title(GTK_WINDOW(pWindow),Title.c_str());
            g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(gtk_main_quit),NULL);

            //Add fixed frame to window
            pFixedFrame = gtk_fixed_new();
            gtk_container_add(GTK_CONTAINER(pWindow),pFixedFrame);

            //Add src list label to window
            pRmtSrcListLabel = gtk_label_new("Remote Sources: ");
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pRmtSrcListLabel,widthP/4-120,10);

            //Add remote source dropdown box to frame
            pRmtSrcList = gtk_combo_box_new_text();
            gtk_widget_set_size_request(pRmtSrcList,widthP/2,25);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pRmtSrcList,widthP/4,10);

            pArg = new GtkCallbackArgs(pSF,pRmtSrcList,pNRmtSrcs);

            //Add src add button to frame
            pSelRmtSrcButton = gtk_button_new_with_label("Add");
            gtk_widget_set_size_request(pSelRmtSrcButton,50,25);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pSelRmtSrcButton,widthP/4,45);
            g_signal_connect(G_OBJECT(pSelRmtSrcButton),"clicked",G_CALLBACK(OnSelRmtSrcPressed),pArg);

            //Add refresh rmt src list button to frame
            pRefreshRmtSrcListButton = gtk_button_new_with_label("Refresh");
            gtk_widget_set_size_request(pRefreshRmtSrcListButton,65,25);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pRefreshRmtSrcListButton,3*widthP/4-65,45);
            g_signal_connect(G_OBJECT(pRefreshRmtSrcListButton),"clicked",G_CALLBACK(OnRefreshRmtSrcListPressed),pArg);

            //Display window and start main loop
            gtk_widget_show_all(pWindow);
            gtk_main();
        }

        ~MainScreen()
        {
            delete pArg;
        }
};

void OnSelRmtSrcPressed(GtkWidget *pWidget,gpointer pData)
{
    GtkCallbackArgs *pArg = (GtkCallbackArgs *)pData;
    gchar *srcName = gtk_combo_box_get_active_text((GtkComboBox *)(pArg->pRmtSrcList));

    if((*(pArg->pNRmtSrcs))>0 && srcName!=NULL)
    {
        int srcEntId = pArg->pSF->GetEntityIdForRmtSrc(1001,srcName);
        if(srcEntId>-1) SelectRmtSource(pArg->pSF,srcEntId,ENTITY_KIND_H264DECODERSINK);
    }

    return;
}

void OnRefreshRmtSrcListPressed(GtkWidget *pWidget,gpointer pData)
{
    GtkCallbackArgs            *pArg = (GtkCallbackArgs *)pData;
    std::map<std::string,int>   RmtSrcList;

    if(pArg->pSF->GetRmtSrcEntIdList(1001,RmtSrcList))
    {
        while((*(pArg->pNRmtSrcs))>0)
        {
            std::cout << "n: " << (*(pArg->pNRmtSrcs)) << std::endl;
            gtk_combo_box_remove_text((GtkComboBox *)(pArg->pRmtSrcList),(*(pArg->pNRmtSrcs))-1);
            (*(pArg->pNRmtSrcs))--;
        }

        *(pArg->pNRmtSrcs) = RmtSrcList.size();
        std::cout << "nNew: " << *(pArg->pNRmtSrcs) << std::endl;
        for(std::map<std::string,int>::iterator it=RmtSrcList.begin(); it!=RmtSrcList.end(); it++)
            gtk_combo_box_append_text((GtkComboBox *)(pArg->pRmtSrcList),it->first.c_str());
    }

    return;
}
#endif // MAINSCREEN_H_
