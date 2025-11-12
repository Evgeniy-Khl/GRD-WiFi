/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "..\Lib\st7796\myLCD.h"
#include "..\Lib\st7796\myGUI.h"
#include "..\Lib\Touch\XPT2046_touch.h"
#include "..\Lib\ds18b20\ds18b20.h"
#include "tft_proc.h"
#include "procedure.h"
#include "displ.h"
#include "nvRam.h"
#include "tftArcFill.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

char buffTFT[40];
const char* modeName[4]={"СУШЫННЯ","ОБЖАРКА","ВАРЫННЯ","КОПЧЕННЯ"};
const char* setName[MAX_SET]={"t КАМЕРИ","t ПРОДУКТА","t ДИМА","ТРИВАЛЫСТЬ","ШВИДКЫСТЬ","ТАЙМ.ON","ТАЙМ.OFF","ЫНШЕ"};
const char* otherName[MAX_OTHER]={"ПРОДУВАННЯ","АВАРЫЯ","ГЫСТЕРЕЗ","ОХОЛОДЖ.","Prop","Integ","Diff"};
const char* relayName[7]={"ПЫД","НАГРЫВ","ТАЙМЕР","ВОЛОГА","ЕЛЕКТРО","Кл.ДИМА","Кл.ВОДИ"};
//const char* analogName[2]={"ВЕНТИЛ.","ЫНШЕ"};
//        2.00V        3.15V        4.30V        5.45V        6.60V        7.75V        8.90V        10.00V
//={{1000,0x2F4},{1200,0x4A6},{1400,0x658},{1600,0x80A},{1800,0x9BC},{2000,0xB6E},{2200,0xD20},{2400,0xFFF}};//d=434->1.15V
//={{1000,0x2F4},{1200,0x4A6},{1400,0x655},{1600,0x804},{1800,0x9B6},{2000,0xB65},{2200,0xD14},{2400,0xFFF}};//d=434+коррекция
uint16_t speedData[MAX_SPEED][2], arhErrors[15];
int16_t pvTH, pvRH, tmrCounter, resetDispl=0, displOff=DISPLAYOFF;
uint16_t touch_x, touch_y, Y_str, X_left, Y_top, Y_bottom, fillScreen, color0, color1, checkSmoke, checkTime;
uint8_t displ_num=0, oldNumSet, buttonAmount, lost;
uint8_t timer10ms, tmrVent, ticBeep, pwTriac, invers;
uint8_t familycode[MAX_SENSOR][8], dsErr[MAX_SENSOR];
int8_t ds18b20_amount, numSet=0, tmrWater;
int8_t relaySet[8]={-1,-1,-1,-1,-1,-1,-1,-1};
int8_t analogSet[2]={-1,-1};
uint8_t analogOut[2]={0};

union Upv upv;
union Byte portFlag;
union Byte relayOut;
union d4v crc;
PIDController pid;
#ifdef MANUAL_CHECK
  float flT0=320, dpv0;
#endif

//uint16_t xpos; 
//uint16_t ypos; 
//uint8_t radius; 
//int16_t value; 
//int16_t sp;

GrafDispl grafDispl[2] = {
    { 80,160,80, 0, 0},    // Инициализация grafDispl[0]
    {240,160,80, 0, 0},    // Инициализация grafDispl[1]
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM1_Init(void);
static void MX_CRC_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//-------- Обратный вызов с истекшим периодом --------------
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if(htim->Instance == TIM1){ //check if the interrupt comes from TIM1 (10 ms)
    checkTime++; timer10ms++;
    if(pwTriac) --pwTriac; else {
      TRIAC = OFF;                    // отключить (SSR-25DA)
      invers = ~relayOut.value;
      HAL_I2C_Master_Transmit(&hi2c1,0x4E,&invers,1,1000);
    }
    if(ticBeep){ --ticBeep; HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);}// бипер
    else {HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);}
  }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  int16_t i16;
  uint16_t u16;
