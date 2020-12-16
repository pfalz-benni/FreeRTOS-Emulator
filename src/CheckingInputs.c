#include "CheckingInputs.h"

void xGetButtonInput(void)
{
	if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
		xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
		xSemaphoreGive(buttons.lock);
	}
}

void processStateChangeInput()
{
	if (xSemaphoreTake(changeState.lock, portMAX_DELAY) == pdTRUE) {
		changeState.value = 1;
		xSemaphoreGive(changeState.lock);
	}
}

int validateButtonPress(unsigned char keycode, TickType_t *lastPressTime,
			TickType_t *debounceDelay)
{
	if (buttons.buttons[keycode]) {
		// if enough time since last counted button press has passed,
		// the current button press is counted as vaild
		if (xTaskGetTickCount() - *lastPressTime > *debounceDelay) {
			*lastPressTime = xTaskGetTickCount();
			return 1;
		} else {
			return 0;
		}
	}
	// if button of interest is released, lastPressTime is reset.
	// This ensures that the next button press is counted, no matter
	// how short the amout of time is that has passed since the
	// preceding button press
	else {
		*lastPressTime = 0;
		return 0;
	}
}

void processButtonPressABCD(unsigned char keycode)
{
	if (xSemaphoreTake(buttonPressCountABCD.lock, portMAX_DELAY) ==
	    pdTRUE) {
		buttonPressCountABCD
			.values[keycode - OFFSET_KEYCODE_BUTTONPRESSCOUNT]++;
		xSemaphoreGive(buttonPressCountABCD.lock);
	}
}

//start "task notification task" by sending task notification
void processButtonPressN()
{
	xTaskNotifyGive(ButtonPressNotificationTask);
}

//start "binary semaphore task" by triggering binary semaphore
void processButtonPressS()
{
	xSemaphoreGive(ButtonSPressed);
}

void controlCountingSecondsTask()
{
	static int isSuspended = 0;
	if (isSuspended) {
		vTaskResume(CountingSecondsTask);
	} else {
		vTaskSuspend(CountingSecondsTask);
	}
	isSuspended = !isSuspended;
}

void resetButtonPressCountIfEntered()
{
	if (tumEventGetMouseLeft() || tumEventGetMouseRight()) {
		if (xSemaphoreTake(buttonPressCountABCD.lock, portMAX_DELAY) ==
		    pdTRUE) {
			for (int i = 0; i < NUMBER_OF_TRACKED_KEYS; i++) {
				buttonPressCountABCD.values[i] = 0;
			}
			xSemaphoreGive(buttonPressCountABCD.lock);
		}
	}
}

void vCheckingInputsTask(void *pvParameters)
{
	static TickType_t lastPressTimeA, lastPressTimeB, lastPressTimeC,
		lastPressTimeD, lastPressTimeE, lastPressTimeN, lastPressTimeS,
		lastPressTimeP = 0;
	static TickType_t debounceDelay =
		portTICK_PERIOD_MS * TIME_DEBOUNCE_BUTTON_MS;

	while (1) {
		xGetButtonInput(); // Update global input

		// `buttons` is a global shared variable and as such needs to be
		// guarded with a mutex, mutex must be obtained before accessing the
		// resource and given back when you're finished. If the mutex is not
		// given back then no other task can access the reseource.
		if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
			if (buttons.buttons[KEYCODE(
				    Q)]) { // Equiv to SDL_SCANCODE_Q
				exit(EXIT_SUCCESS);
			}

			if (validateButtonPress(KEYCODE(A), &lastPressTimeA,
						&debounceDelay))
				processButtonPressABCD(KEYCODE(A));
			if (validateButtonPress(KEYCODE(B), &lastPressTimeB,
						&debounceDelay))
				processButtonPressABCD(KEYCODE(B));
			if (validateButtonPress(KEYCODE(C), &lastPressTimeC,
						&debounceDelay))
				processButtonPressABCD(KEYCODE(C));
			if (validateButtonPress(KEYCODE(D), &lastPressTimeD,
						&debounceDelay))
				processButtonPressABCD(KEYCODE(D));

			//Check for and process state change input
			if (validateButtonPress(KEYCODE(E), &lastPressTimeE,
						&debounceDelay))
				processStateChangeInput();

			//Check for and process input to start a certain task (3.2.3)
			if (validateButtonPress(KEYCODE(N), &lastPressTimeN,
						&debounceDelay))
				processButtonPressN();
			if (validateButtonPress(KEYCODE(S), &lastPressTimeS,
						&debounceDelay))
				processButtonPressS();

			//Checking wheather CountingSecondsTask has to be paused or not
			if (validateButtonPress(KEYCODE(P), &lastPressTimeP,
						&debounceDelay))
				controlCountingSecondsTask();

			resetButtonPressCountIfEntered();

			xSemaphoreGive(buttons.lock);
		}

		vTaskDelay(portTICK_PERIOD_MS *
			   TIME_CHECKINGINPUTSTASK_DELAY_MS);
	}
}