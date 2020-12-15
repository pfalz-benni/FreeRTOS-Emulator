/**
 * @file BlinkingButtonsDisplay.h
 * @author Benedikt Witteler
 * @date 14 December 2020
 * @brief Functionality used in vBlinkingButtonsDynamicTask(), 
 * vBlinkingButtonsStaticTask() and others drawing on the display 
 * for exerices 3.
 */

#ifndef __BLINKING_BUTTONS_DISPLAY_H__
#define __BLINKING_BUTTONS_DISPLAY_H__

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "TUM_Draw.h"
#include "TUM_Font.h"

#include "SharedResources.h"
#include "Shapes.h"

#define DISTANCE_CIRCLE_CENTER 80
#define RADIUS_CIRCLES 40
#define TIME_PERIOD_DYNAMIC_MS 1000
#define TIME_PERIOD_STATIC_MS 500
#define TIME_BLINKINGBUTTONSDRAWTASK_DELAY_MS 16
#define DISTANCE_VERTICAL_MESSAGE_CENTER 80
#define LENGTH_STRING_NS_DRAWN 50

void vDrawFPS(void);
void drawButtonsNSPressMessage(Message_h_t buttonsNSPressMessage,
			       buttonPresses_t buttonPressCountNS);

void vBlinkingButtonsDrawTask(void *pvParameters);
void vBlinkingButtonsDynamicTask(void *pvParameters);
void vBlinkingButtonsStaticTask(void *pvParameters);

void vButtonPressSemaphoreTask(void *pvParameters);
void vButtonPressNotificationTask(void *pvParameters);
void vButtonPressResetTask(void *pvParameters);

void vCountingSecondsTask(void *pvParameters);

#endif