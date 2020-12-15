#include "StateMachine.h"

void manageTasksState1To2() {
    //suspend 2.x task
    if (MovingShapesDisplayTask) {
        vTaskSuspend(MovingShapesDisplayTask);
    }
    //resume 3.x tasks
    if (BlinkingButtonsDrawTask) {
        vTaskResume(BlinkingButtonsDrawTask);
    }
    if (BlinkingButtonsDynamicTask) {
        vTaskResume(BlinkingButtonsDynamicTask);
    }
    if (BlinkingButtonsStaticTask) {
        vTaskResume(BlinkingButtonsStaticTask);
    }
    if (ButtonPressNotificationTask) {
        vTaskResume(ButtonPressNotificationTask);
    }
    if (ButtonPressSemaphoreTask) {
        vTaskResume(ButtonPressSemaphoreTask);
    }
    if (ButtonPressResetTask) {
        vTaskResume(ButtonPressResetTask);
    }
}

void manageTasksState2To3() {
    //suspend 3.x tasks
    if (BlinkingButtonsDrawTask) {
        vTaskSuspend(BlinkingButtonsDrawTask);
    }
    if (BlinkingButtonsDynamicTask) {
        vTaskSuspend(BlinkingButtonsDynamicTask);
    }
    if (BlinkingButtonsStaticTask) {
        vTaskSuspend(BlinkingButtonsStaticTask);
    }
    if (ButtonPressNotificationTask) {
        vTaskSuspend(ButtonPressNotificationTask);
    }
    if (ButtonPressSemaphoreTask) {
        vTaskSuspend(ButtonPressSemaphoreTask);
    }
    if (ButtonPressResetTask) {
        vTaskSuspend(ButtonPressResetTask);
    }
    //resume 4.x tasks
    //but first clear the queue
    xQueueReset(numbersToPrint);
    if (Exercise4_1Task) {
        vTaskResume(Exercise4_1Task);
        xTaskNotifyGive(Exercise4_1Task);
    }
    if (Exercise4_2Task) {
        vTaskResume(Exercise4_2Task);
        xTaskNotifyGive(Exercise4_2Task);
    }
    if (Exercise4_3Task) {
        vTaskResume(Exercise4_3Task);
        xTaskNotifyGive(Exercise4_3Task);
    }
    if (Exercise4_4Task) {
        vTaskResume(Exercise4_4Task);
        xTaskNotifyGive(Exercise4_4Task);
    }
    if (PrintTaskOutputsTask) {
        vTaskResume(PrintTaskOutputsTask);
        xTaskNotifyGive(PrintTaskOutputsTask);
    }
}

void manageTasksState3To1() {
    //suspend 4.x tasks
    if (Exercise4_1Task) {
        vTaskSuspend(Exercise4_1Task);
    }
    if (Exercise4_2Task) {
        vTaskSuspend(Exercise4_2Task);
    }
    if (Exercise4_3Task) {
        vTaskSuspend(Exercise4_2Task);
    }                        
    if (Exercise4_4Task) {
        vTaskSuspend(Exercise4_4Task);
    }
    if (PrintTaskOutputsTask) {
        vTaskSuspend(PrintTaskOutputsTask);
    }
    //resume 2.x task
    if (MovingShapesDisplayTask) {
        vTaskResume(MovingShapesDisplayTask);
        //set flag that task knows it has been resumed and can
        //adjust accordingly
        if (xSemaphoreTake(movingShapesDisplayTaskResumed.lock, portMAX_DELAY) == pdTRUE) {
            movingShapesDisplayTaskResumed.value = 1;
            xSemaphoreGive(movingShapesDisplayTaskResumed.lock);
        }
    }
}

void vStateMachineTask(void *pvParameters) {
    enum states {stateOne = 1, stateTwo, stateThree};
    int currentState = stateOne;

    // initial screen are the moving shapes from exercise 2
    vTaskResume(MovingShapesDisplayTask);
    
    while (1) {
        if (xSemaphoreTake(changeState.lock, portMAX_DELAY) == pdTRUE) {
            if (changeState.value) {

                printf("State Changing\n");
                changeState.value = 0;

                switch (currentState) {
                    case stateOne:
                        currentState = stateTwo;
                        manageTasksState1To2();
                        break;
                    case stateTwo:
                        currentState = stateThree;
                        manageTasksState2To3();
                        break;
                    case stateThree:
                        currentState = stateOne;
                        manageTasksState3To1();
                        break;
                    //TODO Make sure to give back all mutexes in order not to create deadlocks!!!
                }
                // tumFUtilPrintTaskStateList();
            }
            xSemaphoreGive(changeState.lock);
        }
        vTaskDelay(portTICK_PERIOD_MS * TIME_STATEMACHINETASK_DELAY);
    }
}