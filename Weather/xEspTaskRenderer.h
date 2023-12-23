#pragma once

typedef void* TaskHandle_t;

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
	MainTimeRendererTask() : xEspTaskRenderer("MainTimeRendererTask", 2000, 2, 1, 800) {
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
class StockRendererTask : public xEspTaskRenderer {
public:
	StockRendererTask() : xEspTaskRenderer("StockRendererTask", 6000, 1, 1, 45000) { setMarketOpen(false); }
	void renderTask(int opt, void* data);
	void setMarketOpen(int isOpen) { mMarketOpen = isOpen; }
private:
	void renderStocksData();
	int mMarketOpen;
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
	WeatherRendererTask() : xEspTaskRenderer("WeatherRendererTask", 6000, 1, 1, 200000) {}
	void renderTask(int opt, void* data);
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class CheckButtonsTask : public xEspTaskRenderer {
public:
	CheckButtonsTask() : xEspTaskRenderer("CheckButtonsTask", 2000, 1, 0, 10) {}
	void renderTask(int opt, void* data);
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class MsgReciverTask : public xEspTaskRenderer {
public:
	MsgReciverTask() : xEspTaskRenderer("MsgReciverTask", 5000, 1, 1, 10) {}
	void renderTask(int opt, void* data);
};