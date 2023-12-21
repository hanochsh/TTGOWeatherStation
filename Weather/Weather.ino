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

#include "Orbitron_Medium_20.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "ViewPortsStack.h"
//#include "Captive_Portal_WiFi_Manager.h"
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#define blue 0x5D9B
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;


const char *ssid = "ShiriHome";               ///EDIIIT
const char *password = "shirihome035274757";  //EDI8IT
String town = "Tel Aviv";                     //EDDIT
String Country = "IL";                        //EDDIT
const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + town + "," + Country + "&units=metric&APPID=";
//const String stockUrl = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=PTC&apikey=CQ4BMVCC56VEWRRJ";
const String ptcStockUrl = "https://finnhub.io/api/v1/quote?symbol=PTC&token=clj68h9r01qok8f2vjf0clj68h9r01qok8f2vjfg";
// http://api.openweathermap.org/data/2.5/weather?q=Tel Aviv,IL&units=metric&APPID=91697f781be1daa54d31d7b4bcb75e5b
const String key = "91697f781be1daa54d31d7b4bcb75e5b"; /*EDDITTTTTTTTTTTTTTTTTTTTTTTT */

// Weather Icons Externs
extern const uint16_t *getWeather01d();
extern const uint16_t *getWeather01n();
extern const uint16_t *getWeather02d();
extern const uint16_t *getWeather02n();
extern const uint16_t *getWeather03d();
extern const uint16_t *getWeather03n();
extern const uint16_t *getWeather04d();
extern const uint16_t *getWeather04n();
extern const uint16_t *getWeather09d();
extern const uint16_t *getWeather09n();
extern const uint16_t *getWeather10d();
extern const uint16_t *getWeather10n();
extern const uint16_t *getWeather11d();
extern const uint16_t *getWeather11n();
extern const uint16_t *getWeather13d();
extern const uint16_t *getWeather13n();
extern const uint16_t *getWeather50d();
extern const uint16_t *getWeather50n();
extern unsigned short *getAnimFrame(int frame);

extern "C" const unsigned short *getPTCIcon32x32();
extern "C" const unsigned short *getPTCIcon48x48();


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
EspTimeEvents espTimeEvents();
ViewPortsStack viewPorts(&tft);



byte curBckLight = 1;

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
SemaphoreHandle_t mWifiMutex;
SemaphoreHandle_t mTFTMutex;

MainTimeRendererTask  timeRenderer;
StockRendererTask     StockRenderer;
AnimationRendererTask animRenderer;
WeatherRendererTask   weatherRenderer;
CheckButtonsTask      checkBtnsTask;

SemaphoreHandle_t getWifiMutex() {
  return mWifiMutex;
}

QueueHandle_t getMessageQueue()
{
  return messageQueue;
}

///////////////////////////////////////////////////////////
//
void drwWeatherIcon(String icon) {
  const uint16_t *iconArr = NULL;

  if (icon == "01n") {
    iconArr = getWeather01n();
  } else if (icon == "02n") {
    iconArr = getWeather02n();
  } else if (icon == "03n") {
    iconArr = getWeather03d();
  } else if (icon == "04n") {
    iconArr = getWeather04d();
  } else if (icon == "09n") {
    iconArr = getWeather09d();
  } else if (icon == "10n") {
    iconArr = getWeather10n();
  } else if (icon == "11n") {
    iconArr = getWeather11n();
  } else if (icon == "13n") {
    iconArr = getWeather13d();
  } else if (icon == "50n") {
    iconArr = getWeather50n();
  } else if (icon == "01d") {
    iconArr = getWeather01d();
  } else if (icon == "02d") {
    iconArr = getWeather02d();
  } else if (icon == "03d") {
    iconArr = getWeather03d();
  } else if (icon == "04d") {
    iconArr = getWeather04d();
  } else if (icon == "09d") {
    iconArr = getWeather09d();
  } else if (icon == "10d") {
    iconArr = getWeather10d();
  } else if (icon == "11d") {
    iconArr = getWeather11d();
  } else if (icon == "13d") {
    iconArr = getWeather13d();
  } else if (icon == "50d") {
    iconArr = getWeather50d();
  }

  if (iconArr != NULL)
    tft.pushImage(TempXPos + 14, TempYPos - 38, 32, 32, iconArr);
}

