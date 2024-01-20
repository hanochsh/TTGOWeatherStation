#pragma once
#include <ArduinoJson.h> 

#ifndef TRUE
#define TRUE true
#define FALSE false
#endif

class EspButton;

extern String buildQuaryUrl(String* outBuff, ...);
extern int    getJsonFromUrl(String url, JsonDocument* josn);
extern int    getJsonWithFilterFromUrl(String url, JsonDocument* josn, JsonDocument filter);
extern int    getPayloadFromUrl(String url, String* payload);
extern void   removeCharFromString(String& str, char charToRemove);
extern void   disableMainSCRInfo();
extern EspButton* getRightBtn();
extern EspButton* getLeftBtn();
extern SemaphoreHandle_t getWifiMutex();
extern SemaphoreHandle_t getTFTMutex();
extern QueueHandle_t getMessageQueue();