#include <Arduino.h>
#include "EspUtils.h"
#include "EspIcons.h"
#include "EspButton.h"
#include "xEspTaskRenderer.h"
#include "EspTimeBasedEvents.h"
#include "EspColors.h"
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
    StaticJsonDocument<500> doc;
    String url = "";

    buildQuaryUrl(&url, finnhubURL, finTickerProfile, ticker, finnhubKey, NULL);
    //Serial.printf("StockRendererTask::getPeerName url:%s \n", url.c_str());
    if (getJsonFromUrl(url, &doc)) {
        //Serial.printf("StockRendererTask::getPeerName got the doc !\n");
        String tmp = doc["name"];
        trimString(tmp, 10);
        *name = String(tmp.c_str());

        //Serial.printf("StockRendererTask::getPeerName name :%s vs %s \n", name->c_str(), tmp.c_str());
    }
}

///////////////////////////////////////////////////////////
//
void StockRendererTask::getPeers() {
    String url;
    int index = 0;
    int lastIndex = -1;
    String payload;

    //Serial.printf("StockRendererTask::getPeers mPeers \n ");
    buildQuaryUrl(&url, iexapiURL, iexapiStockPrefix, "PTC", iexapiPeersPostfix, iexapiKey, NULL);
    //Serial.printf("StockRendererTask::getPeers url %s \n ", url.c_str());
    if (getPayloadFromUrl(url, &payload)) {
        payload.remove(payload.length() - 1, 1);
        payload.remove(0, 1);
        removeCharFromString(payload, '"');
        payload = "PTC," + payload;
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
    }

}

///////////////////////////////////////////////////////////
//
void StockRendererTask::verifyCurrentMarketStatus() {
    StaticJsonDocument<250> doc;
    String url = "";
    //Serial.println("StockRendererTask::verifyCurrentMarketStatus");
    buildQuaryUrl(&url, finnhubURL, finMarketStatusPrefix, finnhubKey, NULL);
    //Serial.printf("StockRendererTask::verifyCurrentMarketStatus url:%s \n", url.c_str());
    if (getJsonFromUrl(url, &doc)) {
        //Serial.printf("StockRendererTask::verifyCurrentMarketStatus got the doc !\n");
        setMarketWasOpen();
        mMarketOpen = doc["isOpen"];
        String tmp = doc["holiday"];
        mHoliday = tmp != "null" ? tmp : "";
    }
    //Serial.printf("StockRendererTask::verifyCurrentMarketStatus %d Holyday %s ", mMarketOpen,mHoliday.c_str());
}



///////////////////////////////////////////////////////////
//
int StockRendererTask::getStockQuate(String ticker, String& current, String& dayHigh, String& dayLow)
{
	String buff;
    StaticJsonDocument<250> doc;
    
	if (getJsonFromUrl(buildQuaryUrl(&buff, finnhubURL, finStockQoutePrefix, ticker.c_str(), finnhubKey, NULL), &doc)) {
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

#ifdef _DEBUG_INO
    Serial.println("renderStocksData()");
#endif

#ifdef _DEBUG_INO
    Serial.println("renderStocksData() isMarketOpen:" + String(isMarketOpenNow));
#endif
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
    getPeersLastPrice();
    int yPlot = 0;
    int idx = 0;
    String lastPrice;
    while  (getRightBtn()->getIsDown())
    {
        if (yPlot < 127)
          yPlot = idx * 16;
        if (yPlot > 127) yPlot = 127;
        //scroller.drawNumber(i, lOffset, yPlot, 2); // plot value in font 2
        if (xSemaphoreTake(getTFTMutex(), portMAX_DELAY)) {
            scroller.setTextColor(TFT_WHITE, MyDarkGREY);
            scroller.drawString(mPeersStocks[idx].mName, lOffset, yPlot, 2); // plot value in font 2
            
            //mTftPtr->setTextColor(stockStat == StockDOWN ? TFT_RED : TFT_GREEN, MyDarkGREY);
            if (mPeersStocks[idx].mLastTrade != 0.0)
                lastPrice = String(mPeersStocks[idx].mLastTrade);
            else
                lastPrice = "-";
            scroller.drawString(lastPrice, viewPortWidth/2 + 13, yPlot, 2);
            idx++;

            scroller.pushSprite(0, 32);

            if (idx >= mNumPeers)
                idx = 0;

            if (yPlot == 127)
                scroller.scroll(0, -16); // scroll stext 0 pixels left/right, 16 up

            xSemaphoreGive(getTFTMutex());
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
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