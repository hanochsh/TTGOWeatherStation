#include <Arduino.h>
//#include <task.h>
#define configUSE_NEWLIB_REENTRANT 0
//#include <Arduino_FreeRTOS.h>
//#include <freertos/FreeRTOS.h>
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
	const TickType_t xFrequency = pThis->mFreq;

	// Initialise the xLastWakeTime variable with the current time.
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {

		pThis->renderTask(pThis->mRenderOpt, pThis->mData);
		// Wait for the next cycle.
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}
}