/**
 * @file PrintingTasksDisplay.h
 * @author Benedikt Witteler
 * @date 15 December 2020
 * @brief Functionality used in exerices 4 to control the 
 * several different tasks used.
 */

#ifndef __PRINTING_TASKS_DISPLAY_H__
#define __PRINTING_TASKS_DISPLAY_H__

#include "FreeRTOS.h"
#include "task.h"

#include "TUM_Draw.h"
#include "TUM_Font.h"

#include "SharedResources.h"
#include "Shapes.h"


#define COORD_TASK_MESSAGE (coord_t) {20, 20}
#define LENGTH_TEMPORARY_STRING 50
#define DISTANCE_NEW_LINE 20
#define NUMBER_TICKS_LAST_EXECUTION 15
#define DISTANCE_VERTICAL_CHARACTER 20
#define DISTANCE_VERTICAL_LINE_BEGIN 80

void vPrintTaskOutputsTask(void *pvParameters);

void sendToQueueAndSuspendTask(TaskHandle_t Exercise4_xTask, tuple_t *toQueue, TickType_t *xLastWakeTime,
        TickType_t *initialWakeTime, TickType_t delay);

void vExercise4_1Task(void *pvParameters);
void vExercise4_2Task(void *pvParameters);
void vExercise4_3Task(void *pvParameters);
void vExercise4_4Task(void *pvParameters);



#endif