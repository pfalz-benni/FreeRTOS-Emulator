#include "Shapes.h"

int TriangleCoords__init(TriangleCoords_t *this, coord_t center, int height)
{
	this->_topCorner.x = center.x;
    this->_topCorner.y = center.y - height / 2;
	this->_leftCorner.x = center.x - height / 2;
	this->_leftCorner.y = center.y + height / 2;
	this->_rightCorner.x = center.x + height / 2;
	this->_rightCorner.y = center.y + height / 2;
    
	return 0;
}