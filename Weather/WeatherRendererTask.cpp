#include <Arduino.h>
#include "EspUtils.h"
#include "EspIcons.h"
#include "EspButton.h"
#include "xEspTaskRenderer.h"
//#include "EspTimeBasedEvents.h"
#include "EspColors.h"
#include <TFT_eSPI.h>

#define TempXPos 0    // 60
#define TempYPos 180  //148
#define TempXWid 65   // 67

#define Town "Tel Aviv"
#define Country "IL"
//const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + town + "," + Country + "&units=metric&APPID=";
#define openWeatherMapPostFix "&units=metric&APPID="
#define openWeatherMapBase "http://api.openweathermap.org/data/2.5/weather?q="
#define openWeatherMapKey "91697f781be1daa54d31d7b4bcb75e5b"
//https://api.weatherbit.io/v2.0/forecast/daily?city=TelAviv,IL&key=f846dc6a4520446f8d6861f4d4b7bcb2
#define weatherBitBase "https://api.weatherbit.io/v2.0/"
#define weatherBitPrefix "forecast/daily?city="
#define weatherBitKey "&key=f846dc6a4520446f8d6861f4d4b7bcb2"

extern "C" const GFXfont* getOrbitronLight6Font();
extern "C" const GFXfont* getOrbitronMedium6Font();
extern "C" const GFXfont* getOrbitronMedium8Font();
extern const GFXfont* getOrbitonFontMed20();

