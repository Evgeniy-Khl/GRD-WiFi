/*
 * XPT2046_touch.c
 *
 *  Created on: 20 sep. 2019.
 *      Author: Andriy Honcharenko
 */
#include <stdio.h>
#include <stdlib.h>
#include "..\Lib\Touch\XPT2046_touch.h"
#include "..\Lib\st7796\myLCD.h"
#include "..\Lib\st7796\myGUI.h"

//#if (ORIENTATION == 0)
//#define READ_X 0xD0
//#define READ_Y 0x90
//#elif (ORIENTATION == 1)
//#define READ_Y 0xD0
//#define READ_X 0x90
//#elif (ORIENTATION == 2)
//#define READ_X 0xD0
//#define READ_Y 0x90
//#elif (ORIENTATION == 3)
//#define READ_Y 0xD0
//#define READ_X 0x90
//#endif

//#define SPI_CLK_L()		HAL_GPIO_WritePin(TFT_SCK_GPIO_Port, TFT_SCK_Pin, GPIO_PIN_RESET)
//#define SPI_CLK_H()		HAL_GPIO_WritePin(TFT_SCK_GPIO_Port, TFT_SCK_Pin, GPIO_PIN_SET)
//#define SPI_MOSI_L()	HAL_GPIO_WritePin(TFT_MOSI_GPIO_Port, TFT_MOSI_Pin, GPIO_PIN_RESET)
//#define SPI_MOSI_H()	HAL_GPIO_WritePin(TFT_MOSI_GPIO_Port, TFT_MOSI_Pin, GPIO_PIN_SET)
//#define SPI_MISO()		HAL_GPIO_ReadPin(TFT_MISO_GPIO_Port, TFT_MISO_Pin)

extern _lcd_dev lcddev;
extern char buffTFT[];
extern uint8_t ticBeep;
extern uint16_t point_color;
uint16_t back_color;

#ifdef SOFTWARE_SPI
static void spi_write_byte(uint8_t data){
	for(size_t i = 0; i < 8; i++){
		if (data & 0x80) SPI_MOSI_H();
		else SPI_MOSI_L();
		data = data << 1;
		SPI_CLK_L();
		SPI_CLK_H();
	}
}

static uint8_t spi_read_byte(void){
	uint8_t i, ret , value;
	ret = 0;
	i = 8;

	do {
		i--;
		SPI_CLK_L();
		value = SPI_MISO();
		if (value)ret |= (1 << i);//set the bit to 0 no matter what
		SPI_CLK_H();
	} while (i > 0);
	return ret;
}
#endif

void XPT2046_TouchSelect(void){
  if(HAL_SPI_DeInit(&T_SPI_PORT) != HAL_OK) {
//    GUI_WriteString(5, lcddev.height-160, "HAL_SPI_DeInit failed!", Font_11x18, MAGENTA, back_color);
    return;
  }
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  if(HAL_SPI_Init(&T_SPI_PORT) != HAL_OK) {
//    GUI_WriteString(5, lcddev.height-140, "HAL_SPI_Init failed!", Font_11x18, MAGENTA, back_color);
    return;
  }

  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_RESET);
//  GUI_WriteString(5, lcddev.height-120, "TouchSelect() Ok!", Font_11x18, GREEN, back_color);
}

void XPT2046_TouchUnselect(void){
  if(HAL_SPI_DeInit(&T_SPI_PORT) != HAL_OK) {
//    GUI_WriteString(5, lcddev.height-160, "HAL_SPI_DeInit failed!", Font_11x18, MAGENTA, back_color);
  }
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  if(HAL_SPI_Init(&T_SPI_PORT) != HAL_OK) {
//    GUI_WriteString(5, lcddev.height-140, "HAL_SPI_Init failed!", Font_11x18, MAGENTA, back_color);
  }
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET);
//  GUI_WriteString(5, lcddev.height-100, "TouchUnselect() Ok!", Font_11x18, GREEN, back_color);
}

bool XPT2046_TouchPressed(void)
{
    return HAL_GPIO_ReadPin(T_IRQ_GPIO_Port, T_IRQ_Pin) == GPIO_PIN_RESET;// 0u
}

