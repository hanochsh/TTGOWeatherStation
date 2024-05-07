#include <SPI.h>
#include <TFT_eSPI.h>  // Hardware-specific library
#include "EspTimeBasedEvents.h"
#define configUSE_NEWLIB_REENTRANT 0
//#include <Arduino_FreeRTOS.h>
#include <freertos/FreeRTOS.h>
#include <ArduinoJson.h>  //https://github.com/bblanchon/ArduinoJson.git
#include <NTPClient.h>    //https://github.com/taranais/NTPClient
#include "DebugPrefTools.h"
#include "EspButton.h"
#include "xEspTaskRenderer.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "ViewPortsStack.h"
#include "EspColors.h"
#include "EspUtils.h"
#include "EspIcons.h"
//#define dbgVerbose 1
#include "EspDebug.h"
//#include "Captive_Portal_WiFi_Manager.h"
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library


const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;


const char *ssid = "ShiriHome";               ///EDIIIT
const char *password = "shirihome035274757";  //EDI8IT

//const String stockUrl = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=PTC&apikey=CQ4BMVCC56VEWRRJ";



extern const GFXfont *getOrbitonFontMed20();
extern "C" const GFXfont *getOrbitronMedium8Font();
extern "C" const GFXfont *getOrbitronLight6Font();

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


ViewPortsStack viewPorts(&tft);



byte curBckLight = 2;

#define TempXPos 0    // 60
#define TempYPos 180  //148
#define TempXWid 65   // 67

#define StocksXPos 73  // 75
#define StocksYPos 144

#define lftBTN 0
#define rgtBTN 35

LeftBtn leftBtn(lftBTN);
RightBtn rightBtn(rgtBTN);

// Create a message queue handle
#define QUEUE_SIZE 10
QueueHandle_t messageQueue;
TaskHandle_t hEspTimeEventsTaskCore;
TaskHandle_t hAnimationTaskCore;
TaskHandle_t hRegularTimeDisplayTaskCore;
TaskHandle_t hBottonsCheck;
TaskHandle_t hWeatherUpdateTask;
TaskHandle_t hStockUpdateTask;
TaskHandle_t hMsgReciverTask;
SemaphoreHandle_t mWifiMutex;
SemaphoreHandle_t mTFTMutex;

MainTimeRendererTask timeRenderer;
StockRendererTask stockRenderer;
AnimationRendererTask animRenderer;
WeatherRendererTask weatherRenderer;
CheckButtonsTask checkBtnsTask;
EspTimeEvents espTimeEvents;
MsgReciverTask espMsgsReciver;

EspButton *getRightBtn() {
  return &rightBtn;
}

EspButton *getLeftBtn() {
  return &leftBtn;
}

SemaphoreHandle_t getWifiMutex() {
  return mWifiMutex;
}

SemaphoreHandle_t getTFTMutex() {
  return mTFTMutex;
}


QueueHandle_t getMessageQueue() {
  return messageQueue;
}


///////////////////////////////////////////////////////////
//
int getBckLgt(int i) {
#define NumBckLgt 6

  int backlight[NumBckLgt] = { 0, 1, 30, 60, 120, 220 };

  if (i > (NumBckLgt - 1))
    return getCurBckLgt();

  return backlight[i];
}

///////////////////////////////////////////////////////////
//
int getCurBckLgt() {
  return getBckLgt(curBckLight);
}

///////////////////////////////////////////////////////////
//
int isDispVisible() {
  return getCurBckLgt() > 0;
}

///////////////////////////////////////////////////////////
//
void drawCurBckLgt() {
  for (int i = 0; i < curBckLight + 1; i++)
    tft.fillRect(78 + (i * 7), 216, 3, 10, blue);
}

///////////////////////////////////////////////////////////
//
void clearScreen() {
#ifdef _DEBUG_INO
  Serial.println("clearScreen()");
#endif
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
}

