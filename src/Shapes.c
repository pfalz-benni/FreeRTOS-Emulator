#include "Shapes.h"


struct Circle {
	PositionProperties_h_t _positionProperties;
	PIXELS _radius;
};

int Circle__init(Circle_h_t *handle, coord_t center, PIXELS radius, unsigned int color) {
	struct Circle *this = (struct Circle*) handle;

	PositionProperties__init(&(this->_positionProperties), center.x, center.y, color);
	this->_radius = radius;

	return 0;
}

PositionProperties_h_t Circle__getPositionProperties(Circle_h_t *handle) {
	return ((struct Circle*) handle)->_positionProperties;
}
PIXELS Circle__getRadius(Circle_h_t *handle) {
	return ((struct Circle*) handle)->_radius;
}

// -------------------------------------

struct Rectangle {
    PositionProperties_h_t _positionProperties;
    coord_t _topLeftCorner;
    PIXELS _width;
    PIXELS _height;
};

int Rectangle__init(Rectangle_h_t *handle, coord_t center, PIXELS width, PIXELS height, unsigned int color) {
	struct Rectangle *this = (struct Rectangle*) handle;

	PositionProperties__init(&(this->_positionProperties), center.x, center.y, color);
	this->_topLeftCorner = (coord_t) {center.x - width / 2, center.y - height / 2};
	this->_width = width;
	this->_height = height;

	return 0;
}

int Rectangle__updateCorner(Rectangle_h_t *handle){
	struct Rectangle *this = (struct Rectangle*) handle;
	
	this->_topLeftCorner = (coord_t) {PositionProperties__getX(this->_positionProperties) - this->_width / 2,
									  PositionProperties__getY(this->_positionProperties) - this->_height / 2};

	return 0;
}

PositionProperties_h_t Rectangle__getPositionProperties(Rectangle_h_t *handle) {
	return ((struct Rectangle*) handle)->_positionProperties;
}
coord_t Rectangle__getTopLeftCorner(Rectangle_h_t *handle){
	return ((struct Rectangle*) handle)->_topLeftCorner;
}
PIXELS Rectangle__getWidth(Rectangle_h_t *handle) {
	return ((struct Rectangle*) handle)->_width;
}
PIXELS Rectangle__getHeight(Rectangle_h_t *handle) {
	return ((struct Rectangle*) handle)->_height;
}

// -------------------------------------

struct Triangle {
	PositionProperties_h_t _positionProperties;
    coord_t _corners[3]; //< Triangle's corners in the order of top, bottom-left, bottom-right
    PIXELS _height;
};

int Triangle__init(Triangle_h_t *handle, coord_t center, PIXELS height, unsigned int color) {
	struct Triangle *this = (struct Triangle*) handle;

	PositionProperties__init(&(this->_positionProperties), center.x, center.y, color);
	this->_corners[0] = (coord_t) {center.x, center.y - height / 2};
	this->_corners[1] = (coord_t) {center.x - height / 2, center.y + height / 2};
	this->_corners[2] = (coord_t) {center.x + height / 2, center.y + height / 2};
	this->_height = height;
    
	return 0;
}

PositionProperties_h_t Triangle__getPositionProperties(Triangle_h_t *handle) {
	return ((struct Triangle*) handle)->_positionProperties;
}
coord_t* Triangle__getCorners(Triangle_h_t *handle) {
	return ((struct Triangle*) handle)->_corners;
}


// -------------------------------------

struct Message {
    PositionProperties_h_t _positionProperties;
    coord_t _topLeftCorner;
    char *_text;
    int _textWidth;
    int _textHeight;
};

int Message__init(Message_h_t *handle, coord_t center, char *text, unsigned int color) {
	struct Message *this = (struct Message*) handle;
	
	PositionProperties__init(&(this->_positionProperties), center.x, center.y, color);
	this->_text = text;
	tumGetTextSize(text, &(this->_textWidth), &(this->_textHeight));

	this->_topLeftCorner = (coord_t) {center.x - this->_textWidth / 2, center.y - this->_textHeight / 2};

	return 0;
}

int Message__updateCorner(Message_h_t *handle) {
	struct Message *this = (struct Message*) handle;

	this->_topLeftCorner = (coord_t) {PositionProperties__getX(this->_positionProperties) - this->_textWidth / 2,
									  PositionProperties__getX(this->_positionProperties) - this->_textHeight / 2};

	return 0;
}

PositionProperties_h_t Message__getPositionProperties(Message_h_t *handle) {
	return ((struct Message*) handle)->_positionProperties;
}
char* Message__getText(Message_h_t *handle) {
	return ((struct Message*) handle)->_text;
}
coord_t Message__getTopLeftCorner(Message_h_t *handle) {
	return ((struct Message*) handle)->_topLeftCorner;
}