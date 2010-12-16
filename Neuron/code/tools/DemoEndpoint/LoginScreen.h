#ifndef LOGINSCREEN_H_
#define LOGINSCREEN_H_

#include <string>
#include <gtk/gtk.h>

#define LOGIN_SCREEN_WIDTH_DEFAULT  500
#define LOGIN_SCREEN_HEIGHT_DEFAULT 100

void OnOkPressed(GtkWidget *pWidget,gpointer pData);

class LoginScreen
{
    private:

        std::string     UserName;
        GtkWidget      *pWindow;
        GtkWidget      *pFixedFrame;
        GtkWidget      *pLoginLabel;
        GtkWidget      *pLoginTextBox;
        GtkWidget      *pOkButton;

    public:

        LoginScreen(int widthP,int heightP,std::string &UserNameOut)
        {
            //Create login window
            pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_default_size(GTK_WINDOW(pWindow),widthP,heightP);
            gtk_window_set_title(GTK_WINDOW(pWindow),"Neuron Demo Endpoint Registration");
            g_signal_connect(G_OBJECT(pWindow),"destroy",G_CALLBACK(gtk_main_quit),NULL);

            //Add fixed frame to window
            pFixedFrame = gtk_fixed_new();
            gtk_container_add(GTK_CONTAINER(pWindow),pFixedFrame);

            //Add login label to window
            pLoginLabel = gtk_label_new("Login: ");
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pLoginLabel,widthP/4-50,10);

            //Add login text box to frame
            pLoginTextBox = gtk_entry_new();
            gtk_widget_set_size_request(pLoginTextBox,widthP/2,25);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pLoginTextBox,widthP/4,10);

            //Add ok button to frame
            pOkButton = gtk_button_new_with_label("OK");
            gtk_widget_set_size_request(pOkButton,50,25);
            gtk_fixed_put(GTK_FIXED(pFixedFrame),pOkButton,widthP/2-25,45);
            g_signal_connect(G_OBJECT(pOkButton),"clicked",G_CALLBACK(OnOkPressed),this);

            //Display window and start main loop
            gtk_widget_show_all(pWindow);
            gtk_main();

            UserNameOut = UserName;
        }

        ~LoginScreen()
        {
        }

        void SetUserName(const char *uname)
        {
            UserName = uname;
            return;
        }

        const char *GetLoginText(void)
        {
            return gtk_entry_get_text(GTK_ENTRY(pLoginTextBox));
        }

        void Hide(void)
        {
            gtk_widget_hide(pWindow);
        }
};

void OnOkPressed(GtkWidget *pWidget,gpointer pData)
{
    LoginScreen *pScr = (LoginScreen *)pData;
    pScr->SetUserName(pScr->GetLoginText());
    pScr->Hide();
    gtk_main_quit();
    return;
}

#endif // LOGINSCREEN_H_
