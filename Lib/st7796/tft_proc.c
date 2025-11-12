#include "main.h"
#include "rtc.h"
#include "nvRam.h"
#include "tft_proc.h"
#include "procedure.h"

void setData(uint8_t m);

int16_t newval[MAX_SET];
uint16_t maxVal, minVal;
extern int8_t relaySet[8];
extern int8_t analogSet[2];
extern int16_t pvT[];
extern uint16_t fillScreen;
extern uint16_t speedData[MAX_SPEED][2];
extern uint8_t displ_num, ticBeep, show, Y_str, X_left, Y_top, Y_bottom, buttonAmount, resetDispl;
extern int8_t ds18b20_amount, numSet, tiimeDispl, oldNumSet;

extern union Byte portFlag;
extern union DataRam dataRAM;
struct {int x,y; char w,h;} buttons[4];

//void TFT_init(){
//  ILI9341_Unselect();
//  ILI9341_TouchUnselect();
//  ILI9341_Init();
//  GUI_Clear(fillScreen);
//  Y_str = 5; X_left = 5;
//  GUI_WriteString(35, Y_str, "GRD Max", Font_16x26, WHITE, fillScreen);
//  GUI_WriteString(165, Y_str+5, " v 2.8", Font_11x18, WHITE, fillScreen);
//  Y_str = Y_str+18+35;
//}

void WindowDraw(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t setcolor, const char* str){
 uint16_t strColor, bordColor; 
  switch (setcolor)
  {
  	case BLACK: strColor=WHITE; bordColor = WHITE; break;
    case BLUE:  strColor=WHITE; bordColor = WHITE; break;
  	default: strColor=BLACK; bordColor = BLACK; break;
  }
  GUI_FillRectangle(x, y, w, h, setcolor);
  for(int i = x; i < x+w+1; i++){
     GUI_DrawPixel(i, y-1, bordColor);
     GUI_DrawPixel(i, y+h+1, bordColor);
  }
  for(int i = y; i < y+h+1; i++){
     GUI_DrawPixel(x-1, i, bordColor);
     GUI_DrawPixel(x+w+1, i, bordColor);
  }
  GUI_WriteString(x+(w-11)/2, y+(h-18)/2, str, Font_11x18, strColor, setcolor);
}

void initializeButtons(uint8_t col, uint8_t row, uint8_t h)// высота кнопки
{
  uint8_t i,j,indx;
  uint16_t x, y, w;
  switch (col)                  // ширина кнопки зависит от кол-ва кнопок в строке
   {
    case 4: w = 72; break;
    case 3: w = 100; break;
    case 2: w = 150; break;   
    default: w = lcddev.width - 6;
   };
  if(h<20) h=20;
  y = lcddev.height - h - 4;      // верхний контур кнопки
  indx = 0;
  for (j=0; j<row; j++)
   {
    x = 4;// начало 1 кнопки
    for (i=0; i<col; i++)
      {
        buttons[indx].x = x+i*(w+8);// интервал между кнопками по горизонтали
        buttons[indx].w = w;
        buttons[indx].h = h;
        buttons[indx].y = y;
        indx++;
      }
    y -= (h*(row-1)+4);// интервал между кнопками по вертикали
   }
  Y_bottom = y;// нижняя граница до которой можно закрашивать экран
  buttonAmount = col * row;// обшее количество кнопок
}
//-------------------- цвет фона --- цвет рамки -- цвет текста --- номер --- текст ---------
void drawButton(uint16_t setcolor, uint8_t b, char *str)
{
  uint16_t x, y, w, h;
  w = buttons[b].w;      // ширина кнопки
  h = buttons[b].h;      // высота кнопки
  x = buttons[b].x;      // начало контура кнопки
  y = buttons[b].y;      // начало контура кнопки

  GUI_FillRectangle(x, y, w, h, setcolor);

  uint16_t strColor, bordColor;
  if (fillScreen == BLACK) bordColor=WHITE;
  else bordColor=BLACK;
  switch (setcolor){
  	case BLACK: strColor=WHITE; break;
    case BLUE:  strColor=WHITE; break;
  	default: strColor=BLACK; break;
  }
  GUI_FillRectangle(x, y, w, h, setcolor);
  for(uint16_t i = x; i < x+w+1; i++){
     GUI_DrawPixel(i, y-1, bordColor);
     GUI_DrawPixel(i, y+h+1, bordColor);
  }
  for(uint16_t i = y; i < y+h+1; i++){
     GUI_DrawPixel(x-1, i, bordColor);
     GUI_DrawPixel(x+w+1, i, bordColor);
  }

  x = x + w/2 - strlen(str)*11/2;  // ширина символа 11
  y = y + h/2 - 9;                 // высота символа 18 / 2
  
  GUI_WriteString(x, y, str, Font_11x18, strColor, setcolor);
//  sprintf(buffTFT,"sizeof=%d",strlen(str));
//  GUI_WriteString(5, ILI9341_HEIGHT-(45+18+5+(18+5)*b), buffTFT, Font_11x18, WHITE, BLACK); 
}

