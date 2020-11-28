#include "Shapes.h"


int PositionProperties__init(PositionProperties_t *this, PIXELS x,
			 PIXELS y, float speed, unsigned int color)
{
	this->_x = x;
	this->_y = y;
	this->_f_x = x;
	this->_f_y = y; 
	this->_dx = 0;
	this->_dy = 0;
	this->_speed = speed;
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

int PositionProperties__setSpeed(PositionProperties_t *this, float dx, float dy) {
	this->_dx = dx;
	this->_dy = dy;

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

void PositionProperties__printPositionAndSpeed(PositionProperties_t *this) {
	printf("Coordinates rounded: %d %d,  exact: %f %f,  Speed: %f %f\n",
			this->_x, this->_y,
			this->_f_x, this->_f_y,
			this->_dx, this->_dy);
}
// -------------------------------------

int Circle__init(Circle_t *this, coord_t center, PIXELS radius, unsigned int color) {
	PositionProperties__init(&(this->_positionProperties), center.x, center.y, 0, color);
	this->_radius = radius;

	return 0;
}

// -------------------------------------

int Rectangle__init(Rectangle_t *this, coord_t center, PIXELS width, PIXELS height, unsigned int color) {
	PositionProperties__init(&(this->_positionProperties), center.x, center.y, 0, color);
	this->_topLeftCorner = (coord_t) {center.x - width / 2, center.y - height / 2};
	this->_width = width;
	this->_height = height;

	return 0;
}

int Rectangle__updateCorner(Rectangle_t *this){
	this->_topLeftCorner = (coord_t) {this->_positionProperties._x - this->_width / 2,
									  this->_positionProperties._y - this->_height / 2};

	return 0;
}

// -------------------------------------

int Triangle__init(Triangle_t *this, coord_t center, PIXELS height, unsigned int color)
{
	PositionProperties__init(&(this->_positionProperties), center.x, center.y, 0, color);
	this->_corners[0] = (coord_t) {center.x, center.y - height / 2};
	this->_corners[1] = (coord_t) {center.x - height / 2, center.y + height / 2};
	this->_corners[2] = (coord_t) {center.x + height / 2, center.y + height / 2};
	this->_height = height;
    
	return 0;
}