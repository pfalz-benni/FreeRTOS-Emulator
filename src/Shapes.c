#include "Shapes.h"


int MovementProperties__init(PositionProperties_t *this, PIXELS x,
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


int Circle__init(Circle_t *this, coord_t center, PIXELS radius, unsigned int color) {
	MovementProperties__init(&(this->_positionProperties), center.x, center.y, 0, color);
	this->_radius = radius;

	return 0;
}


int Rectangle__init(Rectangle_t *this, coord_t center, PIXELS width, PIXELS height, unsigned int color) {
	MovementProperties__init(&(this->_positionProperties), center.x, center.y, 0, color);
	this->_topLeftCorner = (coord_t) {center.x - width / 2, center.y - height / 2};
	this->_width = width;
	this->_height = height;

	return 0;
}


int Triangle__init(Triangle_t *this, coord_t center, PIXELS height, unsigned int color)
{
	MovementProperties__init(&(this->_positionProperties), center.x, center.y, 0, color);
	this->_corners[0] = (coord_t) {center.x, center.y - height / 2};
	this->_corners[1] = (coord_t) {center.x - height / 2, center.y + height / 2};
	this->_corners[2] = (coord_t) {center.x + height / 2, center.y + height / 2};
	this->_height = height;
    
	return 0;
}