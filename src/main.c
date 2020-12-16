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
#include "BlinkingButtonsDisplay.h"
#include "TimerFunctionality.h"
#include "PrintingTasksDisplay.h"

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
TaskHandle_t CountingSecondsTask = NULL; //exercise 3.2.4
TaskHandle_t PrintTaskOutputsTask = NULL; //exercise 4
TaskHandle_t Exercise4_1Task = NULL;
TaskHandle_t Exercise4_2Task = NULL;
TaskHandle_t Exercise4_3Task = NULL;
TaskHandle_t Exercise4_4Task = NULL;

// Variables that are declares as extern because they guard resources
// and must therefore be used in different tasks in different
// source files with differnet compilation units.
SemaphoreHandle_t ScreenLock = NULL;
SemaphoreHandle_t ButtonSPressed = NULL;
SemaphoreHandle_t wakeTask4_3 = NULL;
buttons_buffer_t buttons = { 0 };
buttonPresses_t buttonPressCountABCD = { 0 };
buttonPresses_t buttonPressCountNS = { 0 }; //not the entire value array is used
/**
 * Infomation wheather the state machien has changed state or not
 */
sharedIntegerVariable_t changeState = { 0 };
/**
 * Infomation wheather the MovingShapesDisplayTask() has been resumed
 * (after it has been suspended) or not
 */
sharedIntegerVariable_t movingShapesDisplayTaskResumed = { 0 };
/**
 * Global variable being counted up every second unless counting
 * is paused.
 */
sharedIntegerVariable_t secondsPassedTotal = { 0 };
TimerHandle_t deleteButtonCountNS = NULL;
QueueHandle_t numbersToPrint = NULL;

//Staic task allocation
/**
 * Structure that holds the task's data structures (TCB) of the
 * statically created task vBlinkingButtonsStaticTask()
 */
StaticTask_t xTaskBuffer;
/**
 * Buffer that vBlinkingButtonsStaticTask() will use as its stack
 */
