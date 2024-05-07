#include <Arduino.h>
#include "EspUtils.h"
#include "EspIcons.h"
#include "EspButton.h"
#include "xEspTaskRenderer.h"
#include "EspTimeBasedEvents.h"
#include "EspColors.h"
#define dbgVerbose 0
#include "EspDebug.h"
#include <TFT_eSPI.h> 

//extern "C" const GFXfont * getOrbitronLight15Font();
//extern "C" const GFXfont * getOrbitronLight4Font();
//extern "C" const GFXfont * getOrbitronLight6Font();
//extern "C" const GFXfont * getOrbitronLight8Font();
extern "C" const GFXfont* getOrbitronMedium8Font() ;
//extern const GFXfont* getOrbitonFontMed20();

const char* finnhubKey = "&token=clj68h9r01qok8f2vjf0clj68h9r01qok8f2vjfg";
const char* finStockQoutePrefix = "quote?symbol=";
const char* finnhubURL = "https://finnhub.io/api/v1/";
const char* finMarketStatusPrefix = "stock/market-status?exchange=US";
const char* finTickerPeersPrefix = "stock/peers?symbol=";
const char* finTickerProfile = "stock/profile2?symbol=";
//https://cloud.iexapis.com/v1/tops/last?symbols=PTC&token=pk_9821757254014256a6f2ef404020db6b
// https://cloud.iexapis.com/v1/stock/PTC/company&token=pk_9821757254014256a6f2ef404020db6b does nto wrok


#define iexapiURL "https://cloud.iexapis.com/v1/"
#define iexapiMSymbols "tops/last?symbols="
#define iexapiKey "token=pk_9821757254014256a6f2ef404020db6b"
#define iexapiStockPrefix "stock/"
#define iexapiPeersPostfix "/peers?"
#define iexapiCompaniesData "data/core/company/"
#define iexapiCompaniesDataPostfix "?"

//https://finnhub.io/api/v1/stock/peers?symbol=PTC&token=clj68h9r01qok8f2vjf0clj68h9r01qok8f2vjfg  peers /stock/market-status?exchange=US
//https://finnhub.io/api/v1/stock/market-status?exchange=US&token=clj68h9r01qok8f2vjf0clj68h9r01qok8f2vjfg
//https://finnhub.io/api/v1/stock/quote?symbol=IXIC&token=clj68h9r01qok8f2vjf0clj68h9r01qok8f2vjfg
//https://finnhub.io/api/v1/stock/profile2?symbol=PTC&token=clj68h9r01qok8f2vjf0clj68h9r01qok8f2vjfg
// https://cloud.iexapis.com/v1/stock/PTC/peers?token=pk_9821757254014256a6f2ef404020db6b

#define StockUP 1
#define StockDOWN -1
#define StockErr 0

#define StocksXPos 73  // 75
#define StocksYPos 144

//////////////////////////////////////////////
//
void StockRendererTask::vTimerPeersCallback(TimerHandle_t xTimer)
{
    //StockRendererTask *pThis = (StockRendererTask *)pvTimerGetTimerID(xTimer);
    int sendMsg = GetTickerPeersEvnt;
    //Serial.printf("StockRendererTask::vTimerPeersCallback \n");  
    xQueueSend(getMessageQueue(), &sendMsg, 0);
 }

//////////////////////////////////////////////
//
void StockRendererTask::preTaskLoop(void* data)
{
    TimerHandle_t xTimer = xTimerCreate("StockRendererTaskPeersTimer", 
                                        pdMS_TO_TICKS(5000), pdFALSE, (void*)this, StockRendererTask::vTimerPeersCallback);
    xTimerStart(xTimer, 0);

    if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
        mTftPtr->pushImage(StocksXPos + 13, StocksYPos + 1, 32, 32, getPTCIcon32x32());
        xSemaphoreGive(getTFTMutex());
    }
}



///////////////////////////////////////////////////////////
//
void trimString(String &str, size_t maxLength) {

   // Serial.printf("trimString str input:%s \n", str.c_str());
    // Check if the string length is already within the limit
    if (str.length() <= maxLength) {
       // Serial.printf("trimString str output:%s \n", str.c_str());
        return;  // No need to trim
    }

    // Find the last space within the first maxLength characters
    for (size_t i = maxLength; i > 0; i--) {
        if (str[i] == ' ') {
            // Found a space, trim the string at this position
            str[i] = '\0';
           // Serial.printf("trimString str output:%s \n", str.c_str());
            return;
        }
    }

    // If no space is found, simply truncate the string at maxLength
    str[maxLength] = '\0';
   // Serial.printf("trimString str output:%s \n", str.c_str());
}

