#ifndef MEDIAEVENT_H_
#define MEDIAEVENT_H_

#include "eventhandler.h"

#define MEDIA_INPUT_EVENT   1000

template<typename MediaType> class MediaInputEvent : public Event
{
    private:

        MediaType   data;

    public:

        MediaInputEvent(MediaType dataP):Event(MEDIA_INPUT_EVENT)
        {
            data = dataP;
        }

        ~MediaInputEvent()
        { }

        MediaType GetData(void)
        {
            return data;
        }
};

#endif // MEDIAEVENT_H_
