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
#include "TimerFunctionality.h"

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
TaskHandle_t ButtonPressSemaphoreTask = NULL; //exercise 3.2.3
TaskHandle_t ButtonPressNotificationTask = NULL; //exercise 3.2.3
TaskHandle_t ButtonPressResetTask = NULL; //exercise 3.2.3


// Variables that are declares as extern because they guard resources
// and must therefore be used in different tasks in different
// source files with differnet compilation units.
SemaphoreHandle_t ScreenLock = NULL;
SemaphoreHandle_t ButtonSPressed = NULL;
buttons_buffer_t buttons = { 0 };
buttonPresses_t buttonPressCountABCD = { 0 };
buttonPresses_t buttonPressCountNS = { 0 }; //not the entire value array is used
/**
 * Infomation wheather the state machien has changed state or not
 */
genericBinaryState_t changeState = { 0 };
/**
 * Infomation wheather the MovingShapesDisplayTask() has been resumed
 * (after it has been suspended) or not
 */
genericBinaryState_t movingShapesDisplayTaskResumed = { 0 };
TimerHandle_t deleteButtonCountNS = NULL;


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

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static – otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task’s state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task’s stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
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

// ---------------SEMAPHORES-----------------------------------------
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

	buttonPressCountABCD.lock = xSemaphoreCreateMutex();
	if (!buttonPressCountABCD.lock) {
		PRINT_ERROR("Failed to create buttonPressCountABCD lock");
		goto err_buttonpresscountabcd_lock;
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

	ButtonSPressed = xSemaphoreCreateBinary();
	if (!ButtonSPressed) {
		PRINT_ERROR("Failed to create ButtonSPressed semaphore");
		goto err_buttonspressed_semaphore;
	}

	buttonPressCountNS.lock = xSemaphoreCreateMutex();
	if (!buttonPressCountNS.lock) {
		PRINT_ERROR("Failed to create buttonPressCountNS lock");
		goto err_buttonpresscountns_lock;
	}

// ---------------TIMER----------------------------------------------

	deleteButtonCountNS = xTimerCreate("deleteButtonCountNS", pdMS_TO_TICKS(15000), pdTRUE,
			(void *) 0, deleteButtonCountNSCallback);
	if (!deleteButtonCountNS) {
		PRINT_ERROR("Failed to create deleteButtonCountNS timer");
		goto err_deletebuttoncountns_timer;
	}
	
// ---------------TASKS----------------------------------------------

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

	if (xTaskCreate(vButtonPressSemaphoreTask, "ButtonPressSemaphoreTask",
			mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
			&ButtonPressSemaphoreTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'ButtonPressSemaphoreTask'");
		goto err_buttonpresssemaphore_task;
	}

	if (xTaskCreate(vButtonPressNotificationTask, "ButtonPressNotificationTask",
			mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY + 2,
			&ButtonPressNotificationTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'ButtonPressNotificationTask'");
		goto err_buttonpressnotification_task;
	}

	if (xTaskCreate(vButtonPressResetTask, "ButtonPressResetTask",
			mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
			&ButtonPressResetTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'ButtonPressResetTask'");
		goto err_buttonpressreset_task;
	}

    //Suspending diffent tasks because they are managed 
    //from inside the state machine task
    vTaskSuspend(MovingShapesDisplayTask);
	vTaskSuspend(BlinkingButtonsDrawTask);
	vTaskSuspend(BlinkingButtonsDynamicTask);
	vTaskSuspend(BlinkingButtonsStaticTask);
	vTaskSuspend(ButtonPressSemaphoreTask);
	vTaskSuspend(ButtonPressNotificationTask);
	vTaskSuspend(ButtonPressResetTask);

    xTimerStart(deleteButtonCountNS, portMAX_DELAY);


	printf("Numer of tasks: %lu\n\n", uxTaskGetNumberOfTasks());
	tumFUtilPrintTaskStateList();
	printf("\n");

	vTaskStartScheduler();

	return EXIT_SUCCESS;


err_buttonpressreset_task:
	vTaskDelete(ButtonPressNotificationTask);
err_buttonpressnotification_task:
	vTaskDelete(ButtonPressSemaphoreTask);
err_buttonpresssemaphore_task:
	vTaskDelete(BlinkingButtonsStaticTask);
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
	xTimerDelete(deleteButtonCountNS, 0);
err_deletebuttoncountns_timer:
	vSemaphoreDelete(buttonPressCountNS.lock);
err_buttonpresscountns_lock:
	vSemaphoreDelete(ButtonSPressed);
err_buttonspressed_semaphore:
	vSemaphoreDelete(movingShapesDisplayTaskResumed.lock);
err_movingShapesDisplayTaskResumed_lock:
    vSemaphoreDelete(changeState.lock);
err_changestate_lock:
	vSemaphoreDelete(buttonPressCountABCD.lock);
err_buttonpresscountabcd_lock:
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