///////////////////////////////////////////////////////////
//
void timeSetup() {
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(7200); /*EDDITTTTTTTTTTTTTTTTTTTTTTTT */
  timeClient.update();
}
///////////////////////////////////////////////////////////
//
void wifiSetup() {
#ifdef _DEBUG_INO
  Serial.println("wifiSetup()");
#endif
  mWifiMutex = xSemaphoreCreateMutex();
  tft.print("Connecting to ");
  tft.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    tft.print(".");
  }

  tft.println("");
  tft.println("WiFi connected.");
  tft.println("IP address: ");
  tft.println(WiFi.localIP());
}


///////////////////////////////////////////////////////////
//
void onRgtBtnDPress() {
  curBckLight++;
  if (curBckLight >= 5)
    curBckLight = 0;

  //drawCurBckLgt();
  ledcWrite(pwmLedChannelTFT, getCurBckLgt());
}






///////////////////////////////////////////////////////////
//
void setupBtns() {
  leftBtn.setup();
  rightBtn.setup();
}







///////////////////////////////////////////////////////////
//
void AnimationRendererTask::renderTask(int opt, void *data) {
  //animateImageFrame();
  if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
    tft.pushImage(0, 82, 135, 63, getAnimFrame(mFrame));
    // Release the mutex when done
    xSemaphoreGive(mTFTMutex);
  }

  mFrame++;
  if (mFrame >= 10)
    mFrame = 0;
}

///////////////////////////////////////////////////////////
//
void MainTimeRendererTask::renderTimeDisplay(int opt, int vpX, int vpY) {
  static String lastDate = "";
  String curTime = "";
  String dateStamp;
  String formattedDate;
  String timeStamp;
#ifdef _DEBUG_INO
  Serial.println("renderTimeDisplay");
#endif

  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dateStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  curTime = timeStamp.substring(0, 5);  // Time

  if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
    if (opt)
      viewPorts.pushViewPort(vpX, vpY, TFT_WIDTH, 55, VP_RestorePrev);  // define new View Port
    if (lastDate != dateStamp) {
      tft.setTextColor(TFT_ORANGE, TFT_BLACK);
      //tft.setFreeFont(getOrbitronLight6Font());
      tft.drawString(dateStamp, 6, 42, 2);  // Draw date
      lastDate = dateStamp;
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    if (curTime != mdispTime) {  // Need to update the Time display

      mdispTime = curTime;
      tft.fillRect(0, 0, TFT_WIDTH, 32, TFT_BLACK);  // was 1,1
      tft.setFreeFont(&Orbitron_Light_32);
// was 19
#define SEC_STR_WIDTH 20
#define MAX_HOUR_WIDTH 111

      mcurY = tft.textWidth(mdispTime);  // No Space for seconds display
      mshowSec = mcurY > MAX_HOUR_WIDTH ? FALSE : SEC_STR_WIDTH;

      tft.setCursor((TFT_WIDTH - tft.textWidth(mdispTime) - mshowSec) / 2, 32);
      tft.print(mdispTime);

      mcurX = tft.getCursorX();
      mcurY = tft.getCursorY();
    }

    if (mshowSec)  // There is enough width to show the seconds digits
    {
      curTime = timeStamp.substring(6, 8);  // Seconds
      if (curTime != mdispSeconds) {
        mdispSeconds = curTime;
        tft.setFreeFont(getOrbitronLight6Font());
        tft.fillRect(mcurX, mcurY - 13, SEC_STR_WIDTH, 15, TFT_BLACK);  // was 1,1
        tft.drawString(":" + mdispSeconds, mcurX -2, mcurY - 13);  // -14, 2);  // Draw sec
      }
    }

    if (opt)
      viewPorts.popViewPort();
    xSemaphoreGive(mTFTMutex);
  }
}

///////////////////////////////////////////////////////////
//
void disableMainSCRInfo() {
  animRenderer.disable();
  stockRenderer.disable();
  weatherRenderer.disable();
}

