#ifndef __MYLCD_H
#define __MYLCD_H

#include "main.h"


typedef struct  
{										    
	uint16_t width;     //LCD ширина
	uint16_t height;    //LCD высокий
	uint16_t id;        //LCD ID
	uint8_t  dir;			  //Управление горизонтальным или книжным экраном: 0, книжный экран 1, альбомный экран;	
	uint16_t wramcmd;		//Зпись команды
	uint16_t setxcmd;		//Команда установки координаты x
	uint16_t setycmd;		//Команда установки координаты Y	 
}_lcd_dev; 	

//Параметры ЖК-дисплея
extern _lcd_dev lcddev;	//Управление важными параметрами ЖК-дисплея

#define USE_VERTICAL0   0  //Определите направление вращения ЖК-экрана по часовой стрелке: вращение 0–0 градусов, 
#define USE_HORIZONT0   1  // вращение 1–90 градусов,
#define USE_VERTICAL1   2  // вращение 2–180 градусов,
#define USE_HORIZONT1   3  // вращение 3–270 градусов.

//Размер ЖК-дисплея
#define LCD_W 320
#define LCD_H 480

extern SPI_HandleTypeDef hspi2;
#define TFT_SPI_PORT hspi2

////Установлен GPIO (высокий уровень)
//#define	LCD_CS_SET  HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_SET)    //Выбор чипа
//#define	LCD_DC_SET	HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET)    //данные/команда
//#define	LCD_RST_SET	HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_SET)  //перезагрузить

////Сброс GPIO (низкий уровень)							    
//#define	LCD_CS_CLR  HAL_GPIO_WritePin(TFT_CS_GPIO_Port, TFT_CS_Pin, GPIO_PIN_RESET)  //Выбор чипа
//#define	LCD_DC_CLR	HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_RESET)  //данные/команда 
//#define	LCD_RST_CLR	HAL_GPIO_WritePin(TFT_RST_GPIO_Port, TFT_RST_Pin, GPIO_PIN_RESET)//перезагрузить

// Default color definitions
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */

//цвет кисти
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
//#define BRED        0XF81F
//#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0      /* 255, 255,   0 */
#define ORANGE      0xFDA0      /* 255, 180,   0 */
#define BROWN 			0xBC40 //коричневый
#define BRRED 			0xFC07 //коричнево-красный
#define GRAY  			0x8430 //серый
//Цвет графического интерфейса
#define DARKBLUE    0x01CF	//темно-синий
#define LIGHTBLUE   0x7D7C	//светло-голубой  
#define GRAYBLUE    0x5458 //серо-синий
//Вышеуказанные три цвета-это цвета панели. 
#define LIGHTGREEN  0x841F //светло-зеленый
#define LIGHTGRAY   0xEF5B //светло-серый(PANNEL)
#define LGRAY       0xC618 //светло-серый(PANNEL),Цвет фона формы
#define LGRAYBLUE   0xA651 //Светло-серо-синий (цвет среднего слоя)
#define LBBLUE      0x2B12 //Светло-коричнево-синий (выберите обратный цвет записи)

void LCD_Select(void);
void LCD_Unselect(void);
void LCD_Reset(void);
static void LCD_WriteCommand(uint8_t cmd);
void LCD_WriteData(uint8_t* buff, uint16_t buff_size);
void LCD_Init(uint8_t dir);
void LCD_Direction(uint8_t direction);
void LCD_Clear(uint16_t Color);
void LCD_SetWindow(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);


#endif // __MYLCD_H