StackType_t xStack[STACK_SIZE_STATIC];

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
		PRINT_ERROR(
			"Failed to create movingShapesDisplayTaskResumed lock");
		goto err_movingShapesDisplayTaskResumed_lock;
	}

	ButtonSPressed = xSemaphoreCreateBinary();
	if (!ButtonSPressed) {
		PRINT_ERROR("Failed to create ButtonSPressed semaphore");
		goto err_buttonspressed_semaphore;
	}

	wakeTask4_3 = xSemaphoreCreateBinary();
	if (!wakeTask4_3) {
		PRINT_ERROR("Failed to create wakeTask4_3 semaphore");
		goto err_wakeTask4_3_semaphore;
	}

	buttonPressCountNS.lock = xSemaphoreCreateMutex();
	if (!buttonPressCountNS.lock) {
		PRINT_ERROR("Failed to create buttonPressCountNS lock");
		goto err_buttonpresscountns_lock;
	}

	secondsPassedTotal.lock = xSemaphoreCreateMutex();
	if (!secondsPassedTotal.lock) {
		PRINT_ERROR("Failed to create secondsPassedTotal lock");
		goto err_secondspassedtotal_lock;
	}

	// ---------------QUEUE----------------------------------------------
	numbersToPrint = xQueueCreate(NUMBER_QUEUE_LENGTH, sizeof(tuple_t));
	if (!numbersToPrint) {
		PRINT_ERROR("Failed to create secondsPassedTotal lock");
		goto err_numberstoprint_queue;
	}

	// ---------------TIMER----------------------------------------------
	deleteButtonCountNS =
		xTimerCreate("deleteButtonCountNS", pdMS_TO_TICKS(15000),
			     pdTRUE, (void *)0, deleteButtonCountNSCallback);
	if (!deleteButtonCountNS) {
		PRINT_ERROR("Failed to create deleteButtonCountNS timer");
		goto err_deletebuttoncountns_timer;
	}

	// ---------------TASKS----------------------------------------------
	if (xTaskCreate(vSwapBufferTask, "SwapBufferTask",
			mainGENERIC_STACK_SIZE * 2, NULL, 10,
			&StateMachineTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'SwapBufferTask'");
		goto err_swapbuffer_task;
	}

	if (xTaskCreate(vStateMachineTask, "StateMachineTask",
			mainGENERIC_STACK_SIZE * 2, NULL, 9,
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
			mainGENERIC_STACK_SIZE * 2, NULL, 6,
			&CheckingInputsTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'CheckingInputsTask'");
		//could use something like on master TASK_PRINT_ERROR
		goto err_checkinginputs_task;
	}

	if (xTaskCreate(vBlinkingButtonsDrawTask, "BlinkingButtonsDrawTask",
			mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
			&BlinkingButtonsDrawTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'BlinkingButtonsDrawTask'");
		goto err_blinkingbuttonsdraw_task;
	}

	if (xTaskCreate(vBlinkingButtonsDynamicTask,
			"BlinkingButtonsDynamicTask", mainGENERIC_STACK_SIZE,
			NULL, mainGENERIC_PRIORITY,
			&BlinkingButtonsDynamicTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'BlinkingButtonsDynamicTask'");
		goto err_blinkingbuttonsdynamic_task;
	}

	BlinkingButtonsStaticTask =
		xTaskCreateStatic(vBlinkingButtonsStaticTask,
				  "BlinkingButtonsStaticTask",
				  STACK_SIZE_STATIC, NULL, mainGENERIC_PRIORITY,
				  xStack, &xTaskBuffer);
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

	if (xTaskCreate(vButtonPressNotificationTask,
			"ButtonPressNotificationTask", mainGENERIC_STACK_SIZE,
			NULL, mainGENERIC_PRIORITY + 2,
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

	if (xTaskCreate(vCountingSecondsTask, "CountingSecondsTask",
			mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
			&CountingSecondsTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'CountingSecondsTask'");
		goto err_countingseconds_task;
	}

	if (xTaskCreate(vPrintTaskOutputsTask, "PrintTaskOutputsTask",
			mainGENERIC_STACK_SIZE, NULL, 5,
			&PrintTaskOutputsTask) != pdPASS) {
		PRINT_ERROR("Failed to create 'PrintTaskOutputsTask'");
		goto err_printtaskoutputs_task;
	}

	if (xTaskCreate(vExercise4_1Task, "Exercise4_1Task",
			mainGENERIC_STACK_SIZE, NULL, 1,
			&Exercise4_1Task) != pdPASS) {
		PRINT_ERROR("Failed to create 'Exercise4_1Task'");
		goto err_exercise4_1_task;
	}

	if (xTaskCreate(vExercise4_2Task, "Exercise4_2Task",
			mainGENERIC_STACK_SIZE, NULL, 2,
			&Exercise4_2Task) != pdPASS) {
		PRINT_ERROR("Failed to create 'Exercise4_2Task'");
		goto err_exercise4_2_task;
	}

	if (xTaskCreate(vExercise4_3Task, "Exercise4_3Task",
			mainGENERIC_STACK_SIZE, NULL, 3,
			&Exercise4_3Task) != pdPASS) {
		PRINT_ERROR("Failed to create 'Exercise4_3Task'");
		goto err_exercise4_3_task;
	}

	if (xTaskCreate(vExercise4_4Task, "Exercise4_4Task",
			mainGENERIC_STACK_SIZE, NULL, 4,
			&Exercise4_4Task) != pdPASS) {
		PRINT_ERROR("Failed to create 'Exercise4_4Task'");
		goto err_exercise4_4_task;
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
	vTaskSuspend(PrintTaskOutputsTask);
	vTaskSuspend(Exercise4_1Task);
	vTaskSuspend(Exercise4_2Task);
	vTaskSuspend(Exercise4_3Task);
	vTaskSuspend(Exercise4_4Task);

	xTimerStart(deleteButtonCountNS, portMAX_DELAY);

	printf("Numer of tasks: %lu\n\n", uxTaskGetNumberOfTasks());
	tumFUtilPrintTaskStateList();
	printf("\n");

	vTaskStartScheduler();

	return EXIT_SUCCESS;

// Everything created before has to be deleted. The item calling the error
// routin isn't deleted because it was not actually created.
err_exercise4_4_task:
	vTaskDelete(Exercise4_3Task);
err_exercise4_3_task:
	vTaskDelete(Exercise4_2Task);
err_exercise4_2_task:
	vTaskDelete(Exercise4_1Task);
err_exercise4_1_task:
	vTaskDelete(PrintTaskOutputsTask);
err_printtaskoutputs_task:
	vTaskDelete(CountingSecondsTask);
err_countingseconds_task:
	vTaskDelete(ButtonPressResetTask);
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
err_blinkingbuttonsdraw_task:
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
	vQueueDelete(numbersToPrint);
err_numberstoprint_queue:
	vSemaphoreDelete(secondsPassedTotal.lock);
err_secondspassedtotal_lock:
	vSemaphoreDelete(buttonPressCountNS.lock);
err_buttonpresscountns_lock:
	vSemaphoreDelete(wakeTask4_3);
err_wakeTask4_3_semaphore:
	vSemaphoreDelete(ButtonSPressed);
err_buttonspressed_semaphore:
	vSemaphoreDelete(movingShapesDisplayTaskResumed.lock);
err_movingShapesDisplayTaskResumed_lock:
	vSemaphoreDelete(changeState.lock);
err_changestate_lock:
	vSemaphoreDelete(buttonPressCountABCD.lock);
err_buttonpresscountabcd_lock:
	vSemaphoreDelete(ScreenLock);
err_screen_lock:
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