///////////////////////////////////////////////////////////
//
void enableMainSCRInfo() {
  animRenderer.enable();
  stockRenderer.enable();
  weatherRenderer.enable();
}

///////////////////////////////////////////////////////////
//
void MsgReciverTask::renderTask(int opt, void *data) {
  int recEvent = -1;

  if (xQueueReceive(getMessageQueue(), &recEvent, 0) == pdTRUE) {
    //Serial.print("MsgReciverTask::renderTask received data from queue: ");
    // Serial.println(recEvent);

    switch (recEvent) {
      case StopAnimationEvnt:
        LOG_INF(" received StopAnimationEvnt");
        animRenderer.disable();
        //vTaskSuspend(hAnimationTaskCore);
        break;
      case ResumeAnimationEvnt:
        LOG_INF(" received ResumeAnimationEvnt");
        animRenderer.enable();
        //vTaskResume(hAnimationTaskCore);
        break;
      case StartNightEvnt:
        LOG_INF(" received StartNightEvnt");
        ledcWrite(pwmLedChannelTFT, 1);
        disableMainSCRInfo();
        if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
          viewPorts.pushViewPortExt(0, 0, TFT_WIDTH, TFT_HEIGHT, VP_SaveUnder, TFT_BLACK);
          // Release the mutex when done
          xSemaphoreGive(mTFTMutex);
        }
        timeRenderer.disableMainScreen();
        deleteApplicationTasks();
        break;
      case EndNightEvnt:
        LOG_INF(" received EndNightEvnt");
        ledcWrite(pwmLedChannelTFT, getCurBckLgt());

        if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
          viewPorts.popViewPort();
          // Release the mutex when done
          xSemaphoreGive(mTFTMutex);
        }
        timeRenderer.enableMainScreen();
        //enableMainSCRInfo();
        createApplicationTasks();
        enableMainSCRInfo();
        break;
      case MarketOpenEvnt:
        LOG_INF(" received MarketOpenEvnt");
        stockRenderer.verifyCurrentMarketStatus();
        break;
      case MarketCloseEvnt:
        Serial.println("received MarketCloseEvnt");
          stockRenderer.verifyCurrentMarketStatus();
        break;
      case GetTickerPeersEvnt:
        LOG_INF("received GetTickerPeersEvnt");
        stockRenderer.getPeers();
        break;
      case GoToDeepSleepEvnt:
  #define SleepTime 60*200 // seconds
         esp_sleep_enable_timer_wakeup(SleepTime * 1e6);
         LOG_INF("Setup ESP32 to sleep for every %f Seconds",SleepTime);
         esp_deep_sleep_start();
         break;
    }
    LOG_INF("done handleing last event");
  }
}



///////////////////////////////////////////////////////////
//
void CheckButtonsTask::renderTask(int opt, void *data) {
  chkBtns();
}

///////////////////////////////////////////////////////////
//
void createApplicationTasks()
{
  stockRenderer.createCoreTask(&hStockUpdateTask);
  //Serial.println(" Setup() stockRenderer");
  weatherRenderer.createCoreTask(&hWeatherUpdateTask);
  //Serial.println(" Setup() weatherRenderer");
  animRenderer.createCoreTask(&hAnimationTaskCore);
  //Serial.println(" Setup() animRenderer");
}

///////////////////////////////////////////////////////////
//
void deleteApplicationTasks()
{
  vTaskDelete(hStockUpdateTask);
  vTaskDelete(hWeatherUpdateTask);
  vTaskDelete(hAnimationTaskCore);
}

