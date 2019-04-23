#include "main.h"
#include "stm32f0xx_hal.h"

extern ADC_HandleTypeDef hadc;
#define VREF_CAL                         *(__IO uint16_t *)(0x1FFFF7BA) 
#define FILTER_CNT    10

uint32_t adc_get_value(uint8_t ch)
{
    uint32_t adc_val[3] = {0};
    uint32_t adc_ch0_val = 0, adc_ch1_val = 0;
    uint8_t i, j;
    for(j = 0; j < FILTER_CNT; j++) {
        HAL_ADC_Start(&hadc);
        if(HAL_ADC_PollForConversion(&hadc, 100) != HAL_OK)
            return 0;
        adc_ch0_val = HAL_ADC_GetValue(&hadc);
        adc_val[0] += adc_ch0_val;
      
        HAL_ADC_Start(&hadc);
        if(HAL_ADC_PollForConversion(&hadc, 100) != HAL_OK)
            return 0;
        adc_ch1_val = HAL_ADC_GetValue(&hadc);
        adc_val[1] += adc_ch1_val;

        // printf("adc_val = %d \t %d\n", adc_ch0_val, adc_ch1_val);
        HAL_ADC_Stop(&hadc);
    }
    return (adc_val[ch] / FILTER_CNT);
}


void adc_test(void)
{
    static int cnt = 0;
    static float sum = 0.0f;
    uint32_t adc_ch0_val = 0, adc_ref_val = 0, vref_val = 0;
    float voltage = 0.00f;
    vref_val = VREF_CAL;
    HAL_ADC_Start(&hadc);
    if(HAL_ADC_PollForConversion(&hadc, 100) != HAL_OK)
        return ;
    adc_ch0_val = HAL_ADC_GetValue(&hadc);
    
    HAL_ADC_Start(&hadc);
    if(HAL_ADC_PollForConversion(&hadc, 100) != HAL_OK)
        return ;    
    adc_ref_val = HAL_ADC_GetValue(&hadc);

    voltage = (float)((3.3f*vref_val*adc_ch0_val)/(adc_ref_val*4095));
    sum += voltage;
    cnt++;
    if(cnt == 10) {
        printf("-----voltage = %f -----------\r\n", (float)(sum / 10));
    }
    printf("voltage = %f \t %f\r\n", voltage, (voltage/0.666));
}


uint32_t adc_once_dat[2] = {0};
int get_once_adc_val(void)
{
    HAL_ADC_Start(&hadc);
    if(HAL_ADC_PollForConversion(&hadc, 5) != HAL_OK)
        return -1;
    adc_once_dat[0] = HAL_ADC_GetValue(&hadc);
    
    HAL_ADC_Start(&hadc);
    if(HAL_ADC_PollForConversion(&hadc, 5) != HAL_OK)
        return -1;
    adc_once_dat[1] = HAL_ADC_GetValue(&hadc);
    HAL_ADC_Stop(&hadc);
    return 0;
}