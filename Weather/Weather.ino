#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson.git
#include <NTPClient.h> //https://github.com/taranais/NTPClient
#include "DebugPrefTools.h"
#include "EspButton.h"
#include "Orbitron_Medium_20.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "ViewPortsStack.h"
//#include "Captive_Portal_WiFi_Manager.h"
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

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


const char* ssid = "ShiriHome"; ///EDIIIT
const char* password = "shirihome035274757"; //EDI8IT
String town = "Tel Aviv"; //EDDIT
String Country = "IL"; //EDDIT
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

extern "C" const unsigned short* getPTCIcon32x32();
extern "C" const unsigned short* getPTCIcon48x48();


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
ViewPortsStack viewPorts(&tft);



byte curBckLight = 1;

#define TempXPos 0 // 60
#define TempYPos 180  //148
#define TempXWid 65 // 67

#define StocksXPos 73 // 75
#define StocksYPos 144

#define lftBTN 0
#define rgtBTN 35

LeftBtn leftBtn(lftBTN);
RightBtn rightBtn(rgtBTN);


///////////////////////////////////////////////////////////
//
void drwWeatherIcon(String icon)
{
  const uint16_t *iconArr = NULL;

  if(icon=="01n"){
    iconArr = getWeather01n();  
  }else if(icon=="02n"){
    iconArr = getWeather02n(); 
  }else if(icon=="03n"){
    iconArr = getWeather03d(); 
  }else if(icon=="04n"){
    iconArr = getWeather04d();
  }else if(icon=="09n"){
    iconArr = getWeather09d();
  }else if(icon=="10n"){
    iconArr = getWeather10n(); 
  }else if(icon=="11n"){
    iconArr = getWeather11n(); 
  }else if(icon=="13n"){
    iconArr = getWeather13d();
  }else if(icon=="50n"){
    iconArr = getWeather50n();
  }else if(icon=="01d"){
    iconArr = getWeather01d(); 
  }else if(icon=="02d"){
    iconArr = getWeather02d(); 
  }else if(icon=="03d"){
    iconArr = getWeather03d();
  }else if(icon=="04d"){
    iconArr = getWeather04d();
  }else if(icon=="09d"){
    iconArr = getWeather09d();
  }else if(icon=="10d"){
    iconArr = getWeather10d(); 
  }else if(icon=="11d"){
    iconArr = getWeather11d(); 
  }else if(icon=="13d"){
    iconArr = getWeather13d(); 
  }else if(icon=="50d"){
    iconArr = getWeather50d(); 
    
  }

  if ( iconArr != NULL)
    tft.pushImage(TempXPos + 14, TempYPos - 38,  32, 32, iconArr);
   
}

///////////////////////////////////////////////////////////
//
int getBckLgt(int i)
{
  #define NumBckLgt 5

  int backlight[NumBckLgt] = { 0,30,60,120,220 };

  if (i > (NumBckLgt -1))
    return getCurBckLgt();

  return backlight[i];
}

///////////////////////////////////////////////////////////
//
int getCurBckLgt()
{
  return getBckLgt(curBckLight);
}

///////////////////////////////////////////////////////////
//
int isDispVisible()
{
  return getCurBckLgt() > 0 ;
}

///////////////////////////////////////////////////////////
//
void drawCurBckLgt()
{
  for (int i = 0; i < curBckLight + 1; i++)
		tft.fillRect(78 + (i * 7), 216, 3, 10, blue);
}

///////////////////////////////////////////////////////////
//
void clearScreen()
{
  tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.setTextSize(1);
}

