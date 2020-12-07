#include <stdlib.h>

#include "PositionProperties.h"

/**
 * @brief Object to reperesent position as well as movement of a shape.
 * 
 * An object of PositionProperties is stored in a shape object to represent
 * position and shape of the center of that object.
 */
struct PositionProperties {
	PIXELS _x; /**< X pixel coord relative to center on screen */
    PIXELS _y; /**< Y pixel coord relative to center on screen */

    float _f_x; /**< Absolute X location relative to center */
    float _f_y; /**< Absolute Y location relative to center */    

    float _dx; /**< X axis speed in pixels/second */
    float _dy; /**< Y axis speed in pixels/second */

    unsigned int _color; /**< Hex RGB colour of the ball */
};



PositionProperties_h_t PositionProperties__init(PIXELS x, PIXELS y, unsigned int color) {
	struct PositionProperties *this = malloc(sizeof(struct PositionProperties));

	this->_x = x;
	this->_y = y;
	this->_f_x = x;
	this->_f_y = y; 
	this->_dx = 0;
	this->_dy = 0;
	this->_color = color;

	return (PositionProperties_h_t) this;
}

int PositionProperties__setPosition(PositionProperties_h_t handle, float f_x, float f_y) {
	struct PositionProperties *this = (struct PositionProperties *) handle;

	this->_x = round(f_x);
	this->_y = round(f_y);
	this->_f_x = f_x;
	this->_f_y = f_y; 

	return 0;
}

int PositionProperties__updatePosition(PositionProperties_h_t handle, unsigned int timePassed_ms) {
	struct PositionProperties *this = (struct PositionProperties *) handle;

	double timePassed_s = timePassed_ms / 1000.0;
	PositionProperties__setPosition(handle, this->_f_x + this->_dx * timePassed_s,
									this->_f_y + this->_dy * timePassed_s);
	return 0;
}

int PositionProperties__setSpeedMoveOnCircle(PositionProperties_h_t handle, PIXELS radius, double phase,
                                     unsigned int timePeriod_ms, unsigned int timePassedInTotal_ms) {
	struct PositionProperties *this = (struct PositionProperties *) handle;

	double timePeriod_s = timePeriod_ms / 1000.0;
	
	this->_dx = (-1) * sin((2 * M_PI * timePassedInTotal_ms) / timePeriod_ms + phase)
				* (radius * 2 * M_PI) / timePeriod_s;
	this->_dy = (-1) * cos((2 * M_PI * timePassedInTotal_ms) / timePeriod_ms + phase)
				* (radius * 2 * M_PI) / timePeriod_s;


	return 0;
}

int PositionProperties__moveVetically(PositionProperties_h_t handle, float dx, PIXELS distanceToBorder) {
	struct PositionProperties *this = (struct PositionProperties *) handle;

	int spaceToRight = this->_x < SCREEN_WIDTH - distanceToBorder;
	int spaceToLeft = this->_x > distanceToBorder;

	//if the right or left border is reached, turn speed arround
	if ((spaceToRight && this->_dx >= 0) || (!spaceToLeft && this->_dx < 0))
		this->_dx = dx;
	else
		this->_dx = -dx;

	
	return 0;
}

int PositionProperties__adjustPositionToNewScreenCenter(PositionProperties_h_t handle,
		coord_t oldScreenCenter, coord_t newScreenCenter) {

	struct PositionProperties *this = (struct PositionProperties *) handle;

	this->_f_x = this->_f_x + oldScreenCenter.x - newScreenCenter.x;
	this->_f_y = this->_f_y + oldScreenCenter.y - newScreenCenter.y;

	this->_x = round(this->_f_x);
	this->_y = round(this->_f_y);

	return 0;
}

// Getters:
PIXELS PositionProperties__getX(PositionProperties_h_t handle) {
	return ((struct PositionProperties *) handle)->_x;
}
PIXELS PositionProperties__getY(PositionProperties_h_t handle) {
	return ((struct PositionProperties *) handle)->_y;
}
unsigned int PositionProperties__getColor(PositionProperties_h_t handle) {
	return ((struct PositionProperties*) handle)->_color;
}



void PositionProperties__printPositionAndSpeed(PositionProperties_h_t handle) {
	struct PositionProperties *this = (struct PositionProperties *) handle;

	printf("Coordinates rounded: %d %d,  exact: %f %f,  Speed: %f %f\n",
			this->_x, this->_y,
			this->_f_x, this->_f_y,
			this->_dx, this->_dy);
}