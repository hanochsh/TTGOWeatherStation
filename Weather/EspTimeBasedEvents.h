#pragma once
#include <Time.h>
#include <NTPClient.h> 
#include "xEspTaskRenderer.h"

enum EspEventType { NoEvent, StopAnimationEvnt, ResumeAnimationEvnt, StartNightEvnt, EndNightEvnt, StartSleepEvnt, 
	                MarketOpenEvnt, MarketCloseEvnt, GetTickerPeersEvnt, GetPeersQuatesEvnt, GoToDeepSleepEvnt};

#define Sun  0b00000001
#define Mon  0b00000010
#define Tue  0b00000100
#define Wed  0b00001000
#define Thu  0b00010000
#define Fri  0b00100000
#define Sat  0b01000000
#define Week 0b01111111
#define USAW 0b00111110
#define ISRW 0b00011111

const unsigned short WeekDays[7] = {Sun, Mon, Tue, Wed, Thu, Fri, Sat};

struct EspTimeEvent
{
	EspEventType mStartEvent;
	EspEventType mEndEvent;
	unsigned short mStartDay;
	int mStartHour;
	int mStartMinut;
	int mDuration;
	EspEventType mLastSent;
};


///////////////////////////////////////////////////
/// </summary>
///////////////////////////////////////////////////
class EspTimeEvents : public xEspTaskRenderer {
public:
	EspTimeEvents() :xEspTaskRenderer ("EspTimeEvents", 2000, 1, 0, 45000) { mEvents = NULL; mNumEvents = 0; mNTPClient = NULL; }
	~EspTimeEvents() { delete[] mEvents; }

	void addEvent(EspEventType startEvent, int startHour, int startMinuts, int duration , EspEventType endEvent);
	void addEventDay(EspEventType startEvent, unsigned short startDay, int startHour, int startMinuts, int duration, EspEventType endEvent);
	void renderTask(int opt, void* data);
	void preTaskLoop(void* data);
	
private:
	EspEventType  getEventTypeToSend(EspTimeEvent* event);
	static void vTimerUpdateCallback(TimerHandle_t xTimer);
	void updateNTPClient();

	void resizeArray(int newsize);
	EspTimeEvent *mEvents;
	int mNumEvents;

};