char WeekDays[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

#define WX_CONDITIONS 55
struct Condx_S {
  int code;
  const char* condition;
  const char* icon;
};
const struct Condx_S Conditions[WX_CONDITIONS] = {
  // Put the more frequent ones at the top.
  { 800, "Clear Sky", "01d" },
  { 801, "Few Clouds: 11 - 25%", "02d" },
  { 802, "Scattered Clouds: 25 - 50%", "02d" },
  { 803, "Broken Clouds: 51 - 84%", "02d" },
  { 804, "Overcast Clouds: 85 - 100%", "03d" },

  { 500, "Light Rain", "04d" },
  { 501, "Moderate Rain", "04d" },
  { 502, "Heavy Intensity Rain", "09d" },
  { 503, "Very Heavy Rain", "09d" },
  { 504, "Extreme Rain", "09d" },
  { 511, "Freezing Rain", "09d" },
  { 520, "Light Intensity Shower Rain", "10d" },
  { 521, "Shower Rain", "10d" },
  { 522, "Heavy Intensity Shower Rain", "10d" },
  { 531, "Ragged Shower Rain", "10d" },

  { 200, "Thunderstorm with Light Rain", "02d" },
  { 201, "Thunderstorm with Rain", "02d" },
  { 202, "Thunderstorm with Heavy Rain", "02d" },
  { 210, "Light Thunderstorm", "02d" },
  { 211, "Thunderstorm", "02d" },
  { 212, "Heavy Thunderstorm", "02d" },
  { 221, "Ragged Thunderstorm", "02d" },
  { 230, "Thunderstorm w/Light Drizzle", "11d" },
  { 231, "Thunderstorm with Drizzle", "11d" },
  { 232, "Thunderstorm w/Heavy Drizzle", "11d" },

  { 300, "Light Intensity Drizzle", "02d" },
  { 301, "Drizzle", "02d" },
  { 302, "Heavy Intensity Drizzle", "02d" },
  { 310, "Light Intensity Drizzle Rain", "02d" },
  { 311, "Drizzle Rain", "02d" },
  { 312, "Heavy Intensity Drizzle Rain", "02d" },
  { 313, "Shower Rain And Drizzle", "02d" },
  { 314, "Heavy Shower Rain & Drizzle", "02d" },
  { 321, "Shower Drizzle", "02d" },

  { 600, "Light Snow", "02d" },
  { 601, "Snow", "02d" },
  { 602, "Heavy Snow", "02d" },
  { 611, "Sleet", "02d" },
  { 612, "Light Shower Sleet", "02d" },
  { 613, "Shower Sleet", "02d" },
  { 615, "Light Rain And Snow", "02d" },
  { 616, "Rain And Snow", "02d" },
  { 620, "Light Shower Snow", "02d" },
  { 621, "Shower Snow", "02d" },
  { 622, "Heavy Shower Snow", "02d" },

  { 701, "Mist", "50d" },
  { 711, "Smoke", "50d" },
  { 721, "Haze", "50d" },
  { 731, "Sand / Dust Whirls", "50d" },
  { 741, "Fog", "50d" },
  { 751, "Sand", "50d" },
  { 761, "Dust", "50d" },
  { 762, "Volcanic Ash", "50d" },
  { 771, "Squalls", "50d" },
  { 781, "Tornado", "50d" },
};

///////////////////////////////////////////////////////////
//
const uint16_t* WeatherRendererTask::getWeatherIcon(String icon) {

  if (icon == "01n")
    return getWeather01n();

  else if (icon == "02n")
    return getWeather02n();

  else if (icon == "03n")
    return getWeather03d();

  else if (icon == "04n")
    return getWeather04d();

  else if (icon == "09n")
    return getWeather09d();

  else if (icon == "10n")
    return getWeather10n();

  else if (icon == "11n")
    return getWeather11n();

  else if (icon == "13n")
    return getWeather13d();

  else if (icon == "50n")
    return getWeather50n();

  else if (icon == "01d")
    return getWeather01d();

  else if (icon == "02d")
    return getWeather02d();

  else if (icon == "03d")
    return getWeather03d();

  else if (icon == "04d")
    return getWeather04d();

  else if (icon == "09d")
    return getWeather09d();

  else if (icon == "10d")
    return getWeather10d();

  else if (icon == "11d")
    return getWeather11d();

  else if (icon == "13d")
    return getWeather13d();

  else if (icon == "50d")
    return getWeather50d();

  return NULL;
}
///////////////////////////////////////////////////////////
//
void WeatherRendererTask::drwWeatherIcon(String icon) {
  const uint16_t* iconArr = getWeatherIcon(icon);

  if (iconArr != NULL)
    mTftPtr->pushImage(TempXPos + 14, TempYPos - 38, 32, 32, iconArr);
}

///////////////////////////////////////////////////////////
//
const char* WeatherRendererTask::getIconByCode(int code) {
  for (int idx = 0; idx < WX_CONDITIONS; idx++)
    if (Conditions[idx].code == code)
      return Conditions[idx].icon;

  return NULL;
}
///////////////////////////////////////////////////////////
//
void WeatherRendererTask::renderTask(int opt, void* data) {
  StaticJsonDocument<1000> doc;
  String url;
#ifdef _DEBUG_INO
  Serial.println("renderWeatherData()");
#endif

  buildQuaryUrl(&url, openWeatherMapBase, Town, ",", Country, openWeatherMapPostFix, openWeatherMapKey, NULL);
  //Serial.printf("WeatherRendererTask::renderTask URL %s \n", url.c_str());
  if (getJsonFromUrl(url, &doc)) {

    String temp = doc["main"]["temp"];
    String humi = doc["main"]["humidity"];
    String town2 = doc["name"];
    String weatherIcon = doc["weather"][0]["icon"];


    // Debug data
    //Serial.println("Temperature" + temp);
    //printf("%s/n",doc["main"]["temp"]);
    //Serial.println("Humidity" + humi);
    //Serial.println(town);
    if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
      drwWeatherIcon(weatherIcon);

      // Draw Tempreture
      //tft.setFreeFont(&Orbitron_Medium_20);
      mTftPtr->setFreeFont(getOrbitronMedium8Font());
      mTftPtr->fillRect(TempXPos, TempYPos, TempXWid, 22, TFT_BLACK);  // 20->22
      mTftPtr->setCursor(TempXPos + 7, TempYPos + 17);                 // +18
      //temp = "88.8";
      mTftPtr->setTextColor(TFT_WHITE, TFT_BLACK, true);  // The true works only for smooth fonts
      mTftPtr->print(temp.substring(0, 4));
      int x, y;
      x = mTftPtr->getCursorX();
      y = mTftPtr->getCursorY();
      mTftPtr->drawCircle(x + 3, y - 12, 2, TFT_WHITE);  // Degrees circle
      // Draw Humidity value
      mTftPtr->fillRect(TempXPos, TempYPos + 46, TempXWid, 20, TFT_BLACK);
      mTftPtr->setCursor(TempXPos + 7, TempYPos + 58);  //60
      mTftPtr->println(humi + "%");
      mTftPtr->unloadFont();  // Remove the font to recover memory used
      xSemaphoreGive(getTFTMutex());
    }  // close Mutex

  }  // End of SUCCESSful call
}

///////////////////////////////////////////////////////////
//
void WeatherRendererTask::getDaysForcast() {
  StaticJsonDocument<2000> doc;
  String url;
#ifdef _DEBUG_INO
  Serial.println("getDaysForcast()");
#endif

  buildQuaryUrl(&url, weatherBitBase, weatherBitPrefix Town, ",", Country, weatherBitKey, NULL);
  //Serial.printf("WeatherRendererTask::getDaysForcast URL %s \n", url.c_str());
  if (getJsonFromUrl(url, &doc)) {
    for (int day = 0; day < 7; day++) {
      String contry = doc["city_name"];
      mDaysForcast[day].mCode = doc["data"][day]["weather"]["code"];
      mDaysForcast[day].mDayMax = doc["data"][day]["max_temp"];
      mDaysForcast[day].mDayMin = doc["data"][day]["min_temp"];
      mDaysForcast[day].mPercip = doc["data"][day]["precip"];
      String icon = doc["data"][day]["weather"]["icon"];
      mDaysForcast[day].mIcon = icon.substring(1, 4);
      //Serial.printf("getDaysForcast contry %s icon %s code %d percip %f\n", contry.c_str(), mDaysForcast[day].mIcon.c_str(), mDaysForcast[day].mCode, mDaysForcast[day].mPercip);
    }
  }
}


