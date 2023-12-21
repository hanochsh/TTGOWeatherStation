#pragma once
#include <Time.h>
#include <NTPClient.h> 


enum EspEvetType { StartNight, EndNight, StartSleep };

struct EspTimeEvent
{
	int mEvent;
	int mHour;
	int mMinut;
};


class EspTimeEvents
{
public:
	EspTimeEvents(NTPClient* ntpTime) { mEvents = NULL; mNumEvents = 0; mNTPTime = ntpTime; }
	EspTimeEvents() { mEvents = NULL; mNumEvents = 0; mNTPTime = NULL; }
	~EspTimeEvents() { delete[] mEvents; }

	void addEvent(int type, int hour, int minuts );
	static void TaskCore(void* pvParameters);
	static void setNTPClient(void* ntpTime) { mNTPTime = ntpTime; }
private:
	static void* mNTPTime;
	void resizeArray(int newsize);
	EspTimeEvent *mEvents;
	int mNumEvents;

};