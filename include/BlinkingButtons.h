/**
 * @file BlinkingButtons.h
 * @author Benedikt Witteler
 * @date 14 December 2020
 * @brief Functionality used in vBlinkingButtonsDynamicTask() and
 * vBlinkingButtonsStaticTask().
 * 
 * The tasks create and manage the two blinking buttons for exercise
 * 3.2,2.
 */

#ifndef __BLINKING_BUTTONS_H__
#define __BLINKING_BUTTONS_H__

#include "FreeRTOS.h"
#include "task.h"

#include "TUM_Draw.h"
#include "TUM_Font.h"

#include "SharedResources.h"
#include "Shapes.h"

#define DISTANCE_CIRCLE_CENTER 80
#define RADIUS_CIRCLES 40
#define TIME_PERIOD_DYNAMIC_MS 1000
#define TIME_PERIOD_STATIC_MS 500
#define TIME_BLINKINGBUTTONSDRAWTASK_DELAY_MS 16

void vDrawFPS(void);

void vBlinkingButtonsDrawTask(void *pvParameters);
void vBlinkingButtonsDynamicTask(void *pvParameters);
void vBlinkingButtonsStaticTask(void *pvParameters);




#endif