#include "main.h"
#include "tft_proc.h"
#include "..\Lib\st7796\myLCD.h"
#include "..\Lib\st7796\myGUI.h"
#include "..\Lib\ds18b20\ds18b20.h"
#include "displ.h"
#include "rtc.h"
#include "nvRam.h"
#include "tftArcFill.h"
#include "procedure.h"

extern char buffTFT[];
extern const char* setName[];
extern const char* modeName[];
extern const char* otherName[];
extern const char* relayName[];
extern const char* analogName[];
extern uint8_t displ_num, ds18b20_amount, ds18b20_num, familycode[][8], newDate, ticBeep;
extern uint16_t speedData[MAX_SPEED][2], fillScreen, Y_str, X_left, Y_top, Y_bottom, color0, color1, mainTimer, tmrCounter, checkSmoke;
extern int8_t numSet, numDate;
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef sTime;
extern RTC_DateTypeDef sDate;
extern union DataRam dataRAM;
extern int8_t relaySet[8],analogSet[2],analogOut[2];
extern float flT0, dpv0;

int16_t min(int16_t a, int16_t b ) {
   return a < b ? a : b;
}

int16_t max(int16_t a, int16_t b ) {
   return a > b ? a : b;
}

//--------- ОСНОВНОЙ ЭКРАН ----------------------
void displ_0(void){
  uint8_t sensor;
  Y_str = Y_top+15;  // 15
  const char* point[3] = {"  ","  ","  "};
  uint32_t curTime = sTime.Hours*3600 + sTime.Minutes*60 + sTime.Seconds;
  if(WORK){
//    if(INSIDE) point[1] = "->";
    if(upv.pv.set[TMR0]) point[2] = "->";
    else  point[0] = "->";
  }
  if(NEWBUTT){
    GUI_Clear(fillScreen);
    initializeButtons(3,1,40);// 3 колонки; одна строка; высота 40
    if(WORK|VENTIL|PURGING) drawButton(MAGENTA, 0, "СТОП");
    else drawButton(GREEN, 0, "ПУСК");
    drawButton(YELLOW, 1, "Керуван.");
    drawButton(CYAN, 2, "Налаштув.");
  }
  //------------- BackUp -------------------
  uint8_t oldMinutes = sTime.Minutes;
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  if(oldMinutes != sTime.Minutes && WORK){
    uint32_t current_time_minutes = sTime.Hours*60 + sTime.Minutes;
    newMitutesBackUp(current_time_minutes);
  }
  //----------------------------------------
  X_left = 15;
  if(WORK) GUI_WriteString(X_left, Y_str, " ON  ", Font_16x26, BLACK, GREEN);
  else if(VENTIL) GUI_WriteString(X_left, Y_str, "VENT ", Font_16x26, BLACK, YELLOW);
  else if(PURGING) GUI_WriteString(X_left, Y_str, "PURG ", Font_16x26, BLACK, CYAN);
  else {
    GUI_WriteString(X_left, Y_str, " OFF ", Font_16x26, YELLOW, RED);
    color0 = WHITE; color1 = WHITE;
  }
  
  GUI_WriteString(120, Y_str, "РЕЖИМ:", Font_11x18, YELLOW, fillScreen);
  sprintf(buffTFT,"%8s", modeName[upv.pv.modeCell]);
  GUI_WriteString(190, Y_str, buffTFT, Font_11x18, BLACK, WHITE);
  Y_str = Y_str+26+15; //56
  //----------------------
  X_left = 20;
  if(upv.pv.errors & 0x01) GUI_WriteString(X_left, Y_str, " ПОМИЛКА  ", Font_11x18, YELLOW, RED);
  else if(upv.pv.errors & ERR3) GUI_WriteString(X_left, Y_str, " ПЕРЕГРIВ ", Font_11x18, YELLOW, RED);
  else if(upv.pv.errors & ERR5) GUI_WriteString(X_left, Y_str, "ВIДХIЛЕННЯ", Font_11x18, YELLOW, RED);
  else GUI_WriteString(X_left, Y_str, "  КАМЕРА  ", Font_11x18, YELLOW, fillScreen);
  //----------------------
  X_left = 180;
  if(upv.pv.errors & 0x02) GUI_WriteString(X_left, Y_str, " ПОМИЛКА  ", Font_11x18, YELLOW, RED);
  else if(upv.pv.errors & ERR4) GUI_WriteString(X_left, Y_str, " ПЕРЕГРIВ ", Font_11x18, YELLOW, RED);
  else GUI_WriteString(X_left, Y_str, "  ПРОДУКТ ", Font_11x18, YELLOW, fillScreen);
  //----------------------
  if(grafDispl[0].value != upv.pv.t[0] || NEWBUTT) {
      grafDispl[0].value = upv.pv.t[0];
      diagram(grafDispl[0], color0);
  }
  if(grafDispl[1].value != upv.pv.t[1] || NEWBUTT) {
      grafDispl[1].value = upv.pv.t[1];
      diagram(grafDispl[1], color1);
  }
  NEWBUTT = OFF;
  Y_str = 240;
  //-------------------------------------------------------------------------------------------
  X_left = 30;
  GUI_WriteString(X_left, Y_str, "   ТРИВАЛIСТЬ РЕЖИМУ   ", Font_11x18, YELLOW, fillScreen);
  Y_str = Y_str+18+15; // 204
  if(WORK|PURGING){
    sprintf(buffTFT,"%2s %02u:%02u:%02u ", point[2], sTime.Hours, sTime.Minutes, sTime.Seconds);
    GUI_WriteString(15, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
  }
  uint16_t tmr = upv.pv.set[TMR0];
  if(PURGING) {tmr = upv.pv.set[TMR1]; sprintf(buffTFT," %iхвл.%02iсек.", tmr/60, tmr%60);}
  else sprintf(buffTFT," %iгод.%02iхвл.", tmr/60, tmr%60);
  GUI_WriteString(165, Y_str, buffTFT, Font_11x18, BLACK, WHITE);
  Y_str = Y_str+18+15;  // 237
  
  if(upv.pv.modeCell<3 && VENTIL && curTime>2 && curTime<12){
    ticBeep = 10;
    GUI_FillRectangle(42, Y_str, lcddev.width - 75, 60, RED);// Y_str = 344+56 = 400
    if(upv.pv.modeCell) GUI_WriteString(70, Y_str+5, "ЗАКРИЙТЕ ЗАСЛЫНКИ", Font_11x18, YELLOW, RED);
    else GUI_WriteString(65, Y_str+5, "ВЫДКРИЙТЕ ЗАСЛЫНКИ", Font_11x18, YELLOW, RED);
    GUI_WriteString(110, Y_str+35, "вентиляцыъ!", Font_11x18, YELLOW, RED);
//    Y_str = Y_str+18+15; // 270
  }
  else if(upv.pv.modeCell<3 && VENTIL && curTime>2 && curTime==12) GUI_FillRectangle(42, Y_str, lcddev.width - 75, 60, fillScreen); 
  else if(upv.pv.modeCell>1)
  {
    if(upv.pv.modeCell==2){
      sensor = T3; 
      if(upv.pv.errors & 0x0008) GUI_WriteString(80, Y_str, "ПОМИЛКА ДАТЧИКА", Font_11x18, YELLOW, RED);
      else GUI_WriteString(80, Y_str, "ВОЛОГИЙ ДАТЧИК ", Font_11x18, YELLOW, fillScreen);
    }
    else if(upv.pv.modeCell==3){
      sensor = T2;
      if(upv.pv.errors & 0x0004) GUI_WriteString(30, Y_str, "    ПОМИЛКА ДАТЧИКА    ", Font_11x18, YELLOW, RED);
      else if(upv.pv.errors & ERR6){
        if(upv.pv.set[sensor]*10 > upv.pv.t[sensor]) GUI_WriteString(30, Y_str, "ДИМ НИЗЬКОЪ ТЕМПЕРАТУРИ", Font_11x18, YELLOW, RED);
        else  GUI_WriteString(30, Y_str, "ДИМ ВИСОКОЪ ТЕМПЕРАТУРИ", Font_11x18, YELLOW, RED);
      }
      else GUI_WriteString(30, Y_str, "       ДАТЧИК ДИМУ     ", Font_11x18, YELLOW, fillScreen);
    }
    Y_str = Y_str+18+15; // 270
    
    if(upv.pv.t[sensor]<1000) sprintf(buffTFT,"%3.1f$ ",(float)upv.pv.t[sensor]/10);
    else if(upv.pv.t[sensor]<1270) sprintf(buffTFT,"%5d$ ", upv.pv.t[sensor]/10);
    else sprintf(buffTFT," ---  ");
    GUI_WriteString(55, Y_str, buffTFT, Font_16x26, WHITE, BLACK);
    sprintf(buffTFT,"%3i.0$ ", upv.pv.set[sensor]);
    GUI_WriteString(175, Y_str, buffTFT, Font_16x26, BLACK, WHITE);
    Y_str = Y_str+26+15;  // 311
  }
  
  if(VENTIL && curTime > 12){
    if(upv.pv.errors & ERR8) GUI_WriteString(30, Y_str, "  НЕ ПРАЦЮЭ ВЕНТИЛЯТОР  ", Font_11x18, YELLOW, RED);
    else {
      sprintf(buffTFT,"%12s: %4i об/хвл.", setName[4], speedData[upv.pv.set[VENT]][0]);
      GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
    }
  }  
  GUI_FillRectangle(0, 0, 1, 1, fillScreen);//???????????????????????????????????
}

//-------------------------------- СТАН ВЫХОДІВ ------------------------------------------------------
void displ_1(void){
 uint8_t i, bit;
 char txt[10];
 uint16_t color_txt, color_box; 
    Y_str = Y_top+10;
    if(NEWBUTT){ NEWBUTT = OFF;
      GUI_Clear(fillScreen);
      GUI_WriteString(X_left+60, Y_str,"СТАН ВИХОДЫВ",Font_11x18,YELLOW,fillScreen);
      initializeButtons(4,1,40);// четире колонки; одна строка; высота 40
      drawButton(BLUE, 0, "Вихыд");
      drawButton(YELLOW, 1, "Вибыр");
      drawButton(MAGENTA, 2, "+");
      drawButton(CYAN, 3, "-");
    }
//---- РЕЛЕЙНЫЕ ВЫХОДЫ ----
    Y_str = Y_str+18+5;
    for (i=0;i<7;i++){
        bit = 1<<i;
        sprintf(buffTFT,"%7s",relayName[i]);
        sprintf(txt," N%u: ",i+1);
        strcat(buffTFT,txt);
        if(relaySet[i]<0) strcat(buffTFT,"AUTO"); else if(relaySet[i]==1) strcat(buffTFT," ON "); else strcat(buffTFT," OFF");
        if(i == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
        GUI_WriteString(X_left+10, Y_str, buffTFT, Font_11x18, color_txt, color_box);
        if(relayOut.value & bit) color_box=YELLOW; else color_box=GRAY; // ILI9341_COLOR565(128, 128, 128);
        GUI_FillRectangle(X_left+200,Y_str,30,18,color_box);
        if(i==0){
          sprintf(buffTFT,"%3u %%", upv.pv.dsplPW);
          GUI_WriteString(X_left+240, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
        }
        Y_str = Y_str+18+5;
    }
//---- ВХОДЫ ----
    Y_str = Y_str+18+5;
    GUI_WriteString(X_left+40,Y_str, "ВХЫД N1:", Font_11x18, WHITE, BLACK);
    if(HAL_GPIO_ReadPin(Input0_GPIO_Port, Input0_Pin) == GPIO_PIN_RESET) color_box=YELLOW; else color_box=GRAY; // напряжение подано
    GUI_FillRectangle(X_left+150,Y_str,30,18,color_box);
    Y_str = Y_str+18+5;
    GUI_WriteString(X_left+40,Y_str, "ВХЫД N2:", Font_11x18, WHITE, BLACK);
    if(HAL_GPIO_ReadPin(Input1_GPIO_Port, Input1_Pin) == GPIO_PIN_RESET) color_box=YELLOW; else color_box=GRAY; // напряжение подано
    GUI_FillRectangle(X_left+150,Y_str,30,18,color_box);
//==============================================================================================================
#ifdef MANUAL_CHECK
    Y_str = Y_str+25+5;
    sprintf(buffTFT,"flT0=%2.3f; dpv0=%2.3f", flT0, dpv0);
    GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);

    Y_str = Y_str+18+5;
    sprintf(buffTFT,"D1%2x; D2%2x; D3%2x; D4%2x;", dsErr[0], dsErr[1], dsErr[2], dsErr[3]);
    GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
    Y_str = Y_str+18+5;
    sprintf(buffTFT,"Out=%+5d; T=%3.1f; E=%+3d", pid.output, (float)upv.pv.t[0]/10, pid.prev_error);
    GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
#endif
    Y_str = Y_str+18+5;
    sprintf(buffTFT,"pPart=%8.3f", pid.pPart);
    GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
    Y_str = Y_str+18+5;
    sprintf(buffTFT,"iPart=%8.3f", pid.iPart);
    GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
    Y_str = Y_str+18+5;
    sprintf(buffTFT,"dPart=%+4d.0   ", pid.dPart);
    GUI_WriteString(10, Y_str, buffTFT, Font_11x18, YELLOW, fillScreen);
}

//--------- НАЛАШТУВАННЯ ----------------------------------
void displ_2(void){
  char txt[12];
  int8_t i, sensor;
  uint16_t color_txt, color_box;
  float flSet;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Вихыд");
    drawButton(GREEN, 1, "v");
    drawButton(GREEN, 2, "^");
    drawButton(YELLOW, 3, "Вибыр");
  }
  Y_str = Y_str+10;
  for (i=-1; i<MAX_SET; i++){
    if(i==-1) sprintf(buffTFT,"       РЕЖИМ: %8s", modeName[upv.pv.modeCell]);
    else if(i==3) sprintf(buffTFT,"%12s: %iгод.%02iхвл.", setName[i], upv.pv.set[TMR0]/60, upv.pv.set[TMR0]%60);  // "ТРИВАЛIСТЬ"
    else if(i==4) sprintf(buffTFT,"%12s: %4i об/хвл.", setName[i], speedData[upv.pv.set[VENT]][0]);        // "ШВИДКIСТЬ"
    else if(i==5){
      if(upv.pv.set[TMON]){
        // если ВАРКА (modeCell==2) задается в mсек.[от 0.1сек. до 10 сек.] (период 10 mсек.)
        if(upv.pv.modeCell==2) flSet = (float)upv.pv.set[TMON]/10; else flSet = upv.pv.set[TMON];
        sprintf(buffTFT,"%12s: %2.1fсек.", setName[i], flSet);                                // "ТАЙМ.ON","ТАЙМ.OFF" 
      }
      else sprintf(buffTFT,"%12s:", "-----");
    }
    else if(i==6){
      if(upv.pv.set[TMOFF]){
        // если ВАРКА (modeCell==2) задается в mсек.[от 0.1сек. до 10 сек.] (период 10 mсек.)
        if(upv.pv.modeCell==2) flSet = (float)upv.pv.set[TMOFF]/10; else flSet = upv.pv.set[TMOFF];
        sprintf(buffTFT,"%12s: %2.1fсек.", setName[i], flSet);                                // "ТАЙМ.ON","ТАЙМ.OFF" 
      }
      else sprintf(buffTFT,"%12s:", "-----");
    }
    else if(i==7) sprintf(buffTFT,"%12s:", setName[i]);                                       // "IНШЕ"
    else {                                                                                    // "t КАМЕРИ","t ПРОДУКТА","t ДИМА"
      if(upv.pv.modeCell==2 && i==2) {sensor = T3; strcpy(txt,"t ВОЛОГОГО");}                        // "ВАРIННЯ"
      else if(upv.pv.modeCell==3 && i==2) {sensor = T2; sprintf(txt,"%12s",setName[i]);}             // "КОПЧЕННЯ"
      else {sensor = i; sprintf(txt,"%12s",setName[i]);}             
      if(upv.pv.set[sensor]){sprintf(buffTFT,"%12s: %3i$ ", txt, upv.pv.set[sensor]);} 
      else sprintf(buffTFT,"%12s:", "-----");
    }
    if(i == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
    GUI_WriteString(X_left, Y_str, buffTFT, Font_11x18, color_txt, color_box);
    Y_str = Y_str+18+5;
  }
}

//--------- ЗМІНА ТЕМПЕРАТУР ----------------------------------
void displ_3(void){
  char txt[12];
  float flSet;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,2,40);// четыре колонки; две строки; высота 40
    drawButton(BLUE, 0, "Отм.");
    drawButton(GREEN, 1, "+1");
    drawButton(GREEN, 2, "-1");
    drawButton(MAGENTA, 3, "Зап.");
    drawButton(YELLOW, 4, "+10");
    drawButton(YELLOW, 5, "-10");
    drawButton(CYAN, 6, "+50");
    drawButton(CYAN, 7, "-50");
  }
  Y_str = Y_str+50;
  
  if(numSet<3){
    if(numSet==2){
      if(upv.pv.modeCell==2) strcpy(txt,"t ВОЛОГОГО");   // "ВАРIННЯ"
      else sprintf(txt,"%12s",setName[numSet]);   // "КОПЧЕННЯ"
      GUI_WriteString(X_left+20, Y_str, txt, Font_11x18, WHITE, BLACK);
    }
    else {
      sprintf(buffTFT,"%12s:", setName[numSet]);  // "СУШЫННЯ","ОБЖАРКА"
      GUI_WriteString(X_left+20, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
    }
    sprintf(buffTFT,"%3i$", newval[numSet]);
    Y_str = Y_str-4;
    GUI_WriteString(X_left+180, Y_str, buffTFT, Font_16x26, WHITE, BLACK);
  }
  else {
    if(numSet==3) sprintf(buffTFT,"%12s: %iгод.%02iхвл.", setName[numSet], newval[numSet]/60, newval[numSet]%60);
    else if(numSet==5 || numSet==6){
      // если ВАРКА (modeCell==2) задается в mсек.[от 0.1сек. до 10 сек.] (период 10 mсек.)
      if(upv.pv.modeCell==2) flSet = (float)newval[numSet]/10; else flSet = newval[numSet];
      sprintf(buffTFT,"%12s: %2.1fсек.", setName[numSet], flSet);                                // "ТАЙМ.ON" "ТАЙМ.OFF"
    }
    GUI_WriteString(X_left+20, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
  }
}

//--------- ЗМІНА РЕЖИМУ ----------------------------------
void displ_4(void){
  uint8_t i;
  uint16_t color_txt, color_box;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Вихыд");
    drawButton(GREEN, 1, "v");
    drawButton(GREEN, 2, "^");
    drawButton(YELLOW, 3, "Вибыр");
  }
  if(newval[1]!=newval[0]){
    newval[1] = newval[0];
    Y_str = Y_str+50;
    for (i=0; i<MAX_MODE; i++){
      sprintf(buffTFT,"%8s", modeName[i]);
      if(i == newval[0]){color_txt = BLACK; color_box = GREEN;} else {color_txt = WHITE; color_box = GRAY;}
      GUI_FillRectangle(lcddev.width/2-70, Y_str-20, 140, 60, color_box);
      GUI_WriteString(lcddev.width/2-50, Y_str, buffTFT, Font_11x18, color_txt, color_box);
      Y_str = Y_str+60+5;
    }
  }
}

//--------- IНШЕ ----------------------------------
void displ_5(void){
  uint8_t i;
  uint16_t color_txt, color_box;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Вихыд");
    drawButton(GREEN, 1, "v");
    drawButton(GREEN, 2, "^");
    drawButton(YELLOW, 3, "Вибыр");
  }
  Y_str = Y_str+10;
  for (i=0; i<MAX_OTHER; i++){
    if(i==0) sprintf(buffTFT,"%12s: %3iсек.", otherName[i], upv.pv.set[TMR1]);   // "ПРОДУВАННЯ"
    else if(i==1) sprintf(buffTFT,"%12s: %3i$", otherName[i], upv.pv.set[ALRM]); // "АВАРИЯ"
    else if(i==2) sprintf(buffTFT,"%12s: %2.1f$", otherName[i], (float)upv.pv.set[HIST]/10); // "ГИСТЕРЕЗ"
    else if(i==3) sprintf(buffTFT,"%12s: %3i", otherName[i], upv.pv.set[CHILL]); // "ОХОЛОДЖ."
    else sprintf(buffTFT,"%12s: %3i", otherName[i], dataRAM.config.koff[upv.pv.modeCell][i-4]); // "Prop","Integ","Diff"
    if(i == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
    GUI_WriteString(X_left, Y_str, buffTFT, Font_11x18, color_txt, color_box);
    Y_str = Y_str+18+5;
  }
}

//--------- ЗМІНА IНШЕ ----------------------------------
void displ_6(void){
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,2,40);// четыре колонки; две строки; высота 40
    drawButton(BLUE, 0, "Отм.");
    drawButton(GREEN, 1, "+1");
    drawButton(GREEN, 2, "-1");
    drawButton(MAGENTA, 3, "Зап.");
    drawButton(YELLOW, 4, "+10");
    drawButton(YELLOW, 5, "-10");
    drawButton(CYAN, 6, "+50");
    drawButton(CYAN, 7, "-50");
  }
  Y_str = Y_str+50;
  sprintf(buffTFT,"%12s:", otherName[numSet]);
  GUI_WriteString(X_left+20, Y_str, buffTFT, Font_11x18, WHITE, BLACK);

  if(numSet==0) sprintf(buffTFT,"%3icek.", newval[numSet]);           // "ПРОДУВАННЯ"
  else if(numSet==1) sprintf(buffTFT,"%3i$", newval[numSet]);         // "АВАРИЯ"
  else if(numSet==2) sprintf(buffTFT,"%1.1f$", (float)newval[numSet]/10); // "ГИСТЕРЕЗ"
  else sprintf(buffTFT,"%4i", newval[numSet]);                        // "ОХОЛОДЖ.","Prop","Integ"

  Y_str = Y_str-4;
  GUI_WriteString(X_left+180, Y_str, buffTFT, Font_16x26, WHITE, BLACK);
}

