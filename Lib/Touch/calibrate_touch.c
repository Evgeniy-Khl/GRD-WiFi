/*
 *  Copyright (C) 2019, VadRov, all right reserved.
 *
 *  Калибровка тачскрина по 5 точкам
 *
 *  Допускается свободное распространение без целей коммерческого использования.
 *  При коммерческом использовании необходимо согласование с автором.
 *  Распространятся по типу "как есть", то есть использование осуществляете на свой страх и риск.
 *  Автор не предоставляет никаких гарантий.
 *
 *  https://www.youtube.com/@VadRov
 *  https://dzen.ru/vadrov
 *  https://vk.com/vadrov
 *  https://t.me/vadrov_channel
 *
 */
#include <stdio.h>
#include "..\Lib\st7796\myLCD.h"
#include "..\Lib\st7796\myGUI.h"
#include "..\Lib\Touch\calibrate_touch.h"
#include "..\Lib\Touch\XPT2046_touch.h"

extern char buffTFT[];
extern uint8_t checkTime, beep;
extern uint16_t touch_x, touch_y;

/*
 * Расчет коэффициентов для преобразования координат тачскрина в дисплейные координаты
 */
static void CoefCalc(tPoint *p_d, tPoint *p_t, tCoef *coef, uint8_t all_points)
{
	uint64_t a = 0, b = 0, c = 0, d = 0, e = 0, X1 = 0, X2 = 0, X3 = 0, Y1 = 0, Y2 = 0, Y3 = 0;
	for(uint8_t i = 0; i < all_points; i++)	{
		a += p_t[i].x * p_t[i].x;
		b += p_t[i].y * p_t[i].y;
		c += p_t[i].x * p_t[i].y;
		d += p_t[i].x;
		e += p_t[i].y;
		X1 += p_t[i].x * p_d[i].x;
		X2 += p_t[i].y * p_d[i].x;
		X3 += p_d[i].x;
		Y1 += p_t[i].x * p_d[i].y;
		Y2 += p_t[i].y * p_d[i].y;
		Y3 += p_d[i].y;
	}
	coef->D = all_points * (a * b - c * c) + 2 * c *  d * e - a * e * e - b * d * d;
	coef->Dx1 = all_points * (X1 * b - X2 * c) + e * (X2 * d - X1 * e) + X3 * (c * e - b * d);
	coef->Dx2 = all_points * (X2 * a - X1 * c) + d * (X1 * e - X2 * d) + X3 * (c * d - a * e);
	coef->Dx3 = X3 * (a * b - c * c) + X1 * (c * e - b * d) + X2 * (c * d - a * e);
	coef->Dy1 = all_points * (Y1 * b - Y2 * c) + e * (Y2 * d - Y1 * e) + Y3 * (c * e - b * d);
	coef->Dy2 = all_points * (Y2 * a - Y1 * c) + d * (Y1 * e - Y2 * d) + Y3 * (c * d - a * e);
	coef->Dy3 = Y3 * (a * b - c * c) + Y1 * (c * e - b * d) + Y2 * (c * d -a * e);
}

/*
 * Преобразование координат тачскрина в дисплейные/экранные координаты:
 * - в переменной p_t (тип tPoint) принимает координаты тачскрина;
 * - в переменной coef (тип tCoef) принимает коэффициенты преобразования;
 * - в переменной p_d (тип tPoint) возвращает дисплейные координаты.
 */
void XPT2046_ConvertPoint(tPoint *p_d, volatile tPoint *p_t, tCoef *coef)
{
	p_d->x = (int)((p_t->x * coef->Dx1 + p_t->y * coef->Dx2 + coef->Dx3) / coef->D);
	p_d->y = (int)((p_t->x * coef->Dy1 + p_t->y * coef->Dy2 + coef->Dy3) / coef->D);
}
#define BIAS 10

