/**
 * @file MovingShapesDisplay.h
 * @author Benedikt Witteler
 * @date 07 December 2020
 * @brief Functionality used in vMovingShapesDisplayTask() that creates all the
 * content that's drawn on the screen for exercis 2 with the moving shapes and texts.
 */

#ifndef __RUNNING_DISPLAY_H__
#define __RUNNING_DISPLAY_H__

#include "FreeRTOS.h"
#include "task.h"

#include "TUM_Event.h"

#include "Shapes.h"
#include "SharedResources.h"

// Constants for drawn objects
#define DISTANCE_TO_TRIANGLE 150
#define RADIUS_CIRCLE 50
#define HEIGHT_SQUARE 80
#define HEIGHT_TRIANGLE 80
#define TIME_ORBIT_PERIOD 5000
#define DISTANCE_VERTICAL_TO_BORDER 35
#define DISTANCE_HORIZONTAL_TO_BORDER 250
#define SPEED_MOVING_MESSAGE 100
#define COORD_BUTTON_PRESS_MESSAGE (coord_t) {30, 50}
#define LENGTH_STRINGS_DRAWN 50
/**
 * Ratio of mouse cursor position to screen center position when moving screen
 */
#define RATION_CURSER_CENTER 5

#define TIME_MOVINGSHAPESDISPLAYTASK_DELAY_MS 20

void updateShapeAndMessagePositions(Circle_h_t circle, Rectangle_h_t rectangle,
				    Message_h_t topMessage,
				    TickType_t xLastWakeTime,
				    TickType_t prevWakeTime,
				    TickType_t initialWakeTime);

void drawShapes(Circle_h_t circle, Rectangle_h_t rectangle,
		Triangle_h_t triangle);

void drawSimpleTextMessages(Message_h_t topMessage, Message_h_t bottomMessage);

void drawButtonPressMessage(Message_h_t buttonPressMessage,
			    buttonPresses_t buttonPressCountABCD);

void drawMouseCoordMessage(Message_h_t mouseCoordMessage);

void moveScreenInCursorDirection(coord_t *mobileScreenCenter, Circle_h_t circle,
				 Rectangle_h_t rectangle, Triangle_h_t triangle,
				 Message_h_t bottomMessage,
				 Message_h_t topMessage,
				 Message_h_t buttonPressMessage,
				 Message_h_t mouseCoordMessage);

void vMovingShapesDisplayTask(void *pvParameters);

#endif