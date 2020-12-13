#include "StateMachine.h"

void vStateMachineTask(void *pvParameters) {
    enum states {stateOne = 1, stateTwo, stateThree};
    int currentState = stateOne;

    // initial screen are the moving shapes from exercise 2
    vTaskResume(MovingShapesDisplayTask);
    
    while (1) {

        if (xSemaphoreTake(changeState.lock, portMAX_DELAY) == pdTRUE) {
            if (changeState.value) {
                //change state: switch current display
                printf("State Changing\n");
                changeState.value = 0;

                switch (currentState) {
                    case stateOne:
                        currentState = stateTwo;
                        if (MovingShapesDisplayTask) {
                            vTaskSuspend(MovingShapesDisplayTask);
                        }
                        //resume 3.2 tasks
                        break;
                    case stateTwo:
                        currentState = stateThree;
                        //suspend and resume tasks
                        break;
                    case stateThree:
                        currentState = stateOne;
                        //suspend 4.x tasks
                        if (MovingShapesDisplayTask) {
                            vTaskResume(MovingShapesDisplayTask);
                            //set flag that task knows it has been resumed and can
                            //adjust accordingly
                            if (xSemaphoreTake(movingShapesDisplayTaskResumed.lock, portMAX_DELAY) == pdTRUE) {
                                movingShapesDisplayTaskResumed.value = 1;
                                xSemaphoreGive(movingShapesDisplayTaskResumed.lock);
                            }
                        }
                        break;
                    //TODO Make sure to give back all mutexes in order not to create deadlocks!!!
                }
            }
            xSemaphoreGive(changeState.lock);

        }


        vTaskDelay(portTICK_PERIOD_MS * 100);
    }
}