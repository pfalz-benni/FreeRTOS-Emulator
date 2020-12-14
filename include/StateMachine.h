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

#include "SharedResources.h"
#include "TUM_Draw.h"


void clearScreen();
void vStateMachineTask(void *pvParameters);






#endif