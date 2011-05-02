#ifndef MEDIAEVENT_H_
#define MEDIAEVENT_H_

#include "entityinfo.h"
#include "entityinfoSupport.h"
#include "neuroncommon.h"

#define MEDIA_INPUT_EVENT				1000
#define ENTINFO_INPUT_EVENT				MEDIA_INPUT_EVENT+1
#define	UPLINE_ENTITY_LOST_EVENT		MEDIA_INPUT_EVENT+2
#define	UPLINE_ENTITY_SHUTDOWN_EVENT	MEDIA_INPUT_EVENT+3

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

class EntInfoInputEvent : public Event
{
	private:

		com::xvd::neuron::media::EntityInfo	EntInfo;

	public:

		EntInfoInputEvent(com::xvd::neuron::media::EntityInfo EntInfoP):
		Event(ENTINFO_INPUT_EVENT)
		{
			EntInfo = EntInfoP;
		}

		~EntInfoInputEvent()
		{
		}

		com::xvd::neuron::media::EntityInfo GetEntInfo(void)
		{
			return EntInfo;
		}
};

class UplineEntLostEvent : public Event
{
	public:

		UplineEntLostEvent() : Event(UPLINE_ENTITY_LOST_EVENT)
		{
		}

		~UplineEntLostEvent()
		{
		}
};

class UplineEntShutdownEvent : public Event
{
	public:

		UplineEntShutdownEvent() : Event(UPLINE_ENTITY_SHUTDOWN_EVENT)
		{
		}

		~UplineEntShutdownEvent()
		{
		}
};

#endif // MEDIAEVENT_H_