///////////////////////////////////////////////////////////
//
void StockRendererTask::getPeerName(String ticker, String* name)
{
    StaticJsonDocument<40> doc;
    String url = "";
    JsonDocument filter;
     filter["name"] = true;

    buildQuaryUrl(&url, finnhubURL, finTickerProfile, ticker, finnhubKey, NULL);
    //Serial.printf("StockRendererTask::getPeerName url:%s \n", url.c_str());
    if (getJsonWithFilterFromUrl(url, &doc, filter)) {
        //Serial.printf("StockRendererTask::getPeerName got the doc !\n");
        String tmp = doc["name"];
        trimString(tmp, 10);
        *name = String(tmp.c_str());

        //Serial.printf("StockRendererTask::getPeerName name :%s vs %s \n", name->c_str(), tmp.c_str());
    }
}
///////////////////////////////////////////////////////////
//
void StockRendererTask::getPeersNames()
{
    StaticJsonDocument<500> doc;
    JsonDocument filter;
    String url = "";
    filter[0]["companyName"] = true;
 
    buildQuaryUrl(&url, iexapiURL, iexapiCompaniesData, mPeersGroup.c_str(), iexapiCompaniesDataPostfix, iexapiKey, NULL);
    if (getJsonWithFilterFromUrl(url, &doc,filter))
    {
        for (int i = 0; i < mNumPeers; i++)
        {
            String tmp = doc[i]["companyName"];
            //Serial.printf("StockRendererTask::getPeersNames tmp %s \n", tmp.c_str());
            trimString(tmp, 10);
            mPeersStocks[i].mName = String(tmp.c_str());
        }
    }
}

///////////////////////////////////////////////////////////
//
void StockRendererTask::getPeers() {
    String url;
    int index = 0;
    int lastIndex = -1;
    String payload;

    if (mNumPeers != 0) // its a restart of the events but the peers data is already in memory
        return;

    LOG_DBG("");

    //buildQuaryUrl(&url, iexapiURL, iexapiStockPrefix, "PTC", iexapiPeersPostfix, iexapiKey, NULL);
    buildQuaryUrl(&url, finnhubURL, finTickerPeersPrefix, "PTC", finnhubKey, NULL);
    //Serial.printf("StockRendererTask::getPeers url %s \n ", url.c_str());
    if (getPayloadFromUrl(url, &payload)) {
        payload.remove(payload.length() - 1, 1);
        payload.remove(0, 1);
        removeCharFromString(payload, '"');
        payload = "ADSK,CDNS,TEAM,DASTY," + payload;
        mPeersGroup = payload;
        payload += ",";
        
       // Serial.printf("StockRendererTask::getPeers payload %s ", payload.c_str());

        for (int i = 0; i < payload.length(); i++) {
            if (payload[i] == ',') {
                resizeArray(mNumPeers + 1);
                mPeersStocks[mNumPeers].mTicker = payload.substring(lastIndex + 1, i);

                getPeerName(mPeersStocks[mNumPeers].mTicker, &mPeersStocks[mNumPeers].mName);
                mPeersStocks[mNumPeers].mLastTrade = 0.0;
               // Serial.printf("StockRendererTask::getPeers Peer %s %s \n", mPeersStocks[mNumPeers].mTicker.c_str(), 
                   // mPeersStocks[mNumPeers].mName.c_str());
               
                mNumPeers++;
                lastIndex = i;
            }
        }
        //getPeersNames(); // get the companies names 
    }

}

///////////////////////////////////////////////////////////
//
void StockRendererTask::verifyCurrentMarketStatus() {
    StaticJsonDocument<250> doc;
    String url = "";
    LOG_DBG("");
    buildQuaryUrl(&url, finnhubURL, finMarketStatusPrefix, finnhubKey, NULL);
    LOG_DBG(" url:%s \n", url.c_str());
    if (getJsonFromUrl(url, &doc)) {
        LOG_DBG("got the doc !\n");
        setMarketWasOpen();
        mMarketOpen = doc["isOpen"];
        String tmp = doc["holiday"];
        mHoliday = tmp != "null" ? tmp : "";
    }
    LOG_DBG(" %d Holyday %s ", mMarketOpen,mHoliday.c_str());
}