///////////////////////////////////////////////////////////
//
int getBckLgt(int i) {
#define NumBckLgt 5

  int backlight[NumBckLgt] = { 0, 30, 60, 120, 220 };

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
void drawOpenMarketHeaders() {
  tft.setTextFont(2);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(StocksXPos, StocksYPos + 60);
  tft.print("Day");
  tft.setTextFont(1);
  tft.setCursor(StocksXPos, StocksYPos + 79);
  tft.print("H: ");
  tft.setCursor(StocksXPos, StocksYPos + 89);
  tft.print("L: ");
}

///////////////////////////////////////////////////////////
//
void setupBtns() {
  leftBtn.setup();
  rightBtn.setup();
}





///////////////////////////////////////////////////////////
//
void TaskRenderRegularTime(void *pvParameters) {
  TickType_t xLastWakeTime;
  const TickType_t xFrequency = 900;

  //vTaskDelay(1000);
  // Initialise the xLastWakeTime variable with the current time.
  xLastWakeTime = xTaskGetTickCount();
  int receivedData = -1;
  for (;;) {

    //Serial.println("Running Animation");
    if (xQueueReceive(getMessageQueue(), &receivedData, 10) == pdTRUE) {
      Serial.print("Receiver Task TaskRenderRegularTime received data from queue: ");
      Serial.println(receivedData);
    }

    renderTimeDisplay();
    // Wait for the next cycle.
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}



///////////////////////////////////////////////////////////
//
void MainTimeRendererTask::renderTask(int opt, void *data )
{
  renderTimeDisplay();
}

///////////////////////////////////////////////////////////
//
void StockRendererTask::renderTask(int opt, void *data )
{
  renderStocksData();
}

///////////////////////////////////////////////////////////
//
void AnimationRendererTask::renderTask(int opt, void *data )
{
  animateImageFrame();
}

///////////////////////////////////////////////////////////
//
void WeatherRendererTask::renderTask(int opt, void *data )
{
  renderWeatherData();
}

///////////////////////////////////////////////////////////
//
void CheckButtonsTask::renderTask(int opt, void *data )
{
  chkBtns();
}

///////////////////////////////////////////////////////////
//
void setup(void) {
  setupBtns();

  tft.init();
  tft.setRotation(0);

  clearScreen();
  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, getCurBckLgt());
  Serial.begin(115200);
  wifiSetup();
  delay(3000);
  clearScreen();

  tft.setSwapBytes(true);

  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(TempXPos, TempYPos + 24, 2);
  tft.println("Humidity"); /*
  tft.setCursor(StocksXPos , StocksYPos + 60);
  tft.print("Day");
  tft.setTextFont(1);
  tft.setCursor(StocksXPos , StocksYPos + 79);
  tft.print("H: $");
  tft.setCursor(StocksXPos , StocksYPos + 89);
  tft.print("L: $"); */
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&Orbitron_Medium_20);
  tft.setCursor(6, 80);
  tft.println(town);

  //drawCurBckLgt();
  tft.drawLine(TempXPos + TempXWid + 1, 150, TempXPos + TempXWid + 1, 240, TFT_GREY);

  tft.pushImage(StocksXPos + 13, StocksYPos + 1, 32, 32, getPTCIcon32x32());
  timeSetup();
  mTFTMutex = xSemaphoreCreateMutex();
  // Create the message queue
  messageQueue = xQueueCreate(QUEUE_SIZE, sizeof(int)); 
 
  delay(500);
  xTaskCreatePinnedToCore(
    EspTimeEvents::TaskCore,   /* Task function. */
    "EspTimeEvents::TaskCore", /* name of task. */
    2000,                      /* Stack size of task */
    &timeClient,               /* parameter of the task */
    1,                         /* priority of the task */
    &hEspTimeEventsTaskCore,   /* Task handle to keep track of created task */
    0);                        /* pin task to core 0 */

   timeRenderer.createCoreTask(&hRegularTimeDisplayTaskCore);
   StockRenderer.createCoreTask(&hStockUpdateTask);
   weatherRenderer.createCoreTask(&hWeatherUpdateTask);
   checkBtnsTask.createCoreTask(&hBottonsCheck);
   animRenderer.createCoreTask(&hAnimationTaskCore);

    // Start the FreeRTOS scheduler
  vTaskStartScheduler();

#ifdef _DEBUG_INO
  Serial.print(" End of Setup(), Main is running on core ");
  Serial.println(xPortGetCoreID());
#endif
}



//int i = 0;

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

String dispSeconds = "";
String dispTime = "";

