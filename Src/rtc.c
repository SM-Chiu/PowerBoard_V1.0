#include "main.h"
#include "stm32f0xx_hal.h"

extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef rtc_getTime(void)
{
    RTC_TimeTypeDef stimestructureget;
    RTC_DateTypeDef sdatestructureget;
    HAL_RTC_GetTime(&hrtc,&stimestructureget,RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc,&sdatestructureget,RTC_FORMAT_BIN);
    return stimestructureget;
}