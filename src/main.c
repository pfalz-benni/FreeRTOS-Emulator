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
#include "BlinkingButtons.h"

// FreeRTOS specific
#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define STACK_SIZE_STATIC 200 


// Task handles
TaskHandle_t MovingShapesDisplayTask = NULL;
TaskHandle_t CheckingInputsTask = NULL;
TaskHandle_t StateMachineTask = NULL;
TaskHandle_t SwapBufferTask = NULL;
TaskHandle_t BlinkingButtonsDrawTask = NULL; //exercise 3.2.2
TaskHandle_t BlinkingButtonsDynamicTask = NULL; //exercise 3.2.2
TaskHandle_t BlinkingButtonsStaticTask = NULL; //exercise 3.2.2


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


//Staic task allocation
/**
 * Structure that holds the task's data structures (TCB) of the
 * statically created task vBlinkingButtonsStaticTask()
 */
StaticTask_t xTaskBuffer;
/**
 * Buffer that vBlinkingButtonsStaticTask() will use as its stack
 */
StackType_t xStack[ STACK_SIZE_STATIC ];

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static – otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task’s
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task’s stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
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

	if (xTaskCreate(vBlinkingButtonsDrawTask, "BlinkingButtonsDrawTask",
			mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
			&BlinkingButtonsDrawTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'BlinkingButtonsDrawTask'");
		goto err_blinkingbuttonsdrawtask_task;
	}

	if (xTaskCreate(vBlinkingButtonsDynamicTask, "BlinkingButtonsDynamicTask",
			mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
			&BlinkingButtonsDynamicTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'BlinkingButtonsDynamicTask'");
		goto err_blinkingbuttonsdynamic_task;
	}

	BlinkingButtonsStaticTask = xTaskCreateStatic(vBlinkingButtonsStaticTask,
			"BlinkingButtonsStaticTask", STACK_SIZE_STATIC, NULL,
			mainGENERIC_PRIORITY, xStack , &xTaskBuffer);
	if (!BlinkingButtonsStaticTask) {
		PRINT_ERROR("Failed to create 'BlinkingButtonsStaticTask'");
		goto err_blinkingbuttonsstatic_task;
	}

    //Suspending diffent tasks because they are managed 
    //from inside the state machine task
    vTaskSuspend(MovingShapesDisplayTask);
	vTaskSuspend(BlinkingButtonsDrawTask);
	vTaskSuspend(BlinkingButtonsDynamicTask);
	vTaskSuspend(BlinkingButtonsStaticTask);


	printf("Numer of tasks: %lu\n\n", uxTaskGetNumberOfTasks());
	tumFUtilPrintTaskStateList();
	printf("\n");

	vTaskStartScheduler();

	return EXIT_SUCCESS;


err_blinkingbuttonsstatic_task:
	vTaskDelete(BlinkingButtonsDynamicTask);
err_blinkingbuttonsdynamic_task:
	vTaskDelete(BlinkingButtonsDrawTask);
err_blinkingbuttonsdrawtask_task:
	vTaskDelete(CheckingInputsTask);
err_checkinginputs_task:
    vTaskDelete(MovingShapesDisplayTask);
err_movingshapesdisplay_task:
    vTaskDelete(StateMachineTask);
err_statemachine_task:
    vTaskDelete(SwapBufferTask);
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
