#include "CheckingInputs.h"

void xGetButtonInput(void) {
	if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
		xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
		xSemaphoreGive(buttons.lock);
	}
}

void processStateChangeInput() {
	if(xSemaphoreTake(changeState.lock, portMAX_DELAY) == pdTRUE) {
		changeState.value = 1;
		xSemaphoreGive(changeState.lock);
	}
}

void checkAndProcessButtonPress(unsigned char keycode, TickType_t *lastPressTime, TickType_t *debounceDelay) {
	if (buttons.buttons[keycode]) {
		// if enough time since last counted button press has passed,
		// the current button press is counted
		if (xTaskGetTickCount() - *lastPressTime > *debounceDelay) {
			//button press E is treaded differently than the rest
			if (keycode == KEYCODE(E)) {
				processStateChangeInput();
			} else if (xSemaphoreTake(buttonPressCount.lock, portMAX_DELAY) == pdTRUE) {
				buttonPressCount.valuesABCD [keycode - OFFSET_KEYCODE_BUTTONPRESSCOUNT]++;
				xSemaphoreGive(buttonPressCount.lock);
			}
			*lastPressTime = xTaskGetTickCount();
		}
	}
	// if button of interest is released, lastPressTime is reset. This ensures that the
	// next button press is counted, no matter how short the amout of time is that has
	// passed since the preceding button press 
	else {
		*lastPressTime = 0;
	}
}

void resetButtonPressCountIfEntered() {
	if (tumEventGetMouseLeft() || tumEventGetMouseRight()) {
		if (xSemaphoreTake(buttonPressCount.lock, portMAX_DELAY) ==
		    pdTRUE) {
			for (int i = 0; i < NUMBER_OF_TRACKED_KEYS; i++) {
				buttonPressCount.valuesABCD[i] = 0;
			}
			xSemaphoreGive(buttonPressCount.lock);
		}
	}
}

void vCheckingInputsTask(void *pvParameters) {
	static TickType_t lastPressTimeA, lastPressTimeB, lastPressTimeC, lastPressTimeD, lastPressTimeE = 0;
	static TickType_t debounceDelay = portTICK_PERIOD_MS * TIME_DEBOUNCE_BUTTON_MS;

	while(1) {
		xGetButtonInput(); // Update global input

		// `buttons` is a global shared variable and as such needs to be
		// guarded with a mutex, mutex must be obtained before accessing the
		// resource and given back when you're finished. If the mutex is not
		// given back then no other task can access the reseource.
		if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
			if (buttons.buttons[KEYCODE(Q)]) { // Equiv to SDL_SCANCODE_Q
				exit(EXIT_SUCCESS);
			}

			checkAndProcessButtonPress(KEYCODE(A), &lastPressTimeA, &debounceDelay);
			checkAndProcessButtonPress(KEYCODE(B), &lastPressTimeB, &debounceDelay);
			checkAndProcessButtonPress(KEYCODE(C), &lastPressTimeC, &debounceDelay);
			checkAndProcessButtonPress(KEYCODE(D), &lastPressTimeD, &debounceDelay);

			//Check for and process state change input
			checkAndProcessButtonPress(KEYCODE(E), &lastPressTimeE, &debounceDelay);

			resetButtonPressCountIfEntered();

			xSemaphoreGive(buttons.lock);
		}

		vTaskDelay(portTICK_PERIOD_MS * TIME_CHECKINGINPUTSTASK_DELAY_MS);
	}
}