///////////////////////////////////////////////////////////
//
void setup(void) {
  setupBtns();

  tft.init();
  tft.setRotation(0);

  clearScreen();
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin( 18 /*TFT_BL*/, pwmLedChannelTFT); // commenting out since the library update from 2.5.34 missing symbole TFT_BL 
  ledcWrite(pwmLedChannelTFT, getCurBckLgt());
  Serial.begin(115200);
  wifiSetup();
  delay(3000);
  clearScreen();

  tft.setSwapBytes(true);

  tft.drawLine(TempXPos + TempXWid + 1, 150, TempXPos + TempXWid + 1, 240, TFT_GREY);

  timeSetup();
  mTFTMutex = xSemaphoreCreateMutex();
  // Create the message queue
  messageQueue = xQueueCreate(QUEUE_SIZE, sizeof(int));

  //delay(500);
  // Update the base class statics as apperntly the intial update is not enough
  espTimeEvents.setNTPClientPtr(&timeClient);
  espTimeEvents.setTFTPtr(&tft);
  espTimeEvents.createCoreTask(&hEspTimeEventsTaskCore);
  
  //Serial.println(" Setup() espTimeEvents");
  timeRenderer.createCoreTask(&hRegularTimeDisplayTaskCore);
  //Serial.println(" Setup() timeRenderer");
  createApplicationTasks();
  //stockRenderer.createCoreTask(&hStockUpdateTask);
  //Serial.println(" Setup() stockRenderer");
  //weatherRenderer.createCoreTask(&hWeatherUpdateTask);
  //Serial.println(" Setup() weatherRenderer");
  checkBtnsTask.createCoreTask(&hBottonsCheck);
  //Serial.println(" Setup() checkBtnsTask");
  //animRenderer.createCoreTask(&hAnimationTaskCore);
  //Serial.println(" Setup() animRenderer");
  espMsgsReciver.createCoreTask(&hMsgReciverTask);
  //Serial.println(" Setup() espMsgsReciver");

  // Start the FreeRTOS scheduler
  //vTaskStartScheduler(); // must not be called in ESP32 ?

#ifdef _DEBUG_INO
  Serial.print(" End of Setup(), Main is running on core ");
  Serial.println(xPortGetCoreID());
#endif
}





///////////////////////////////////////////////////////////
//
void animateImageFrame() {
  static int frame = 0;

  if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
    tft.pushImage(0, 82, 135, 63, getAnimFrame(frame));
    // Release the mutex when done
    xSemaphoreGive(mTFTMutex);
  }

  frame++;
  if (frame >= 10)
    frame = 0;
}



///////////////////////////////////////////////////////////
//
void LeftBtn::onDoubleClick() {
#ifdef _DEBUG_INO
  Serial.println("LeftBtn::onDoubleClick");
#endif
  mInv = !mInv;
  tft.invertDisplay(mInv);
}


///////////////////////////////////////////////////////////
//
void RightBtn::onDoubleClick() {
#ifdef _DEBUG_INO
  Serial.println("RightBtn::onDoubleClick");
#endif
  onRgtBtnDPress();
}

///////////////////////////////////////////////////////////
//
void RightBtn::onStartLongPress() {
#ifdef _DEBUG_INO
  Serial.println("RightBtn::onStartLongPress");
#endif
#define Y_ViewPort 60
  disableMainSCRInfo();
  if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
    viewPorts.pushViewPort(0, Y_ViewPort, TFT_WIDTH, TFT_HEIGHT - Y_ViewPort, VP_SaveUnder | VP_WithFram);
    xSemaphoreGive(mTFTMutex);
    stockRenderer.renderPeersInViewPort();
    // Release the mutex when done
  }
#undef Y_ViewPort
}


///////////////////////////////////////////////////////////
//
void RightBtn::onEndLongPress() {
#ifdef _DEBUG_INO
  Serial.println("Button 2 longPress stop");
#endif
  if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
    viewPorts.popViewPort();

    // Release the mutex when done
    xSemaphoreGive(mTFTMutex);
  }

  enableMainSCRInfo();
}  // longPressStop2

