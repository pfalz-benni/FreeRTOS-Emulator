/**
 * @file Shapes.h
 * @author Benedikt Witteler
 * @date 27 November 2020
 * @brief Different classes to handle shapes or texts that will
 * be drawn using TUM_Draw.
 */

#ifndef __SHAPES_H__
#define __SHAPES_H__

#include "TUM_Draw.h"
#include "PositionProperties.h"

typedef void *Circle_h_t;

Circle_h_t Circle__init(coord_t center, PIXELS radius, unsigned int color);
int Circle__destruct(Circle_h_t *handle);

PositionProperties_h_t Circle__getPositionProperties(Circle_h_t handle);
PIXELS Circle__getRadius(Circle_h_t handle);

// -------------------------------------

typedef void *Rectangle_h_t;

Rectangle_h_t Rectangle__init(coord_t center, PIXELS width, PIXELS height, unsigned int color);
int Rectangle__destruct(Rectangle_h_t *handle);

int Rectangle__updateCorner(Rectangle_h_t handle);

PositionProperties_h_t Rectangle__getPositionProperties(Rectangle_h_t handle);
coord_t Rectangle__getTopLeftCorner(Rectangle_h_t handle);
PIXELS Rectangle__getWidth(Rectangle_h_t handle);
PIXELS Rectangle__getHeight(Rectangle_h_t handle);

// -------------------------------------
typedef void *Triangle_h_t;

Triangle_h_t Triangle__init(coord_t center, PIXELS height, unsigned int color);
int Triangle__destruct(Triangle_h_t *handle);

int Triangle__updateCorners(Triangle_h_t handle);

PositionProperties_h_t Triangle__getPositionProperties(Triangle_h_t handle);
coord_t* Triangle__getCorners(Triangle_h_t handle);


// -------------------------------------

typedef void *Message_h_t;

Message_h_t Message__init(coord_t center, char *text, unsigned int color);
int Message__destruct(Message_h_t *handle);

/**
 * Initializes Object at a given top left coordinate instead of at a given center coordinate
 */
Message_h_t Message__initTopLeftCorner(coord_t topLeftCorner, char *text, unsigned int color);

int Message__updateCorner(Message_h_t handle);

int Message__setText(Message_h_t handle, char *text);

PositionProperties_h_t Message__getPositionProperties(Message_h_t handle);
char* Message__getText(Message_h_t handle);
coord_t Message__getTopLeftCorner(Message_h_t handle);
int Message__getTextHeight(Message_h_t handle);



#endif