///////////////////////////////////////////////////////////
//
int StockRendererTask::getStockQuate(String ticker, String& current, String& dayHigh, String& dayLow)
{
	String buff;
    StaticJsonDocument<250> doc;
    LOG_DBG("Ticker %s", ticker.c_str());
	if (getJsonFromUrl(buildQuaryUrl(&buff, finnhubURL, finStockQoutePrefix, ticker.c_str(), finnhubKey, NULL), &doc)) {
        LOG_DBG("Parsing the JSON");
        String tmpStr = doc["c"];   current = tmpStr;
        String tmpStr1 = doc["h"];  dayHigh = tmpStr1;
        String tmpStr2 = doc["l"];  dayLow  = tmpStr2;
        String tmpStr3 = doc["d"];

		//return dayChange.substring(0, 1) == "-" ? StockDOWN : StockUP;
        return tmpStr3[0] == '-' ? StockDOWN : StockUP;
	}

	return StockErr;
}
///////////////////////////////////////////////////////////
//
void StockRendererTask::renderStocksData() {
    static int notTheFirstTime = FALSE;

    if ( !notTheFirstTime)
        verifyCurrentMarketStatus();

    LOG_DBG("");

    //Serial.printf("MarketOpen %d, WasOpen %d, notTheFirstTime %d \n",mMarketOpen,mMarketWasOpen, notTheFirstTime  );
    if (mMarketOpen != mMarketWasOpen) {
        if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
            if (mMarketOpen)
                drawOpenMarketHeaders();
            else
                mTftPtr->fillRect(StocksXPos, StocksYPos + 53, TFT_WIDTH - StocksXPos, TFT_HEIGHT - (StocksYPos + 53), TFT_BLACK);
            mMarketWasOpen = mMarketOpen;
            xSemaphoreGive(getTFTMutex());
        }
    }
    if (!mMarketOpen && notTheFirstTime)
        return;
   
    int stockStatus;
    String ptcCurrent; 
	String ptcDayHigh;
	String ptcDayLow;
	if (StockErr != (stockStatus = getStockQuate("PTC", ptcCurrent, ptcDayHigh, ptcDayLow))) {
		notTheFirstTime = TRUE;

		if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
            mTftPtr->fillRect(StocksXPos, StocksYPos + 41, TFT_WIDTH - StocksXPos, 16, TFT_BLACK);
			mTftPtr->setCursor(StocksXPos-2, StocksYPos + 52); // 40
            mTftPtr->setFreeFont(getOrbitronMedium8Font());
            //mTftPtr->setTextDatum(BL_DATUM);
			//mTftPtr->setTextFont(2);
			// if (ptcDayChange.substring(0, 1) == "-")

			mTftPtr->setTextColor(stockStatus == StockDOWN ? TFT_RED : TFT_GREEN, TFT_BLACK);

			mTftPtr->print("$" + ptcCurrent.substring(0, 5));
			//tft.setCursor(StocksXPos , StocksYPos + 58);
			//tft.print("Day :");
			if (mMarketOpen) {
				mTftPtr->setTextFont(1);
				mTftPtr->setTextColor(TFT_WHITE, TFT_BLACK);
				mTftPtr->setCursor(StocksXPos + 12, StocksYPos + 79);
				mTftPtr->print("$" + ptcDayHigh.substring(0, 6));
				mTftPtr->setCursor(StocksXPos + 12, StocksYPos + 89);
				mTftPtr->print("$" + ptcDayLow.substring(0, 6));
			}
			//Serial.println(StocksYPos + 89);
			xSemaphoreGive(getTFTMutex());
		}
	}
}

///////////////////////////////////////////////////////////////
/// </summary>
void StockRendererTask::resizeArray(int newSize) {
    Stock* tempArray = new Stock[newSize];

    // Copy elements from the old array to the new array
    if (mNumPeers > 0)
    {
        memcpy(tempArray, mPeersStocks, sizeof(Stock) * mNumPeers);

        // Deallocate the memory of the old array
        delete[] mPeersStocks;
    }

    // Point dynamicArray to the new array
    mPeersStocks = tempArray;
}

///////////////////////////////////////////////////////////
//
void StockRendererTask::getPeersLastPrice()
{
    String buff;
    StaticJsonDocument<1000> doc;
    int offset = 0;
    //char* symbol = NULL;
    if (getJsonFromUrl(buildQuaryUrl(&buff, iexapiURL, iexapiMSymbols,  mPeersGroup.c_str(),"&",  iexapiKey, NULL), &doc)) {
       //Serial.println("getPeersLastPrice " + buff);
		for (int i = 0; i < mNumPeers; i++)
		{
            String symbol = doc[i - offset]["symbol"];
			if (!strcmp(mPeersStocks[i].mTicker.c_str(), symbol.c_str()))
                mPeersStocks[i].mLastTrade = doc[i - offset]["price"];
            else
            {
                //Serial.printf("Peer %s ticker %s Symbol %s Price %f \n", mPeersStocks[i].mName.c_str(), mPeersStocks[i].mTicker.c_str(), symbol.c_str(), mPeersStocks[i].mLastTrade);
                offset++; // Dassult is not in this list as it is not traded in the US ?
                mPeersStocks[i].mLastTrade = 0.0;
            }
            //Serial.printf("Peer %s Price %f \n", mPeersStocks[i].mName.c_str(), mPeersStocks[i].mLastTrade);
		}

    }
}



