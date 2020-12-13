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

void testMemoryAllocAndFree() {
	Circle_h_t circleNull;
	printf("Adress:%p\n", circleNull);

	circleNull = Circle__init((coord_t) {100,100}, 50, TUMBlue);
	printf("Adress:%p\n", circleNull);
	printf("x=%d, y=%d\n", PositionProperties__getX(Circle__getPositionProperties(circleNull)),
		   PositionProperties__getY(Circle__getPositionProperties(circleNull)));

	Circle__destruct(&circleNull);
	printf("Adress:%p\n", circleNull);
	// printf("Rasius:%d\n", Circle__getRadius(circle));

	Circle_h_t circle;
	Rectangle_h_t rectangle;
	Triangle_h_t triangle;
	Message_h_t message;
	for (int i = 0; i < 50000; i++) {
		circle = Circle__init((coord_t) {100,100}, 50, TUMBlue);
		rectangle = Rectangle__init((coord_t) {200,200}, 50, 50, Black);
		triangle = Triangle__init((coord_t) {40,40}, 80, TUMBlue);
		message = Message__init((coord_t) {8,8},"Hello" , Black);

		Circle__destruct(&circle);
		Rectangle__destruct(&rectangle);
		Triangle__destruct(&triangle);
		Message__destruct(&message);

	}
}