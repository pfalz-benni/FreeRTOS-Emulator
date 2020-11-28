/**
 * @file Shapes.h
 * @author Benedikt Witteler
 * @date 27 November 2020
 * @brief Different classes to handle shapes or texts from TUM_Draw.
 * 
 * Created for the exercise part of the ESPL.
 */

#include "TUM_Draw.h"
#include "PositionProperties.h"

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

// -------------------------------------

typedef struct Message {
    PositionProperties_t _positionProperties;
    coord_t _topLeftCorner;
    char *_text;
    int _textWidth;
    int _textHeight;
} Message_t;

int Message__init(Message_t *this, coord_t center, char *text, unsigned int color);

int Message__updateCorner(Message_t *this);