/* Калибровка тачскрина */
void XPT2046_CalibrateTouch(void){
	//Таблица с дисплейными координатами 5 точек калибровки (4 - края + 1 - центр).
	uint16_t pos_xy[] = {5, 5, 5, lcddev.height - 5, lcddev.width - 5, 5, lcddev.width - 5, lcddev.height - 5, lcddev.width/2, lcddev.height/2};
	//Массивы для хранения данных о точках калибровки.
	tPoint p_display[5], p_touch[5];
  tPoint point; //Координаты последнего считанного касания (в координатах тачскрина)
  tCoef coef;   //Коэффициенты преобразования координат тачскина в дисплейные координаты
	//Таблица с дисплейными координатами для проверки после калибровки.
	uint16_t ver_xy[] = {lcddev.width / 2, 20, lcddev.width / 2, lcddev.height - 20};
	uint8_t f_error, yStr, i;  //Флаг ошибки при проверки калибровки и счетчик точек калибровки.

	while (1) { //Пока калибровка не будет успешной.
		i = 0; 
		GUI_Clear(BLACK); //Очищаем дисплей.
		//Вывод сообщения о том, что необходимо кликнуть на пять точек (по краям и в центре дисплея).
    yStr=0;
    GUI_WriteString(90, yStr, "Calibration.", Font_11x18, WHITE, BLACK);yStr += 28;
//    GUI_WriteString(0, yStr, "prompted to click on 5 dots", Font_11x18, WHITE, BLACK);yStr += 22;
//    GUI_WriteString(0, yStr, "(on the edges andin", Font_11x18, WHITE, BLACK);yStr += 22;
//    GUI_WriteString(0, yStr, "the center of the display).", Font_11x18, WHITE, BLACK);yStr += 22;
//    GUI_WriteString(0, yStr, "Get ready.", Font_11x18, WHITE, BLACK);
		HAL_Delay(1000); //Ждем 4 секунды.
		GUI_Clear(BLACK); //Очищаем дисплей.
		while (i < 5) { //Цикл по точкам калибровки.
			//Рисуем окружность c перекрестием - точка калибровки.
//			draw_calibrate_point(lcd, pos_xy[2 * i], pos_xy[2 * i + 1], 5);
      GUI_FillRectangle(pos_xy[2 * i]-2, pos_xy[2 * i + 1]-2, 4, 4, RED);// Рисуем точку
			//Ждем касания тачскрина и снятия этого касания.
			while (1){
        if(XPT2046_TouchPressed()){
          HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
          HAL_Delay(200);
          HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
          XPT2046_TouchGetCoordinates(&touch_x, &touch_y);
          break;
        }
      }
      //Запоминаем дисплейные координаты - центр калибровочного круга.
			p_display[i].x = pos_xy[2 * i];
			p_display[i].y = pos_xy[2 * i + 1];
			//Запоминаем координаты тачскрина, соответствующие дисплейным.
			p_touch[i].x = touch_x;
      p_touch[i].y = touch_y;
      //-----------------------------------------
      sprintf(buffTFT,"T%u; dspl X:%i; dspl Y:%i",i,p_display[i].x,p_display[i].y);
      GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
      sprintf(buffTFT,"T%u; tach X:%i; tach Y:%i",i,touch_x,touch_y);
      GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
        //-----------------------------------------
			GUI_FillRectangle(pos_xy[2 * i]-2, pos_xy[2 * i + 1]-2, 4, 4, BLACK);//Очищаем точку
			HAL_Delay(300); //Ждем 0.3 секунды.
			i++; //Переходим к следующей точке калибровки.
		}
		//Раcсчитываем коэффициенты для перехода от координат тачскрина в дисплейные координаты.
		CoefCalc(p_display, p_touch, &coef, 5);
		//Проверка калибровочных параметров.
    while (1){
      if(XPT2046_TouchPressed()){
        HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
        break;
      }
    }
		//Вывод сообщения о том, что необходимо кликнуть на две точки (вверху и внизу дисплея).
    yStr=0;
    GUI_WriteString(50, yStr, "Checking calibration.", Font_11x18, WHITE, BLACK);yStr += 22;
//    GUI_WriteString(0, yStr, "You will be prompted", Font_11x18, WHITE, BLACK);yStr += 22;
//    GUI_WriteString(0, yStr, "to click on two dots", Font_11x18, WHITE, BLACK);yStr += 22;
//    GUI_WriteString(0, yStr, "(top and bottom", Font_11x18, WHITE, BLACK);yStr += 22;
//    GUI_WriteString(0, yStr, "of the display).", Font_11x18, WHITE, BLACK);yStr += 22;
//    GUI_WriteString(0, yStr, "Get ready.", Font_11x18, WHITE, BLACK);
		HAL_Delay(1000); //Ждем 4 секунды.
		GUI_Clear(BLACK); //Очищаем дисплей.
		f_error = i = 0; //Инициализируем флаг ошибки и счетчик точек проверки калибровки
    yStr = 0;
		while (i < 2) {
			//Рисуем окружность c перекрестием - точка калибровки.
//			draw_calibrate_point(lcd, ver_xy[2 * i], ver_xy[2 * i + 1], 5);
      GUI_FillRectangle(pos_xy[2 * i]-2, pos_xy[2 * i + 1]-2, 4, 4, RED);// Рисуем точку
			//Ждем касания тачскрина и снятия этого касания.
			while (1){
        if(XPT2046_TouchPressed()){
          HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
          HAL_Delay(200);
          HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
          XPT2046_TouchGetCoordinates(&touch_x, &touch_y);
          break;
        }
      }
      //Координаты последнего считанного касания (в координатах тачскрина)
      point.x = touch_x;
      point.y = touch_y;
      //-----------------------------------------
      sprintf(buffTFT,"T%u; dspl X:%i; dspl Y:%i",i,p_display[i].x,p_display[i].y);
      GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
      sprintf(buffTFT,"T%u; tach X:%i; tach Y:%i",i,touch_x,touch_y);
      GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
      //-----------------------------------------
			//Переходим от координат тачскрина к дисплейным координатам.
			XPT2046_ConvertPoint(&p_display[0], &point, &coef);
			//Проверяем, принадлежит ли точка касания тачскрина заданному диапазону координат.
      //-----------------------------------------
      sprintf(buffTFT,"T%u; dspl X0:%i; dspl Y0:%i",i,p_display[0].x,p_display[0].y);
      GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
      sprintf(buffTFT,"T%u; tach X0:%i; tach Y0:%i",i,point.x,point.y);
      GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
      //-----------------------------------------
			//Если выходит за границы диапазона, то устанавливаем флаг ошибки калибровки.
			if (p_display[0].x < ver_xy[2 * i] - BIAS || p_display[0].x > ver_xy[2 * i] + BIAS ||
          p_display[0].y < ver_xy[2 * i + 1] - BIAS || p_display[0].y > ver_xy[2 * i + 1] + BIAS) f_error = 1;
			GUI_FillRectangle(pos_xy[2 * i]-2, pos_xy[2 * i + 1]-2, 4, 4, RED); //Очищаем точку.
			HAL_Delay(300); //Ждем 0.3 секунды.
			i++; //К следующей точке.
		}
    while (1){
      if(XPT2046_TouchPressed()){
        HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
        HAL_Delay(200);
        HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
        break;
      }
    }
		if (!f_error)  { //Если нет ошибок в калибровочных параметрах, то выводим сообщение и выходим.
			GUI_WriteString(50, 0, "Calibration OK!", Font_11x18, WHITE, BLACK);
			HAL_Delay(2000); //Ждем 2 секунды
			break;
		}
		//Если есть ошибки в калибровочных параметрах, то выводим сообщение и повторяем процесс калибровки.
    yStr = 0;
    GUI_WriteString(50, 0, "Calibration Error!", Font_11x18, WHITE, BLACK);yStr += 28;
    sprintf(buffTFT,"display X:%i",p_display[0].x);
    GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
    sprintf(buffTFT,"tach X:%i",ver_xy[2 * i]);
    GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
    sprintf(buffTFT,"display Y:%i",p_display[0].y);
    GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
    sprintf(buffTFT,"tach Y:%i",ver_xy[2 * i + 1]);
    GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
    sprintf(buffTFT,"error X:%i",p_display[0].x-ver_xy[2 * i]);
    GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
    sprintf(buffTFT,"error Y:%i",p_display[0].y - ver_xy[2 * i + 1]);
    GUI_WriteString(0, yStr, buffTFT, Font_11x18, WHITE, BLACK);yStr += 22;
    while (1){
        if(XPT2046_TouchPressed()){
          HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
          HAL_Delay(200);
          HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
          break;
        }
      }
//		HAL_Delay(2000); //Ждем 2 секунды
	}
	GUI_Clear(BLACK); //Очищаем дисплей
}
