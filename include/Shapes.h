/**
 * @file Shapes.h
 * @author Benedikt Witteler
 * @date 27 November 2020
 * @brief Different classes to handle the shapes (especially regarding positioning)
 * from TUM_Draw. Created for the exercisepart of the ESPL.
 */

#include "TUM_Draw.h"

#define SCREEN_CENTER                                                          \
	(coord_t)                                                              \
	{                                                                      \
		SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2                            \
	}

typedef struct TriangleCoords {
	//attributes
	coord_t _topCorner;
	coord_t _leftCorner;
	coord_t _rightCorner;
} TriangleCoords_t;

int TriangleCoords__init(TriangleCoords_t *this, coord_t center, int height);