#include "main.h"
#include "stm32f0xx_hal.h"
#include "math.h"
#include "stdbool.h"

uint32_t voltage = 0;
uint32_t current = 0;
float product_cumul = 0.0f;

extern uint32_t adc_once_dat[2];
uint32_t adc_val_filter_1ms[2][10] = {0};
uint32_t adc_val_filter_10ms[2][10] = {0};
uint32_t adc_val_filter_100ms[2][5] = {0};
float adc_ref_val = 0;

void BubbleSort(uint32_t *array, uint32_t len)
{
    uint32_t i, j, temp;
    for (i = 0; i < len; i++) {
        for (j = i + 1; j < len; j++) {
            if (array[i] > array[j]) {
                temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
}
int get_once_adc_val(void);
void rt_calc_data(void)
{
    static uint32_t cnt_1ms = 0;
    static uint32_t cnt_10ms = 0;
    static uint32_t cnt_100ms = 0;
    static uint32_t adc_ch0_val = 0, adc_ch1_val = 0;
    float vol = 0.0f, curr = 0.0f,  tmp = 0.0f;
    int i, ret;
 
    ret = get_once_adc_val();
    if(ret < 0)
      return ;
    adc_val_filter_1ms[0][cnt_1ms] = adc_once_dat[0];
    //printf("adc0=%d\n",adc_val_filter_1ms[0][cnt_1ms]);
    
    adc_val_filter_1ms[1][cnt_1ms] = adc_once_dat[1];
     
    cnt_1ms++;
    if(cnt_1ms < 10) {
        return;
    } 
    else if(cnt_1ms == 10) /* 10ms */
    {
        BubbleSort(adc_val_filter_1ms[0], 10);
        BubbleSort(adc_val_filter_1ms[1], 10);
        adc_ch0_val = 0;
        adc_ch1_val = 0;
        for(i = 1; i < 9; i++)
        {
          adc_ch0_val += adc_val_filter_1ms[0][i];
          adc_ch1_val += adc_val_filter_1ms[1][i];
        }
        adc_ch0_val /= 8;
        adc_ch1_val /= 8;
        //printf("adc0=%d\t adc1=%d\n",adc_ch0_val,adc_ch1_val);

        cnt_1ms = 0;
        adc_val_filter_10ms[0][cnt_10ms] = adc_ch0_val;
        adc_val_filter_10ms[1][cnt_10ms] = adc_ch1_val;
        cnt_10ms++;
    }
    
    
    if(cnt_10ms == 10) /* 100ms */
    {
        cnt_10ms = 0;
        adc_ch0_val = 0;
        adc_ch1_val = 0;
        for(i = 0; i < 10; i++)
        {
          adc_ch0_val += adc_val_filter_10ms[0][i];
          adc_ch1_val += adc_val_filter_10ms[1][i];
        }
        adc_ch0_val /= 10;
        adc_ch1_val /= 10;
        if(cnt_100ms == 0)
        {
          adc_val_filter_100ms[0][cnt_100ms] = adc_ch0_val;
          adc_val_filter_100ms[1][cnt_100ms] = adc_ch1_val;
        }
        else {
          adc_val_filter_100ms[0][cnt_100ms] = adc_val_filter_100ms[0][cnt_100ms-1]*0.3 + adc_ch0_val*0.7;
          adc_val_filter_100ms[1][cnt_100ms] = adc_val_filter_100ms[1][cnt_100ms-1]*0.3 + adc_ch1_val*0.7;
        }
        cnt_100ms ++;
    }

    if(cnt_100ms == 5) /* 500ms */
    {
      cnt_100ms = 0;
      adc_ch0_val = 0;
      adc_ch1_val = 0;
      for(i = 0; i < 5; i++)
      {
        adc_ch0_val += adc_val_filter_100ms[0][i];
        adc_ch1_val += adc_val_filter_100ms[1][i];
      }

      adc_ch0_val /= 5;
      adc_ch0_val = (adc_ch0_val+4.88)/1.004; /* 人工AD校准 */
      adc_ch0_val = (adc_ch0_val+4.8175)/0.9985; /* 人工AD校准 */
      adc_ch0_val -= 17;  /* 人工AD校准 */
      adc_ch1_val /= 5;
      adc_ch1_val = (adc_ch1_val+4.88)/1.004;  /* 人工AD校准 */
      adc_ch1_val = (adc_ch1_val+4.8175)/0.9985;  /* 人工AD校准 */
      adc_ch1_val += 5;  /* 人工AD校准 */
      //printf("adc0=%d\t adc1=%d\n",adc_ch0_val,adc_ch1_val);

      // float vol =  (float)(adc_ch0_val)*3.3/4096;
      // vol = vol /0.666 - 0.5;
      // vol = ((vol + 0.0005))/0.4;
      // voltage =  vol*1000;

      // vol =  (float)adc_ch1_val*3.3/4096;
      // current =  (vol/0.33 + 0.005)*1000;

      tmp =  (float)(adc_ch0_val)*3.3/4096; 
      tmp = tmp - adc_ref_val;
      if(tmp < 0) 
        curr = 0;
      else 
      {
        curr = tmp/0.4;
        if(curr < 0.196)
        {
          curr -= 0.003;
          if (curr < 0) 
            curr = 0;          
        }
        else
        {
          curr -= (0.014 * curr + 0.003);
        }
      }
        
      //printf("I=%d\n",(int)(curr*1000));

      tmp =  (float)adc_ch1_val*3.3/4096;
      vol =  tmp/0.334;
        //printf("V=%d\n",(int)(tmp*1000));

      voltage = curr*1000;
      current = vol*100;
      // printf("adc_val = %d \t %d \t\tcalc_val = %dmv \t %dmv\n", 
      //   adc_ch0_val, adc_ch1_val, voltage, current);
    }
}


#define ZOOM_NUM  1000
uint32_t  power_idx = 0;
// float product_copy = 0.0f;
// RTC_TimeTypeDef stimestructureget={};
// float vol_copy = 0.0f;
// bool cnt_1s_flag = false;
// RTC_TimeTypeDef rtc_getTime(void);
void rt_calc_power(void)
{
    static uint32_t cnt_1ms = 0;
    static uint32_t cnt_10ms = 0;
    static uint32_t cnt_50ms = 0;
    uint32_t adc_ch0_val = 0, adc_ch1_val = 0;
    float vol = 0.0f, curr = 0.0f,  tmp = 0.0f;
    static float product = 0.0f;
    // RTC_TimeTypeDef stimestructureget;
    // static float product_cumul = 0.0f;
    int i, ret;
    // product = 0.0f;   //重置product = 0；

    ret = get_once_adc_val();
   
    if(ret < 0)
      return ;
    
    adc_val_filter_1ms[0][cnt_1ms] = adc_once_dat[0];    
    adc_val_filter_1ms[1][cnt_1ms] = adc_once_dat[1];  
    cnt_1ms++;
    if(cnt_1ms < 10) {
        return;
    } 
    else if(cnt_1ms == 10) /* 10ms */
    {
        BubbleSort(adc_val_filter_1ms[0], 10);
        BubbleSort(adc_val_filter_1ms[1], 10);
        adc_ch0_val = 0;
        adc_ch1_val = 0;
        for(i = 1; i < 9; i++)
        {
          adc_ch0_val += adc_val_filter_1ms[0][i];
          adc_ch1_val += adc_val_filter_1ms[1][i];
        }
        adc_ch0_val /= 8;
        adc_ch1_val /= 8;
        cnt_1ms = 0;
    }
    
    if(cnt_10ms == 0) 
    {
        adc_val_filter_10ms[0][cnt_10ms] = adc_ch0_val;
        adc_val_filter_10ms[1][cnt_10ms] = adc_ch1_val;
    }
    else 
    {
        adc_val_filter_10ms[0][cnt_10ms] = adc_val_filter_10ms[0][cnt_10ms-1]*0.3 + adc_ch0_val*0.7;
        adc_val_filter_10ms[1][cnt_10ms] = adc_val_filter_10ms[1][cnt_10ms-1]*0.3 + adc_ch1_val*0.7;
    }
    cnt_10ms++;
    if(cnt_10ms == 5)
    {
        adc_ch0_val = 0;
        adc_ch1_val = 0;
        for(i = 0; i < 5; i++)
        {
          adc_ch0_val += adc_val_filter_10ms[0][i];
          adc_ch1_val += adc_val_filter_10ms[1][i];
        }

        adc_ch0_val /= 5;
        adc_ch0_val = (adc_ch0_val+4.88)/1.004; /* 人工AD校准 */
        adc_ch0_val = (adc_ch0_val+4.8175)/0.9985; /* 人工AD校准 */
        adc_ch0_val -= 17;  /* 人工AD校准 */
        adc_ch1_val /= 5;
        adc_ch1_val = (adc_ch1_val+4.88)/1.004;  /* 人工AD校准 */
        adc_ch1_val = (adc_ch1_val+4.8175)/0.9985;  /* 人工AD校准 */
        adc_ch1_val += 5;  /* 人工AD校准 */
        //printf("adc0=%d\t adc1=%d\n",adc_ch0_val,adc_ch1_val);
        cnt_10ms = 0;

        tmp =  (float)(adc_ch0_val)*3.3/4096; 
        tmp = tmp - adc_ref_val;
        if(tmp < 0) 
          curr = 0;
        else 
        {
          curr = tmp/0.4;
          if(curr < 0.196)
          {
            curr -= 0.003;
            if (curr < 0) 
              curr = 0;          
          }
          else
          {
            curr -= (0.014 * curr + 0.003);
          }
        }
        
        //printf("I=%d\n",(int)(curr*1000));

        tmp =  (float)adc_ch1_val*3.3/4096;
        vol =  tmp/0.334;
        //printf("V=%d\n",(int)(tmp*1000));

        product += (vol*curr);
        cnt_50ms++;

        // printf("adc val: %d \t %d\r\n", adc_ch0_val, adc_ch1_val);
    }

    if(cnt_50ms == 20) /* 1s */
    {
        // cnt_1s_flag = true;
        // stimestructureget = rtc_getTime();
        product /= 20;
        // product_copy = product;
        // product_cumul += product;
        power_idx++,
        // vol_copy = vol;
        printf("%d \t%d.%03d\tW \t%d.%04d\tA \t%d.%03d\tV\r\n",
            power_idx,
            (int)product,
            (int)(product*ZOOM_NUM)%ZOOM_NUM,
            (int)(product/vol),
            (int)((product/vol)*ZOOM_NUM*10)%(ZOOM_NUM*10), 
            (int)vol,
            (int)(vol*ZOOM_NUM)%ZOOM_NUM
        );

          // printf("hello\t\r\n");
        //  printf("P:%d \t I:%d.%03d \t V:%d\r\n", (int)(product*1000), (int)(curr), (int)(curr*1000)%1000, (int)(vol*1000));
        cnt_50ms = 0;
        product = 0;
    }
}


extern ADC_HandleTypeDef hadc;
void CalibrationRefVol(void)
{
    uint32_t adc_val = 0;
    uint32_t cnt = 50;
    ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.LowPowerAutoWait = ENABLE;
    hadc.Init.LowPowerAutoPowerOff = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = ENABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
    if (HAL_ADC_Init(&hadc) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

      /**Configure for the selected ADC regular channel to be converted. 
      */
    sConfig.Channel = ADC_CHANNEL_3;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    HAL_ADCEx_Calibration_Start(&hadc);

    do {
      HAL_ADC_Start(&hadc);
      if(HAL_ADC_PollForConversion(&hadc, 5) != HAL_OK)
        continue;
      else
        cnt --;
      adc_val += HAL_ADC_GetValue(&hadc);
      HAL_ADC_Stop(&hadc);
    } while(cnt);

    //printf("adc_ref=%d\n",adc_val / 50);
    adc_ref_val = (float)(adc_val / 50);
    adc_ref_val = (adc_ref_val+4.88)/1.004; /* 人工AD校准 */
    adc_ref_val = (adc_ref_val+4.8175)/0.9985; /* 人工AD校准 */
    adc_ref_val = adc_ref_val*3.3/4096/5.769;  /* 人工AD校准 */
    printf("adc_ref_val: %d.%03d\r\n", (int)(adc_ref_val), (int)(adc_ref_val*1000)%1000);

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_ADC_DeInit(&hadc);
}