/**
 * @file SwapBuffer.h
 * @author Benedikt Witteler
 * @date 12 December 2020
 * @brief Functionality used to update the screen.
 * 
 * Contains the vSwapBufferTask() that holds GL context by calling
 * tumDrawBindThread().
 */

#ifndef __SWAP_BUFFER_H__
#define __SWAP_BUFFER_H__

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "TUM_Draw.h"
#include "TUM_Event.h"

#include "SharedResources.h"

#define TIME_FRAMERATE_PERIOD_MS (TickType_t) 20

extern SemaphoreHandle_t ScreenLock;

void vSwapBufferTask(void *pvParameters);

#endif