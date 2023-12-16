#pragma once
#include <Arduino.h>
#include <OneButton.h>

enum { None, SingleClick, DoubleClick, startLongPress, endLongPress };



///////////////////////////////////////////////////
/// </summary>
///////////////////////////////////////////////////
class EspButton
{
public:
	EspButton(byte pinBut) { 
		mGPIO = pinBut;
		mBtn = new OneButton(pinBut, true);
		mLastTStamp = -1;
		mLastState = -1; }
	void setup();
	int chkButton();
	void loop() { mBtn->tick(); }
	void tick();
private:
	virtual void onSingleClick()    {}
	virtual void onDoubleClick()    {}
	virtual void onStartLongPress() {}
	virtual void onEndLongPress()   {}

	static void onSingleClickCB(void*);
	static void onDoubleClickCB(void*);
	static void onStartLongPresCB(void*);
	static void onEndLongPressCB(void*);

	OneButton* mBtn;
	byte mGPIO;
	byte  mLastState;
	unsigned long mLastTStamp;
};

///////////////////////////////////////////////////
/// </summary>
///////////////////////////////////////////////////
class LeftBtn: public EspButton
{
public:
	LeftBtn(byte pinBut) : EspButton(pinBut) { mInv = true; }
private:
	void onSingleClick() {}
	void onDoubleClick();
	void onStartLongPress() {}
	void onEndLongPress() {}

	int mInv;
};

///////////////////////////////////////////////////
/// </summary>
///////////////////////////////////////////////////
class RightBtn:public EspButton
{
public:
	RightBtn(byte pinBut) : EspButton(pinBut) {}
private:
	void onSingleClick() {}
	void onDoubleClick();
	void onStartLongPress();
	void onEndLongPress();
};