///////////////////////////////////////////////////////////
//
void renderTimeDisplay() {
  static int curX, curY;
  static int showSec = FALSE;
  String curTime = "";
  String dateStamp;
  String formattedDate;
  String timeStamp;
#ifdef _DEBUG_INO
  Serial.println("renderTimeDisplay");
#endif
  //Serial.println("before Client update");

  // Serial.println("after Client update");
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  //Serial.println(formattedDate);
  viewPorts.pushViewPort(0, 0, TFT_WIDTH, 55, VP_RestorePrev);

  int splitT = formattedDate.indexOf("T");
  dateStamp = formattedDate.substring(0, splitT);
#ifdef _DEBUG_INO
  Serial.println("renderTimeDisplay() : dateStamp = formattedDate.substring(0, splitT)");
  Serial.println("renderTimeDisplay() : dateStamp = " + dateStamp);
#endif
  if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setCursor(6, 42);
#ifdef _DEBUG_INO
    //dateStamp = "2023-12-20";
    Serial.println("renderTimeDisplay() :before: tft.println(dateStamp) dateStamp = " + dateStamp);
#endif
    // tft.println(dateStamp); // Date crash is here
    tft.drawString(dateStamp, 6, 42, 2);
#ifdef _DEBUG_INO
    Serial.println("renderTimeDisplay() :before: tft.setTextColor(TFT_WHITE, TFT_BLACK)");
#endif
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

#ifdef _DEBUG_INO
    Serial.println("renderTimeDisplay() :before: timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1) ");
#endif
    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
#ifdef _DEBUG_INO
    Serial.println("renderTimeDisplay() : timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1)");
    Serial.println("renderTimeDisplay() : timeStamp = " + timeStamp);
#endif
    curTime = timeStamp.substring(0, 5);  // Time
#ifdef _DEBUG_INO
    Serial.println("renderTimeDisplay() : curTime = timeStamp.substring(0, 5)");
    Serial.println("renderTimeDisplay() : curTime = " + curTime);
#endif
    if (curTime != dispTime) {
#ifdef _DEBUG_INO
      Serial.println("renderTimeDisplay() :if (curTime != dispTime)");
#endif
      dispTime = curTime;
      tft.fillRect(1, 1, TFT_WIDTH, 32, TFT_BLACK);
      tft.setFreeFont(&Orbitron_Light_32);

#define SEC_STR_WIDTH 19
#define MAX_HOUR_WIDTH 111
      //Serial.println(tft.textWidth(dispTime));
      curY = tft.textWidth(dispTime);  // No Space for seconds display
      showSec = curY > MAX_HOUR_WIDTH ? FALSE : SEC_STR_WIDTH;

      tft.setCursor((TFT_WIDTH - tft.textWidth(dispTime) - showSec) / 2, 32);
      tft.print(dispTime);

      curX = tft.getCursorX();
      curY = tft.getCursorY();
    }
    //Serial.println("Show sec" + String(showSec));
    if (showSec)  // There is enough width to show the seconds digits
    {
#ifdef _DEBUG_INO
      Serial.println("renderTimeDisplay() : if ( showSec)");
#endif
      //Serial.println("in the if Show sec");
      curTime = timeStamp.substring(6, 8);  // Seconds
      if (curTime != dispSeconds) {
#ifdef _DEBUG_INO
        Serial.println("renderTimeDisplay() : if ( showSec) if (curTime != dispSeconds) ");
#endif
        dispSeconds = curTime;
        tft.setTextFont(2);

        tft.setCursor(curX, curY - 14);
        //Serial.println(tft.fontHeight());
        //Serial.println( tft.textWidth(t));
        tft.print(":" + dispSeconds);
      }
    }
#ifdef _DEBUG_INO
    Serial.println("renderTimeDisplay() : before popViewPOrt() ");
#endif
    viewPorts.popViewPort();
    xSemaphoreGive(mTFTMutex);
  }
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

#define Y_ViewPort 87
  vTaskSuspend(hAnimationTaskCore);
  viewPorts.pushViewPort(0, Y_ViewPort, TFT_WIDTH, TFT_HEIGHT - Y_ViewPort, VP_SaveUnder);

}


