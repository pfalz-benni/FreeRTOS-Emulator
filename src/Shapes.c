#include "Shapes.h"


int Circle__init(Circle_t *this, coord_t center, PIXELS radius, unsigned int color) {
	PositionProperties__init(&(this->_positionProperties), center.x, center.y, color);
	this->_radius = radius;

	return 0;
}

// -------------------------------------

int Rectangle__init(Rectangle_t *this, coord_t center, PIXELS width, PIXELS height, unsigned int color) {
	PositionProperties__init(&(this->_positionProperties), center.x, center.y, color);
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
	PositionProperties__init(&(this->_positionProperties), center.x, center.y, color);
	this->_corners[0] = (coord_t) {center.x, center.y - height / 2};
	this->_corners[1] = (coord_t) {center.x - height / 2, center.y + height / 2};
	this->_corners[2] = (coord_t) {center.x + height / 2, center.y + height / 2};
	this->_height = height;
    
	return 0;
}

// -------------------------------------

int Message__init(Message_t *this, coord_t center, char *text, unsigned int color) {
	PositionProperties__init(&(this->_positionProperties), center.x, center.y, color);
	this->_text = text;
	tumGetTextSize(text, &(this->_textWidth), &(this->_textHeight));

	this->_topLeftCorner = (coord_t) {center.x - this->_textWidth / 2, center.y - this->_textHeight / 2};

	return 0;
}

int Message__updateCorner(Message_t *this) {
	this->_topLeftCorner = (coord_t) {this->_positionProperties._x - this->_textWidth / 2,
									  this->_positionProperties._y - this->_textHeight / 2};

	return 0;
}