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
#include "TUM_FreeRTOS_Utils.h"

#include "AsyncIO.h"

#include "Shapes.h"

// FreeRTOS specific
#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

// Task delays
#define TIME_RUNNINGDISPLAYTASK_DELAY_MS 20
#define TIME_CHECKINGINPUTSTASK_DELAY_MS 40

// Button Press Processing
#define OFFSET_KEYCODE_BUTTONPRESSCOUNT 4
#define TIME_DEBOUNCE_BUTTON_MS 300

// Constants for drawn objects
#define DISTANCE_TO_TRIANGLE 150
#define RADIUS_CIRCLE 50
#define HEIGHT_SQUARE 80
#define HEIGHT_TRIANGLE 80
#define TIME_ORBIT_PERIOD 5000
#define DISTANCE_VERTICAL_TO_BORDER 35
#define DISTANCE_HORIZONTAL_TO_BORDER 250
#define SPEED_MOVING_MESSAGE 100
#define COORD_BUTTON_PRESS_MESSAGE (coord_t) {30, 50}

static TaskHandle_t RunningDisplayTask = NULL;
static TaskHandle_t CheckingInputsTask = NULL;
static SemaphoreHandle_t ScreenLock = NULL;

/**
 * @brief Contains information, whether a button on the keyboard is pressed or not.
 * 
 * This is a shared resource. Therefore access is protected using a mutex.
 */
typedef struct buttons_buffer {
	unsigned char buttons[SDL_NUM_SCANCODES];
	SemaphoreHandle_t lock;
} buttons_buffer_t;
static buttons_buffer_t buttons = { 0 };

/**
 * @brief Contains information, how often a button on the keyboard has been pressed
 * last reset.
 * 
 * This is a shared resource. Therefore access is protected using a mutex.
 * In this case only the buttons A, B, C and D are tracked
 */
typedef struct buttonPresses {
	unsigned int valuesABCD[4];
	SemaphoreHandle_t lock;
} buttonPressCount_t;
static buttonPressCount_t buttonPressCount = { 0 };

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
		DISTANCE_TO_TRIANGLE, M_PI, TIME_ORBIT_PERIOD, xLastWakeTime - initialWakeTime);
	PositionProperties__updatePosition(Circle__getPositionProperties(circle),
										xLastWakeTime - prevWakeTime);


	PositionProperties__setSpeedMoveOnCircle(Rectangle__getPositionProperties(rectangle),
		DISTANCE_TO_TRIANGLE, 0, TIME_ORBIT_PERIOD, xLastWakeTime - initialWakeTime);
	PositionProperties__updatePosition(Rectangle__getPositionProperties(rectangle),
										xLastWakeTime - prevWakeTime);
	Rectangle__updateCorner(rectangle);


	PositionProperties__moveVetically(Message__getPositionProperties(topMessage), SPEED_MOVING_MESSAGE,
										DISTANCE_HORIZONTAL_TO_BORDER);
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

void drawMessages(Message_h_t topMessage, Message_h_t bottomMessage) {
	tumDrawText(Message__getText(bottomMessage), Message__getTopLeftCorner(bottomMessage).x,
				Message__getTopLeftCorner(bottomMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(bottomMessage)));

	tumDrawText(Message__getText(topMessage), Message__getTopLeftCorner(topMessage).x,
				Message__getTopLeftCorner(topMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(topMessage)));
}

