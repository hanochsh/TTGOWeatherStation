#include <Arduino.h>
#include "xEspTaskRenderer.h"

///////////////////////////////////////////////////////////
//
void MainTimeRendererTask::renderTask(int opt, void* data) {

	renderTimeDisplay(opt, 0, 0);
}

