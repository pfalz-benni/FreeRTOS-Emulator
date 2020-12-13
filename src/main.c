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
#include "MovingShapesDisplay.h"
#include "CheckingInputs.h"
#include "SharedResources.h"
#include "StateMachine.h"
#include "SwapBuffer.h"

// FreeRTOS specific
#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

// Task handles
TaskHandle_t MovingShapesDisplayTask = NULL;
TaskHandle_t CheckingInputsTask = NULL;
TaskHandle_t StateMachineTask = NULL;
TaskHandle_t SwapBufferTask = NULL;

// Variables that are declares as extern because they guard resources
// and must therefore be used in different tasks in different
// source files with differnet compilation units.
SemaphoreHandle_t ScreenLock = NULL;
buttons_buffer_t buttons = { 0 };
buttonPresses_t buttonPressCount = { 0 };
/**
 * Infomation wheather the state machien has changed state or not
 */
genericBinaryState_t changeState = { 0 };
/**
 * Infomation wheather the MovingShapesDisplayTask() has been resumed
 * (after it has been suspended) or not
 */
genericBinaryState_t movingShapesDisplayTaskResumed = { 0 };


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

    changeState.lock = xSemaphoreCreateMutex();
	if (!changeState.lock) {
		PRINT_ERROR("Failed to create changeState lock");
		goto err_changestate_lock;
	}

	movingShapesDisplayTaskResumed.lock = xSemaphoreCreateMutex();
	if (!movingShapesDisplayTaskResumed.lock) {
		PRINT_ERROR("Failed to create movingShapesDisplayTaskResumed lock");
		goto err_movingShapesDisplayTaskResumed_lock;
	}

    if (xTaskCreate(vSwapBufferTask, "SwapBufferTask",
			mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES,
			&StateMachineTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'SwapBufferTask'");
		goto err_swapbuffer_task;
	}

    if (xTaskCreate(vStateMachineTask, "StateMachineTask",
			mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES - 1,
			&StateMachineTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'StateMachineTask'");
		goto err_statemachine_task;
	}

	if (xTaskCreate(vMovingShapesDisplayTask, "MovingShapesDisplayTask",
			mainGENERIC_STACK_SIZE * 2, NULL, mainGENERIC_PRIORITY,
			&MovingShapesDisplayTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'MovingShapesDisplayTask'");
		goto err_movingshapesdisplay_task;
	}

	if (xTaskCreate(vCheckingInputsTask, "CheckingInputsTask",
			mainGENERIC_STACK_SIZE * 2, NULL, mainGENERIC_PRIORITY,
			&CheckingInputsTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'CheckingInputsTask'");
        //could use something like on master TASK_PRINT_ERROR
		goto err_checkinginputs_task;
	}

    //Suspending diffent tasks because they are managed 
    //from inside the state machine task
    vTaskSuspend(MovingShapesDisplayTask);


	printf("Numer of tasks: %lu\n\n", uxTaskGetNumberOfTasks());
	tumFUtilPrintTaskStateList();
	printf("\n");

	vTaskStartScheduler();

	return EXIT_SUCCESS;

err_checkinginputs_task:
    vTaskDelete(vMovingShapesDisplayTask);
err_movingshapesdisplay_task:
    vTaskDelete(vStateMachineTask);
err_statemachine_task:
    vTaskDelete(vSwapBufferTask);
err_swapbuffer_task:
	vSemaphoreDelete(movingShapesDisplayTaskResumed.lock);
err_movingShapesDisplayTaskResumed_lock:
    vSemaphoreDelete(changeState.lock);
err_changestate_lock:
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
