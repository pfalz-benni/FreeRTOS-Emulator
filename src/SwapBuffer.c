#include "SwapBuffer.h"

void vSwapBufferTask(void *pvParameters) {
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

	// Needed such that Gfx library knows which thread controlls drawing
	// Only one thread can call tumDrawUpdateScreen while and thread can call
	// the drawing functions to draw objects. This is a limitation of the SDL
	// backend.
    tumDrawBindThread();

    while (1) {
        if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE) {
            tumDrawUpdateScreen(); // Refresh the screen
							       // Everything written on the screen before landed this kind of back buffer
            tumEventFetchEvents(FETCH_EVENT_BLOCK); // Query events backend for new events, ie. button presses
            xSemaphoreGive(ScreenLock);
            // xSemaphoreGive(DrawSignal);
            vTaskDelayUntil(&xLastWakeTime,
                            pdMS_TO_TICKS(TIME_FRAMERATE_PERIOD_MS));
        }
    }
}