//--------- вибір ШВИДКІСТІ обертання ----------------------------------
void displ_7(void){
  uint8_t i;
  uint16_t color_txt, color_box;
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Вихыд");
    drawButton(GREEN, 1, "v");
    drawButton(MAGENTA, 2, "Корек");
    drawButton(YELLOW, 3, "Вибыр");
  }
  Y_str = Y_str+10;
  for (i=0; i<MAX_SPEED; i++){
    sprintf(buffTFT,"%4u об/хвл.", speedData[i][0]);
    if(i == numSet){color_txt = BLACK; color_box = WHITE;} else {color_txt = WHITE; color_box = BLACK;}
    GUI_WriteString(X_left, Y_str, buffTFT, Font_11x18, color_txt, color_box);
    Y_str = Y_str+18+5;
  }
}

//--------- ЗМІНА ЗНАЧЕННЯ ШВИДКІСТІ обертання ----------------------------------
void displ_8(void){
  Y_str = Y_top; X_left = 5;
  if(NEWBUTT){ NEWBUTT = OFF;
    GUI_Clear(fillScreen);
    initializeButtons(4,1,40);// четыре колонки; одна строка; высота 40
    drawButton(BLUE, 0, "Отм.");
    drawButton(GREEN, 1, "+");
    drawButton(GREEN, 2, "-");
    drawButton(MAGENTA, 3, "Зап.");
  }
  Y_str = Y_str+50;
  sprintf(buffTFT,"Значення: 0x%03x выд.один.", newval[0]);
  GUI_WriteString(X_left+20, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
}

void display(void){
  switch (displ_num){
  	case 0: displ_0(); break;//- СТАН КАМЕРИ --
  	case 1: displ_1(); break;//- СТАН ВЫХОДІВ -
    case 2: displ_2(); break;//- НАЛАШТУВАННЯ -
    case 3: displ_3(); break;//- ЗМІНА ТЕМПЕРАТУР -
    case 4: displ_4(); break;//- ЗМІНА РЕЖИМУ -
    case 5: displ_5(); break;//- ІНШЕ -
    case 6: displ_6(); break;//- ЗМІНА ІНШЕ -
    case 7: displ_7(); break;//- вибір ШВИДКІСТІ обертання -
    case 8: displ_8(); break;//- ЗМІНА ЗНАЧЕННЯ ШВИДКІСТІ обертання -
  	default: displ_0();	break;//- СТАН КАМЕРИ -
  }
}
