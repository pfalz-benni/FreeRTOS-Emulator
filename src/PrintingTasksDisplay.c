#include "PrintingTasksDisplay.h"

void vPrintTaskOutputsTask(void *pvParameters)
{
	TickType_t xLastWakeTime;

	char tempString[LENGTH_TEMPORARY_STRING];
	int x = COORD_TASK_MESSAGE.x;
	int y = COORD_TASK_MESSAGE.y - DISTANCE_NEW_LINE;
	int counter = 1; //counts number of ticks

	tuple_t tempTuple = { 0 };
	TickType_t prevReceiveTick = 0;

	while (1) {
		xLastWakeTime = xTaskGetTickCount();

		xSemaphoreTake(ScreenLock, portMAX_DELAY);

		//reset task if it was resumed (TaskNotification from state machine)
		if (ulTaskNotifyTake(pdTRUE, 0)) {
			counter = 1;
			y = COORD_TASK_MESSAGE.y - DISTANCE_NEW_LINE;
			tumDrawClear(White);
		}

		while (xQueueReceive(numbersToPrint, &tempTuple, 0) == pdTRUE) {
			//determin which queue elements have been sent within the
			//same tick
			if (tempTuple.tick > prevReceiveTick) {
				y += DISTANCE_NEW_LINE;
				x = COORD_TASK_MESSAGE.x;

				//lign up the columns of numbers under each other
				if (counter < 10) {
					sprintf(tempString, "%d. Tick:   %c",
						counter++, tempTuple.value);
				} else {
					sprintf(tempString, "%d. Tick: %c",
						counter++, tempTuple.value);
				}
				tumDrawText(tempString, x, y, Black);
				x += DISTANCE_VERTICAL_LINE_BEGIN;
			} else {
				sprintf(tempString, "%c", tempTuple.value);
				tumDrawText(tempString, x, y, Black);
				x += DISTANCE_VERTICAL_CHARACTER;
			}

			prevReceiveTick = tempTuple.tick;
		}

		xSemaphoreGive(ScreenLock);

		if (counter > NUMBER_TICKS_LAST_EXECUTION) {
			vTaskSuspend(PrintTaskOutputsTask);
		}

		vTaskDelayUntil(&xLastWakeTime, 1);
	}
}

void sendToQueueAndSuspendTask(TaskHandle_t Exercise4_xTask, tuple_t *toQueue,
			       TickType_t *xLastWakeTime,
			       TickType_t *initialWakeTime, TickType_t delay)
{
	//reset task if it was resumed (TaskNotification from state machine)
	if (ulTaskNotifyTake(pdTRUE, 0)) {
		*initialWakeTime = xTaskGetTickCount();
	}
	*xLastWakeTime = xTaskGetTickCount();

	//execution for no longer than 15 ticks
	if (*xLastWakeTime < *initialWakeTime + NUMBER_TICKS_LAST_EXECUTION) {
		toQueue->tick = *xLastWakeTime;
		xQueueSend(numbersToPrint, toQueue, portMAX_DELAY);

		if (Exercise4_xTask == Exercise4_2Task) {
			xSemaphoreGive(wakeTask4_3);
		}

		//used instead of vTaskDelay() in order not to risk that
		//the task already executed long because it had to wait
		//for the queue
		vTaskDelayUntil(xLastWakeTime, delay);
	} else {
		if (Exercise4_xTask == Exercise4_2Task) {
			vTaskSuspend(Exercise4_3Task);
		}
		vTaskSuspend(Exercise4_xTask);
	}
}

void vExercise4_1Task(void *pvParameters)
{
	TickType_t xLastWakeTime, initialWakeTime;

	tuple_t toQueue = { 0 };
	toQueue.value = '1';

	while (1) {
		// printf("Task4_1\n");
		sendToQueueAndSuspendTask(Exercise4_1Task, &toQueue,
					  &xLastWakeTime, &initialWakeTime, 1);
	}
}

void vExercise4_2Task(void *pvParameters)
{
	TickType_t xLastWakeTime, initialWakeTime;

	tuple_t toQueue = { 0 };
	toQueue.value = '2';

	while (1) {
		// printf("Task4_2\n");
		sendToQueueAndSuspendTask(Exercise4_2Task, &toQueue,
					  &xLastWakeTime, &initialWakeTime, 2);
	}
}

void vExercise4_3Task(void *pvParameters)
{
	TickType_t xLastWakeTime;

	tuple_t toQueue = { 0 };
	toQueue.value = '3';

	while (1) {
		if (xSemaphoreTake(wakeTask4_3, portMAX_DELAY) == pdTRUE) {
			// printf("Task4_3\n");
			xLastWakeTime = xTaskGetTickCount();
			toQueue.tick = xLastWakeTime;
			xQueueSend(numbersToPrint, &toQueue, portMAX_DELAY);
		}
	}
}

void vExercise4_4Task(void *pvParameters)
{
	TickType_t xLastWakeTime, initialWakeTime;

	tuple_t toQueue = { 0 };
	toQueue.value = '4';

	while (1) {
		// printf("Task4_4\n");
		sendToQueueAndSuspendTask(Exercise4_4Task, &toQueue,
					  &xLastWakeTime, &initialWakeTime, 4);
	}
}