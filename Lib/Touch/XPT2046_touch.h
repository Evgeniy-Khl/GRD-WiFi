/*
 * XPT2046_touch.h
 *
 *  Created on: 20 sep. 2019.
 *      Author: Andriy Honcharenko
 */

#ifndef XPT2046_TOUCH_H
#define XPT2046_TOUCH_H

#include "main.h"
#include <stdbool.h>

#define T_SPI_PORT hspi2
extern SPI_HandleTypeDef T_SPI_PORT;

//#define TOUCH_ORIENTATION_PORTRAIT 			(0U)
//#define TOUCH_ORIENTATION_LANDSCAPE 		(1U)
//#define TOUCH_ORIENTATION_PORTRAIT_MIRROR 	(2U)
//#define TOUCH_ORIENTATION_LANDSCAPE_MIRROR 	(3U)

//#define ORIENTATION	(TOUCH_ORIENTATION_LANDSCAPE)

//// change depending on screen orientation
//#if (ORIENTATION == 0)
//#define XPT2046_SCALE_X 240
//#define XPT2046_SCALE_Y 320
//#elif (ORIENTATION == 1)
//#define XPT2046_SCALE_X 320
//#define XPT2046_SCALE_Y 240
//#elif (ORIENTATION == 2)
//#define XPT2046_SCALE_X 240
//#define XPT2046_SCALE_Y 320
//#elif (ORIENTATION == 3)
//#define XPT2046_SCALE_X 320
//#define XPT2046_SCALE_Y 240
//#endif

//#define DIAGONAL    40          // 24 -> для дисплеев 2,4"; 28 -> для дисплеев 2,8"

//#if DIAGONAL==24
//    #define XPT2046_MIN_RAW_X 4000
//    #define XPT2046_MAX_RAW_X 31000
//    #define XPT2046_MIN_RAW_Y 1500
//    #define XPT2046_MAX_RAW_Y 30000
//#elif DIAGONAL==28
//    #define XPT2046_MIN_RAW_X 1500
//    #define XPT2046_MAX_RAW_X 29000
//    #define XPT2046_MIN_RAW_Y 2000
//    #define XPT2046_MAX_RAW_Y 30000
//#else
    #define XPT2046_MIN_RAW_X 1700
    #define XPT2046_MAX_RAW_X 31000
    #define XPT2046_MIN_RAW_Y 2000
    #define XPT2046_MAX_RAW_Y 31500
//#endif


void XPT2046_TouchSelect(void);
void XPT2046_TouchUnselect(void);
bool XPT2046_TouchPressed(void);
bool XPT2046_TouchGetCoordinates(uint16_t* x, uint16_t* y);

#endif /* XPT2046_TOUCH_H_ */
