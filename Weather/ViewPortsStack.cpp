#include "ViewPortsStack.h"

#define TFT_GREY 0x5AEB
#define ST7735_GRAY    0x8410

////////////////////////////////////////////////////////////
//
uint16_t  ViewPortsStack::pushViewPort(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t Opt)
{
	mCurViewPortPtr++;
	if (mCurViewPortPtr == MaxStackSize)
	{
		mCurViewPortPtr--;
		return FALSE;
	}

	mStackArr[mCurViewPortPtr].opt = Opt;
	mStackArr[mCurViewPortPtr].x = x;
	mStackArr[mCurViewPortPtr].y = y;
	mStackArr[mCurViewPortPtr].w = w;
	mStackArr[mCurViewPortPtr].h = h;

	if (Opt & VP_RestorePrev)
	{
		mStackArr[mCurViewPortPtr].px = mtftPtr->getViewportX();      // Always returns viewport x coordinate relative to screen left edge
		mStackArr[mCurViewPortPtr].py = mtftPtr->getViewportY();  // Always returns viewport y coordinate relative to screen top edge
		mStackArr[mCurViewPortPtr].pw = mtftPtr->getViewportWidth();  // Always returns width of viewport
		mStackArr[mCurViewPortPtr].ph = mtftPtr->getViewportHeight(); // Always returns height of viewport
		mtftPtr->resetViewport();
		mtftPtr->setViewport(x, y, w, h);
	}

	if (Opt & VP_SaveUnder)
	{
		mStackArr[mCurViewPortPtr].saveUnder = (uint16_t*)malloc(sizeof(uint16_t) * (h * w));
		mtftPtr->setViewport(x, y, w, h);
		mtftPtr->readRect(0, 0, w, h, mStackArr[mCurViewPortPtr].saveUnder);
		//mtftPtr->frameViewport(TFT_GREY, -2);
		mtftPtr->fillScreen(ST7735_GRAY);
		
	}
	return TRUE;
}

////////////////////////////////////////////////////////////
//
void ViewPortsStack::popViewPort()
{
	
	if (mStackArr[mCurViewPortPtr].opt & VP_RestorePrev)
	{
		mtftPtr->resetViewport();
		mtftPtr->setViewport(mStackArr[mCurViewPortPtr].px, mStackArr[mCurViewPortPtr].py, 
			                 mStackArr[mCurViewPortPtr].pw, mStackArr[mCurViewPortPtr].ph);
		mStackArr[mCurViewPortPtr].px = 0;      // Always returns viewport x coordinate relative to screen left edge
		mStackArr[mCurViewPortPtr].py = 0;  // Always returns viewport y coordinate relative to screen top edge
		mStackArr[mCurViewPortPtr].pw = 0;  // Always returns width of viewport
		mStackArr[mCurViewPortPtr].ph = 0; // Always returns height of viewport
		
	}

	if (mStackArr[mCurViewPortPtr].opt & VP_SaveUnder)
	{
		mtftPtr->pushRect(0, 0, mStackArr[mCurViewPortPtr].w, mStackArr[mCurViewPortPtr].h, 
			              mStackArr[mCurViewPortPtr].saveUnder);
		
		free(mStackArr[mCurViewPortPtr].saveUnder);
		mStackArr[mCurViewPortPtr].saveUnder = NULL;
		mStackArr[mCurViewPortPtr].opt = 0;
		mtftPtr->resetViewport();
	}
	
	mCurViewPortPtr--;
}

////////////////////////////////////////////////////////////
//
uint16_t  ViewPortsStack::isMainWindow()
{
	return mtftPtr->getViewportHeight() == TFT_HEIGHT;
}