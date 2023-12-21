#include "EspTimeBasedEvents.h"
#include <string.h>
#include <HardwareSerial.h>
#include "DebugPrefTools.h"

extern SemaphoreHandle_t getWifiMutex();
extern QueueHandle_t     getMessageQueue();
///////////////////////////////////////////////////////////////
/// </summary>
void EspTimeEvents::addEvent(int event, int hour, int minuts) {
  resizeArray(mNumEvents + 1);
  mEvents[mNumEvents].mEvent = event;
  mEvents[mNumEvents].mHour = hour;
  mEvents[mNumEvents].mMinut = minuts;
  mNumEvents++;
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
  mNumEvents = newSize;
}
void deepSleepTimerCallback(TimerHandle_t xTimer) {
  Serial.println("Timer expired. Going to deep sleep...");
  esp_deep_sleep_start(); // Trigger deep sleep
}

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