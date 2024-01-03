#include <Arduino.h>

#include "xEspTaskRenderer.h"

//////////////////////////////////////////////
//
void xEspTaskRenderer::createCoreTask(TaskHandle_t * handle)
{
	mHandle = *handle;
	xTaskCreatePinnedToCore(
		xEspTaskRenderer::taskCoreCB,        /* Task function. */
		mName,      /* name of task. */
		mStackSize,                         /* Stack size of task */
		(void *)this,                         /* parameter of the task */
		mPriority,                            /* priority of the task */
		handle, /* Task handle to keep track of created task */
		mCore);                           /* pin task to core 1 */
}

//////////////////////////////////////////////
//
void xEspTaskRenderer::taskCoreCB(void* vThis)
{
	xEspTaskRenderer* pThis = (xEspTaskRenderer*)vThis;
	TickType_t xLastWakeTime;
	//Serial.println("EspTaskRenderer::taskCoreCB is set for:" + String(pThis->mName));
	const TickType_t xFrequency = pThis->mFreq;

	pThis->preTaskLoop(pThis->mData);
	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		//Serial.println("EspTaskRenderer::taskCoreCB loop: " + String(pThis->mName));
		if ( pThis->mEnabled ) // suspend from outside
		  pThis->renderTask(pThis->mRenderOpt, pThis->mData);
		// Wait for the next cycle.
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}