///////////////////////////////////////////////////////////
//
void LeftBtn::onStartLongPress() {
#ifdef _DEBUG_INO
  Serial.println("LeftBtn::onStartLongPress");
#endif

#define Y_ViewPort 60
  disableMainSCRInfo();
 // Serial.println("LeftBtn::onStartLongPress after disableMainSCRInfo");
  if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
   // Serial.println("LeftBtn::onStartLongPress inside  xSemaphoreTake");
    viewPorts.pushViewPort(0, Y_ViewPort, TFT_WIDTH, TFT_HEIGHT - Y_ViewPort, VP_SaveUnder | VP_WithFram);
    //Serial.println("LeftBtn::onStartLongPress after pushViewPort");
    xSemaphoreGive(mTFTMutex);
    weatherRenderer.renderForcast();
  }
}


///////////////////////////////////////////////////////////
//
void LeftBtn::onEndLongPress() {
#ifdef _DEBUG_INO
  Serial.println("LeftBtn longPress stop");
#endif
  if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
    viewPorts.popViewPort();

    // Release the mutex when done
    xSemaphoreGive(mTFTMutex);
  }

  enableMainSCRInfo();
}  // longPressStop2




///////////////////////////////////////////////////////////
//
void chkBtns() {
#ifdef _DEBUG_INO && 0
  //Serial.println("chkBtns");
#endif
  leftBtn.loop();
  rightBtn.loop();
}

///////////////////////////////////////////////////////////
//
void loop() {
}

///////////////////////////////////////////////////////////
//
int getPayloadFromUrl(String url, String *payload)

{
  int status = FALSE;

//#ifdef _DEBUG_INO
  LOG_DBG("%s",url.c_str());

//#endif


  if (xSemaphoreTake(mWifiMutex, portMAX_DELAY)) {
    //Serial.println("getPayloadFromUrl : passed the Semaphore check");
    if ((WiFi.status() == WL_CONNECTED)) {  //Check the current connection status
      *payload = "";                        //whole String

      HTTPClient http;
      //Serial.println("getPayloadFromUrl : before http.begin");
      http.begin(url);  //Specify the URL
      //Serial.println("getPayloadFromUrl : after http.begin");
      if (http.GET() > 0) {  //Make the request and Check for the returning code
                             // SUCCESS

        //Serial.println("getPayloadFromUrl : GET > 0");
        *payload = http.getString();
        LOG_DBG("Got the payload %s", payload->c_str());
        //Serial.println(String("Payload :") + *payload);

        status = TRUE;
      }  // if get

      http.end();  //Free the resources

    }  // if Wifi status
    // Release the mutex when done

    xSemaphoreGive(mWifiMutex);
  }  //if wifiMutex

  return status;
}

///////////////////////////////////////////////////////////
//
int getJsonFromUrl(String url, JsonDocument *josn) {
  String payload;
  LOG_DBG("");
  if (getPayloadFromUrl(url, &payload)) {
    LOG_DBG("got the payload: %s", payload.c_str());
    deserializeJson(*josn, payload.c_str());
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////
//
int getJsonWithFilterFromUrl(String url, JsonDocument *josn, JsonDocument filter) {
  String payload;
  if (getPayloadFromUrl(url, &payload)) {
    deserializeJson(*josn, payload.c_str(), DeserializationOption::Filter(filter));
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////
//
String buildQuaryUrl(String *outBuff, ...) {
  va_list args;
  va_start(args, outBuff);
  char *value = NULL;
  *outBuff = "";
  while (NULL != (value = va_arg(args, char *)))
    *outBuff += value;

  va_end(args);
  const char *find = " ";
  const char *replace = "%20";
  outBuff->replace(find, replace); // some Sites can not accept space e.g in "Tel Aviv"
  return *outBuff;
}

///////////////////////////////////////////////////////////
//
void removeCharFromString(String &str, char charToRemove) {
  // Iterate through the string and remove the specified character
  for (int i = 0; i < str.length(); i++) {
    if (str[i] == charToRemove) {
      str.remove(i, 1);
      i--;  // Adjust index after removal
    }
  }
}

