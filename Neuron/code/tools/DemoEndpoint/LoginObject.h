#ifndef LOGINOBJECT_H_
#define LOGINOBJECT_H_

#include "LoginScreen.h"
#include "registration.h"

class LoginObject
{
    private:

        LoginScreen        *pLoginScreen;
        RegistrationClient *pRegClient;

    public:

        LoginObject(const char *regServLocalIP,std::string &UserNameOut,int &sfIdOut,int &sessionIdOut,int &ubrainIdOut)
        {
            bool bFlag;

            pLoginScreen = new LoginScreen(LOGIN_SCREEN_WIDTH_DEFAULT,LOGIN_SCREEN_HEIGHT_DEFAULT,UserNameOut);
            pRegClient = new RegistrationClient(regServLocalIP,80,true,UserNameOut.c_str());
            pRegClient->registerClient();
            std::cout << "Received gstun_id: " << pRegClient->publicPairs["gstun_id"] << std::endl;
            sfIdOut = FromString<int>(pRegClient->publicPairs["ep_sf_id"],bFlag);
            sessionIdOut = 1001;
            ubrainIdOut = 0;
        }

        ~LoginObject()
        {
            delete pLoginScreen;
            delete pRegClient;
        }
};

#endif //LOGINOBJECT_H_