///////////////////////////////////////////////////////////
//
void RightBtn::onEndLongPress() {
#ifdef _DEBUG_INO
  Serial.println("Button 2 longPress stop");
#endif
  viewPorts.popViewPort();
  vTaskResume(hAnimationTaskCore);
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
int getJsonFromUrl(String url, JsonDocument *josn)

{
  int status = FALSE;
#ifdef _DEBUG_INO
  Serial.println("getJsonFromUrl");
#endif
  if (xSemaphoreTake(mWifiMutex, portMAX_DELAY)) {
    if ((WiFi.status() == WL_CONNECTED)) {  //Check the current connection status
      String payload = "";                  //whole json

      HTTPClient http;

      http.begin(url);  //Specify the URL

      if (http.GET() > 0) {  //Make the request and Check for the returning code
                             // SUCCESS
        payload = http.getString();
        //Serial.println(payload);

        deserializeJson(*josn, payload.c_str());

        status = TRUE;
      }

      http.end();  //Free the resources
    }
    // Release the mutex when done
    xSemaphoreGive(mWifiMutex);
  }
  return status;
}

///////////////////////////////////////////////////////////
//
int isMarketOpen() {

  int today = timeClient.getDay();
  //Serial.println("isMarketOpen today is " + String(today));
  if (today == 0 || today == 6)
    return FALSE;

  int hour = timeClient.getHours();
  //Serial.println("isMarketOpen hour is " + String(hour));
  if (hour < 16 || ((hour >= 23) && (timeClient.getMinutes() > 30)))
    return FALSE;

  return TRUE;
}

///////////////////////////////////////////////////////////
//
void renderStocksData() {
  static int notTheFirstTime = FALSE;
  static int wasMarketOpen = -1;
  int isMarketOpenNow;
#ifdef _DEBUG_INO
  Serial.println("renderStocksData()");
#endif
  StaticJsonDocument<250> doc;

  isMarketOpenNow = isMarketOpen();
#ifdef _DEBUG_INO
Serial.println("renderStocksData() isMarketOpen:" + String(isMarketOpenNow));
#endif
  if (isMarketOpenNow != wasMarketOpen) {
    if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
      if (isMarketOpenNow)
        drawOpenMarketHeaders();
      else
        tft.fillRect(StocksXPos, StocksYPos + 50, TFT_WIDTH - StocksXPos, TFT_HEIGHT - (StocksYPos + 50), TFT_BLACK);
      wasMarketOpen = isMarketOpenNow;
      xSemaphoreGive(mTFTMutex);
    }
  }
  if (!isMarketOpenNow && notTheFirstTime)
    return;

  // Serial.println("Inside renderStocksData,  isMarketOpen()" + String(isMarketOpen()) + " notThefirst time" + String(notTheFirstTime));

  if (getJsonFromUrl(ptcStockUrl, &doc)) {
    notTheFirstTime = TRUE;
    String ptcCurrent = doc["c"];
    String ptcDayHigh = doc["h"];
    String ptcDayLow = doc["l"];
    String ptcDayChange = doc["d"];


    if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
      tft.setCursor(StocksXPos, StocksYPos + 40);
      tft.setTextFont(2);
      if (ptcDayChange.substring(0, 1) == "-")
        tft.setTextColor(TFT_RED, TFT_BLACK);
      else
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.print("$" + ptcCurrent.substring(0, 6));
      //tft.setCursor(StocksXPos , StocksYPos + 58);
      //tft.print("Day :");
      if (isMarketOpenNow) {
        tft.setTextFont(1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setCursor(StocksXPos + 12, StocksYPos + 79);
        tft.print("$" + ptcDayHigh.substring(0, 6));
        tft.setCursor(StocksXPos + 12, StocksYPos + 89);
        tft.print("$" + ptcDayLow.substring(0, 6));
      }
      //Serial.println(StocksYPos + 89);
      xSemaphoreGive(mTFTMutex);
    }
  }
}

///////////////////////////////////////////////////////////
//
void renderWeatherData() {
  StaticJsonDocument<1000> doc;
#ifdef _DEBUG_INO
  Serial.println("renderWeatherData()");
#endif
  if (getJsonFromUrl(endpoint + key, &doc)) {

    String temp = doc["main"]["temp"];
    String humi = doc["main"]["humidity"];
    String town2 = doc["name"];
    String weatherIcon = doc["weather"][0]["icon"];


    // Debug data
    //Serial.println("Temperature" + temp);
    //printf("%s/n",doc["main"]["temp"]);
    //Serial.println("Humidity" + humi);
    //Serial.println(town);
    if (xSemaphoreTake(mTFTMutex, portMAX_DELAY)) {
      drwWeatherIcon(weatherIcon);

      // Draw Tempreture
      tft.setFreeFont(&Orbitron_Medium_20);
      tft.fillRect(TempXPos, TempYPos, TempXWid, 20, TFT_BLACK);
      tft.setCursor(TempXPos, TempYPos + 18);
      //temp = "88.8";
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.print(temp.substring(0, 4));
      int x, y;
      x = tft.getCursorX();
      y = tft.getCursorY();
      tft.drawCircle(x + 3, y - 12, 2, TFT_WHITE);  // Degrees circle
      // Draw Humidity value
      tft.fillRect(TempXPos, TempYPos + 46, TempXWid, 20, TFT_BLACK);
      tft.setCursor(TempXPos, TempYPos + 60);
      tft.println(humi + "%");
      xSemaphoreGive(mTFTMutex);
    }  // close Mutex

  }  // End of SUCCESSful call
}