// проверка попадания пересчитаной координаты в область кнопки.
uint8_t contains(uint16_t touch_X, uint16_t touch_Y, uint8_t b){
 uint16_t beg, end;
   beg = buttons[b].x;
   end = beg + buttons[b].w-3;
   if ((touch_X < beg)||(touch_X > end)) return 0;
   beg = buttons[b].y;
   end = beg + buttons[b].h-3;
   if ((touch_Y < beg)||(touch_Y > end)) return 0;
   else ticBeep = 30;
   return 1;
 }

// проверка попадания в верхнию часть экрана.
uint8_t topDispl(uint16_t touch_X, uint16_t touch_Y){
   if (touch_Y > 80 || touch_X < 100 || touch_X > 250) return 0;
   else ticBeep = 30;
   return 1;
 }

void up_relay(){
  if(++relaySet[numSet]>1) relaySet[numSet]=-1;
}

void down_relay(){
  if(--relaySet[numSet]>1) relaySet[numSet]=1;
}

void checkButtons(uint8_t item){
    switch (displ_num){
      case 0://--------- СТАН КАМЕРИ -----------------------
        switch (item){
          case 0: NEWBUTT = 1; startPrg(0); break;
          case 1: displ_num = 1; NEWBUTT = 1; numSet=0; break;  // Управление реле
          case 2: displ_num = 2; NEWBUTT = 1; numSet=0; break;  // Настройки
        }
        item = 10;
        break;
      case 1://--------- СТАН ВЫХОДІВ ----------------------------------
        switch (item){
          case 0: displ_num = 0; NEWBUTT = 1; break;
          case 1: if(++numSet>6) numSet=0; break;
          case 2: up_relay(); break;
          case 3: down_relay(); break;
        }
        item = 10;
        break;
      case 2://--------- НАЛАШТУВАННЯ ----------------------------------
        switch (item){
          case 0: displ_num = 0; NEWBUTT = 1; break;
          case 1: if (++numSet>MAX_SET-1) numSet = -1;	break;
          case 2: if (--numSet<-1) numSet = MAX_SET-1;	break;
          case 3: oldNumSet = numSet; displ_num=3;
                  if(numSet==0 || numSet==1) newval[numSet]=upv.pv.set[numSet];    // ЗМІНА ТЕМПЕРАТУР
                  else if(numSet==2){
                    if(upv.pv.modeCell==3) newval[numSet]=upv.pv.set[T2];                 // ЗМІНА ТЕМПЕРАТУРИ Дима
                    else if(upv.pv.modeCell==2) newval[numSet]=upv.pv.set[T3];            // ЗМІНА ТЕМПЕРАТУРИ Вологого
                  }
                  else if(numSet==3) newval[numSet]=upv.pv.set[TMR0];              // "ТРИВАЛЫСТЬ"
                  else if(numSet==4){numSet=upv.pv.set[VENT]; displ_num=7;}        // вибір ШВИДКІСТІ обертання
                  else if(numSet==5) newval[numSet]=upv.pv.set[TMON];              // "ТАЙМ.ON"
                  else if(numSet==6) newval[numSet]=upv.pv.set[TMOFF];             // "ТАЙМ.OFF"
                  else if(numSet==7){numSet=0; newval[0]=0; displ_num=5;}   // IНШЕ
                  else {newval[0] = upv.pv.modeCell; newval[1]=10; displ_num = 4;} // ЗМІНА РЕЖИМУ
                  NEWBUTT = 1; break;
        }
        item = 10;
        break;
      case 3://--------- ЗМІНА ТЕМПЕРАТУР -------------------------
        if(numSet<3){maxVal=110, minVal=18;} // T0,T1,T2 грд
        else if(numSet==3){maxVal=1440, minVal=0;}// Длительность режима мин.
        else if(numSet==5 || numSet==6){
          // если ВАРКА (modeCell=2) задается в mсек.[от 0.1сек. до 10 сек.] (период 10 mсек.)
          if(upv.pv.modeCell==2){maxVal=100, minVal=0;} else {maxVal=500, minVal=0;}// Таймер ON/OFF
        }
        switch (item){
          case 0: displ_num = 2; NEWBUTT = 1; break;
          case 1: newval[numSet]+=1;	
                  if(newval[numSet]>=maxVal) newval[numSet]=maxVal;	
          break;
          case 2: newval[numSet]-=1; 
                  if(newval[numSet]<=minVal) newval[numSet]=minVal;	
          break;
          case 4: newval[numSet]+=10;	
                  if(newval[numSet]>=maxVal) newval[numSet]=maxVal;	
          break;
          case 5: newval[numSet]-=10; 
                  if(newval[numSet]<=minVal) newval[numSet]=minVal;	
          break;
          case 6: newval[numSet]+=50;	
                  if(newval[numSet]>=maxVal) newval[numSet]=maxVal;	
          break;
          case 7: newval[numSet]-=50; 
                  if(newval[numSet]<=minVal) newval[numSet]=minVal;	
          break;
          case 3: 
            GUI_FillRectangle(0, Y_top, lcddev.width, lcddev.height, fillScreen);
            GUI_WriteString(lcddev.width/2-90,lcddev.height/2-60, "ВИКОНАЮ ЗАПИС!", Font_11x18, GREEN, BLACK);
            if(numSet==2){
              if(upv.pv.modeCell==3) upv.pv.set[T2] = newval[numSet];     // температура (Дым)
              if(upv.pv.modeCell==2) upv.pv.set[T3] = newval[numSet];     // температура (Влажный)
            }
            else if(numSet==3) upv.pv.set[TMR0] = newval[numSet];  // Длительность режима
            else if(numSet==5) upv.pv.set[TMON] = newval[numSet];  // Таймер ON
            else if(numSet==6) upv.pv.set[TMOFF] = newval[numSet]; // Таймер OFF
            else upv.pv.set[numSet] = newval[numSet];     // температура T0, T1
            uint32_t er = writeData();        // запишем значения во FLASH
            if(er) GUI_WriteString(lcddev.width/2-40,lcddev.height/2-20, "ПОМИЛКА!", Font_11x18, YELLOW, RED);
            else GUI_WriteString(lcddev.width/2-10,lcddev.height/2+20, "OK", Font_11x18, GREEN, BLACK);
            HAL_Delay(1000);
            displ_num = 2; NEWBUTT = 1; break;
        }
        item = 10;
        break;
      case 4://--------- ЗМІНА РЕЖИМУ ----------------------------------
        switch (item){
          case 0: 
          if(newval[2]==-1) {displ_num = 0; newval[2] = 0;}
            else displ_num = 2;  
            NEWBUTT = 1; 
          break;
          case 1: if(++newval[0]>MAX_MODE-1) newval[0] = 0;	break;
          case 2: if(--newval[0]<0) newval[0] = MAX_MODE-1;	break;
          case 3: 
            GUI_FillRectangle(0, Y_top, lcddev.width, lcddev.height, fillScreen);
            GUI_WriteString(lcddev.width/2-90,lcddev.height/2-60, "ВИКОНАЮ ЗАПИС!", Font_11x18, GREEN, BLACK);
            upv.pv.modeCell = newval[0];
            setData(upv.pv.modeCell);                // установим новые значения
            uint32_t er = writeData();        // запишем значения во FLASH
            if(er) GUI_WriteString(lcddev.width/2-40,lcddev.height/2-20, "ПОМИЛКА!", Font_11x18, YELLOW, RED);
            else GUI_WriteString(lcddev.width/2-10,lcddev.height/2+20, "OK", Font_11x18, GREEN, BLACK);
            HAL_Delay(1000);
            if(newval[2]==-1) {displ_num = 0; newval[2] = 0;}
            else displ_num = 2;
            NEWBUTT = 1; 
          break;
        }
        item = 10;
        break;
	 case 5://--------- IНШЕ ----------------------------------
        switch (item){
          case 0: displ_num = 2; numSet = oldNumSet;  NEWBUTT = 1; break;
          case 1: if (++numSet>MAX_OTHER-1) numSet = 0;	break;
          case 2: if (--numSet<0) numSet = MAX_OTHER-1;	break;
          case 3: 
                  if(numSet<4) newval[numSet] = upv.pv.set[numSet+8];
                  else newval[numSet] = dataRAM.config.koff[upv.pv.modeCell][numSet-4];
				  displ_num = 6; NEWBUTT = 1; break;
        }
        item = 10;
        break;
	 case 6://--------- ЗМІНА IНШЕ -------------------------
        if(numSet==0){maxVal=1800, minVal=0;}// продувкa сек.
        else if(numSet==1){maxVal=50, minVal=1;}// Авария грд
        else if(numSet==2){maxVal=50, minVal=1;}// Гистерезис грд/10
        else if(numSet==3){maxVal=3, minVal=0;} // прямое/инвесное управление
        else if(numSet==4){maxVal=1000, minVal=1;}// пропорциональный
        else if(numSet==5){maxVal=10000, minVal=0;}// интегральный
        else if(numSet==6){maxVal=100, minVal=0;}// диференциальный
        switch (item){
          case 0: displ_num = 5; NEWBUTT = 1; break;
          case 1: newval[numSet]+=1;	
                  if(newval[numSet]>=maxVal) newval[numSet]=maxVal;	
          break;
          case 2: newval[numSet]-=1; 
                  if(newval[numSet]<=minVal) newval[numSet]=minVal;	
          break;
          case 4: newval[numSet]+=10;	
                  if(newval[numSet]>=maxVal) newval[numSet]=maxVal;	
          break;
          case 5: newval[numSet]-=10; 
                  if(newval[numSet]<=minVal) newval[numSet]=minVal;	
          break;
          case 6: newval[numSet]+=50;	
                  if(newval[numSet]>=maxVal) newval[numSet]=maxVal;	
          break;
          case 7: newval[numSet]-=50; 
                  if(newval[numSet]<=minVal) newval[numSet]=minVal;	
          break;
          case 3: 
            GUI_FillRectangle(0, Y_top, lcddev.width, lcddev.height, fillScreen);
            GUI_WriteString(lcddev.width/2-90,lcddev.height/2-60, "ВИКОНАЮ ЗАПИС!", Font_11x18, GREEN, BLACK);
            if(numSet<4) upv.pv.set[numSet+8] = newval[numSet];     // установим новые значения
            else dataRAM.config.koff[upv.pv.modeCell][numSet-4] = newval[numSet];
            uint32_t er = writeData();        // запишем значения во FLASH
            if(er) GUI_WriteString(lcddev.width/2-40,lcddev.height/2-20, "ПОМИЛКА!", Font_11x18, YELLOW, RED);
            else GUI_WriteString(lcddev.width/2-10,lcddev.height/2+20, "OK", Font_11x18, GREEN, BLACK);
            HAL_Delay(1000);
            displ_num = 5; NEWBUTT = 1; break;
        }
        item = 10;
        break;
   case 7://--------- вибір ШВИДКІСТІ обертання ----------------------------------
        switch (item){
          case 0: displ_num = 2; numSet = oldNumSet; NEWBUTT = 1; break;
          case 1: if(++numSet==MAX_SPEED) numSet = 0;	break;
          case 2: displ_num = 8; NEWBUTT = 1;
                  newval[0] = speedData[numSet][1];
                  maxVal=newval[0]+100, minVal=newval[0]-100;
                  if(maxVal>0xFFF) maxVal = 0xFFF;
                  sendToI2c(newval[0]);
            break;
          case 3: 
            GUI_FillRectangle(0, Y_top, lcddev.width, lcddev.height, fillScreen);
            GUI_WriteString(lcddev.width/2-90,lcddev.height/2-60, "ВИКОНАЮ ЗАПИС!", Font_11x18, GREEN, BLACK);
            upv.pv.set[VENT] = numSet;     // установим новые значения
            for(uint8_t i=0;i<MAX_SPEED;i++){
              for(uint8_t x=0;x<2;x++){
                dataRAM.config.speedData[i][x] = speedData[i][x];
              }
            }
            if(WORK|VENTIL|PURGING) sendToI2c(speedData[upv.pv.set[VENT]][1]);
            uint32_t er = writeData();        // запишем значения во FLASH
            if(er) GUI_WriteString(lcddev.width/2-40,lcddev.height/2-20, "ПОМИЛКА!", Font_11x18, YELLOW, RED);
            else GUI_WriteString(lcddev.width/2-10, lcddev.height/2+20, "OK", Font_11x18, GREEN, BLACK);
            HAL_Delay(1000);
            displ_num = 2; NEWBUTT = 1; numSet = oldNumSet; break;
        }
        item = 10;
        break;
    case 8://--------- ЗМІНА ЗНАЧЕННЯ ШВИДКІСТІ обертання -------------------------
      switch (item){
          case 0: displ_num = 7; NEWBUTT = 1; break;
          case 1: newval[0]+=5;	
                  if(newval[0]>maxVal) newval[0]=maxVal;
                  sendToI2c(newval[0]);
          break;
          case 2: newval[0]-=5; 
                  if(newval[0]<minVal) newval[0]=minVal;
                  sendToI2c(newval[0]);
          break;
          case 3: 
            speedData[numSet][1] = newval[0];   // установим новые значения
            sendToI2c(0);                       // выключим вентилятор
            displ_num = 7; NEWBUTT = 1; break;
        }
        item = 10;
        break;
    }
    if(displ_num) resetDispl = 180;  // 3 min.
}
