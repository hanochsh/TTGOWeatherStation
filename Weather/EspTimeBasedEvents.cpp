#include "EspTimeBasedEvents.h"
#include <string.h>
#include <HardwareSerial.h>
#include "DebugPrefTools.h"
#include "EspDebug.h"

extern SemaphoreHandle_t getWifiMutex();
extern QueueHandle_t     getMessageQueue();

///////////////////////////////////////////////////////////////
/// </summary>
void EspTimeEvents::addEvent(EspEventType startEvent, int startHour, int startMinuts, int duration /*minutes*/, EspEventType endEvent) {
    addEventDay(startEvent, Week, startHour, startMinuts, duration, endEvent);
}

///////////////////////////////////////////////////////////////
/// </summary>
void EspTimeEvents::addEventDay(EspEventType startEvent, unsigned short startDay, int startHour, int startMinuts, int duration, EspEventType endEvent)
{
  resizeArray(mNumEvents + 1);

  mEvents[mNumEvents].mStartDay = startDay;
  mEvents[mNumEvents].mStartEvent = startEvent;
  mEvents[mNumEvents].mStartHour = startHour;
  mEvents[mNumEvents].mStartMinut = startMinuts;
  mEvents[mNumEvents].mDuration = duration;
  mEvents[mNumEvents].mEndEvent = endEvent;
  mEvents[mNumEvents].mLastSent = NoEvent;
  mNumEvents++;
}

///////////////////////////////////////////////////////////////
/// </summary>
EspEventType EspTimeEvents::getEventTypeToSend(EspTimeEvent *event)
{
    int curHour   = mNTPClient->getHours();
    int curMinute = mNTPClient->getMinutes();
    int durHours  = event->mDuration / 60;
    int endHour   = event->mStartHour + durHours;
    int endMinute = event->mStartMinut + (event->mDuration % 60);
    while (endMinute >= 60)
    {
        endHour++;
        endMinute = endMinute - 60;
    };
    while (endHour >= 24)
    {
        endHour = endHour - 24;
    };
    /*
    Serial.println("EspTimeEvents::getEventTypeToSend: Cur Time" + String(curHour) + ":" + String(curMinute));
    Serial.println("EspTimeEvents::getEventTypeToSend: Start Time" + String(event->mStartHour) + ":" + String(event->mStartMinut));
    Serial.println("EspTimeEvents::getEventTypeToSend: Duration : " + String(durHours) + ":" + String((event->mDuration % 60)));
    Serial.println("EspTimeEvents::getEventTypeToSend: End Time" + String(endHour) + ":" + String(endMinute)); 
    Serial.println("EspTimeEvents::getEventTypeToSend: (curHour  >= event->mStartHour) : " + String((curHour >= event->mStartHour)));
    Serial.println("EspTimeEvents::getEventTypeToSend: (curMinute >= event->mStartMinut) : " + String((curMinute >= event->mStartMinut)));
    Serial.println("EspTimeEvents::getEventTypeToSend: (curHour <= endHour) : " + String((curHour <= endHour)));
    Serial.println("EspTimeEvents::getEventTypeToSend: (curMinute <= endMinute) : " + String((curMinute <= endMinute))); */

    unsigned short today = WeekDays[mNTPClient->getDay()];
   // Serial.println("EspTimeEvents::getEventTypeToSend:  event->mStartDay " + String(event->mStartDay));
    if ((0 == (today & event->mStartDay)) && (event->mLastSent == NoEvent))
    { /*
        Serial.println("EspTimeEvents::getEventTypeToSend:  Sorry, this Event does not start today");
        //Serial.println("EspTimeEvents::getEventTypeToSend:  mNTPClient->getDay() " + String(mNTPClient->getDay()));
        Serial.println("EspTimeEvents::getEventTypeToSend:  today " + String(today));
       
        Serial.println("EspTimeEvents::getEventTypeToSend:  (today & event->mStartDay) " + String((today & event->mStartDay)));
        Serial.println("EspTimeEvents::getEventTypeToSend:  (event->mLastSent == NoEvent) " + String((event->mLastSent == NoEvent)));*/
        return NoEvent;
    }

    //if ((curHour  >= event->mStartHour) && (curMinute >= event->mStartMinut ) &&
     //   (curHour <= endHour) && (  curMinute <= endMinute))
        if (((curHour > event->mStartHour) || ((curHour == event->mStartHour) && (curMinute >= event->mStartMinut) ))
            && ((curHour < endHour) || ((curHour == endHour) && (curMinute <= endMinute))))
            

    { // within Range of the event
            //Serial.println("EspTimeEvents::getEventTypeToSend: (After start): " + String(((curHour > event->mStartHour) || ((curHour == event->mStartHour) && (curMinute >= event->mStartMinut)))));
            //Serial.println("EspTimeEvents::getEventTypeToSend: (before end): " + String(((curHour < endHour)) || ((curHour == endHour) && (curMinute <= endMinute))));
        //Serial.println("EspTimeEvents::getEventTypeToSend:  Within Rang");
        if (event->mLastSent == NoEvent) // This is the first send
            return event->mLastSent = event->mStartEvent;
        

        if ((event->mLastSent == event->mStartEvent) &&// already send Start, sould check if end arrived
            ((curHour == endHour) && (curMinute == endMinute)))
        {
            //Serial.println("EspTimeEvents::getEventTypeToSend: END Event should be sent");
            return event->mLastSent = event->mEndEvent; // sending end event
        }

    }
    //Serial.println("EspTimeEvents::getEventTypeToSend: (After start): " + String(((curHour > event->mStartHour) || ((curHour == event->mStartHour) && (curMinute >= event->mStartMinut)))));
    //Serial.println("EspTimeEvents::getEventTypeToSend: (before end): " + String(((curHour < endHour)) || ((curHour == endHour) && (curMinute <= endMinute))));
   // Serial.println("EspTimeEvents::getEventTypeToSend: (curHour <= endHour): " + String((curHour <= endHour)));
   // Serial.println("EspTimeEvents::getEventTypeToSend: (curMinute <= endMinute): " + String((curMinute <= endMinute)));
    //Serial.println("EspTimeEvents::getEventTypeToSend:  Returning NoEvent");
    if (event->mLastSent == event->mEndEvent) // End event was already send
      return event->mLastSent = NoEvent;

    return NoEvent;
}

