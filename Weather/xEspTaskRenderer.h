#pragma once
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

typedef void* TaskHandle_t;
//typedef void* TFT_eSPI;
///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class xEspTaskRenderer
{
public:
	xEspTaskRenderer(char* name, int stackSize, int priority, int core, int freq) {
		mName = name;
		mStackSize = stackSize;
		mPriority = priority;
		
		mCore = core;
		mFreq = freq;
		mRenderOpt = -1;
		mData = NULL;
		enable();
	}

	void createCoreTask(TaskHandle_t* handle);
	void createCoreTask(TaskHandle_t* handle, void* data) {
		mData = data;  
		createCoreTask(handle);
	}
	static void taskCoreCB(void* vThis);
	
	virtual void renderTask(int opt, void *data ) {}
	virtual void preTaskLoop(void* data) {}
	void setRenderOpt(int set) { mRenderOpt = set; }
	void enable() { mEnabled = true; }
	void disable() { mEnabled = false; }

	
	
private:
	char* mName;
	int   mStackSize;
	int   mPriority;
	TaskHandle_t mHandle;
	int   mCore;
	int   mFreq;
	int   mRenderOpt;
	int   mEnabled;
	void* mData;
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class MainTimeRendererTask : public xEspTaskRenderer {
public:
	MainTimeRendererTask() : xEspTaskRenderer("MainTimeRendererTask", 2000, 2, 0, 800) {
		setRenderOpt(true);
		resetStatics();
	}
	void renderTask(int opt, void* data);
	void renderTimeDisplay(int opt, int vpX, int vpY);
	void enableMainScreen()
	{
		setRenderOpt(true);
		resetStatics();
	}
	void disableMainScreen()
	{
		setRenderOpt(false);
		resetStatics();
	}
	
private:
	void resetStatics() {
		mdispSeconds = mdispTime = "";
		mshowSec = false;
		mcurX = mcurY = -1;
	}
	
	String mdispSeconds;
	String mdispTime;
	int mcurX;
	int mcurY;
	int mshowSec;
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

struct Stock
{
	String mTicker;
	String mName;
	float mLastTrade;
};

class StockRendererTask : public xEspTaskRenderer {
public:
	StockRendererTask() : xEspTaskRenderer("StockRendererTask", 8000, 1, 1, 45000) { mMarketOpen = false;
	                                                                                 mMarketWasOpen =-1;
																					 mNumPeers = 0;
																					 mPeersStocks = NULL;
	}
	void renderTask(int opt, void* data);
	//void setMarketOpen(int isOpen) { mMarketOpen = isOpen; }
	void verifyCurrentMarketStatus();
	//void setMarketWasOpen(int wasOpen) { mMarketWasOpen = wasOpen; }
	void setMarketWasOpen() { mMarketWasOpen = mMarketOpen; }
	void preTaskLoop(void* data);
	void getPeers();
	void renderPeersInViewPort();
	void setTFT(TFT_eSPI* tft) { mTftPtr = tft; }
private:
	int getStockQuate(String ticker, String& current, String& dayHigh, String& dayLow);
	static void vTimerPeersCallback(TimerHandle_t xTimer);
	void getPeerName(String ticker, String *name);
	void renderStocksData();
	void getPeersLastPrice();
	void drawOpenMarketHeaders();
	void resizeArray(int newSize);
	Stock  *mPeersStocks;
	String mHoliday;
	//String mPeers[15];
	//String mPeerNames[15];
	String mPeersGroup;
	int mNumPeers;
	int mMarketOpen;
	int mMarketWasOpen;
	TFT_eSPI* mTftPtr;
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class AnimationRendererTask : public xEspTaskRenderer {
public:
	AnimationRendererTask() : xEspTaskRenderer("AnimationRendererTask", 3000, 0, 0, 150) { mFrame = 0; }
	void renderTask(int opt, void* data);
private:
	int mFrame;
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class WeatherRendererTask : public xEspTaskRenderer {
public:
	WeatherRendererTask() : xEspTaskRenderer("WeatherRendererTask", 6000, 1, 1, 500000) {}
	void renderTask(int opt, void* data);
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class CheckButtonsTask : public xEspTaskRenderer {
public:
	CheckButtonsTask() : xEspTaskRenderer("CheckButtonsTask", 6000, 1, 0, 10) {}
	void renderTask(int opt, void* data);
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class MsgReciverTask : public xEspTaskRenderer {
public:
	MsgReciverTask() : xEspTaskRenderer("MsgReciverTask", 8000, 1, 1, 100) {}
	void renderTask(int opt, void* data);
};