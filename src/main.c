#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"
#include "TUM_Font.h"

#include "AsyncIO.h"

#include "Shapes.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

static TaskHandle_t RunningDisplayTask = NULL;

typedef struct buttons_buffer {
	unsigned char buttons[SDL_NUM_SCANCODES];
	SemaphoreHandle_t lock;
} buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

void xGetButtonInput(void)
{
	if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
		xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
		xSemaphoreGive(buttons.lock);
	}
}

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR


void vRunningDisplayTask(void *pvParameters)
{
	// Task initializations:

	// Needed such that Gfx library knows which thread controlls drawing
	// Only one thread can call tumDrawUpdateScreen while and thread can call
	// the drawing functions to draw objects. This is a limitation of the SDL
	// backend.
	tumDrawBindThread();

	while (1) {
		tumEventFetchEvents(
			FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses
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
			xSemaphoreGive(buttons.lock);
		}

		tumDrawClear(White); // Clear screen


        // Create shape objects and draw them afterwards
        Circle_t circle;
        Circle__init(&circle, (coord_t) {SCREEN_CENTER.x - 150, SCREEN_CENTER.y},
                     50, TUMBlue);
		tumDrawCircle(circle._positionProperties._x, circle._positionProperties._y,
                      circle._radius, circle._positionProperties._color);

        Rectangle_t rectangle;
        Rectangle__init(&rectangle, (coord_t) {SCREEN_CENTER.x + 150, SCREEN_CENTER.y}, 
                        80, 80, Olive);
        tumDrawFilledBox(rectangle._topLeftCorner.x, rectangle._topLeftCorner.y,
                   rectangle._width, rectangle._height, rectangle._positionProperties._color);

		Triangle_t triangle;
		Triangle__init(&triangle, SCREEN_CENTER, 80, Magenta);
		tumDrawTriangle(triangle._corners, Magenta);


		tumDrawUpdateScreen(); // Refresh the screen to draw string

		// Basic sleep of 1000 milliseconds
		vTaskDelay((TickType_t)1000);
	}
}

int main(int argc, char *argv[])
{
	char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);

	printf("Initializing: ");

	if (tumDrawInit(bin_folder_path)) {
		PRINT_ERROR("Failed to initialize drawing");
		goto err_init_drawing;
	}

	if (tumEventInit()) {
		PRINT_ERROR("Failed to initialize events");
		goto err_init_events;
	}

	if (tumSoundInit(bin_folder_path)) {
		PRINT_ERROR("Failed to initialize audio");
		goto err_init_audio;
	}

	buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
	if (!buttons.lock) {
		PRINT_ERROR("Failed to create buttons lock");
		goto err_buttons_lock;
	}

	if (xTaskCreate(vRunningDisplayTask, "DemoTask",
			mainGENERIC_STACK_SIZE * 2, NULL, mainGENERIC_PRIORITY,
			&RunningDisplayTask) != pdPASS) {
		goto err_demotask;
	}

	vTaskStartScheduler();

	return EXIT_SUCCESS;

err_demotask:
	vSemaphoreDelete(buttons.lock);
err_buttons_lock:
	tumSoundExit();
err_init_audio:
	tumEventExit();
err_init_events:
	tumDrawExit();
err_init_drawing:
	return EXIT_FAILURE;
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
	/* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
	struct timespec xTimeToSleep, xTimeSlept;
	/* Makes the process more agreeable when using the Posix simulator. */
	xTimeToSleep.tv_sec = 1;
	xTimeToSleep.tv_nsec = 0;
	nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
