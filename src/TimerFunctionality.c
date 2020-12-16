#include "TimerFunctionality.h"

void deleteButtonCountNSCallback(TimerHandle_t deleteButtonCountNS)
{
	xTaskNotifyGive(ButtonPressResetTask);
}