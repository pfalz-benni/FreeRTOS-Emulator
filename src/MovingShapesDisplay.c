#include "MovingShapesDisplay.h"

//Pointers to pointers have to be passed as variables for the shape
//objects because they are not only adapted but actually newly created.
//As a result the adresses pointed to have to be edited, hence be passed by refetence
void resetMovingObjectsAfterResumption(Circle_h_t *circle, Rectangle_h_t *rectangle, Message_h_t *topMessage,
			   coord_t *mobileScreenCenter, TickType_t *xLastWakeTime, TickType_t *initialWakeTime) {
	if(xSemaphoreTake(movingShapesDisplayTaskResumed.lock, portMAX_DELAY) == pdTRUE) {
		if (movingShapesDisplayTaskResumed.value) {
			Circle__destruct(circle);
			Rectangle__destruct(rectangle);
			Message__destruct(topMessage);

			*circle = Circle__init((coord_t) {mobileScreenCenter->x - DISTANCE_TO_TRIANGLE, mobileScreenCenter->y},
									RADIUS_CIRCLE, TUMBlue);
			*rectangle = Rectangle__init((coord_t) {mobileScreenCenter->x + DISTANCE_TO_TRIANGLE, mobileScreenCenter->y}, 
										HEIGHT_SQUARE, HEIGHT_SQUARE, Olive);
			*topMessage = Message__init((coord_t) {mobileScreenCenter->x,
					mobileScreenCenter->y - SCREEN_CENTER.y + DISTANCE_VERTICAL_TO_BORDER},
					"Hello, I can move!", Black);

			*initialWakeTime = *xLastWakeTime;

			movingShapesDisplayTaskResumed.value = 0;
		}
		xSemaphoreGive(movingShapesDisplayTaskResumed.lock);
	}

}

void updateShapeAndMessagePositions(Circle_h_t circle, Rectangle_h_t rectangle, Message_h_t topMessage,
									TickType_t xLastWakeTime, TickType_t prevWakeTime, TickType_t initialWakeTime) {
	PositionProperties__setSpeedMoveOnCircle(Circle__getPositionProperties(circle),
		DISTANCE_TO_TRIANGLE, M_PI, TIME_ORBIT_PERIOD, xLastWakeTime - initialWakeTime);
	PositionProperties__updatePosition(Circle__getPositionProperties(circle),
										xLastWakeTime - prevWakeTime);


	PositionProperties__setSpeedMoveOnCircle(Rectangle__getPositionProperties(rectangle),
		DISTANCE_TO_TRIANGLE, 0, TIME_ORBIT_PERIOD, xLastWakeTime - initialWakeTime);
	PositionProperties__updatePosition(Rectangle__getPositionProperties(rectangle),
										xLastWakeTime - prevWakeTime);
	Rectangle__updateCorner(rectangle);


	PositionProperties__moveVetically(Message__getPositionProperties(topMessage), SPEED_MOVING_MESSAGE,
										DISTANCE_HORIZONTAL_TO_BORDER);
	PositionProperties__updatePosition(Message__getPositionProperties(topMessage),
										xLastWakeTime - prevWakeTime);
	Message__updateCorner(topMessage);
}

void drawShapes(Circle_h_t circle, Rectangle_h_t rectangle, Triangle_h_t triangle) {
	tumDrawCircle(PositionProperties__getX(Circle__getPositionProperties(circle)),
					PositionProperties__getY(Circle__getPositionProperties(circle)),
					Circle__getRadius(circle),
					PositionProperties__getColor(Circle__getPositionProperties(circle)));

	tumDrawFilledBox(Rectangle__getTopLeftCorner(rectangle).x,
						Rectangle__getTopLeftCorner(rectangle).y,
						Rectangle__getWidth(rectangle),
						Rectangle__getHeight(rectangle),
						PositionProperties__getColor(Rectangle__getPositionProperties(rectangle)));

	tumDrawTriangle(Triangle__getCorners(triangle),
					PositionProperties__getColor(Triangle__getPositionProperties(triangle)));
}

