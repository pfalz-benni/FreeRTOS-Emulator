/**
 * @file otherFunctions.c
 * @author Benedikt Witteler
 * @date 28 November 2020
 * @brief Written functions that are not in use. This file serves to store them.
 */

#include "Shapes.h"

// Funktion kann nicht vernünftig definiert werden, weil an dieser Stelle "struct PositionProperties"
// unbekannt ist
// double calculatePhase(PositionProperties_h_t *handle, PIXELS radius) {
// 	struct PositionProperties *this = (struct PositionProperties *) handle;

//     double phi;// Phase
// 	if (SCREEN_CENTER.y - this->_f_y >= 0) { // Calculation of the argument of the center point in the complex plain
// 		double a = this->_f_x - SCREEN_CENTER.x;
// 		if (!round(a))
// 			phi = M_PI / 2;
// 		else
// 			phi = acos(a / radius);
// 	}
// 	else {
// 		double a = this->_f_x - SCREEN_CENTER.x;
// 		if (!round(a))
// 			phi = M_PI * 3 / 2;
// 		else
// 			phi = (-1) *acos(a / radius);
// 	}

// 	return phi;
// }