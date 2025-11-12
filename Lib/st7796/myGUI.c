#include "myLCD.h"
#include "myGUI.h"

/*****************************************************************************
 * @name       :void LCD_Clear(u16 Color)
 * @date       :2018-08-09 
 * @function   :Full screen filled LCD screen
 * @parameters :color:Filled color
 * @retvalue   :None
******************************************************************************/	
void GUI_Clear(uint16_t color){
  GUI_FillRectangle(0, 0, lcddev.width, lcddev.height, color);
}

void GUI_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color){
    // clipping
    if((x >= lcddev.width) || (y >= lcddev.height)) return;
    if((x + w - 1) >= lcddev.width) w = lcddev.width - x;
    if((y + h - 1) >= lcddev.height) h = lcddev.height - y;
    LCD_Select();
    LCD_SetWindow(x, y, x+w-1, y+h-1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
            HAL_SPI_Transmit(&TFT_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
        }
    }
    LCD_Unselect();
}

void GUI_DrawPixel(uint16_t x, uint16_t y, uint16_t color){
    if((x >= lcddev.width) || (y >= lcddev.height)) return;
    LCD_Select();
    LCD_SetWindow(x, y, x+1, y+1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    LCD_WriteData(data, sizeof(data));
    LCD_Unselect();
}

static void GUI_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor){
    uint32_t i, b, j;
    LCD_SetWindow(x, y, x+font.width-1, y+font.height-1);
    for(i = 0; i < font.height; i++) {
      if (ch>=32 && ch<127) b = font.data[(ch - 32) * font.height + i];// латиница
      else b = font.data[(ch - 97) * font.height + i];  // кирилица 192 - 96 = 96
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                LCD_WriteData(data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                LCD_WriteData(data, sizeof(data));
            }
        }
    }
}

void GUI_WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor){
  LCD_Select();
  while(*str){
    if(x + font.width >= lcddev.width){
      x = 0;
      y += font.height;
      if(y + font.height >= lcddev.height) break;
      if(*str == ' '){str++; continue;}// skip spaces in the beginning of the new line
    }
    GUI_WriteChar(x, y, *str, font, color, bgcolor);
    x += font.width;
    str++;
  }
  LCD_Unselect();
}

/*******************************************************************
 * @name       :void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
 * @date       :2018-08-09 
 * @function   :fill the specified area
 * @parameters :sx:the bebinning x coordinate of the specified area
                sy:the bebinning y coordinate of the specified area
								ex:the ending x coordinate of the specified area
								ey:the ending y coordinate of the specified area
								color:the filled color value
 * @retvalue   :None
********************************************************************/
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{  	
	uint16_t i,j;			
	uint16_t width=ex-sx+1; 		//Получить ширину отступа
	uint16_t height=ey-sy+1;		//высокий
  LCD_Select();
  LCD_SetWindow(sx,sy,ex,ey);
  uint8_t data[] = { color >> 8, color & 0xFF };
  HAL_GPIO_WritePin(TFT_DC_GPIO_Port, TFT_DC_Pin, GPIO_PIN_SET);
  for(i=0;i<height;i++) {
      for(j=0;j<width;j++) {
          HAL_SPI_Transmit(&TFT_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
      }
  }
  LCD_Unselect();
}

void _swap(uint16_t* a, uint16_t* b) {
    uint16_t temp = *a;
    *a = *b;
    *b = temp;
}


/*****************************************************************************
 * @name       :void Fill_Triangel(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
 * @date       :2018-08-09 
 * @function   :filling a triangle at a specified position
 * @parameters :x0:the bebinning x coordinate of the triangular edge 
                y0:the bebinning y coordinate of the triangular edge 
								x1:the vertex x coordinate of the triangular
								y1:the vertex y coordinate of the triangular
								x2:the ending x coordinate of the triangular edge 
								y2:the ending y coordinate of the triangular edge 
 * @retvalue   :None
******************************************************************************/ 
void Fill_Triangel(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2, uint16_t point_color)
{
	uint16_t a, b, y, last;
	int dx01, dy01, dx02, dy02, dx12, dy12;
	long sa = 0;
	long sb = 0;
 	if (y0 > y1) 
	{
    _swap(&y0,&y1); 
		_swap(&x0,&x1);
 	}
 	if (y1 > y2) 
	{
    _swap(&y2,&y1); 
		_swap(&x2,&x1);
 	}
  if (y0 > y1) 
	{
    _swap(&y0,&y1); 
		_swap(&x0,&x1);
  }
	if(y0 == y2) 
	{ 
		a = b = x0;
		if(x1 < a)
    {
			a = x1;
    }
    else if(x1 > b)
    {
			b = x1;
    }
    if(x2 < a)
    {
			a = x2;
    }
		else if(x2 > b)
    {
			b = x2;
    }
		LCD_Fill(a,y0,b,y0,point_color);
    return;
	}
	dx01 = x1 - x0;
	dy01 = y1 - y0;
	dx02 = x2 - x0;
	dy02 = y2 - y0;
	dx12 = x2 - x1;
	dy12 = y2 - y1;
	
	if(y1 == y2)
	{
		last = y1; 
	}
  else
	{
		last = y1-1; 
	}
	for(y=y0; y<=last; y++) 
	{
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
    sb += dx02;
    if(a > b)
    {
			_swap(&a,&b);
		}
		LCD_Fill(a,y,b,y,point_color);
	}
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for(; y<=y2; y++) 
	{
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		if(a > b)
		{
			_swap(&a,&b);
		}
		LCD_Fill(a,y,b,y,point_color);
	}
}