///////////////////////////////////////////////////////////////
/// </summary>
void EspTimeEvents::resizeArray(int newSize) {
  EspTimeEvent* tempArray = new EspTimeEvent[newSize];

  // Copy elements from the old array to the new array
  memcpy(tempArray, mEvents, sizeof(EspTimeEvent) * mNumEvents);

  // Deallocate the memory of the old array
  delete[] mEvents;

  // Point dynamicArray to the new array
  mEvents = tempArray;

  // Update the arraySize variable
  //mNumEvents = newSize; Resize does not mean the number of events changed
}
void deepSleepTimerCallback(TimerHandle_t xTimer) {
  Serial.println("Timer expired. Going to deep sleep...");
  esp_deep_sleep_start(); // Trigger deep sleep
}


///////////////////////////////////////////////////////////////
/// </summary>
void EspTimeEvents::updateNTPClient()
{
    SemaphoreHandle_t mWifiMutex = getWifiMutex();
    if (xSemaphoreTake(mWifiMutex, portMAX_DELAY)) {
        mNTPClient->update();
        //mNTPClient->forceUpdate();
     xSemaphoreGive(mWifiMutex);
    }
}

//////////////////////////////////////////////
//
void EspTimeEvents::vTimerUpdateCallback(TimerHandle_t xTimer)
{
    EspTimeEvents *pThis = (EspTimeEvents*)pvTimerGetTimerID(xTimer);
    pThis->updateNTPClient();
}

