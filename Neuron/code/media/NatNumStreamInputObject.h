#ifndef NATNUMSTREAMINPUTOBJECT_H_
#define NATNUMSTREAMINPUTOBJECT_H_

#include <unistd.h>
#include "MediaEvent.h"
#include "MediaInputObject.h"
#include "ThreadSingle.h"

#define MIO_LOG_PROMPT(ownerId) "MIO(" << ownerId << ")"

class NatNumStreamInputObject : public MediaInputObject
                              , public ThreadSingle
{
    private:

        int streamULimit;
        int streamPeriodSecs;

    public:

        NatNumStreamInputObject(EventHandler *pOwnerEventHandlerP,int ownerIdP,int uLimitP,int streamPeriodSecsP):
        MediaInputObject(pOwnerEventHandlerP,ownerIdP),ThreadSingle()
        {
            streamULimit = uLimitP;
            streamPeriodSecs = streamPeriodSecsP;
        }

        ~NatNumStreamInputObject()
        { }

        void StreamMedia(void)
        {
            MediaInputEvent<int> *pEvent = NULL;
            while(!isStopRequested)
            {
                for(int num=0; num<streamULimit && !isStopRequested; num++)
                {
                    pEvent = new MediaInputEvent<int>(num);
                    pOwnerEventHandler->SignalEvent((Event *)pEvent);
#ifdef VERBOSE_OUTPUT
                    std::cout << MIO_LOG_PROMPT(ownerId) << ": MediaInputEvent(Data=" << num << ")" << endl;
#endif
                    sleep(streamPeriodSecs);
                }
            }

            return;
        }

        int workerBee(void)
        {
            StreamMedia();

            return 0;
        }
};

#endif // NATNUMSTREAMINPUTOBJECT_H_
