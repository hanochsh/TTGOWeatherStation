#pragma once
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

//#define NULL (uint16_t *)0;
#define VP_SaveUnder    0b00000001
#define VP_RestorePrev  0b00000010
#define VP_FullRestore  0b00000011
#define VP_FrameOffset  2

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

struct ViewPort
{
	
	int32_t   x, y, w, h;
	int32_t   px, py, pw, ph;
	uint16_t  opt;
	uint16_t* saveUnder;

	ViewPort(){
		x = 0; y = 0; w = 0; h = 0;
		px =  py =  pw = ph = 0;
		saveUnder = NULL;
	}
};

#define EmptyStack -1
#define MaxStackSize    5

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////
class ViewPortsStack
{
public:
	ViewPortsStack(TFT_eSPI *tft)  {
		mCurViewPortPtr = EmptyStack;
		mtftPtr = tft;
	}
	uint16_t  pushViewPort(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t Opt);
	void popViewPort();
	uint16_t  isMainWindow();
private:
	uint16_t mCurViewPortPtr;
	TFT_eSPI *mtftPtr;
	ViewPort  mStackArr[MaxStackSize];
};
