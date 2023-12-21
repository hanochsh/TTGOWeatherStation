#include "EspButton.h"




///////////////////////////////////////////////////////////////
/// </summary> 
/*
int EspButton::chkButton()
{
    const  unsigned long SINGLCLKTIMEOUT = 400;
    //const  unsigned long SINGLCLKTIMEOUT  = 1000;
    unsigned long msec = millis();

    // was something before and the time left since was larger then the single click timeout
    // might be long press
    if (mLastTStamp && ((msec - mLastTStamp) > SINGLCLKTIMEOUT)) {
        mLastTStamp = 0;
        return SingleClick;
    }

    byte but = digitalRead(mGPIO);
    // New button state is diffrent from the last one
    if (mLastState != but) {
        mLastState = but;
        delay(10);           // **** debounce

        if (LOW == but) {   // press
            if (mLastTStamp) { // 2nd press
                mLastTStamp = 0;
                return DoubleClick;
            }
            else
                mLastTStamp = 0 == msec ? 1 : msec;
        }
    }

    return None;
} */

///////////////////////////////////////////////////////////////
/// </summary>
void EspButton::setup()
{
    pinMode(mGPIO, INPUT_PULLUP);
    mBtn->attachClick(onSingleClickCB, (void *)this);
    mBtn->attachDoubleClick(onDoubleClickCB, (void*)this);


    mBtn->attachLongPressStart(onStartLongPresCB, (void*)this);
    mBtn->attachLongPressStop(onEndLongPressCB, (void*)this);

}

///////////////////////////////////////////////////////////////
/// </summary>
void EspButton::onSingleClickCB(void *pVThis)
{
    EspButton* pThis = (EspButton*)pVThis;
    pThis->onSingleClick();
}

///////////////////////////////////////////////////////////////
/// </summary>
void EspButton::onDoubleClickCB(void *pVThis)
{
    EspButton* pThis = (EspButton*)pVThis;
    pThis->onDoubleClick();
}

///////////////////////////////////////////////////////////////
/// </summary>
void EspButton::onStartLongPresCB(void *pVThis)
{
    EspButton* pThis = (EspButton*)pVThis;
    pThis->onStartLongPress();
}

///////////////////////////////////////////////////////////////
/// </summary>
void EspButton::onEndLongPressCB(void *pVThis)
{
    EspButton* pThis = (EspButton*)pVThis;
    pThis->onEndLongPress();
}

///////////////////////////////////////////////////////////////
/*
void EspButton::tick()
{
    switch (chkButton()) {
    case SingleClick:
        Serial.println("Btn single");
        onSingleClick();
        break;

    case DoubleClick:
        Serial.println("Btn double");
        onDoubleClick();
        break;

    case startLongPress:
        Serial.println("Btn startLongPress");
        onStartLongPress();
        break;

    case endLongPress:
        Serial.println("Btn endLongPress");
        onEndLongPress();
        break;

    }
} */