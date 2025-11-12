#include "myLCD.h"
#include "stdlib.h"

//Основные параметры ЖК-дисплея
_lcd_dev lcddev;

void LCD_Select(void){
    HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET);
}

void LCD_Unselect(void){
    HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET);
}

void LCD_Reset(void){
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET);
}

static void LCD_WriteCommand(uint8_t cmd){
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&TFT_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

void LCD_WriteData(uint8_t* buff, uint16_t buff_size){
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
    HAL_SPI_Transmit(&TFT_SPI_PORT, buff, buff_size, HAL_MAX_DELAY);
}

void LCD_Init(uint8_t dir){  
  LCD_Select();
  LCD_Reset();
//************* ST7796S инициализация**********//
  LCD_WriteCommand(0x01); //SWRESET (01h): Software Reset
  HAL_Delay(10);//It will be necessary to wait 5msec before sending new command following software reset.
	LCD_WriteCommand(0xF0);{ // CSCON (F0h): Command Set Control
  uint8_t data[] = {0xC3};// C3h enable command 2 part I 
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xF0);{ //
  uint8_t data[] = {0x96};// 96h enable command 2 part II
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0x36);{ // MADCTL (36h): Memory Data Access Control
  uint8_t data[] = {0x68};
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0x3A);{ // COLMOD (3Ah): Interface Pixel Format
  uint8_t data[] = {0x05};// ‘101’ = 16bit/pixel
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xB0);{ // IFMODE (B0h): Interface Mode Control
  uint8_t data[] = {0x80};// SPI_EN
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xB6);{ // DFC(B6): Display Function Control
  uint8_t data[] = {0x00,0x02};// DATA N3??
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xB5);{ // BPC(B5): Blanking Porch Control
  uint8_t data[] = {0x02,0x03,0x00,0x04};
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xB1);{ // FRMCTR1 (B1h): Frame Rate Control (In Normal Mode/Full Colors)
  uint8_t data[] = {0x80,0x10};
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xB4);{ // DIC (B4h): Display Inversion Control 
  uint8_t data[] = {0x00};// Column inversion
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xB7);{ // EM(B7h): Entry Mode Set
  uint8_t data[] = {0xC6};// Normal display r(0) = b(0) = G(0)
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xC5);{ // VCMPCTL(C5h): VCOM Control
  uint8_t data[] = {0x24};// 1.200
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xE4);{ //??
  uint8_t data[] = {0x31};
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xE8);{ // DOCA (E8h): Display Output Ctrl Adjust
  uint8_t data[] = {0x40,0x8A,0x00,0x00,0x29,0x19,0xA5,0x33};
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xC2); //PWR3 (C2h): Power Control 3 DATA??
	LCD_WriteCommand(0xA7); //??
	
	LCD_WriteCommand(0xE0);{ // PGC (E0h): Positive Gamma Control
  uint8_t data[] = {0xF0,0x09,0x13,0x12,0x12,0x2B,0x3C,0x44,0x4B,0x1B,0x18,0x17,0x1D,0x21};
  LCD_WriteData(data, sizeof(data));
  }

	LCD_WriteCommand(0xE1);{ //NGC (E1h): Negative Gamma Control
  uint8_t data[] = {0xF0,0x09,0x13,0x0C,0x0D,0x27,0x3B,0x44,0x4D,0x0B,0x17,0x17,0x1D,0x21};
  LCD_WriteData(data, sizeof(data));
  }

  LCD_WriteCommand(0X36);{ // MADCTL (36h): Memory Data Access Control
  uint8_t data[] = {0xEC};
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xF0);{ // CSCON (F0h): Command Set Control
  uint8_t data[] = {0xC3};// C3h enable command 2 part I
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0xF0);{ // CSCON (F0h): Command Set Control
  uint8_t data[] = {0x69};// 69h disable command 2 part II
  LCD_WriteData(data, sizeof(data));
  }
	LCD_WriteCommand(0x13); // NORON (13h): Normal Display Mode On
	LCD_WriteCommand(0x11); // SLPOUT (11h): Sleep Out
	LCD_WriteCommand(0x29); // DISPON (29h): Display On
	
  LCD_Direction(dir);//Установите направление ЖК-дисплея
//	LCD_LED=1;//Включить подсветку	 
//	LCD_Clear(RED);//чистый полноэкранный белый
  
  LCD_Unselect();
}

/*****************************************************************************
 * @parameters :direction:0-0 degree
                          1-90 degree
													2-180 degree
													3-270 degree
******************************************************************************/ 
void LCD_Direction(uint8_t direction){ 
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;
	lcddev.wramcmd=0x2C;
  lcddev.dir = direction;
	switch(direction){		  
		case 0:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;		
			LCD_WriteCommand(0x36);{
      uint8_t data[] = {(1<<3)|(1<<6)};
      LCD_WriteData(data, sizeof(data));
      }
		break;
		case 1:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
      LCD_WriteCommand(0x36);{
      uint8_t data[] = {(1<<3)|(1<<5)};
      LCD_WriteData(data, sizeof(data));
      }
		break;
		case 2:						 	 		
			lcddev.width=LCD_W;
			lcddev.height=LCD_H;	
      LCD_WriteCommand(0x36);{
      uint8_t data[] = {(1<<3)|(1<<7)};
      LCD_WriteData(data, sizeof(data));
      }
		break;
		case 3:
			lcddev.width=LCD_H;
			lcddev.height=LCD_W;
      LCD_WriteCommand(0x36);{
      uint8_t data[] = {(1<<3)|(1<<7)|(1<<6)|(1<<5)};
      LCD_WriteData(data, sizeof(data));
      }
		break;	
		default:
      lcddev.width=LCD_W;
			lcddev.height=LCD_H;		
			LCD_WriteCommand(0x36);{
      uint8_t data[] = {(1<<3)|(1<<6)};
      LCD_WriteData(data, sizeof(data));
      }
    break;
	}		
}	 

/*****************************************************************************
 * @parameters :xStar:the bebinning x coordinate of the LCD display window
								yStar:the bebinning y coordinate of the LCD display window
								xEnd:the endning x coordinate of the LCD display window
								yEnd:the endning y coordinate of the LCD display window
******************************************************************************/ 
void LCD_SetWindow(uint16_t xStart, uint16_t yStart,uint16_t xEnd,uint16_t yEnd){	
	LCD_WriteCommand(lcddev.setxcmd);{	
  uint8_t data[] = {xStart>>8,0x00FF&xStart,xEnd>>8,0x00FF&xEnd};
  LCD_WriteData(data, sizeof(data));
  }

	LCD_WriteCommand(lcddev.setycmd);{	
  uint8_t data[] = {yStart>>8,0x00FF&yStart,yEnd>>8,0x00FF&yEnd};
  LCD_WriteData(data, sizeof(data));
  }

	LCD_WriteCommand(lcddev.wramcmd);	//Начать запись в GRAM			
}
