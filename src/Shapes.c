#include <stdlib.h>

#include "Shapes.h"

struct Circle {
	PositionProperties_h_t _positionProperties;
	PIXELS _radius;
};

Circle_h_t Circle__init(coord_t center, PIXELS radius, unsigned int color) {
	struct Circle *this = malloc(sizeof(struct Circle));

	this->_positionProperties = PositionProperties__init(center.x, center.y, color);
	this->_radius = radius;

	return (Circle_h_t) this;
}

int Circle__destruct(Circle_h_t *handle) {
	PositionProperties__destruct(&((struct Circle *) *handle)->_positionProperties);
	free((struct Circle *) *handle);
	*handle = NULL;

	return 0;
}

PositionProperties_h_t Circle__getPositionProperties(Circle_h_t handle) {
	return ((struct Circle*) handle)->_positionProperties;
}
PIXELS Circle__getRadius(Circle_h_t handle) {
	return ((struct Circle*) handle)->_radius;
}

// -------------------------------------

struct Rectangle {
    PositionProperties_h_t _positionProperties;
    coord_t _topLeftCorner;
    PIXELS _width;
    PIXELS _height;
};

Rectangle_h_t Rectangle__init(coord_t center, PIXELS width, PIXELS height, unsigned int color) {
	struct Rectangle *this = malloc(sizeof(struct Rectangle));

	this->_positionProperties = PositionProperties__init(center.x, center.y, color);
	this->_topLeftCorner = (coord_t) {center.x - width / 2, center.y - height / 2};
	this->_width = width;
	this->_height = height;

	return (Rectangle_h_t) this;
}

int Rectangle__destruct(Rectangle_h_t *handle) {
	PositionProperties__destruct(&((struct Rectangle *) *handle)->_positionProperties);
	free((struct Rectangle *) *handle);
	*handle = NULL;

	return 0;
}

int Rectangle__updateCorner(Rectangle_h_t handle){
	struct Rectangle *this = (struct Rectangle*) handle;
	
	this->_topLeftCorner = (coord_t) {PositionProperties__getX(this->_positionProperties) - this->_width / 2,
									  PositionProperties__getY(this->_positionProperties) - this->_height / 2};

	return 0;
}

PositionProperties_h_t Rectangle__getPositionProperties(Rectangle_h_t handle) {
	return ((struct Rectangle*) handle)->_positionProperties;
}
coord_t Rectangle__getTopLeftCorner(Rectangle_h_t handle){
	return ((struct Rectangle*) handle)->_topLeftCorner;
}
PIXELS Rectangle__getWidth(Rectangle_h_t handle) {
	return ((struct Rectangle*) handle)->_width;
}
PIXELS Rectangle__getHeight(Rectangle_h_t handle) {
	return ((struct Rectangle*) handle)->_height;
}

// -------------------------------------

struct Triangle {
	PositionProperties_h_t _positionProperties;
    coord_t *_corners; //< Triangle's corners in the order of top, bottom-left, bottom-right
    PIXELS _height;
};

Triangle_h_t Triangle__init(coord_t center, PIXELS height, unsigned int color) {
	struct Triangle *this = malloc(sizeof(struct Triangle));

	this->_positionProperties = PositionProperties__init(center.x, center.y, color);

	this->_corners = malloc(3 * sizeof(coord_t));
	this->_corners[0] = (coord_t) {center.x, center.y - height / 2};
	this->_corners[1] = (coord_t) {center.x - height / 2, center.y + height / 2};
	this->_corners[2] = (coord_t) {center.x + height / 2, center.y + height / 2};

	this->_height = height;
    
	return (Triangle_h_t) this;
}

int Triangle__destruct(Triangle_h_t *handle) {
	PositionProperties__destruct(&((struct Triangle *) *handle)->_positionProperties);
	free((struct Triangle *) *handle);
	*handle = NULL;

	return 0;
}

int Triangle__updateCorners(Triangle_h_t handle) {
	struct Triangle *this = (struct Triangle *) handle;

	this->_corners[0] = (coord_t) {PositionProperties__getX(this->_positionProperties),
			PositionProperties__getY(this->_positionProperties) - this->_height / 2};
	this->_corners[1] = (coord_t) {PositionProperties__getX(this->_positionProperties) - this->_height / 2,
			PositionProperties__getY(this->_positionProperties) + this->_height / 2};
	this->_corners[2] = (coord_t) {PositionProperties__getX(this->_positionProperties) + this->_height / 2, 
			PositionProperties__getY(this->_positionProperties) + this->_height / 2};

	return 0;
}

PositionProperties_h_t Triangle__getPositionProperties(Triangle_h_t handle) {

	return ((struct Triangle*) handle)->_positionProperties;
}
coord_t* Triangle__getCorners(Triangle_h_t handle) {
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

Message_h_t Message__init(coord_t center, char *text, unsigned int color) {
	struct Message *this = malloc(sizeof(struct Message));
	
	this->_positionProperties = PositionProperties__init(center.x, center.y, color);
	this->_text = text;
	tumGetTextSize(text, &(this->_textWidth), &(this->_textHeight));

	this->_topLeftCorner = (coord_t) {center.x - this->_textWidth / 2, center.y - this->_textHeight / 2};

	return (Message_h_t) this;
}

int Message__destruct(Message_h_t *handle) {
	PositionProperties__destruct(&((struct Message *) *handle)->_positionProperties);
	free((struct Message *) *handle);
	*handle = NULL;

	return 0;
}

Message_h_t Message__initTopLeftCorner(coord_t topLeftCorner, char *text, unsigned int color) {
	struct Message *this = malloc(sizeof(struct Message));

	this->_text = text;
	tumGetTextSize(text, &(this->_textWidth), &(this->_textHeight));

	this->_topLeftCorner = topLeftCorner;

	this->_positionProperties = PositionProperties__init(topLeftCorner.x + this->_textWidth / 2,
														 topLeftCorner.y + this->_textHeight / 2,
														 color);

	return (Message_h_t) this;
}

int Message__updateCorner(Message_h_t handle) {
	struct Message *this = (struct Message*) handle;

	this->_topLeftCorner = (coord_t) {PositionProperties__getX(this->_positionProperties) - this->_textWidth / 2,
									  PositionProperties__getY(this->_positionProperties) - this->_textHeight / 2};

	return 0;
}


int Message__setText(Message_h_t handle, char *text) {
	((struct Message*) handle)->_text = text;

	return 0;
}


PositionProperties_h_t Message__getPositionProperties(Message_h_t handle) {
	return ((struct Message*) handle)->_positionProperties;
}
char* Message__getText(Message_h_t handle) {
	return ((struct Message*) handle)->_text;
}
coord_t Message__getTopLeftCorner(Message_h_t handle) {
	return ((struct Message*) handle)->_topLeftCorner;
}
int Message__getTextHeight(Message_h_t handle) {
	return ((struct Message*) handle)->_textHeight;
}