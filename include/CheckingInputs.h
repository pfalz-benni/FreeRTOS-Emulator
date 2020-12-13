/**
 * @file CheckingInputs.h
 * @author Benedikt Witteler
 * @date 07 December 2020
 * @brief Functionality used in  vCheckingInputsTask() that handles
 * inputs form the keyboard.
 */

#ifndef __CHECKING_INPUTS_H__
#define __CHECKING_INPUTS_H__


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "TUM_Event.h"

#include "SharedResources.h"

// Button Press Processing
#define OFFSET_KEYCODE_BUTTONPRESSCOUNT 4
#define TIME_DEBOUNCE_BUTTON_MS 300

#define TIME_CHECKINGINPUTSTASK_DELAY_MS 40


void xGetButtonInput(void);

void processStateChangeInput();

void checkAndProcessButtonPress(unsigned char keycode, TickType_t *lastPressTime, TickType_t *debounceDelay);

void resetButtonPressCountIfEntered();

void vCheckingInputsTask(void *pvParameters);


#endif