void drawSimpleTextMessages(Message_h_t topMessage, Message_h_t bottomMessage) {
	tumDrawText(Message__getText(bottomMessage), Message__getTopLeftCorner(bottomMessage).x,
				Message__getTopLeftCorner(bottomMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(bottomMessage)));

	tumDrawText(Message__getText(topMessage), Message__getTopLeftCorner(topMessage).x,
				Message__getTopLeftCorner(topMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(topMessage)));
}

void drawButtonPressMessage(Message_h_t buttonPressMessage, buttonPresses_t buttonPressCount) {
	char formatedText[LENGTH_STRINGS_DRAWN];

	if (xSemaphoreTake(buttonPressCount.lock, portMAX_DELAY) == pdTRUE) {
		sprintf(formatedText, "A: %u |  B: %u |  C: %u |  D: %u", buttonPressCount.valuesABCD[0],
				buttonPressCount.valuesABCD[1],buttonPressCount.valuesABCD[2],buttonPressCount.valuesABCD[3]);
		xSemaphoreGive(buttonPressCount.lock);
	}

	Message__setText(buttonPressMessage, formatedText);

	tumDrawText(Message__getText(buttonPressMessage), Message__getTopLeftCorner(buttonPressMessage).x,
				Message__getTopLeftCorner(buttonPressMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(buttonPressMessage)));
}

void drawMouseCoordMessage(Message_h_t mouseCoordMessage) {
	char formatedText[LENGTH_STRINGS_DRAWN];

	sprintf(formatedText, "X-axis: %d |  Y-axis: %d", tumEventGetMouseX(), tumEventGetMouseY());
	Message__setText(mouseCoordMessage, formatedText);

	tumDrawText(Message__getText(mouseCoordMessage), Message__getTopLeftCorner(mouseCoordMessage).x,
				Message__getTopLeftCorner(mouseCoordMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(mouseCoordMessage)));
}

void moveScreenInCursorDirection(coord_t *mobileScreenCenter, Circle_h_t circle,
		Rectangle_h_t rectangle, Triangle_h_t triangle, Message_h_t bottomMessage,
		Message_h_t topMessage, Message_h_t buttonPressMessage, Message_h_t mouseCoordMessage) {

	coord_t oldScreenCenter = *mobileScreenCenter;

	// calculate position of new screen center depending on cursor position
	mobileScreenCenter->x = SCREEN_CENTER.x - (SCREEN_CENTER.x - tumEventGetMouseX()) / 
		(double) RATION_CURSER_CENTER;
	mobileScreenCenter->y = SCREEN_CENTER.y - (SCREEN_CENTER.y - tumEventGetMouseY()) /	
		(double) RATION_CURSER_CENTER;
	
	// update all objects' coordinates according to new screen center
	PositionProperties__adjustPositionToNewScreenCenter(
		Circle__getPositionProperties(circle), oldScreenCenter, *mobileScreenCenter);

	PositionProperties__adjustPositionToNewScreenCenter(
		Rectangle__getPositionProperties(rectangle), oldScreenCenter, *mobileScreenCenter);
	Rectangle__updateCorner(rectangle);

	PositionProperties__adjustPositionToNewScreenCenter(
		Triangle__getPositionProperties(triangle), oldScreenCenter, *mobileScreenCenter);
	Triangle__updateCorners(triangle);

	PositionProperties__adjustPositionToNewScreenCenter(
		Message__getPositionProperties(bottomMessage), oldScreenCenter, *mobileScreenCenter);
	Message__updateCorner(bottomMessage);

	PositionProperties__adjustPositionToNewScreenCenter(
		Message__getPositionProperties(topMessage), oldScreenCenter, *mobileScreenCenter);
	Message__updateCorner(topMessage);

	PositionProperties__adjustPositionToNewScreenCenter(
		Message__getPositionProperties(buttonPressMessage), oldScreenCenter, *mobileScreenCenter);
	Message__updateCorner(buttonPressMessage);

	PositionProperties__adjustPositionToNewScreenCenter(
		Message__getPositionProperties(mouseCoordMessage), oldScreenCenter, *mobileScreenCenter);
	Message__updateCorner(mouseCoordMessage);
}

