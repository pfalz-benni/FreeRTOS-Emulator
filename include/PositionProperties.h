/**
 * @file PositionProperties.h
 * @author Benedikt Witteler
 * @date 28 November 2020
 * @brief A class to handle the position of objects created using TUM_Draw.
 * 
 * @mainpage FreeRTOS Emulator
 *
 * @section intro_sec Heads up
 * 
 * I decided not to use funcition pointers inside structures as a way to
 * implemente methods. Therefore, "methods" - meaning functions that operate
 * on an instance of a structure - cannot be found in the documentation of
 * its class (= structure) but only in the source file containing the defintion
 * of the class.
 *
 */

#ifndef __POSITION_PROPERTIES_H__
#define __POSITION_PROPERTIES_H__

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "TUM_Draw.h"

/**
 * Type as which screen x- and y-coordinates, radii,
 * width and heights are stored. The coordinates inside coord_t
 * are also stored as signed shot.
 */
#define PIXELS signed short

#define SCREEN_CENTER (coord_t) { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }

typedef void *PositionProperties_h_t;

PositionProperties_h_t PositionProperties__init(PIXELS x, PIXELS y,
						unsigned int color);
int PositionProperties__destruct(PositionProperties_h_t *handle);

int PositionProperties__resetPositionAndSpeed(PositionProperties_h_t handle);
int PositionProperties__setPosition(PositionProperties_h_t handle, float f_x,
				    float f_y);
int PositionProperties__updatePosition(PositionProperties_h_t handle,
				       unsigned int timePassed_ms);
int PositionProperties__setSpeedMoveOnCircle(PositionProperties_h_t handle,
					     PIXELS radius, double phase,
					     unsigned int timePeriod_ms,
					     unsigned int timePassedinTotal_ms);
int PositionProperties__moveVetically(PositionProperties_h_t handle, float dx,
				      PIXELS distanceToBorder);
int PositionProperties__adjustPositionToNewScreenCenter(
	PositionProperties_h_t handle, coord_t oldScreenCenter,
	coord_t newScreenCenter);

PIXELS PositionProperties__getX(PositionProperties_h_t handle);
PIXELS PositionProperties__getY(PositionProperties_h_t handle);
unsigned int PositionProperties__getColor(PositionProperties_h_t handle);

void PositionProperties__printPositionAndSpeed(PositionProperties_h_t handle);

#endif