///////////////////////////////////////////////////////////////
/// </summary>
void EspTimeEvents::preTaskLoop(void* data)
{
    //Serial.println("EspTimeEvents::preTaskLoop before update NTP");
    updateNTPClient();
    // set timer to update the time every 5 min
    TimerHandle_t xTimer = xTimerCreate("vTimerUpdateCallback",
        pdMS_TO_TICKS(5*60000), pdTRUE, (void*)this, EspTimeEvents::vTimerUpdateCallback);
    xTimerStart(xTimer, 0);
    //Serial.println("EspTimeEvents::preTaskLoop After update NTP");
    //enum EspEvetType { StopAnimation, StartNight, EndNight, StartSleep };
    addEvent(StopAnimationEvnt, 22, 18, 2, ResumeAnimationEvnt); 
    addEvent(StopAnimationEvnt, 22, 51, 2, ResumeAnimationEvnt);
    addEvent(StartNightEvnt, 0, 30, 8*60, EndNightEvnt); 
    addEvent(StartNightEvnt, 15, 23, 2, EndNightEvnt);
    addEventDay(MarketOpenEvnt, USAW, 16, 30, 7 * 60, MarketCloseEvnt); 
    addEvent(GoToDeepSleepEvnt, 1, 30, 0, NoEvent); // go to sleep between 1:30 AM to 4:50 AM

}

///////////////////////////////////////////////////////////////
/// </summary>
void EspTimeEvents::renderTask(int opt, void* data)
{
    EspEventType sendMsg = NoEvent;
    int i;

    //updateNTPClient();
    if (mNTPClient->getEpochTime() < 100000)
    {
        LOG_ERR("Date/Time Not updated, skip checking events");
        return;
    }
    for (i = 0; i < mNumEvents; i++)
    {
        sendMsg = getEventTypeToSend(&mEvents[i]);
        if (sendMsg != NoEvent)
        {
            if (xQueueSend(getMessageQueue(), &sendMsg, 0) == pdTRUE) {
                Serial.print("Sent data to queue: ");
                Serial.println(sendMsg);
            }
        }
    }
}

#ifdef old

///////////////////////////////////////////////////////////////
/// </summary>
void EspTimeEvents::TaskCore(void* pvParameters) {
  NTPClient* ntpClient = (NTPClient*)pvParameters;
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 45000;

#ifdef _DEBUG_TIMEVENTS
  Serial.print("EspTimeEvents::TaskCore ");
  Serial.println(xPortGetCoreID());
#endif
// Timer handle
#ifdef NOTWORKING_RIGHTNOW
TimerHandle_t deepSleepTimer;
// Create a timer for deep sleep
  deepSleepTimer = xTimerCreate("DeepSleepTimer", pdMS_TO_TICKS(30 * 1000), pdFALSE, (void *)0, deepSleepTimerCallback);
  // Start the timer
  

  // Start the scheduler
  vTaskStartScheduler();
#endif

  SemaphoreHandle_t mWifiMutex = getWifiMutex();
  if (xSemaphoreTake(mWifiMutex, portMAX_DELAY)) {
    ntpClient->update();
       //ntpClient->forceUpdate();
    xSemaphoreGive(mWifiMutex);
  }
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
  int message = 900;
  for (;;) {
#ifdef _DEBUG_TIMEVENTS
  Serial.println("EspTimeEvents::TaskCore : for (;;) ");
#endif
    if (xSemaphoreTake(mWifiMutex, portMAX_DELAY)) {
      ntpClient->update();
      xSemaphoreGive(mWifiMutex);
    } /*
    if (xQueueSend(getMessageQueue(), &message, portMAX_DELAY) == pdTRUE) {
      Serial.print("Sent data to queue: ");
      Serial.println(message);
      message++;
    }*/
    //Serial.println(ntpClient->getHours());
    //Serial.println(ntpClient->getMinutes());
#ifdef NOTWORKING_RIGHTNOW
// Configure ESP32 to wake up on a timer
  esp_sleep_enable_timer_wakeup(40 * 1e6); // Convert seconds to microseconds
  xTimerStart(deepSleepTimer, 0);
Serial.println("EspTimeEvents::TaskCore : going to sleep for 40 sec ");
  // Enter deep sleep (ESP32 will wake up after the specified sleep duration)
  //esp_deep_sleep_start();
#endif 
    // Wait for the next cycle.
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    //vTaskDelay(45000);
  }
} 
#endif