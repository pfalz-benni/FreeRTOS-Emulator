/**
 * @file StateMachine.h
 * @author Benedikt Witteler
 * @date 12 December 2020
 * @brief Implementation of a simple state machine to switch
 * between different screens / modes on the emulator screen.
 */

#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "TUM_Draw.h"
#include "TUM_FreeRTOS_Utils.h"

#include "SharedResources.h"

#define TIME_STATEMACHINETASK_DELAY 100

void giveResourcesState1To2();
void giveResourcesState2To3();
void giveResourcesState3To1();

void manageTasksState1To2();
void manageTasksState2To3();
void manageTasksState3To1();

void vStateMachineTask(void *pvParameters);

#endif