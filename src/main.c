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

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define DISTANCE_TO_TRIANGLE 150
#define RADIUS_CIRCLE 50
#define HEIGHT_SQUARE 80
#define HEIGHT_TRIANGLE 80
#define TIME_PERIOD 5000
#define DISTANCE_TO_BORDER 35
#define MOVING_MESSAGE_SPEED 100

static TaskHandle_t RunningDisplayTask = NULL;
static SemaphoreHandle_t ScreenLock = NULL;

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

void updateShapeAndMessagePositions(Circle_h_t circle, Rectangle_h_t rectangle, Message_h_t topMessage,
									TickType_t xLastWakeTime, TickType_t prevWakeTime, TickType_t initialWakeTime) {
	PositionProperties__setSpeedMoveOnCircle(Circle__getPositionProperties(circle),
		DISTANCE_TO_TRIANGLE, M_PI, TIME_PERIOD, xLastWakeTime - initialWakeTime);
	PositionProperties__updatePosition(Circle__getPositionProperties(circle),
										xLastWakeTime - prevWakeTime);


	PositionProperties__setSpeedMoveOnCircle(Rectangle__getPositionProperties(rectangle),
		DISTANCE_TO_TRIANGLE, 0, TIME_PERIOD, xLastWakeTime - initialWakeTime);
	PositionProperties__updatePosition(Rectangle__getPositionProperties(rectangle),
										xLastWakeTime - prevWakeTime);
	Rectangle__updateCorner(rectangle);


	PositionProperties__moveVetically(Message__getPositionProperties(topMessage), MOVING_MESSAGE_SPEED,
										DISTANCE_TO_BORDER);
	PositionProperties__updatePosition(Message__getPositionProperties(topMessage),
										xLastWakeTime - prevWakeTime);
	Message__updateCorner(topMessage);
}

void drawShapes(Circle_h_t circle, Rectangle_h_t rectangle, Triangle_h_t triangle) {
	tumDrawCircle(PositionProperties__getX(Circle__getPositionProperties(circle)),
					PositionProperties__getY(Circle__getPositionProperties(circle)),
					Circle__getRadius(circle),
					PositionProperties__getColor(Circle__getPositionProperties(circle)));

	tumDrawFilledBox(Rectangle__getTopLeftCorner(rectangle).x,
						Rectangle__getTopLeftCorner(rectangle).y,
						Rectangle__getWidth(rectangle),
						Rectangle__getHeight(rectangle),
						PositionProperties__getColor(Rectangle__getPositionProperties(rectangle)));

	tumDrawTriangle(Triangle__getCorners(triangle),
					PositionProperties__getColor(Triangle__getPositionProperties(triangle)));
}

void drawTexts(Message_h_t topMessage, Message_h_t bottomMessage) {
	tumDrawText(Message__getText(bottomMessage), Message__getTopLeftCorner(bottomMessage).x,
				Message__getTopLeftCorner(bottomMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(bottomMessage)));

	tumDrawText(Message__getText(topMessage), Message__getTopLeftCorner(topMessage).x,
				Message__getTopLeftCorner(topMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(topMessage)));
}

void vRunningDisplayTask(void *pvParameters)
{
	// Task initializations:
    TickType_t xLastWakeTime, prevWakeTime, initialWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    prevWakeTime = xLastWakeTime;
    initialWakeTime = xLastWakeTime;

    // Create three shapes
    Circle_h_t circle = Circle__init((coord_t) {SCREEN_CENTER.x - DISTANCE_TO_TRIANGLE, SCREEN_CENTER.y},
                    	RADIUS_CIRCLE, TUMBlue);

    Rectangle_h_t rectangle = Rectangle__init((coord_t) {SCREEN_CENTER.x + DISTANCE_TO_TRIANGLE, SCREEN_CENTER.y}, 
                    		  HEIGHT_SQUARE, HEIGHT_SQUARE, Olive);

    Triangle_h_t triangle = Triangle__init(SCREEN_CENTER, HEIGHT_TRIANGLE, Magenta);

	//Create texts
	static char bottomText[50];
	sprintf(bottomText, "Hang loose or press [Q] to quit!"); // 32 characters
	Message_h_t bottomMessage = Message__init((coord_t) {SCREEN_CENTER.x, SCREEN_HEIGHT - DISTANCE_TO_BORDER},
				  							  bottomText, Black);

	static char topText[50];
	sprintf(topText, "Hello, I can move!");
	Message_h_t topMessage = Message__init((coord_t) {SCREEN_CENTER.x, DISTANCE_TO_BORDER},
				  							topText, Black);


	// Needed such that Gfx library knows which thread controlls drawing
	// Only one thread can call tumDrawUpdateScreen while and thread can call
	// the drawing functions to draw objects. This is a limitation of the SDL
	// backend.
	tumDrawBindThread();

	while (1) {
		tumEventFetchEvents(FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses
		xGetButtonInput(); // Update global input

        xLastWakeTime = xTaskGetTickCount();

		// `buttons` is a global shared variable and as such needs to be
		// guarded with a mutex, mutex must be obtained before accessing the
		// resource and given back when you're finished. If the mutex is not
		// given back then no other task can access the reseource.
		if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
			if (buttons.buttons[KEYCODE(Q)]) { // Equiv to SDL_SCANCODE_Q
				exit(EXIT_SUCCESS);
			}
			xSemaphoreGive(buttons.lock);
		}

		//Screen manipulatoins
		xSemaphoreTake(ScreenLock, portMAX_DELAY);

		tumDrawClear(White); // Clear screen

		//Draw all objects in appropriate position
		updateShapeAndMessagePositions(circle, rectangle, topMessage, xLastWakeTime,
									   prevWakeTime, initialWakeTime);
		drawShapes(circle, rectangle, triangle);
		drawTexts(topMessage, bottomMessage);

		tumDrawUpdateScreen(); // Refresh the screen
							   // Everything written on the screen before landet in some kind of back buffer

		xSemaphoreGive(ScreenLock);

		// Basic sleep to free CPU
		vTaskDelay(portTICK_PERIOD_MS * 20);

        prevWakeTime = xLastWakeTime; // to keep track of time intervalls
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

	ScreenLock = xSemaphoreCreateMutex();
	if (!ScreenLock) {
		PRINT_ERROR("Failed to create screen lock");
		goto err_screen_lock;
	}

	if (xTaskCreate(vRunningDisplayTask, "RunningDisplayTask",
			mainGENERIC_STACK_SIZE * 2, NULL, mainGENERIC_PRIORITY,
			&RunningDisplayTask) != pdPASS) {
		goto err_runningdisplaytask;
	}

	vTaskStartScheduler();

	return EXIT_SUCCESS;

err_runningdisplaytask:
	vSemaphoreDelete(ScreenLock);
err_screen_lock: // Everything created before has to be deleted. The item calling the error
	vSemaphoreDelete(buttons.lock); // routin doesn't because it was not actually created.
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
