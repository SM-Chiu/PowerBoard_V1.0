#include "main.h"
#include "stm32f0xx_hal.h"
#include "stdio.h"

#define CPU_SER_PORT  GPIOA
#define CPU_SER_PIN   GPIO_PIN_2
#define CPU_SER_H()   HAL_GPIO_WritePin(CPU_SER_PORT, CPU_SER_PIN, GPIO_PIN_SET)
#define CPU_SER_L()   HAL_GPIO_WritePin(CPU_SER_PORT, CPU_SER_PIN, GPIO_PIN_RESET)

#define SRCLK_PORT  GPIOA
#define SRCLK_PIN   GPIO_PIN_5
#define SRCLK_H()   HAL_GPIO_WritePin(SRCLK_PORT, SRCLK_PIN, GPIO_PIN_SET)
#define SRCLK_L()   HAL_GPIO_WritePin(SRCLK_PORT, SRCLK_PIN, GPIO_PIN_RESET)

#define RCLK_PORT  GPIOA
#define RCLK_PIN   GPIO_PIN_6
#define RCLK_H()   HAL_GPIO_WritePin(RCLK_PORT, RCLK_PIN, GPIO_PIN_SET)
#define RCLK_L()   HAL_GPIO_WritePin(RCLK_PORT, RCLK_PIN, GPIO_PIN_RESET)

#define OE_PORT  GPIOB
#define OE_PIN   GPIO_PIN_1
#define OE_H()   HAL_GPIO_WritePin(OE_PORT, OE_PIN, GPIO_PIN_SET)
#define OE_L()   HAL_GPIO_WritePin(OE_PORT, OE_PIN, GPIO_PIN_RESET)

#define SRCLR_PORT  GPIOA
#define SRCLR_PIN   GPIO_PIN_7
#define SRCLR_H()   HAL_GPIO_WritePin(SRCLR_PORT, SRCLR_PIN, GPIO_PIN_SET)
#define SRCLR_L()   HAL_GPIO_WritePin(SRCLR_PORT, SRCLR_PIN, GPIO_PIN_RESET)


/*����*/
// const unsigned char seg_tbl[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71};
const unsigned char seg_tbl[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x88, 0x83, 0xc6, 0xa1, 0x86, 0x8e, 0xff};
/*λ��*/
const unsigned char seg_com[] = {0xef,0xdf,0xbf,0x7f,0xfe,0xfd,0xfb,0xf7, 0xff};


void CD74HC595_OutPutData(unsigned char dat)
{
  unsigned int i=1;
  for(i = 0; i < 8; i++) {
    SRCLK_L();	
    if(dat & 0x80) {
      CPU_SER_H();
    } 
    else {
      CPU_SER_L();
    }
    dat <<= 1;
    SRCLK_H();
  }
}

void SEG_ShowOneChar(uint8_t pos, uint8_t num, uint8_t dec)
{
  CD74HC595_OutPutData(seg_com[pos]);
  if(dec)
    CD74HC595_OutPutData((seg_tbl[num] & 0x7F));
  else
    CD74HC595_OutPutData(seg_tbl[num]);
  RCLK_L();
  HAL_Delay(1);
  RCLK_H();
}

void CD74HC595_Init(void)
{

  int i;
  OE_H();
  SRCLR_H();
  OE_L();
  for(i=0;i<8;i++)
  {
    SEG_ShowOneChar(i,16,0);
  }
  
}

/* float value max:99.99  min 0.00 */
void SEG_ShwoFloat(uint8_t pos, uint32_t dat)
{
  int i;
  uint8_t buf_idx[4] = {0};
  int integer,pointer;
  if(dat != 0xFFFFFFFF)
  {
    integer = (int)(dat % 10000);
    pointer = 1;
    if (pos != 0)
    {
      if(integer >= 1000)  
        buf_idx[0] = integer / 1000;
      else
        buf_idx[0] = 16;
    }
    else
      buf_idx[0] = integer / 1000;

    buf_idx[1] = integer / 100 % 10;
    buf_idx[2] = integer / 10 % 10;
    buf_idx[3] = integer % 10;
    if(pos == 0)
      pointer = 0;
  }
  else {
    memset(buf_idx, 16, 4);
    pointer = 5;
  }

  for(i = 0; i < 4; i++) {
    if(i == pointer)
      SEG_ShowOneChar(pos + i, buf_idx[i], 1);
    else
      SEG_ShowOneChar(pos + i, buf_idx[i], 0);  
  }
}

void SEG_ShowPoint(uint8_t pos, uint32_t dat)
{
  int i;
  for(i = 0; i < 4; i++)
  {
    SEG_ShowOneChar(pos + i, 16, 1);
  }
}

#define ZOOM 100
void SEG_ShowPower(float product_cumul)
{
  int i;
  uint8_t buf_idx[4] = {0};
  for(i = 0; i < 2; i++)
  {
    SEG_ShowOneChar(i, 16, 1);
  }
  buf_idx[0] = ((int)product_cumul)/10;
  buf_idx[1] = ((int)product_cumul)%10;
  buf_idx[2] = ((int)(product_cumul*ZOOM))%ZOOM/10;
  buf_idx[3] = ((int)(product_cumul*ZOOM))%ZOOM%10;
  for(i = 2; i < 6; i++)
  {
    if(i == 3)
    SEG_ShowOneChar(i, buf_idx[i-2], 1);
    else
    SEG_ShowOneChar(i, buf_idx[i-2], 0);
  }
  for(i = 6;i < 8; i++)
  {
    SEG_ShowOneChar(i, 16, 1);
  }
}