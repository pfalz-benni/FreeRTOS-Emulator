/**
 * @file Shapes.h
 * @author Benedikt Witteler
 * @date 27 November 2020
 * @brief Different classes to handle the shapes (especially regarding positioning)
 * from TUM_Draw. Created for the exercise part of the ESPL.
 */

#include <math.h>
#include <stdio.h>

#include "TUM_Draw.h"

/**
 * Type as which screen x- and y-coordinates, radii,
 * width and heights are stored. The coordinates inside coord_t
 * are also stored as signed shot.
 */
#define PIXELS signed short

#define SCREEN_CENTER (coord_t) { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }

/**
 * @brief Object to reperesent position as well as movement of a shape.
 * 
 * An object of PositionProperties is stored in a shape object to represent
 * position and shape of the center of that object.
 */
typedef struct PositionProperties {
	PIXELS _x; /**< X pixel coord of center on screen */
    PIXELS _y; /**< Y pixel coord of center on screen */

    float _f_x; /**< Absolute X location center */
    float _f_y; /**< Absolute Y location center */    

    float _dx; /**< X axis speed in pixels/second */
    float _dy; /**< Y axis speed in pixels/second */

    float _speed; /**< Speed the ball is able to achieve in
                              pixels/second */

    unsigned int _color; /**< Hex RGB colour of the ball */
} PositionProperties_t;

int PositionProperties__init(PositionProperties_t *this, PIXELS x,
			 PIXELS y, float speed, unsigned int color);

int PositionProperties__setPosition(PositionProperties_t *this, float f_x, float f_y);
int PositionProperties__setSpeed(PositionProperties_t *this, float dx, float dy);

int PositionProperties__updatePosition(PositionProperties_t *this, unsigned int timePassed_ms);

int PositionProperties__setSpeedMoveOnCircle(PositionProperties_t *this, PIXELS radius, double phase,
                                     unsigned int timePeriod_ms, unsigned int timePassedinTotal_ms);

void PositionProperties__printPositionAndSpeed(PositionProperties_t *this);
// -------------------------------------
typedef struct Circle {
	PositionProperties_t _positionProperties;
	PIXELS _radius;
} Circle_t;

int Circle__init(Circle_t *this, coord_t center, PIXELS radius, unsigned int color);

// -------------------------------------

typedef struct Rectangle {
    PositionProperties_t _positionProperties;
    coord_t _topLeftCorner;
    PIXELS _width;
    PIXELS _height;
} Rectangle_t;

int Rectangle__init(Rectangle_t *this, coord_t center, PIXELS width, PIXELS height, unsigned int color);

int Rectangle__updateCorner(Rectangle_t *this);

// -------------------------------------
typedef struct Triangle {
	PositionProperties_t _positionProperties;
    coord_t _corners[3]; //< Triangle's corners in the order of top, bottom-left, bottom-right
    PIXELS _height;
} Triangle_t;

int Triangle__init(Triangle_t *this, coord_t center, PIXELS height, unsigned int color);