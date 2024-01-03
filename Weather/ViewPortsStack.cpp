#include "ViewPortsStack.h"
#include "DebugPrefTools.h"
#include "EspColors.h"


////////////////////////////////////////////////////////////
//
uint16_t  ViewPortsStack::pushViewPort(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t Opt)
{
	return pushViewPortExt(x, y, w, h, Opt, MyDarkGREY);
}

////////////////////////////////////////////////////////////
//
uint16_t  ViewPortsStack::pushViewPortExt(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t Opt, byte bckColor)
{
	mCurViewPortPtr++;
#ifdef _DEBUG_VIEWPORT
  Serial.println("ViewPortsStack::pushViewPort()");
#endif
	if (mCurViewPortPtr == MaxStackSize)
	{
#ifdef _DEBUG_VIEWPORT
  Serial.println("pushViewPort(): if (mCurViewPortPtr == MaxStackSize)");
#endif
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
#ifdef _DEBUG_VIEWPORT
  Serial.println("pushViewPort(): if (Opt & VP_RestorePrev)");
#endif
		mStackArr[mCurViewPortPtr].px = mtftPtr->getViewportX();      // Always returns viewport x coordinate relative to screen left edge
		mStackArr[mCurViewPortPtr].py = mtftPtr->getViewportY();  // Always returns viewport y coordinate relative to screen top edge
		mStackArr[mCurViewPortPtr].pw = mtftPtr->getViewportWidth();  // Always returns width of viewport
		mStackArr[mCurViewPortPtr].ph = mtftPtr->getViewportHeight(); // Always returns height of viewport
#ifdef _DEBUG_VIEWPORT
  Serial.println("pushViewPort(): if (Opt & VP_RestorePrev) all the get functions");
#endif
		mtftPtr->resetViewport();
#ifdef _DEBUG_VIEWPORT
  Serial.println("pushViewPort(): if (Opt & VP_RestorePrev) mtftPtr->resetViewport()");
#endif
		mtftPtr->setViewport(x, y, w, h);
#ifdef _DEBUG_VIEWPORT
  Serial.println("pushViewPort(): if (Opt & VP_RestorePrev) mtftPtr->setViewport(x, y, w, h)");
#endif
	}

	if (Opt & VP_SaveUnder)
	{
#ifdef _DEBUG_VIEWPORT
  Serial.println("pushViewPort(): if (Opt & VP_SaveUnder)");
#endif
		mStackArr[mCurViewPortPtr].saveUnder = (uint16_t*)malloc(sizeof(uint16_t) * (h * w));
		mtftPtr->setViewport(x, y, w, h);
		mtftPtr->readRect(0, 0, w, h, mStackArr[mCurViewPortPtr].saveUnder);
	
		mtftPtr->fillScreen(bckColor);
		
	}

	if (Opt & VP_WithFram)
	{
		mtftPtr->setViewport(x + VP_FrameOffset, y + VP_FrameOffset, w -2* VP_FrameOffset, h -2 * VP_FrameOffset);
		mtftPtr->frameViewport(FrameGRAY, -2);
	}
#ifdef _DEBUG_VIEWPORT
  Serial.println("pushViewPort(): return TRUE");
#endif
	return TRUE;
}

////////////////////////////////////////////////////////////
//
void ViewPortsStack::popViewPort()
{
	#ifdef _DEBUG_VIEWPORT
  Serial.println("ViewPortsStack::popViewPort()");
#endif
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
	if (mStackArr[mCurViewPortPtr].opt & VP_WithFram) // Need to get back to the full ViewPort before the restore
	{
		mtftPtr->setViewport(mStackArr[mCurViewPortPtr].x, mStackArr[mCurViewPortPtr].y, mStackArr[mCurViewPortPtr].w, mStackArr[mCurViewPortPtr].h);
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