/**
 * @file TimerFunctionality.h
 * @author Benedikt Witteler
 * @date 15 December 2020
 * @brief Functionality for the times used in the exercises.
 * 
 * Inlcudes the callback functions for the software timers.
 */

#ifndef __TIMER_FUNCTIONALITY_H__
#define __TIMER_FUNCTIONALITY_H__

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "SharedResources.h"

void deleteButtonCountNSCallback(TimerHandle_t deleteButtonCountNS);



#endif