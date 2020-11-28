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

#define DISTANCE_TO_TRIANGLE 150
#define RADIUS_CIRCLE 50
#define HEIGHT_SQUARE 80
#define HEIGHT_TRIANGLE 80
#define TIME_PERIOD 5000

void vRunningDisplayTask(void *pvParameters)
{
	// Task initializations:
    TickType_t xLastWakeTime, prevWakeTime, initialWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    prevWakeTime = xLastWakeTime;
    initialWakeTime = xLastWakeTime;

    // Create three shapes
    Circle_t circle;
    Circle__init(&circle, (coord_t) {SCREEN_CENTER.x - DISTANCE_TO_TRIANGLE, SCREEN_CENTER.y},
                    RADIUS_CIRCLE, TUMBlue);

    Rectangle_t rectangle;
    Rectangle__init(&rectangle, (coord_t) {SCREEN_CENTER.x + DISTANCE_TO_TRIANGLE, SCREEN_CENTER.y}, 
                    HEIGHT_SQUARE, HEIGHT_SQUARE, Olive);

    Triangle_t triangle;
    Triangle__init(&triangle, SCREEN_CENTER, HEIGHT_TRIANGLE, Magenta);

	// Needed such that Gfx library knows which thread controlls drawing
	// Only one thread can call tumDrawUpdateScreen while and thread can call
	// the drawing functions to draw objects. This is a limitation of the SDL
	// backend.
	tumDrawBindThread();

	while (1) {
		tumEventFetchEvents(
			FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses
		xGetButtonInput(); // Update global input

        xLastWakeTime = xTaskGetTickCount();

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

        //Update shape positions
        PositionProperties__setSpeedMoveOnCircle(&(circle._positionProperties),
			DISTANCE_TO_TRIANGLE, M_PI, TIME_PERIOD, xLastWakeTime - initialWakeTime);
        PositionProperties__updatePosition(&(circle._positionProperties),
                                           xLastWakeTime - prevWakeTime);

        PositionProperties__setSpeedMoveOnCircle(&(rectangle._positionProperties),
			DISTANCE_TO_TRIANGLE, 0, TIME_PERIOD, xLastWakeTime - initialWakeTime);
        PositionProperties__updatePosition(&(rectangle._positionProperties),
                                           xLastWakeTime - prevWakeTime);
		Rectangle__updateCorner(&rectangle);


        // Draw updated shapes
		tumDrawCircle(circle._positionProperties._x, circle._positionProperties._y,
                      circle._radius, circle._positionProperties._color);

        tumDrawFilledBox(rectangle._topLeftCorner.x, rectangle._topLeftCorner.y,
                   rectangle._width, rectangle._height, rectangle._positionProperties._color);

		tumDrawTriangle(triangle._corners, Magenta);


		tumDrawUpdateScreen(); // Refresh the screen to draw string

		// Basic sleep of 1000 milliseconds
		vTaskDelay(portTICK_PERIOD_MS * 10);
        prevWakeTime = xLastWakeTime;
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

	if (xTaskCreate(vRunningDisplayTask, "RunningDisplayTask",
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
