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

        LoginObject(const char *regServLocalIP,std::string &UserNameOut,std::map<std::string,std::string> &LoginInfo)
        {
            bool bFlag;

            pLoginScreen = new LoginScreen(LOGIN_SCREEN_WIDTH_DEFAULT,LOGIN_SCREEN_HEIGHT_DEFAULT,UserNameOut);
            pRegClient = new RegistrationClient(regServLocalIP,8181,true,UserNameOut.c_str());
            pRegClient->registerClient();

            LoginInfo = pRegClient->publicPairs;
            //std::cout << "Received gstun_id: " << pRegClient->publicPairs["gstun_id"] << std::endl;
            //sfIdOut = FromString<int>(pRegClient->publicPairs["ep_sf_id"],bFlag);
            //sessionIdOut = 1001;
            //ubrainIdOut = 0;
        }

        ~LoginObject()
        {
            delete pLoginScreen;
            delete pRegClient;
        }
};

#endif //LOGINOBJECT_H_