///////////////////////////////////////////////////////////
//
struct Trade
{
    String mLastPrice;
    int    mUpDown;
};

void StockRendererTask::renderPeersInViewPort()
{
    TFT_eSprite scroller = TFT_eSprite(mTftPtr); // Sprite object 
    int viewPortWidth;
#define lOffset 4
    if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
        int viewPortHeight = mTftPtr->getViewportHeight();
        viewPortWidth = mTftPtr->getViewportWidth();
        scroller.setColorDepth(8);
        scroller.createSprite(viewPortWidth, viewPortHeight - 32);
        scroller.fillSprite(MyDarkGREY); // Fill sprite with blue
        scroller.setScrollRect(0, 0, viewPortWidth, viewPortHeight - 32, MyDarkGREY);     // here we set scroll gap fill color to blue
        scroller.setTextColor(TFT_WHITE); // White text, no background
        //scroller.setTextDatum(BR_DATUM);  // Bottom right coordinate datum
        mTftPtr->setCursor(lOffset, lOffset * 5);  // Set cursor at top left of screen
        
        mTftPtr->setFreeFont(getOrbitronMedium8Font());
        
        //mTftPtr->setTextColor(TFT_WHITE, MyDarkGREY);
        mTftPtr->println("Market:");
        //mTftPtr->unloadFont();  // Remove the font to recover memory u
        xSemaphoreGive(getTFTMutex());
    }
    //getPeersLastPrice();
    int yPlot = 0;
    int idx = 0;
    int pause;
  
    String dayHigh;
    String dayLow, lastPrice;
    Trade*  lastTrades = new Trade[mNumPeers];

    for (idx = 0; idx < mNumPeers; idx++)
        lastTrades[idx].mUpDown = StockErr; // sort of reset the data
    idx = 0;
    while  (getRightBtn()->getIsDown())
    {
        if (yPlot < 127)
          yPlot = idx * 16;
        if (yPlot > 127) yPlot = 127;

        if (lastTrades[idx].mUpDown == StockErr)
        {
            if (StockErr != (lastTrades[idx].mUpDown = getStockQuate(mPeersStocks[idx].mTicker, lastPrice, dayHigh, dayLow)))
            {
                LOG_DBG("Got the stock quate %s", lastPrice.c_str());
                lastTrades[idx].mLastPrice = lastPrice;
            }
            else
                lastTrades[idx].mLastPrice = "-";
        }

        //scroller.drawNumber(i, lOffset, yPlot, 2); // plot value in font 2
        if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
            scroller.setTextColor(TFT_WHITE, MyDarkGREY);

            if (mPeersStocks[idx].mName.length() == 0)
                LOG_ERR("mPeersStocks[idx].mName.length() == 0");
            scroller.drawString(mPeersStocks[idx].mName, lOffset, yPlot, 2); // plot value in font 2
      
          /*  if (mPeersStocks[idx].mLastTrade != 0.0)
                lastPrice = String(mPeersStocks[idx].mLastTrade);
            else
                lastPrice = "-"; */
            if (lastTrades[idx].mUpDown != StockErr)
                scroller.setTextColor(lastTrades[idx].mUpDown == StockDOWN ? TFT_RED : TFT_GREEN, MyDarkGREY);
            scroller.drawString(lastTrades[idx].mLastPrice, viewPortWidth/2 + 13, yPlot, 2);
            idx++;

            scroller.pushSprite(0, 32);

            if (idx >= mNumPeers)
                idx = 0;

            if (yPlot == 127)
                scroller.scroll(0, -16); // scroll stext 0 pixels left/right, 16 up

            xSemaphoreGive(getTFTMutex());
		} // end of if Semaphore TFT
		// If the next record is not error, the call to get the stock was not done so delay is needed
		pause = lastTrades[idx].mUpDown != StockErr ? 1500 : 300;
        LOG_DBG("Pause is %d", pause);
		vTaskDelay(pdMS_TO_TICKS(pause));

    }

    delete[] lastTrades;
    
}

///////////////////////////////////////////////////////////
//
void StockRendererTask::drawOpenMarketHeaders() {
    mTftPtr->setTextFont(2);
    mTftPtr->setTextColor(TFT_ORANGE, TFT_BLACK);
    mTftPtr->setCursor(StocksXPos, StocksYPos + 60);
    mTftPtr->print("Day");
    mTftPtr->setTextFont(1);
    mTftPtr->setCursor(StocksXPos, StocksYPos + 79);
    mTftPtr->print("H: ");
    mTftPtr->setCursor(StocksXPos, StocksYPos + 89);
    mTftPtr->print("L: ");
}

///////////////////////////////////////////////////////////
//
void StockRendererTask::renderTask(int opt, void* data) {
    renderStocksData();
}