//  uint8_t temp=0, pvspeed=0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_TIM1_Init();
  MX_CRC_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  u16 = sendToI2c(0);//  отключение вентилятора
  
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);

  fillScreen = BLACK; color0 = WHITE;
  Y_bottom=lcddev.height-22; Y_str = 5;
  LCD_Init(USE_VERTICAL0);
  GUI_Clear(fillScreen);
  if((lcddev.dir&1)==0) X_left = 20; else X_left = 100;
  GUI_WriteString(35, Y_str, "GRD Max", Font_16x26, WHITE, fillScreen);
  GUI_WriteString(165, Y_str+5, " v 4.1.6", Font_11x18, WHITE, fillScreen);
  Y_str = Y_str+18+35;
  
  i16 = initData();
  ds18b20_port_init();      // линия 1-Wire
  ds18b20_checkSensor(4);   // check DS18B20 sensors: only 4 pcs
 
  switch (i16){
  	case 0: GUI_WriteString(5, Y_str, "Ыныцыалызацыя успышна.", Font_11x18, GREEN, BLACK);	break;
  	case 1: GUI_WriteString(5, Y_str, "Первинна ыныцыалызацыя.", Font_11x18, YELLOW, BLACK);	break;
    case 3: GUI_WriteString(5, Y_str, "Помилки читання FLASH!", Font_11x18, YELLOW, RED);	break;
  	default:GUI_WriteString(5, Y_str, "Невыдома помилка!", Font_11x18, MAGENTA, BLACK);	break;
  }
  Y_str = Y_str+18+5;
  
  if(i16){
    sprintf(buffTFT,"Check sum: 0x%08X",dataRAM.config.checkSum);
    GUI_WriteString(5, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
    Y_str = Y_str+18+5;
    sprintf(buffTFT,"Number of saves: %u",dataRAM.config.countSave);
    GUI_WriteString(5, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
    Y_str = Y_str+18+5;
    HAL_Delay(1000);
  }
  //---------------------------- линия 1-Wire -----------------------------------
  if(ds18b20_amount){
    for(uint8_t i=0;i<ds18b20_amount;i++) upv.pv.t[i]=1999;
    ds18b20_Convert_T();
  }
  sprintf(buffTFT,"Датчикыв температури: %d шт.",ds18b20_amount);
  GUI_WriteString(5, Y_str, buffTFT, Font_11x18, CYAN, BLACK);
  Y_str = Y_str+18+5;
  
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
  
  HAL_RTCEx_SetSecond_IT(&hrtc);          /* ------  таймер 1Гц.  период  1 сек.    ----*/
  HAL_TIM_Base_Start_IT(&htim1);          /* ------  таймер 100Гц.  период  10 мс.  ----*/
  
  NEWBUTT = ON;
  sprintf(buffTFT,"Kp=%2.1f; Ki=%1.4f; Kd=%3u;",pid.Kp,pid.Ki,pid.Kd);
  GUI_WriteString(5, Y_str, buffTFT, Font_11x18, YELLOW, BLACK);
  Y_str = Y_str+18+5;
  #ifdef MANUAL_CHECK
      sprintf(buffTFT,"WIDTH: %u; HEIGHT: %u",lcddev.width,lcddev.height);
      GUI_WriteString(5, Y_str, buffTFT, Font_11x18, YELLOW, BLACK);
      Y_str = Y_str+18+5;
      upv.pv.t[1]=220; upv.pv.t[2]=150; upv.pv.t[3]=200;
      int8_t dpv1 = 2, dpv2 = 2, dpv3 = 2, count;      
  #endif
  // -------------------------------------- КЛЮЧЕВАЯ ЛОГИКА ВОССТАНОВЛЕНИЯ ПОСЛЕ СБОЯ ПИТАНИЯ -----------------------------------
  // Читаем регистр с флагом если "магическое число" на месте, значит, питание пропало во время сушки.
  if (HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_DRYING_FLAG) == DRYING_PROCESS_FLAG_MAGIC){
    // Восстанавливаем время.
    uint32_t current_time_minutes = HAL_RTCEx_BKUPRead(&hrtc, BKP_REG_REMAINING_TIME);
    sTime.Hours = current_time_minutes/60;
    sTime.Minutes = current_time_minutes%60;
    NEWBUTT = 1; startPrg(1);
    GUI_WriteString(5, Y_str, "Виявлений збый живлення!", Font_11x18, RED, BLACK);
    Y_str = Y_str+18+5;
    sprintf(buffTFT,"Выдновлення з %u год. %u хв.",sTime.Hours, sTime.Minutes);
    GUI_WriteString(5, Y_str, buffTFT, Font_11x18, YELLOW, BLACK);
    Y_str = Y_str+18+5;
    ticBeep=255;
    HAL_Delay(5000);
    ticBeep=255;
    HAL_Delay(5000);
  } else {
    GUI_WriteString(5, Y_str, "Звичайний старт!", Font_11x18, GREEN, BLACK);
    Y_str = Y_str+18+5;
  }
  // ------------------------------------------------- КОНЕЦ ЛОГИКИ ВОССТАНОВЛЕНИЯ ----------------------------------------------------
  HAL_Delay(800);
  temperature_check();
  for (i16 = 0; i16 < ds18b20_amount; i16++){
      sprintf(buffTFT,"Датчик N%u = %3.1f$ ", i16+1,(float)upv.pv.t[i16]/10);
      GUI_WriteString(5, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
      Y_str = Y_str+18+5;
  }
  ticBeep=50;
  HAL_Delay(3000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
        Y_str = 5; X_left = 5;
    //-------------------------- ТАЧСКРИН ---------------------------------------
    if(XPT2046_TouchPressed()&& checkTime>40){
      uint8_t butt_num;
      if(XPT2046_TouchGetCoordinates(&touch_x, &touch_y)){
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);// включение дисплея
        displOff=DISPLAYOFF;
        for (butt_num=0; butt_num<buttonAmount; butt_num++){
            if(contains(touch_x, touch_y, butt_num)) break;     // проверка попадания новой координаты в область кнопки
        }
      }
      checkButtons(butt_num);                           // проверка нажатой кнопки
      if(displ_num==0 && !(WORK|VENTIL|PURGING)){
        if(topDispl(touch_x, touch_y)) {newval[0] = upv.pv.modeCell; newval[1]=-1; newval[2]=-1; displ_num = 4; butt_num = 10; resetDispl = 180; NEWBUTT = 1;} // ЗМІНА РЕЖИМУ
      }
      checkTime = 0; CHECK = ON;
    }
    
    // ----------- УВЛАЖНИТЕЛЬ только в режиме варка modeCell==2 и TMON && TMOFF !=0 --------------------
    if(WORK && upv.pv.modeCell==2){
      if(upv.pv.set[TMON]!=0 && upv.pv.set[TMOFF]!=0){
        if(timer10ms){                  // шаг отсчета интервала таймера 10 милисек.
          timer10ms=0; 
          HUMIDI=humidifier(HUMIDI);    // проверим выход на увлажнитель
          invers = ~relayOut.value;
          HAL_I2C_Master_Transmit(&hi2c1,0x4E,&invers,1,1000);
        }
      }
    }
    
    //-------------- Начало проверки каждую 1 сек. -----------------------
    if(CHECK){ CHECK = OFF; upv.pv.errors=0;  //if(++temp>10) {temp=0; ++pvspeed; pvspeed&=7; upv.pv.t[1] = speedData[pvspeed][0]; sendToI2c(speedData[pvspeed][1]);}
    upv.pv.dsplPW = 0;  
    if(resetDispl) --resetDispl; 
    else if(displ_num){displ_num = 0; NEWBUTT = 1; displOff=DISPLAYOFF;}  // возврат к главному дисплею
    else if(displOff) --displOff;
    else HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);       // отключение дисплея через 5 минут
    
    #ifndef MANUAL_CHECK
      temperature_check();
    #endif
      //---------------------------------- Проверка работы вентилятора -------------------------------------
      if(VENTIL){
        if(HAL_GPIO_ReadPin(Input0_GPIO_Port, Input0_Pin) == GPIO_PIN_RESET) {SPEED=ON; tmrVent=0;} // если контакт замкнут
        else SPEED=OFF;
    #ifdef MANUAL_CHECK
        SPEED=ON; tmrVent=0;
    #endif
        if(tmrVent) --tmrVent;    // ожидаем замыкания контакта частотного преобразователя
        else if(SPEED) WORK=ON;
        else {upv.pv.errors |= ERR8; WORK=OFF; relayOut.value=OFF;}  // НЕ ПРАЦЮЭ ВЕНТИЛЯТОР
      }
      //------------------------------------------- В РАБОТЕ -----------------------------------------------
      if(WORK){
        TIMER=ON;         // всегда включен в работу
        if(upv.pv.modeCell==2){  // только в режиме варка modeCell==2
          if(HAL_GPIO_ReadPin(Input1_GPIO_Port, Input1_Pin) == GPIO_PIN_RESET){
            if(++tmrWater>5) {tmrWater=5; WATER=ON;}
          }
          else {
            if(--tmrWater<0) {tmrWater=0; WATER=OFF;}
          }
        }
        else WATER=OFF;
        //------------ устанавливаем color0 в соответсвии с отклонением ------------------------
        i16 = upv.pv.set[T0]*10 - upv.pv.t[0];           // величина ошибки регулирования датчика 0
        uint16_t abs16 = abs(i16);
        if(abs16 < upv.pv.set[HIST]) PERFECT=ON;         // Вышли на заданную температуру
        u16 = upv.pv.set[ALRM]*10;                     // привяжем к аварии
        
        if(i16<=0){
          if(abs16<u16) color0 = GREEN;         // норма
          else if(abs16>=u16 && abs16<u16*2){upv.pv.errors|=ERR5; color0 = ORANGE;} // ВІДХІЛЕННЯ ТЕМПЕРАТУРИ
          else {upv.pv.errors|=ERR3; color0 = RED;}    // ПЕРЕГРЕВ В КАМЕРЕ
        }
        else {
          if(abs16<u16) color0 = GREEN;         // норма
          else if(abs16>=u16*2){
            color0 = CYAN;                      // НИЖЕ нормы
            if(PERFECT) upv.pv.errors|=ERR5;           // ВІДХІЛЕННЯ ТЕМПЕРАТУРИ
          }
        }
        
        //------------ устанавливаем color1 в соответсвии с отклонением -------------------------
        i16 = upv.pv.set[T1]*10 - upv.pv.t[1];           // величина ошибки регулирования датчика 1
        abs16 = abs(i16);
        
        if(i16<=0){
          if(abs16<u16/2)color1 = GREEN;        // норма
          else if(abs16>=u16/2 && abs16<u16) color1 = ORANGE;  // ВІДХІЛЕННЯ ТЕМПЕРАТУРИ
          else {upv.pv.errors|=ERR4; color1 = RED;}    // ПЕРЕГРЕВ В ПРОДУКТЕ
        }
        else {
          if(abs16<u16/2)color1 = GREEN;        // норма
          else if(abs16>=u16) color1 = CYAN;    // НИЖЕ нормы
        }
        
        // ---------------------------------------- НАГРЕВАТЕЛЬ / ОХЛАДИТЕЛЬ -------------------------------------
        //------ работает как нагреватель
        if(upv.pv.t[0]<1999 && upv.pv.t[0]>1){
          i16 = Relay(upv.pv.set[T0]*10 - upv.pv.t[0], upv.pv.set[HIST]);   // величина ошибки температуры воздуха
        }
        if(ds18b20_amount>1 && upv.pv.t[1]<1999){             // величина ошибки температурs среды
          u16 = Relay(upv.pv.set[T1]*10 - upv.pv.t[1], 0);
        }
        if(u16==ON) pwTriac = UpdatePID(&pid,0);            // ПИД нагреватель
        else {i16 = OFF; upv.pv.errors &= ~ERR5;}
        if(pwTriac) TRIAC = ON;                             // включить (SSR-25DA)
        upv.pv.dsplPW = pwTriac;
        if(upv.pv.dsplPW>100) upv.pv.dsplPW = 100;
        //------ работает как охладитель
        if(upv.pv.set[CHILL]&1){  
          i16 = Relay(upv.pv.t[0] - upv.pv.set[T0]*10, upv.pv.set[HIST]);
          if(upv.pv.t[0] > BEGINCOOL) i16 = OFF;              // температура выше которой ЗАПРЕЩЕНО включение охлаждения
        }
        switch (i16){
          case ON:  HEATER = ON;  break;
          case OFF: HEATER = OFF; break;
        }
        
        //-------------------------- Только для режима КОПЧЕНИЯ ---------------------------------
        if(upv.pv.modeCell==3){
          ELECTRO = ignition(ELECTRO);
          i16 = upv.pv.set[T2]*10 - upv.pv.t[2];   // величина ошибки регулирования датчика 2 (Дым)
          if(++checkSmoke>CHKSMOKE){      // (відхилення 2 грд.Ц) ТЕМПЕРАТУРЫ ДЫМА
            checkSmoke=CHKSMOKE;
            if(abs(i16) > upv.pv.set[ALRM]*10*2) upv.pv.errors|=ERR6;
          }
          u16 = Relay(i16, upv.pv.set[HIST]);  // величина ошибки температуры дыма
          switch (u16){
            case ON:  SMOKE = ON;  break;
            case OFF: SMOKE = OFF; break;
          }
        }
        
        //-------------------------- Только для режима ВАРЕНИЯ ---------------------------------
        if(upv.pv.modeCell==2){
          if(upv.pv.set[TMON]==0 || upv.pv.set[TMOFF]==0){
            i16 = upv.pv.set[T3]*10 - upv.pv.t[3];     // величина ошибки регулирования датчика 3 (Влажность)
            u16 = Relay(i16, upv.pv.set[HIST]);      // ЧЕТВЕРТЫЙ датчик - датчик влажности
            if(upv.pv.t[0] < BEGINHUM) u16=OFF; // запрет увлажнения при температуре ниже 40 грд.
            switch (u16){
              case ON:  HUMIDI = ON;  break;
              case OFF: HUMIDI = OFF; break;
            }
          }
        }
//        else HUMIDI = OFF;

#ifdef MANUAL_CHECK
        //?????? Програмное задание температур ??????????
        count++;
        //-----температура воздуха------
        dpv0 = (float)pid.pPart/500 + (float)(pid.output-5)/100;
        flT0+=dpv0;
        upv.pv.t[0] = flT0;
        int16_t pverr = set[T0]*10 - upv.pv.t[0];
        //----температура среды------
        if(count>3){ count=0;
          pverr = set[T1]*10 - upv.pv.t[1];
          dpv1 =-1;
          if(pverr>200) dpv1 = 6;
          else if(pverr>100) dpv1 = 4;
          else if(pverr>50) dpv1 = 2;
          else if(pverr>10) dpv1 = 1;
          upv.pv.t[1]+=dpv1;
        }
        //-----температура дыма---------
        pverr = set[T2]*10 - upv.pv.t[2];
        if(pverr>50) dpv2 = 5;
        else if(pverr>25) dpv2 = 1;
        else if(pverr<-25) dpv2 = -1;
        if(i16==OFF) dpv2=0;
        upv.pv.t[2]+=dpv2;
        //----влажный датчик--------
        pverr = set[T3]*10 - upv.pv.t[3];
        if(pverr>150) dpv3 = 5;
        else if(HUMIDI==ON) dpv3 = 1;
        else if(HUMIDI==OFF) dpv3 = -1;
        upv.pv.t[3]+=dpv3;
        //????????????????????????????????????????????????
#endif
        
        //------------------------- ЗВЕРШЕНИЕ текущего режима ----------------------------------
        if(upv.pv.set[TMR0]>0){                                // если TMR0>0 то завершение режима по таймеру
          u16 = sTime.Hours*60 + sTime.Minutes;         // всего в минутах
          i16 = (upv.pv.set[TMR0] - u16)*60 - sTime.Seconds;   // осталось до выключения в секундах
          if(i16<30) ticBeep = 5;                       // включить сигнал
          if(i16<=0){                                   // завершение режима
            portFlag.value = OFF; PURGING=ON; relayOut.value=OFF; ticBeep=200;
            //------- далее продувка ---------
              sTime.Hours=0; sTime.Minutes=0; sTime.Seconds=0;
              HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          }
        }
        else if(ds18b20_amount==1){      // если только 1 датчик и продолжительность 0 то завершение по температуре камеры.          
          i16 = Relay(upv.pv.set[T0]*10 - upv.pv.t[0], 0);   // температура камеры
          if(i16==OFF){
            portFlag.value = OFF; PURGING=ON; relayOut.value=OFF; ticBeep=200;
            //------- далее продувка ---------
            sTime.Hours=0; sTime.Minutes=0; sTime.Seconds=0;
            HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          }
        }
        else if(ds18b20_amount>1){      // если датчиков много и продолжительность 0 то завершение по температуре среды.          
          i16 = Relay(upv.pv.set[T1]*10 - upv.pv.t[1], 0);   // температура камеры
          if(i16==OFF){
            portFlag.value = OFF; PURGING=ON; relayOut.value=OFF; ticBeep=200;
            //------- далее продувка ---------
            sTime.Hours=0; sTime.Minutes=0; sTime.Seconds=0;
            HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          }
        }
        
      } //--------------------------- КОНЕЦ в работе -----------------------------------------------
      else if(PURGING){
        u16 = sTime.Minutes*60+sTime.Seconds;           // всего в секундах
        if(u16>=upv.pv.set[TMR1]) {PURGING=OFF; sendToI2c(0); NEWBUTT=ON; ticBeep=200;}
      }
      //------ Проверка на ручное управление ---------------------------------------
      for (i16=0;i16<7;i16++){
          if(relaySet[i16]==1) relayOut.value |= (1<<(i16)); // ручной On
          if(relaySet[i16]==0) relayOut.value &= ~(1<<(i16));// ручной Off
      }
      invers = ~relayOut.value;
      HAL_I2C_Master_Transmit(&hi2c1,0x4E,&invers,1,1000);
//      for (i16=0;i16<2;i16++){  // ручное управление аналоговыми выводами
//        if(analogSet[i16]>-1) analogOut[i16]=analogSet[i16];
//      }
      if(upv.pv.errors && (upv.pv.set[CHILL]&2)==0){    // 2-отключены аварийные звуковые сигналы
        switch (upv.pv.errors){
          case 0x01: ticBeep = 80; break; // ПОМИЛКА ДАТЧИКА N1
          case 0x02: ticBeep = 80; break; // ПОМИЛКА ДАТЧИКА N2
          case 0x04: ticBeep = 80; break; // ПОМИЛКА ДАТЧИКА N3
          case 0x08: ticBeep = 80; break; // ПОМИЛКА ДАТЧИКА N4
          case ERR3: ticBeep = 80; break; // ПЕРЕГРЫВ В КАМЕРI
          case ERR4: ticBeep =120; break; // ПЕРЕГРЫВ В ПРОДУКТI
          case ERR5: ticBeep = 10; break; // ВЫДХЫЛЕННЯ ТЕМПЕРАТУРИ
          case ERR6: ticBeep = 20; break; // ВЫДХЫЛЕННЯ ТЕМПЕРАТУРИ ДИМA
          case ERR7: ticBeep = 60; break; //
          case ERR8: ticBeep = 60; break; // НЕ ПРАЦЮЭ ВЕНТИЛЯТОР
          default: 
            if(upv.pv.errors==0x0C) ticBeep = 80;
            else ticBeep =200;
          break;
        }
      }
      if(upv.pv.errors) ALARM = ON; else ALARM = OFF;  // световой сигнал ошибки
      display();
      // -------------------- UART ----------------------------------
      upv.pv.portFlag = portFlag.value;
      upv.pv.pvOut = relayOut.value;
      upv.pv.fanSpeed = speedData[upv.pv.set[VENT]][0];
      upv.pv.currHour = sTime.Hours;
      upv.pv.currMin = sTime.Minutes;
      upv.pv.currSec = sTime.Seconds;
      //-------------------------------------------------------------
      crc.val[0] = 0;
      for(uint8_t i=0;i<RAMPV_SIZE;i++){
        crc.val[0] += upv.receivedData[i];
        crc.val[0] ^= (crc.val[0]>>2);
      }
      HAL_UART_Transmit(&huart1,(uint8_t*)upv.receivedData,RAMPV_SIZE,0x1000);// отправка upv
      HAL_UART_Transmit(&huart1,(uint8_t*)crc.data,4,0x1000);   // отправка crc, "\r=<CR>, \n=<LF>"
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 719;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 4800;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, T_CS_Pin|TFT_DC_Pin|TFT_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, TFT_RST_Pin|Beep_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : Input0_Pin */
  GPIO_InitStruct.Pin = Input0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Input0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : T_IRQ_Pin */
  GPIO_InitStruct.Pin = T_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(T_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : T_CS_Pin TFT_CS_Pin */
  GPIO_InitStruct.Pin = T_CS_Pin|TFT_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_Pin TFT_DC_Pin */
  GPIO_InitStruct.Pin = LED_Pin|TFT_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : OneWR_Pin Input1_Pin */
  GPIO_InitStruct.Pin = OneWR_Pin|Input1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : TFT_RST_Pin */
  GPIO_InitStruct.Pin = TFT_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TFT_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Beep_Pin */
  GPIO_InitStruct.Pin = Beep_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Beep_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
