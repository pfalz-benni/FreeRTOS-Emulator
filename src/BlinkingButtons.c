#include "BlinkingButtons.h"

#define FPS_AVERAGE_COUNT 50
#define FPS_FONT "IBMPlexSans-Bold.ttf"
#define DEFAULT_FONT_SIZE 15


void vDrawFPS(void) {
    static unsigned int periods[FPS_AVERAGE_COUNT] = { 0 };
    static unsigned int periods_total = 0;
    static unsigned int index = 0;
    static unsigned int average_count = 0;
    static TickType_t xLastWakeTime = 0, prevWakeTime = 0;
    static char str[10] = { 0 };
    static int text_width;
    int fps = 0;
    font_handle_t cur_font = tumFontGetCurFontHandle();

    if (average_count < FPS_AVERAGE_COUNT) {
        average_count++;
    }
    else {
        periods_total -= periods[index];
    }

    xLastWakeTime = xTaskGetTickCount();

    if (prevWakeTime != xLastWakeTime) {
        periods[index] =
            configTICK_RATE_HZ / (xLastWakeTime - prevWakeTime);
        prevWakeTime = xLastWakeTime;
    }
    else {
        periods[index] = 0;
    }

    periods_total += periods[index];

    if (index == (FPS_AVERAGE_COUNT - 1)) {
        index = 0;
    }
    else {
        index++;
    }

    fps = periods_total / average_count;

    tumFontSelectFontFromName(FPS_FONT);

    sprintf(str, "FPS: %2d", fps);

    if (!tumGetTextSize((char *)str, &text_width, NULL)) {
        tumDrawText(str, SCREEN_WIDTH - text_width - 10,
                SCREEN_HEIGHT - DEFAULT_FONT_SIZE * 1.5,
                Skyblue);
    }

    tumFontSelectFontFromHandle(cur_font);
    tumFontPutFontHandle(cur_font);
}

void drawButtonsNSPressMessage(Message_h_t buttonsNSPressMessage, buttonPresses_t buttonPressCountNS) {
	char formatedText[LENGTH_STRING_NS_DRAWN];
    int secondsPassedTotalCopy;

    if (xSemaphoreTake(secondsPassedTotal.lock, portMAX_DELAY == pdTRUE)) {
        secondsPassedTotalCopy = secondsPassedTotal.value;
        xSemaphoreGive(secondsPassedTotal.lock);
    }


	if (xSemaphoreTake(buttonPressCountNS.lock, portMAX_DELAY) == pdTRUE) {
		sprintf(formatedText, "N: %u |  S: %u           Seconds passed: %d", buttonPressCountNS.values[0],
				buttonPressCountNS.values[1], secondsPassedTotalCopy);
		xSemaphoreGive(buttonPressCountNS.lock);
	}

	Message__setText(buttonsNSPressMessage, formatedText);

	tumDrawText(Message__getText(buttonsNSPressMessage), Message__getTopLeftCorner(buttonsNSPressMessage).x,
				Message__getTopLeftCorner(buttonsNSPressMessage).y,
				PositionProperties__getColor(Message__getPositionProperties(buttonsNSPressMessage)));
}

