#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson.git
#include <NTPClient.h> //https://github.com/taranais/NTPClient

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#define blue 0x5D9B
#include "Orbitron_Medium_20.h"
#include <WiFi.h>

#include <WiFiUdp.h>
#include <HTTPClient.h>

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;


const char* ssid = "ShiriHome"; ///EDIIIT
const char* password = "shirihome035274757"; //EDI8IT
String town = "Tel Aviv"; //EDDIT
String Country = "IL"; //EDDIT
const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + town + "," + Country + "&units=metric&APPID=";
//const String stockUrl = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=PTC&apikey=CQ4BMVCC56VEWRRJ";
const String stockUrl = "https://finnhub.io/api/v1/quote?symbol=PTC&token=clj68h9r01qok8f2vjf0clj68h9r01qok8f2vjfg";
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


// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;


byte curBckLight = 1;

#define TempXPos 0 // 60
#define TempYPos 180  //148
#define TempXWid 67 // 57

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
    tft.pushImage(TempXPos + 14, TempYPos - 39,  32, 32, iconArr);
   
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
void setup(void) {
	pinMode(0, INPUT_PULLUP);
	pinMode(35, INPUT);
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

	//tft.setCursor(2, 232, 1);
	//tft.println(WiFi.localIP());

	//tft.setCursor(80, 204, 1);
	//tft.println("BRIGHT:");

	//tft.setCursor(80, 152, 2);
	//tft.println("SEC:");

	//tft.setTextColor(TFT_WHITE, lightblue);
	//tft.setCursor(4, 152, 2);
	//tft.println("TEMP:");
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
	tft.setCursor(TempXPos, TempYPos + 24, 2);
	tft.println("Humidity");

	tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setFreeFont(&Orbitron_Medium_20);
	tft.setCursor(6, 80);
	tft.println(town);

  //drawCurBckLgt();
  tft.drawLine(TempXPos + TempXWid + 1, 150, TempXPos + TempXWid + 1, 240, TFT_GREY);
  timeSetup();
	//getWeatherData(&tmp, &hum);
	delay(500);
}

///////////////////////////////////////////////////////////
//
void onRgtBtnPress() {

  tft.fillRect(78, 216, 44, 12, TFT_BLACK);
//tft.fillRect(78 + (i * 7), 216, 3, 10, blue);
	curBckLight++;
	if (curBckLight >= 5)
		curBckLight = 0;

	//drawCurBckLgt();
	ledcWrite(pwmLedChannelTFT, getCurBckLgt());

}


int i = 0;

int count = 0;
bool inv = 1;
int press1 = 0;
int press2 = 0;////

//int frame = 0;
String dispSeconds = "";
String curTime = "";
String dispTime = "";

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

String weatherIcon = "";
///////////////////////////////////////////////////////////
//
void loop() {
  int curX, curY;

  animateImageFrame();

	if (digitalRead(35) == 0) {
		if (press2 == 0)
		{
			press2 = 1;
      onRgtBtnPress();

		}
	}
	else press2 = 0;

	if (digitalRead(0) == 0) {
		if (press1 == 0)
		{
			press1 = 1;
			inv = !inv;
			tft.invertDisplay(inv);
		}
	}
	else press1 = 0;

  // Update Weather data only every 2000 times
	if (count == 0)
  {
		getWeatherData();
    
  }
	count++;
	if (count > 2000)
		count = 0;

	

	tft.setTextColor(TFT_ORANGE, TFT_BLACK);
	tft.setTextFont(2);
	tft.setCursor(6, 42);
	tft.println(dayStamp); // Date
	tft.setTextColor(TFT_WHITE, TFT_BLACK);

	while (!timeClient.update()) {
		timeClient.forceUpdate();
	}
	// The formattedDate comes with the following format:
	// 2018-05-28T16:00:13Z
	// We need to extract date and time
	formattedDate = timeClient.getFormattedDate();
	//Serial.println(formattedDate);


	int splitT = formattedDate.indexOf("T");
	dayStamp = formattedDate.substring(0, splitT);


	timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  curTime = timeStamp.substring(0, 5); // Time 
	if (curTime != dispTime)
	{
    dispTime = curTime;
		tft.fillRect(1, 1, TFT_WIDTH, 32, TFT_BLACK);
    tft.setFreeFont(&Orbitron_Light_32);

    #define SEC_STR_WIDTH 19
    tft.setCursor((TFT_WIDTH - tft.textWidth(dispTime) - SEC_STR_WIDTH ) / 2, 32);
    tft.print(dispTime );
    
    curX = tft.getCursorX();
    curY = tft.getCursorY();
	}

  curTime = timeStamp.substring(6, 8); // Seconds
	if (curTime != dispSeconds) {
    dispSeconds = curTime;
    tft.setTextFont(2);

    tft.setCursor(curX, curY - 14 );
    //String t = ":" + dispSeconds;
    //Serial.println(tft.fontHeight());
    //Serial.println( tft.textWidth(t));
		tft.print(":" + dispSeconds);
	}

	

	delay(80);
}

///////////////////////////////////////////////////////////
//
void getWeatherData()
{
  String payload = ""; //whole json
  StaticJsonDocument<1000> doc;

	//tft.fillRect(1, 170, 64, 20, TFT_BLACK);
	//tft.fillRect(1, 210, 64, 20, TFT_BLACK);
	if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

		HTTPClient http;

		http.begin(endpoint + key); //Specify the URL

		if (http.GET() > 0) { //Make the request and Check for the returning code
      // SUCCESS 
			payload = http.getString();
			// Serial.println(httpCode);
			Serial.println(payload);

		
	    char inp[1000];
	    payload.toCharArray(inp, 1000);
	    deserializeJson(doc, inp);

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
	    tft.print(temp.substring(0, 4));
      int x, y;
      x = tft.getCursorX();
      y = tft.getCursorY();

      // Draw Humidity value before reducing font size
      tft.fillRect(TempXPos, TempYPos+48, 50, 18, TFT_BLACK);
    	tft.setCursor(TempXPos, TempYPos + 60);
	    tft.println(humi + "%");
      // Draw the celsuice icon
      tft.setTextFont(2);
      tft.setCursor(x +2, y-13);
      tft.print("c");
      tft.setTextFont(1);
      tft.setCursor(x, y-15);
      tft.print(".");
  
    } // End of SUCCESSful call 
		else {
			Serial.println("Error on HTTP request");
		}

		http.end(); //Free the resources
	}

}



