/**
 * @file SharedResources.h
 * @author Benedikt Witteler
 * @date 07 December 2020
 * @brief Definition of structures that guard shared resources using
 * mutexes.
 */

#ifndef __SHARED_RESOURCES_H__
#define __SHARED_RESOURCES_H__

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "semphr.h"


#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR
#define NUMBER_OF_TRACKED_KEYS 4


/**
 * @brief Contains information, whether a button on the keyboard is pressed or not.
 * 
 * This is a shared resource. Therefore access is protected using a mutex.
 */
typedef struct buttons_buffer {
	unsigned char buttons[SDL_NUM_SCANCODES];
	SemaphoreHandle_t lock;
} buttons_buffer_t;

/**
 * @brief Contains information, how often a button on the keyboard has been pressed
 * last reset.
 * 
 * This is a shared resource. Therefore access is protected using a mutex.
 * In this case only the buttons A, B, C and D are tracked.
 */
typedef struct buttonPresses {
	unsigned int valuesABCD[NUMBER_OF_TRACKED_KEYS];
	SemaphoreHandle_t lock;
} buttonPresses_t;

/**
 * @brief Contatins information, wheather a change of some generic
 * kind has happend.
 * 
 * value = 1 indicates a state change, 0 no state change.
 * Examples of state changes expressed: state the state machine is in
 * indicating the display mode, information wheather a task has
 * been resumed (after it has been suspended) or not
 */
typedef struct genericBinaryState {
    unsigned int value;
    SemaphoreHandle_t lock;
} genericBinaryState_t;


// All the globally used variables declared in main.c
extern TaskHandle_t MovingShapesDisplayTask;
extern TaskHandle_t CheckingInputsTask;
extern TaskHandle_t StateMachineTask;
extern TaskHandle_t SwapBufferTask;

extern SemaphoreHandle_t ScreenLock;
extern buttons_buffer_t buttons;
extern buttonPresses_t buttonPressCount;
extern genericBinaryState_t changeState;
extern genericBinaryState_t movingShapesDisplayTaskResumed;

#endif