bool XPT2046_TouchGetCoordinates(uint16_t* x, uint16_t* y)
{
  XPT2046_TouchSelect();
//  ticBeep = 1;
  
//#ifndef SOFTWARE_SPI
static const uint8_t cmd_read_x[] = {0xD0}; // READ_X
static const uint8_t cmd_read_y[] = {0x90}; // READ_Y
static const uint8_t zeroes_tx[] = { 0x00, 0x00 };
//#endif /* SOFTWARE_SPI */
  uint32_t avg_x = 0;
  uint32_t avg_y = 0;
  uint8_t nsamples = 0;

for(uint8_t i = 0; i < 16; i++){
  if(!XPT2046_TouchPressed()) break;
  nsamples++;
  uint8_t y_raw[2];
  uint8_t x_raw[2];

//#ifdef SOFTWARE_SPI
//  spi_write_byte(READ_Y);
//  y_raw[0] = spi_read_byte();
//  y_raw[1] = spi_read_byte();
//  spi_write_byte(READ_X);
//  x_raw[0] = spi_read_byte();
//  x_raw[1] = spi_read_byte();
//#else
  HAL_SPI_Transmit(&T_SPI_PORT, (uint8_t*)cmd_read_y, sizeof(cmd_read_y), HAL_MAX_DELAY);
  HAL_SPI_TransmitReceive(&T_SPI_PORT, (uint8_t*)zeroes_tx, y_raw, sizeof(y_raw), HAL_MAX_DELAY);

  HAL_SPI_Transmit(&T_SPI_PORT, (uint8_t*)cmd_read_x, sizeof(cmd_read_x), HAL_MAX_DELAY);
  HAL_SPI_TransmitReceive(&T_SPI_PORT, (uint8_t*)zeroes_tx, x_raw, sizeof(x_raw), HAL_MAX_DELAY);
//#endif /* SOFTWARE_SPI */

        avg_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
        avg_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
    }

    XPT2046_TouchUnselect();
//----------------------------- Uncomment this line to calibrate touchscreen ------------------------
//    sprintf(buffTFT,"avgX=%6d avgY=%6d",avg_x, avg_y);
//    GUI_WriteString(5, lcddev.height-80, buffTFT, Font_11x18, point_color, back_color);
//---------------------------------------------------------------------------------------------------


    if(nsamples < 16) return false;

    uint32_t raw_x = (avg_x / 16);
    if(raw_x < XPT2046_MIN_RAW_X) raw_x = XPT2046_MIN_RAW_X;
    if(raw_x > XPT2046_MAX_RAW_X) raw_x = XPT2046_MAX_RAW_X;

    uint32_t raw_y = (avg_y / 16);
    if(raw_y < XPT2046_MIN_RAW_Y) raw_y = XPT2046_MIN_RAW_Y;
    if(raw_y > XPT2046_MAX_RAW_Y) raw_y = XPT2046_MAX_RAW_Y;

//----------------------------- Uncomment this line to calibrate touchscreen ------------------------
//    sprintf(buffTFT,"rawX=%6d rawY=%6d",raw_x, raw_y);
//    GUI_WriteString(5, lcddev.height-60, buffTFT, Font_11x18, point_color, back_color);
//---------------------------------------------------------------------------------------------------

  switch (lcddev.dir){
    case 0: //ќпределите направление вращени€ по часовой стрелке: вращение 0Ц0 градусов (книжное)
      *x = (raw_x - XPT2046_MIN_RAW_X) * lcddev.width / (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);
      *y = lcddev.height - (raw_y - XPT2046_MIN_RAW_Y) * lcddev.height / (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);
      break;
    case 1: // вращение 1Ц90 градусов (альбомное)
      *x = lcddev.width -  (raw_x - XPT2046_MIN_RAW_X) * lcddev.width / (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);
      *y = lcddev.height - (raw_y - XPT2046_MIN_RAW_Y) * lcddev.height / (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);
      break;
    case 2: // вращение 2Ц180 градусов (книжное)
      *x = lcddev.width - (raw_x - XPT2046_MIN_RAW_X) * lcddev.width / (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);
      *y = (raw_y - XPT2046_MIN_RAW_Y) * lcddev.height / (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);
      break;
    case 3: // вращение 3Ц270 градусов (альбомное)
      *x = (raw_x - XPT2046_MIN_RAW_X) * lcddev.width / (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);
      *y = (raw_y - XPT2046_MIN_RAW_Y) * lcddev.height / (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);
      break;
    default:
      *x = (raw_x - XPT2046_MIN_RAW_X) * lcddev.width / (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);
      *y = lcddev.height - (raw_y - XPT2046_MIN_RAW_Y) * lcddev.height / (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);
      break;
  }
//------------------------------ Uncomment this line to calibrate touchscreen ----------------------
//    sprintf(buffTFT,"X=%6d Y=%6d",*x, *y);
//    GUI_WriteString(5, lcddev.height-40, buffTFT, Font_11x18, WHITE, BLACK);
//-------  расна€ точка ---------------------
    GUI_DrawPixel(*x, *y, RED);
//--------------------------------------------------------------------------------------------------
    return true;
}
