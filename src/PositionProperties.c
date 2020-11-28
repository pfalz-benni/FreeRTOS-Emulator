#include "PositionProperties.h"

int PositionProperties__init(PositionProperties_t *this, PIXELS x,
			 PIXELS y, unsigned int color)
{
	this->_x = x;
	this->_y = y;
	this->_f_x = x;
	this->_f_y = y; 
	this->_dx = 0;
	this->_dy = 0;
	this->_color = color;

	return 0;
}

int PositionProperties__setPosition(PositionProperties_t *this, float f_x, float f_y) {
	this->_x = round(f_x);
	this->_y = round(f_y);
	this->_f_x = f_x;
	this->_f_y = f_y; 

	return 0;
}

int PositionProperties__updatePosition(PositionProperties_t *this, unsigned int timePassed_ms) {
	double timePassed_s = timePassed_ms / 1000.0;
	PositionProperties__setPosition(this, this->_f_x + this->_dx * timePassed_s,
									this->_f_y + this->_dy * timePassed_s);
	return 0;
}

int PositionProperties__setSpeedMoveOnCircle(PositionProperties_t *this, PIXELS radius, double phase,
                                     unsigned int timePeriod_ms, unsigned int timePassedInTotal_ms) {
	double timePeriod_s = timePeriod_ms / 1000.0;
	
	this->_dx = (-1) * sin((2 * M_PI * timePassedInTotal_ms) / timePeriod_ms + phase)
				* (radius * 2 * M_PI) / timePeriod_s;
	this->_dy = (-1) * cos((2 * M_PI * timePassedInTotal_ms) / timePeriod_ms + phase)
				* (radius * 2 * M_PI) / timePeriod_s;


	return 0;
}

int PositionProperties__moveVetically(PositionProperties_t *this, float dx, PIXELS distanceToBorder) {
	int spaceToRight = this->_x < SCREEN_WIDTH - distanceToBorder;
	int spaceToLeft = this->_x > distanceToBorder;

	//if the right or left border is reached, turn speed arround
	if ((spaceToRight && this->_dx >= 0) || (!spaceToLeft && this->_dx < 0))
		this->_dx = dx;
	else
		this->_dx = -dx;

	
	return 0;
}

void PositionProperties__printPositionAndSpeed(PositionProperties_t *this) {
	printf("Coordinates rounded: %d %d,  exact: %f %f,  Speed: %f %f\n",
			this->_x, this->_y,
			this->_f_x, this->_f_y,
			this->_dx, this->_dy);
}