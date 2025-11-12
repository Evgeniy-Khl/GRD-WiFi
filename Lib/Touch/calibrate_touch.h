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

#ifndef CALIBRATE_TOUCH_H_
#define CALIBRATE_TOUCH_H_

#include "main.h"
/* Коэффициенты для преобразования координат тачскрина в дисплейные координаты */
typedef struct {
	int64_t	Dx1, Dx2, Dx3, Dy1, Dy2, Dy3, D;
} tCoef;

/* Данные с координатами точки касания */
typedef struct {
	int x, y;
} tPoint;

/* Калибровка тачскрина по 5 точкам */
void XPT2046_CalibrateTouch(void);

/* Преобразование координат тачскрина в дисплейные координаты */
void XPT2046_ConvertPoint(tPoint *p_display, volatile tPoint *p_touch, tCoef *coef);

#endif /* CALIBRATE_TOUCH_H_ */
