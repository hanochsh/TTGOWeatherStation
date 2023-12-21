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
	}

	void createCoreTask(TaskHandle_t* handle);
	void createCoreTask(TaskHandle_t* handle, void* data) {
		mData = data;  
		createCoreTask(handle);
	}
	static void taskCoreCB(void* vThis);
	virtual void renderTask(int opt, void *data ) {}
	void setRenderOpt(int set) { mRenderOpt = set; }

private:
	char* mName;
	int   mStackSize;
	int   mPriority;
	TaskHandle_t mHandle;
	int   mCore;
	int   mFreq;
	int   mRenderOpt;
	void* mData;
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class MainTimeRendererTask : public xEspTaskRenderer {
public:
	MainTimeRendererTask() : xEspTaskRenderer("MainTimeRendererTask", 2000, 2, 1,800) {}
	void renderTask(int opt, void* data);
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class StockRendererTask : public xEspTaskRenderer {
public:
	StockRendererTask() : xEspTaskRenderer("StockRendererTask", 6000, 1, 1, 45000) {}
	void renderTask(int opt, void* data);
};

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////
class AnimationRendererTask : public xEspTaskRenderer {
public:
	AnimationRendererTask() : xEspTaskRenderer("AnimationRendererTask", 2000, 0, 0, 150) {}
	void renderTask(int opt, void* data);
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