///////////////////////////////////////////////////////////
//
void WeatherRendererTask::renderForcast() {
  TFT_eSprite scroller = TFT_eSprite(mTftPtr);  // Sprite object
  //Serial.println("renderForcast after TFT_eSprite(mTftPtr)");
  int viewPortWidth;
#define lOffset 3
  if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
    //Serial.println("renderForcast inside  xSemaphoreTake");
    int viewPortHeight = mTftPtr->getViewportHeight();
    viewPortWidth = mTftPtr->getViewportWidth();
    scroller.setColorDepth(8);
    scroller.setSwapBytes(true);
    scroller.createSprite(viewPortWidth, viewPortHeight - 32);
    scroller.fillSprite(TFT_BLACK);                                               // Fill sprite with blue
    scroller.setScrollRect(0, 0, viewPortWidth, viewPortHeight - 32, TFT_BLACK);  // here we set scroll gap fill color to blue
    scroller.setTextColor(TFT_WHITE);                                             // White text, no background

    mTftPtr->setFreeFont(getOrbitronMedium8Font());

    //mTftPtr->setTextColor(TFT_WHITE, MyDarkGREY);

    mTftPtr->drawString("Forcast:", lOffset, 7);
    mTftPtr->drawString("Loading ...", lOffset + 10, 65);
    getDaysForcast();
    //mTftPtr->unloadFont();  // Remove the font to recover memory u
    xSemaphoreGive(getTFTMutex());
  }
  int today = mNTPClient->getDay();
  int todayInx;
  int yPlot = 0;
  int idx = 0;
#define BottomLine 101
#define ScrollStep 34
#define NumDays 7
  char temp[6];
  while (getLeftBtn()->getIsDown()) {
    //Serial.println("getLeftBtn()->getIsDown()");
    if (yPlot < BottomLine)
      yPlot = idx * ScrollStep;
    if (yPlot > BottomLine) yPlot = BottomLine;
    todayInx = idx + today;
    if (todayInx >= NumDays) todayInx = todayInx - NumDays;
    //scroller.drawNumber(i, lOffset, yPlot, 2); // plot value in font 2
    if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
      if (todayInx == today)
        scroller.setFreeFont(getOrbitronMedium6Font());
      else
        scroller.setFreeFont(getOrbitronLight6Font());

      scroller.setTextColor(TFT_WHITE, TFT_BLACK);
      scroller.drawString(WeekDays[todayInx], 0, yPlot + 12);
      scroller.pushImage(29, yPlot, 32, 32, getWeatherIcon(getIconByCode(mDaysForcast[idx].mCode)));
      //scroller.pushImage(29, yPlot, 32, 32, getWeatherIcon("01n"));
      sprintf(temp, "%2.0f/%2.0f", mDaysForcast[idx].mDayMax, mDaysForcast[idx].mDayMin);
      //Serial.printf("Day %s YPlot %d Day Width %d Temps %s \n", WeekDays[idx], yPlot, scroller.textWidth(WeekDays[idx]), temp);

      scroller.drawString(temp, 62, yPlot + 12, 2);
      scroller.drawCircle(79, yPlot + 12, 1, TFT_WHITE);
      scroller.drawCircle(101, yPlot + 12, 1, TFT_WHITE);
      if (mDaysForcast[idx].mPercip > 1.0) {
        sprintf(temp, "%2.0f%c", mDaysForcast[idx].mPercip, '%');
        scroller.drawString(temp, viewPortWidth - 26, yPlot + 12, 2);
      }



      idx++;

      scroller.pushSprite(0, 32);

      if (idx >= NumDays)
        idx = 0;

      if (yPlot == BottomLine)
        scroller.scroll(0, -ScrollStep);  // scroll stext 0 pixels left/right, 16 up

      xSemaphoreGive(getTFTMutex());
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

///////////////////////////////////////////////////////////
//
void WeatherRendererTask::preTaskLoop(void* data) {
  if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
    mTftPtr->setTextColor(TFT_ORANGE, TFT_BLACK, 1);
    //mTftPtr->setCursor(TempXPos, TempYPos + 34, 2);
    mTftPtr->drawString("Humidity", TempXPos, TempYPos + 23, 2);
    //mTftPtr->println("Humidity");
    mTftPtr->setTextColor(TFT_WHITE, TFT_BLACK);
    mTftPtr->setFreeFont((GFXfont*)getOrbitonFontMed20());
    mTftPtr->setCursor(6, 80);
    mTftPtr->println(Town);
    xSemaphoreGive(getTFTMutex());
  }
}