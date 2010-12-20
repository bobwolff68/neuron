#include "LoginObject.h"
#include "MainScreen.h"
#include "sessionfactory.h"

#define DEFAULT_UBRAIN_ID   0
#define DEFAULT_DOMAIN_ID   67

void OfferSourceToUBrain(SessionFactory *pSF,const char *srcName,int entityType,int resW,int resH)
{
    std::string                     OfferString;
    com::xvd::neuron::lscp::State  *pState = NULL;

    OfferString = ToString<int>(entityType)+","+srcName+","+ToString<int>(resW)+","+ToString<int>(resH);

    std::cout << "Offering: " << OfferString << std::endl;

    pState = com::xvd::neuron::lscp::StateTypeSupport::create_data();
    pState->srcId = pSF->GetId();
    pState->sessionId = 1001;
    pState->state = com::xvd::neuron::OBJECT_STATE_OFFERSRC;
    strcpy(pState->payload,OfferString.c_str());
    pSF->SignalEvent(new LSCPEventSessionStateUpdate(pState,new (DDS_SampleInfo)));
    com::xvd::neuron::lscp::StateTypeSupport::delete_data(pState);
    return;
}

void SelectRmtSource(SessionFactory *pSF,int gueid,int sinkType)
{
    std::string                     SelectString;
    com::xvd::neuron::lscp::State  *pState = NULL;

    SelectString = ToString<int>(gueid)+","+ToString<int>(sinkType);

    std::cout << "Requesting: " << SelectString << std::endl;

    pState = com::xvd::neuron::lscp::StateTypeSupport::create_data();
    pState->srcId = pSF->GetId();
    pState->sessionId = 1001;
    pState->state = com::xvd::neuron::OBJECT_STATE_SELECTSRC;
    strcpy(pState->payload,SelectString.c_str());
    pSF->SignalEvent(new LSCPEventSessionStateUpdate(pState,new (DDS_SampleInfo)));
    com::xvd::neuron::lscp::StateTypeSupport::delete_data(pState);
}

class DemoEndpoint
{
    private:

        int     epsfId;
        int     sessionId;
        int     ubrainId;

        std::string                         Name;
        LoginObject                        *pLoginObj;
        MainScreen                         *pMainScreen;

    public:

        SessionFactory *pSF;

        DemoEndpoint(const char *regServLocalIP)
        {
            pLoginObj = new LoginObject(regServLocalIP,Name,epsfId,sessionId,ubrainId);

            std::cout << "Name: " << Name << std::endl;
            std::cout << "epsfId: " << epsfId << std::endl;
            std::cout << "sessionId: " << sessionId << std::endl;
            std::cout << "ubrainId: " << ubrainId << std::endl;

            pSF = new SessionFactory(epsfId,Name.c_str(),DEFAULT_UBRAIN_ID,DEFAULT_DOMAIN_ID);
            pSF->startThread();

            pMainScreen = new MainScreen(pSF,500,200,Name);
        }

        ~DemoEndpoint()
        {
            std::cout << "Deleting Endpoint" << std::endl;
            delete pMainScreen;
            pSF->stopThread();
            delete pSF;
            delete pLoginObj;
        }
};

int main(int argc,char *argv[])
{
    gtk_init(&argc,&argv);
    DemoEndpoint dep(argv[1]);
    return 0;
}