void drawButtonPressMessage(Message_h_t buttonPressMessage,char *buttonPressText, unsigned int *buttonPressCount) {
	sprintf(buttonPressText, "A: %u |  B: %u |  C: %u |  D: %u", buttonPressCount[0],
			buttonPressCount[1],buttonPressCount[2],buttonPressCount[3]);
	Message__setText(buttonPressMessage, buttonPressText);

	tumDrawText(Message__getText(buttonPressMessage), Message__getTopLeftCorner(buttonPressMessage).x,
				Message__getTopLeftCorner(buttonPressMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(buttonPressMessage)));
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

	//Create messages
	static char bottomText[50];
	sprintf(bottomText, "Hang loose or press [Q] to quit!"); // 32 characters
	Message_h_t bottomMessage = Message__init((coord_t) {SCREEN_CENTER.x, SCREEN_HEIGHT - DISTANCE_VERTICAL_TO_BORDER},
				  							  bottomText, Black);

	static char topText[50];
	sprintf(topText, "Hello, I can move!");
	Message_h_t topMessage = Message__init((coord_t) {SCREEN_CENTER.x, DISTANCE_VERTICAL_TO_BORDER},
				  							topText, Black);

	//Create message for buttons
	static char buttonPressText[50];
	sprintf(buttonPressText, "A: %u |  B: %u |  C: %u |  D: %u", 0, 0, 0, 0);
	Message_h_t buttonPressMessage = Message__initTopLeftCorner(COORD_BUTTON_PRESS_MESSAGE, buttonPressText, Black);


	// Needed such that Gfx library knows which thread controlls drawing
	// Only one thread can call tumDrawUpdateScreen while and thread can call
	// the drawing functions to draw objects. This is a limitation of the SDL
	// backend.
	tumDrawBindThread();

	while (1) {
		tumEventFetchEvents(FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses

        xLastWakeTime = xTaskGetTickCount();

		//Screen manipulatoins
		xSemaphoreTake(ScreenLock, portMAX_DELAY);

		tumDrawClear(White); // Clear screen

		//Draw all objects in appropriate position
		updateShapeAndMessagePositions(circle, rectangle, topMessage, xLastWakeTime,
									   prevWakeTime, initialWakeTime);
		drawShapes(circle, rectangle, triangle);
		drawMessages(topMessage, bottomMessage);
		if (xSemaphoreTake(buttonPressCount.lock, 0) == pdTRUE) {
			drawButtonPressMessage(buttonPressMessage, buttonPressText, buttonPressCount.valuesABCD);
			xSemaphoreGive(buttonPressCount.lock);
		}

		tumDrawUpdateScreen(); // Refresh the screen
							   // Everything written on the screen before landet in some kind of back buffer

		xSemaphoreGive(ScreenLock);

		// Basic sleep to free CPU
		vTaskDelay(portTICK_PERIOD_MS * TIME_RUNNINGDISPLAYTASK_DELAY_MS);

        prevWakeTime = xLastWakeTime; // to keep track of time intervalls
	}
}

void checkAndProcessButtonPress(unsigned char keycode, TickType_t *lastPressTime, TickType_t *debounceDelay) {
	if (buttons.buttons[keycode]) {
		// if enough time since last counted button press has passed,
		// the current button press is counted
		if (xTaskGetTickCount() - *lastPressTime > *debounceDelay) {
			if (xSemaphoreTake(buttonPressCount.lock, portMAX_DELAY) == pdTRUE) {
				buttonPressCount.valuesABCD[keycode - OFFSET_KEYCODE_BUTTONPRESSCOUNT]++;
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

void printABCDButtonStates() {
	printf("%d ", buttons.buttons[KEYCODE(A)]);
	printf("%d ", buttons.buttons[KEYCODE(B)]);
	printf("%d ", buttons.buttons[KEYCODE(C)]);
	printf("%d   ", buttons.buttons[KEYCODE(D)]);

	static int i = 0;
	printf("%d\n", i++);
}

void vCheckingInputsTask(void *pvParameters) {
	static TickType_t lastPressTimeA, lastPressTimeB, lastPressTimeC, lastPressTimeD = 0;
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

			xSemaphoreGive(buttons.lock);
		}

		vTaskDelay(portTICK_PERIOD_MS * TIME_CHECKINGINPUTSTASK_DELAY_MS);
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

	buttonPressCount.lock = xSemaphoreCreateMutex();
	if (!buttonPressCount.lock) {
		PRINT_ERROR("Failed to create buttonPressCount lock");
		goto err_buttonpresscount_lock;
	}

	if (xTaskCreate(vRunningDisplayTask, "RunningDisplayTask",
			mainGENERIC_STACK_SIZE * 2, NULL, mainGENERIC_PRIORITY,
			&RunningDisplayTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'RunningDisplayTask'");
		goto err_runningdisplaytask;
	}

	if (xTaskCreate(vCheckingInputsTask, "CheckingInputsTask",
			mainGENERIC_STACK_SIZE * 2, NULL, mainGENERIC_PRIORITY,
			&CheckingInputsTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'CheckingInputsTask'");
        //could use something like on master TASK_PRINT_ERROR
		goto err_checkinginputstask;
	}

	printf("Numer of tasks: %lu\n\n", uxTaskGetNumberOfTasks());
	tumFUtilPrintTaskStateList();
	printf("\n");

	vTaskStartScheduler();

	return EXIT_SUCCESS;

err_checkinginputstask:
    vTaskDelete(vRunningDisplayTask);
err_runningdisplaytask:
	vSemaphoreDelete(buttonPressCount.lock);
err_buttonpresscount_lock:
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