///////////////////////////////////////////////////////////
//
void timeSetup()
{
  // Initialize a NTPClient to get time
	timeClient.begin();
	// Set offset time in seconds to adjust for your timezone, for example:
	// GMT +1 = 3600
	// GMT +8 = 28800
	// GMT -1 = -3600
	// GMT 0 = 0
	timeClient.setTimeOffset(7200); /*EDDITTTTTTTTTTTTTTTTTTTTTTTT */
}
///////////////////////////////////////////////////////////
//
void wifiSetup()
{
	
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
void drawOpenMarketHeaders()
{
  tft.setTextFont(2);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setCursor(StocksXPos , StocksYPos + 60);
  tft.print("Day");
  tft.setTextFont(1);
  tft.setCursor(StocksXPos , StocksYPos + 79);
  tft.print("H: ");
  tft.setCursor(StocksXPos , StocksYPos + 89);
  tft.print("L: ");
}

///////////////////////////////////////////////////////////
//
void setupBtns()
{
  leftBtn.setup();
  rightBtn.setup();
}

hw_timer_t *animTimer = NULL;

///////////////////////////////////////////////////////////
//
void IRAM_ATTR onAnimTimer() {
 if (isDispVisible() && viewPorts.isMainWindow())
   animateImageFrame();
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
  Serial.begin(921600);
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

  tft.pushImage(StocksXPos +13, StocksYPos +1 ,  32, 32, getPTCIcon32x32());
  timeSetup();

  animTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(animTimer, &onAnimTimer, true);
  timerAlarmWrite(animTimer, 150000, true); // every 0.1 seconds
  timerAlarmEnable(animTimer);
	 
	delay(500);
}



//int i = 0;

///////////////////////////////////////////////////////////
//
void animateImageFrame()
{
  static int frame = 0;
  tft.pushImage(0, 82, 135, 63, getAnimFrame(frame));
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

    //Serial.println("before Client update");
   
  // Serial.println("after Client update");
    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    formattedDate = timeClient.getFormattedDate();
    //Serial.println(formattedDate);
    viewPorts.pushViewPort(0, 0, TFT_WIDTH, 55  , VP_RestorePrev);
  
    int splitT = formattedDate.indexOf("T");
    dateStamp = formattedDate.substring(0, splitT);

    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.setTextFont(2);
    tft.setCursor(6, 42);
    tft.println(dateStamp); // Date
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    curTime = timeStamp.substring(0, 5); // Time 
    if (curTime != dispTime)
    {
      dispTime = curTime;
      tft.fillRect(1, 1, TFT_WIDTH, 32, TFT_BLACK);
      tft.setFreeFont(&Orbitron_Light_32);

      #define SEC_STR_WIDTH 19
      #define MAX_HOUR_WIDTH 111
      //Serial.println(tft.textWidth(dispTime));
      curY = tft.textWidth(dispTime); // No Space for seconds display
      showSec = curY > MAX_HOUR_WIDTH ? FALSE : SEC_STR_WIDTH;

      tft.setCursor((TFT_WIDTH - tft.textWidth(dispTime) - showSec ) / 2, 32);
      tft.print(dispTime );
      
      curX = tft.getCursorX();
      curY = tft.getCursorY();
    }
    //Serial.println("Show sec" + String(showSec));
    if ( showSec) // There is enough width to show the seconds digits
    {
      //Serial.println("in the if Show sec");
      curTime = timeStamp.substring(6, 8); // Seconds
      if (curTime != dispSeconds) {
        //Serial.println("in the curTime != dispSeconds");
        dispSeconds = curTime;
        tft.setTextFont(2);

        tft.setCursor(curX, curY - 14 );
        //Serial.println(tft.fontHeight());
        //Serial.println( tft.textWidth(t));
        tft.print(":" + dispSeconds);
      }
    }
   
   viewPorts.popViewPort();
}









///////////////////////////////////////////////////////////
//
void LeftBtn::onDoubleClick()
{
  mInv = !mInv;
	tft.invertDisplay(mInv);
  Serial.println("Button 1 doubleclick.");
}  


///////////////////////////////////////////////////////////
//
void RightBtn::onDoubleClick()
 {
  Serial.println("Button 2 doubleclick.");
  onRgtBtnDPress();
} 

///////////////////////////////////////////////////////////
//
void RightBtn::onStartLongPress()
{

  Serial.println("Button 2 longPress start");
  #define Y_ViewPort 87
  viewPorts.pushViewPort(0, Y_ViewPort, TFT_WIDTH, TFT_HEIGHT - Y_ViewPort  , VP_SaveUnder);

  
  //drawX();
}  


///////////////////////////////////////////////////////////
//
void RightBtn::onEndLongPress()
{
  Serial.println("Button 2 longPress stop");
   viewPorts.popViewPort();

}  // longPressStop2



///////////////////////////////////////////////////////////
//
void chkBtns()
{
  leftBtn.loop();
  rightBtn.loop();
}

///////////////////////////////////////////////////////////
//
void loop() {
  static int count = 0;


  chkBtns();
  
    if (!timeClient.update()) 
    {
      //Serial.println("force Client update");  
    }
   // Serial.println("timeClient.update " + String(getAvgPrefTime(timeupdate)));
   // int rTimeDisplay = startPrefAction("renderTimeDisplay");

   renderTimeDisplay();
   // Serial.println("renderTimeDisplay " + String(getAvgPrefTime(rTimeDisplay)));

  if (  isDispVisible() && viewPorts.isMainWindow())
  {

    if ( count == 0)
       renderWeatherData(); // about every 3 minutes
    if ((count % 500) == 0) // about 4 times in 3 minutes
       renderStocksData();

    count++;
    if (count >= 2000)
      count = 0;
 

  }

	delay(10);
  //Serial.println("Loop " + String(getAvgPrefTime(timeupdate)));
}


///////////////////////////////////////////////////////////
//
int getJsonFromUrl(String url, JsonDocument *josn)

{
  int status = FALSE;

  if ((WiFi.status() == WL_CONNECTED)) 
  { //Check the current connection status
    String payload = ""; //whole json

		HTTPClient http;

		http.begin(url); //Specify the URL

		if (http.GET() > 0) 
    { //Make the request and Check for the returning code
      // SUCCESS 
			payload = http.getString();
			//Serial.println(payload);

	    deserializeJson(*josn, payload.c_str());
      
      status = TRUE;
    }

    http.end(); //Free the resources
  }

  return status;
}

///////////////////////////////////////////////////////////
//
int isMarketOpen()
{

   int today = timeClient.getDay();
   //Serial.println("isMarketOpen today is " + String(today));
   if ( today == 0 || today == 6)
     return FALSE;

   int hour = timeClient.getHours();
    //Serial.println("isMarketOpen hour is " + String(hour));
   if ( hour < 16 || ((hour>= 23) && (timeClient.getMinutes() > 30) ))
     return FALSE;
  
   return TRUE;
}

///////////////////////////////////////////////////////////
//
void renderStocksData()
{
  static int notTheFirstTime = FALSE;
  static int wasMarketOpen = -1;
  int isMarketOpenNow;

  StaticJsonDocument<250> doc;

  isMarketOpenNow = isMarketOpen();
  if ( isMarketOpenNow != wasMarketOpen)
  {
    if (isMarketOpenNow )
     drawOpenMarketHeaders();
    else 
     tft.fillRect(StocksXPos , StocksYPos + 48, TFT_WIDTH - StocksXPos , TFT_HEIGHT - (StocksYPos + 48) , TFT_BLACK);
    wasMarketOpen = isMarketOpenNow;
  }
  if (!isMarketOpenNow && notTheFirstTime )
    return;
  
   // Serial.println("Inside renderStocksData,  isMarketOpen()" + String(isMarketOpen()) + " notThefirst time" + String(notTheFirstTime));

	if (getJsonFromUrl(ptcStockUrl, &doc)) 
  { 
    notTheFirstTime = TRUE;
    String ptcCurrent  = doc["c"];
    String ptcDayHigh  = doc["h"];
    String ptcDayLow   = doc["l"];
    String ptcDayChange= doc["d"];


    
    tft.setCursor(StocksXPos , StocksYPos + 40);
    tft.setTextFont(2);
    if (ptcDayChange.substring(0, 1) == "-")
      tft.setTextColor(TFT_RED, TFT_BLACK);
    else
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print("$" + ptcCurrent.substring(0, 6));
    //tft.setCursor(StocksXPos , StocksYPos + 58);
    //tft.print("Day :");
    if ( isMarketOpenNow)
    {
      tft.setTextFont(1);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setCursor(StocksXPos + 12 , StocksYPos + 79);
      tft.print("$" + ptcDayHigh.substring(0, 6));
      tft.setCursor(StocksXPos +12 , StocksYPos + 89);
      tft.print("$" + ptcDayLow.substring(0, 6));
    }
    //Serial.println(StocksYPos + 89);
  }
}

///////////////////////////////////////////////////////////
//
void renderWeatherData()
{
  StaticJsonDocument<1000> doc;

	if (getJsonFromUrl(endpoint + key, &doc)) 
  { 

	    String temp = doc["main"]["temp"];
	    String humi = doc["main"]["humidity"];
	    String town2 = doc["name"];
      String weatherIcon = doc["weather"][0]["icon"];
  

      // Debug data
	    //Serial.println("Temperature" + temp);
      //printf("%s/n",doc["main"]["temp"]);
	    //Serial.println("Humidity" + humi);
	    //Serial.println(town);

      drwWeatherIcon(weatherIcon);

      // Draw Tempreture
      tft.setFreeFont(&Orbitron_Medium_20);
      tft.fillRect(TempXPos, TempYPos, TempXWid, 20, TFT_BLACK);
      tft.setCursor(TempXPos, TempYPos + 18);
      //temp = "88.8";
	    tft.print(temp.substring(0, 4));
      int x, y;
      x = tft.getCursorX();
      y = tft.getCursorY();
      tft.drawCircle(x +3, y -12, 2, TFT_WHITE); // Degrees circle
      // Draw Humidity value
      tft.fillRect(TempXPos, TempYPos+46, TempXWid, 20, TFT_BLACK);
    	tft.setCursor(TempXPos, TempYPos + 60);
	    tft.println(humi + "%");

  
  } // End of SUCCESSful call 
	

}