void vMovingShapesDisplayTask(void *pvParameters) {
	// Task initializations:
    TickType_t xLastWakeTime, prevWakeTime, initialWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    prevWakeTime = xLastWakeTime;
    initialWakeTime = xLastWakeTime;

	//Center of the screen that is moved around by the mouse cursor
	coord_t mobileScreenCenter = SCREEN_CENTER;

    // Create three shapes
    Circle_h_t circle = Circle__init((coord_t) {mobileScreenCenter.x - DISTANCE_TO_TRIANGLE, mobileScreenCenter.y},
                    	RADIUS_CIRCLE, TUMBlue);

    Rectangle_h_t rectangle = Rectangle__init((coord_t) {mobileScreenCenter.x + DISTANCE_TO_TRIANGLE, mobileScreenCenter.y}, 
                    		  HEIGHT_SQUARE, HEIGHT_SQUARE, Olive);

    Triangle_h_t triangle = Triangle__init(mobileScreenCenter, HEIGHT_TRIANGLE, Magenta);

	//Create messages
	Message_h_t bottomMessage = Message__init((coord_t) {mobileScreenCenter.x, SCREEN_HEIGHT - DISTANCE_VERTICAL_TO_BORDER},
				  							  "Hang loose or press [Q] to quit!", Black);

	Message_h_t topMessage = Message__init((coord_t) {mobileScreenCenter.x,
			mobileScreenCenter.y - SCREEN_CENTER.y + DISTANCE_VERTICAL_TO_BORDER},
			"Hello, I can move!", Black);

	//Create message for buttons and mouse coordinates
	Message_h_t buttonPressMessage = Message__initTopLeftCorner(COORD_BUTTON_PRESS_MESSAGE,
										"A: 0 |  B: 0 |  C: 0 |  D: 0", Black);
	
	Message_h_t mouseCoordMessage = Message__initTopLeftCorner((coord_t) {COORD_BUTTON_PRESS_MESSAGE.x,
										COORD_BUTTON_PRESS_MESSAGE.y + Message__getTextHeight(buttonPressMessage)},
										"X-axis: 0 |  Y-axis: 0", Black);

	while (1) {
		xLastWakeTime = xTaskGetTickCount();

		//Screen manipulatoins
		xSemaphoreTake(ScreenLock, portMAX_DELAY);

		tumDrawClear(White); // Clear screen

		//Set mobileScreenCenter to coordinates indicated by mouse cursor
		moveScreenInCursorDirection(&mobileScreenCenter, circle, rectangle, 
				triangle, bottomMessage, topMessage, buttonPressMessage,
				mouseCoordMessage);
		
		//update positions before drawing objcts
		updateShapeAndMessagePositions(circle, rectangle, topMessage, xLastWakeTime,
									   prevWakeTime, initialWakeTime);

		//reset moving objects if task has been resumed after
		//it has been suspended
		resetMovingObjectsAfterResumption(&circle, &rectangle, &topMessage, &mobileScreenCenter,
						   &xLastWakeTime, &initialWakeTime);

		//Draw all objects in appropriate position
		drawShapes(circle, rectangle, triangle);
		drawSimpleTextMessages(topMessage, bottomMessage);
		drawButtonPressMessage(buttonPressMessage, buttonPressCount);
		drawMouseCoordMessage(mouseCoordMessage);

		xSemaphoreGive(ScreenLock);

		// Basic sleep to free CPU
		vTaskDelay(portTICK_PERIOD_MS * TIME_MOVINGSHAPESDISPLAYTASK_DELAY_MS);

        prevWakeTime = xLastWakeTime; // to keep track of time intervalls
	}
}