#include "main.h"
#include "..\Lib\st7796\myLCD.h"
#include "tft_proc.h"
#include "procedure.h"
#include "nvRam.h"
#include "rtc.h"

extern char buffTFT[];
extern I2C_HandleTypeDef hi2c1;
extern uint16_t color0, color1, checkSmoke;
extern int16_t pvRH, tmrCounter;
extern uint16_t speedData[MAX_SPEED][2];
extern uint8_t familycode[MAX_SENSOR][8], ds18b20_amount, ticBeep, tmrVent;
extern uint16_t fillScreen, Y_str, X_left, Y_top, Y_bottom, color0, color1, mainTimer;

/**
  * @brief  Запускает процесс сушки с заданной продолжительностью.
  * @param  total_seconds: Общее время сушки в секундах.
  * @retval None
  */
void startBackUp(void){
    // Записываем "магическое число", чтобы при следующем старте знать, что процесс был прерван
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_DRYING_FLAG, DRYING_PROCESS_FLAG_MAGIC);
    HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_REMAINING_TIME, 0x0000);
}

/**
  * @brief  Останавливает процесс сушки (штатное завершение).
  * @retval None
  */
void stopBackUp(void){
  // Сбрасываем флаг процесса и сохраненное время
  HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_DRYING_FLAG, 0x0000);
  HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_REMAINING_TIME, 0x0000);
}

void newMitutesBackUp(uint32_t minutres){
  HAL_RTCEx_BKUPWrite(&hrtc, BKP_REG_REMAINING_TIME, minutres);
  #ifdef MANUAL_CHECK
    sprintf(buffTFT,"New minutres %4u хв.",minutres);
    GUI_WriteString(5, Y_bottom-20, buffTFT, Font_11x18, WHITE, BLACK);
  #endif
}

union b2{
    uint16_t val;
    uint8_t data[2];
  } mcp;

void startPrg(uint8_t repair)
{
  if(WORK|VENTIL|PURGING){
    portFlag.value = OFF; CHECK = ON; NEWBUTT=ON;   // если был в работе - все отключаем.
    sendToI2c(0);
    relayOut.value=OFF; color0 = WHITE; color1 = WHITE; ticBeep=100;
    stopBackUp();
  }
  else {          // после нажатия кнопки ПУСК
    VENTIL=ON; sendToI2c(speedData[upv.pv.set[VENT]][1]); tmrVent=20;// 20 сек. ожидания запуска вентилятора
    pid.iPart=0; ticBeep=100; upv.pv.errors=0; tmrCounter=2; checkSmoke=0; // (2сек.) произвольное значение задержки больше 0
    if(repair==0) {sTime.Hours=0; sTime.Minutes=0;}
    sTime.Seconds=0;
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    startBackUp();
  }
}

uint8_t Relay(int16_t err, uint8_t hst) // [n] канал № 1 или 2
{
  uint8_t x=2;
  if(err>hst) x = ON;         // включить
  if(err <= 0) x = OFF;       // отключить
  return x;
}

uint8_t humidifier(uint8_t value){
   if(value){
      if(tmrCounter) tmrCounter--; else {value=OFF; tmrCounter=upv.pv.set[TMOFF]*10;} // Длительность паузы 18*10=180*10msek.=1.8sek
   }
   else {
      if(tmrCounter) tmrCounter--; else {value=ON; tmrCounter=upv.pv.set[TMON]*10;}  // Длительность впрыска 6*10=60*10msek.=0.6sek
   }
  if(upv.pv.t[0] < BEGINHUM) value=OFF;        // запрет увлажнения при температуре ниже 40 грд.
  return value;
}

uint8_t ignition(uint8_t value){
  if(tmrCounter>=0){
    if(value){
        if(tmrCounter) tmrCounter--; else {value=OFF; tmrCounter=-1;}         // Больше не включается
    }
    else {
        if(tmrCounter) tmrCounter--; else {value=ON; tmrCounter=upv.pv.set[TMON];}   // Длительность розжига 180сек.
    }
  }
  else value=OFF;
  return value;
}

uint8_t UpdatePID(PIDController *pid, uint8_t cn){
 int16_t error, derivative;
  // Вычисление ошибки
  error = upv.pv.set[cn]*10 - upv.pv.t[cn];
  // Пропорциональная составляющая
  pid->pPart = (float)error * pid->Kp;
  // Интегральная составляющая
  pid->iPart += (float)error * pid->Ki;// * dt;
  // Дифференциальная составляющая
  derivative = (error - pid->prev_error);// / dt;
  pid->dPart = pid->Kd * derivative;
  // Сохраняем текущую ошибку для следующего вызова
  pid->prev_error = error;
  // Суммарное управляющее воздействие
  pid->output = pid->pPart + pid->iPart + pid->dPart;
  // Ограничение выходного значения и антивиндовинг
  if (pid->output > 100) pid->output = 110;
  else if (pid->output < 0) pid->output = 0;
  if (pid->pPart >= 100) pid->iPart = 0; // Сброс интеграла
  else if (pid->pPart <= -50) pid->iPart = 0; // Сброс интеграла

  error = pid->output;
  return error;
}

void permutation (char a, char b){
  uint8_t i, buff;
  for (i=0;i<9;i++) {
     buff = familycode[a][i];
     familycode[a][i] = familycode[b][i];
     familycode[b][i] = buff; 
  };
}

uint8_t sendToI2c(uint16_t val){
  uint8_t u8arr[3];
  mcp.val = val;
  mcp.val <<= 4;
  u8arr[0] = 0x40;
  u8arr[1] = mcp.data[1];
  u8arr[2] = mcp.data[0];
  val = HAL_I2C_Master_Transmit(&hi2c1,0xC0,u8arr,3,1000);
  return val;
}

