#ifndef _TFTARCFILL_H
#define _TFTARCFILL_H
#include "main.h"
#include "..\Lib\st7796\myLCD.h"
#include "..\Lib\st7796\myGUI.h"

#define DEG2RAD 0.0174532925
#define LOOP_DELAY 10 // Loop delay to slow things down

typedef struct
{
  uint16_t xpos; 
  uint16_t ypos; 
  uint8_t radius; 
  int16_t value; 
  int16_t sp;
} GrafDispl;

extern GrafDispl grafDispl[2];

void initArcFill(void);
void loopArcFill(void);
void fillArc(int x, int y, int start_angle, int seg_count, int rx, int ry, int w, unsigned int colour);
unsigned int rainbow(uint8_t value);
void diagram(GrafDispl grafDispl, uint16_t color);

#endif /* _TFTARCFILL_H */
