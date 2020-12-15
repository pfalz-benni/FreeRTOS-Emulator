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
#include "timers.h"


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
	unsigned int values[NUMBER_OF_TRACKED_KEYS];
	SemaphoreHandle_t lock;
} buttonPresses_t;

/**
 * @brief Contatins an integer variable that is a shared ressource 
 * and therefore guarded by a mutex.
 * 
 * Can also be used as a boolean/binary Variable indication some
 * kind of state change:
 * value = 1 indicates a state change, 0 no state change.
 * Examples of state changes expressed: state wheather the state machine 
 * must switch change its state, information wheather a task has
 * been resumed (after it has been suspended) or not
 */
typedef struct sharedIntegerVariable {
    int value;
    SemaphoreHandle_t lock;
} sharedIntegerVariable_t;


// All the globally used variables declared in main.c
extern TaskHandle_t MovingShapesDisplayTask;
extern TaskHandle_t CheckingInputsTask;
extern TaskHandle_t StateMachineTask;
extern TaskHandle_t SwapBufferTask;
extern TaskHandle_t BlinkingButtonsDrawTask;
extern TaskHandle_t BlinkingButtonsDynamicTask;
extern TaskHandle_t BlinkingButtonsStaticTask;
extern TaskHandle_t ButtonPressSemaphoreTask;
extern TaskHandle_t ButtonPressNotificationTask;
extern TaskHandle_t ButtonPressResetTask;
extern TaskHandle_t CountingSecondsTask;

extern SemaphoreHandle_t ScreenLock;
extern SemaphoreHandle_t ButtonSPressed;
extern buttons_buffer_t buttons;
extern buttonPresses_t buttonPressCountABCD;
extern sharedIntegerVariable_t changeState;
extern sharedIntegerVariable_t movingShapesDisplayTaskResumed;
extern sharedIntegerVariable_t secondsPassedTotal;
extern buttonPresses_t buttonPressCountNS;
extern TimerHandle_t deleteButtonCountNS;

#endif