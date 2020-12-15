#include "PrintingTasksDisplay.h"

void vPrintTaskOutputsTask(void *pvParameters) {
    TickType_t xLastWakeTime, prevWakeTime;

    // Message_h_t taskOutputMessageLines[15];
    // Message_h_t *taskOutputMessageLines = calloc(15, sizeof(Message_h_t));

    // for (int i = 0; i < 15; i++) {
    //     char *lineText = calloc(10, sizeof(char));
    //     sprintf(lineText, "%d. Task: ", i + 1);
    //     // printf(lineText);
    //     // printf("\n");
    //     taskOutputMessageLines[i] = Message__initTopLeftCorner(
    //         (coord_t) {COORD_TASK_MESSAGE.x, COORD_TASK_MESSAGE.y + 20 * i},
    //         lineText, Black);
    // }

    char tempString[50];
    char tempChar;
    int x = COORD_TASK_MESSAGE.x;
    int y = COORD_TASK_MESSAGE.y - 20;
    int counter = 0; //counts number of ticks

    while (1) {
        xLastWakeTime = xTaskGetTickCount();

        xSemaphoreTake(ScreenLock, portMAX_DELAY);
        if (ulTaskNotifyTake(pdTRUE, 0)) {
            counter = 0;
            y = COORD_TASK_MESSAGE.y - 20;
            tumDrawClear(White);
        }
        
        while (xQueueReceive(numbersToPrint, &tempChar, 0) == pdTRUE) {
            // if (tempChar == '1') {
            if (xTaskGetTickCount() > xLastWakeTime) {
                y += 20;
                x = COORD_TASK_MESSAGE.x;

                //lign up the columns of numbers under each other
                if (counter < 10) {
                    sprintf(tempString, "%d. Tick:   %c", counter++, tempChar);
                } else {
                    sprintf(tempString, "%d. Tick: %c", counter++, tempChar);
                }
                tumDrawText(tempString, x, y, Black);
                x += 80;
                break;
            } else {
                sprintf(tempString, "%c", tempChar);
                tumDrawText(tempString, x, y, Black);
                x += 20;
            }
        }

        xSemaphoreGive(ScreenLock);

        if (counter > 15) {
            vTaskSuspend(PrintTaskOutputsTask);
        }

        prevWakeTime = xLastWakeTime;

        // vTaskDelayUntil(&xLastWakeTime, 1);
        vTaskDelay(1);
    }
}

//note: these tasks might run a couple times to often compared
//to what is printed on the screen. However, this is not a 
//problem because the queue is always reset before running
//the third state diplay

void vExercise4_1Task(void *pvParameters) {
    TickType_t xLastWakeTime;
    char one = '1';

    while (1) {
        xLastWakeTime = xTaskGetTickCount();

        xQueueSend(numbersToPrint,&one , portMAX_DELAY);

        //used instead of vTaskDelay() in order not to risk that
        //the task already executed long because it had to wait
        //for the queue
        // vTaskDelayUntil(&xLastWakeTime, 1);
        vTaskDelay(2);
    }
}

void vExercise4_2Task(void *pvParameters) {
    TickType_t xLastWakeTime;
    char two = '2';

    while (1) {
        xLastWakeTime = xTaskGetTickCount();

        xQueueSend(numbersToPrint,&two , portMAX_DELAY);

        // vTaskDelayUntil(&xLastWakeTime, 2);
        vTaskDelay(4);
    }
}

void vExercise4_3Task(void *pvParameters){
}

void vExercise4_4Task(void *pvParameters) {
    TickType_t xLastWakeTime;
    char four = '4';

    while (1) {
        xLastWakeTime = xTaskGetTickCount();

        xQueueSend(numbersToPrint,&four , portMAX_DELAY);

        vTaskDelayUntil(&xLastWakeTime, 4);
    }
}