void vBlinkingButtonsDrawTask(void *pvParameters) {
    Circle_h_t circleDynamic = Circle__init((coord_t)
            {SCREEN_CENTER.x - DISTANCE_CIRCLE_CENTER, SCREEN_CENTER.y},
            RADIUS_CIRCLES, Red);

    Circle_h_t circleStatic = Circle__init((coord_t)
            {SCREEN_CENTER.x + DISTANCE_CIRCLE_CENTER, SCREEN_CENTER.y},
            RADIUS_CIRCLES, Blue);
    
    Message_h_t buttonsNSPressMessage = Message__init((coord_t) {SCREEN_CENTER.x, 
            SCREEN_CENTER.y - DISTANCE_VERTICAL_MESSAGE_CENTER}, 
            "N: 0 |  S: 0           Seconds passed: 0", Black);
    
    uint32_t notification = 0;

    while(1) {
        xSemaphoreTake(ScreenLock, portMAX_DELAY);

        tumDrawClear(White);

        notification = ulTaskNotifyTake(pdTRUE, 0);

        //get the lowest bit
        if (notification & 0x01) {
            tumDrawCircle(PositionProperties__getX(Circle__getPositionProperties(circleDynamic)),
                    PositionProperties__getY(Circle__getPositionProperties(circleDynamic)),
                    Circle__getRadius(circleDynamic),
                    PositionProperties__getColor(Circle__getPositionProperties(circleDynamic)));
        }
        //get the second lowest bit
        if (notification & 0x02) {
            tumDrawCircle(PositionProperties__getX(Circle__getPositionProperties(circleStatic)),
                    PositionProperties__getY(Circle__getPositionProperties(circleStatic)),
                    Circle__getRadius(circleStatic),
                    PositionProperties__getColor(Circle__getPositionProperties(circleStatic)));
        }

        vDrawFPS();

        //draw button press counter
        drawButtonsNSPressMessage(buttonsNSPressMessage, buttonPressCountNS);


        xSemaphoreGive(ScreenLock);

        vTaskDelay(portTICK_PERIOD_MS * TIME_BLINKINGBUTTONSDRAWTASK_DELAY_MS);
    }
}

void vBlinkingButtonsDynamicTask(void *pvParameters) {
    TickType_t periodCounter = 0;

    while (1) {
        periodCounter = xTaskGetTickCount() % TIME_PERIOD_DYNAMIC_MS;

        if (periodCounter < TIME_PERIOD_DYNAMIC_MS / 2) {
            xTaskNotify(BlinkingButtonsDrawTask, 0x01, eSetBits);
        }

        // information is sent to the drawing task every tick
        vTaskDelay(1);
    }
}

void vBlinkingButtonsStaticTask(void *pvParameters) {
    TickType_t periodCounter = 0;

    while (1) {
        periodCounter = xTaskGetTickCount() % TIME_PERIOD_STATIC_MS;

        if (periodCounter < TIME_PERIOD_STATIC_MS / 2) {
            xTaskNotify(BlinkingButtonsDrawTask, 0x02, eSetBits);
        }

        // information is sent to the drawing task every tick
        vTaskDelay(1);
    }
}

void vButtonPressSemaphoreTask(void *pvParameters) {

    while (1) {
        if (xSemaphoreTake(ButtonSPressed, portMAX_DELAY) == pdTRUE) {
            if (xSemaphoreTake(buttonPressCountNS.lock, portMAX_DELAY) == pdTRUE) {
                buttonPressCountNS.values[1]++;
                xSemaphoreGive(buttonPressCountNS.lock);
	        }
        }
    }
}

void vButtonPressNotificationTask(void *pvParameters) {

    while (1) {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
            if (xSemaphoreTake(buttonPressCountNS.lock, portMAX_DELAY) == pdTRUE) {
                buttonPressCountNS.values[0]++;
                xSemaphoreGive(buttonPressCountNS.lock);
	        }
        }
    }
}

void vButtonPressResetTask(void *pvParameters) {

    while (1) {
        if(ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
            if (xSemaphoreTake(buttonPressCountNS.lock, portMAX_DELAY) == pdTRUE) {
                buttonPressCountNS.values[0] = 0;
                buttonPressCountNS.values[1] = 0;
                xSemaphoreGive(buttonPressCountNS.lock);
	        }
        }
    }
}

void vCountingSecondsTask(void *pvParameters) {
     
     
     while (1) {
         if (xSemaphoreTake(secondsPassedTotal.lock, portMAX_DELAY == pdTRUE)) {
             secondsPassedTotal.value++;
             xSemaphoreGive(secondsPassedTotal.lock);
         }

         vTaskDelay(portTICK_PERIOD_MS * 1000);
     }
}