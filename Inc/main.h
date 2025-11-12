/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f1xx_hal_pwr.h"
#include "stm32f1xx_hal_rtc_ex.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Input0_Pin GPIO_PIN_0
#define Input0_GPIO_Port GPIOA
#define T_IRQ_Pin GPIO_PIN_3
#define T_IRQ_GPIO_Port GPIOA
#define T_CS_Pin GPIO_PIN_4
#define T_CS_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_7
#define LED_GPIO_Port GPIOA
#define OneWR_Pin GPIO_PIN_11
#define OneWR_GPIO_Port GPIOB
#define TFT_RST_Pin GPIO_PIN_12
#define TFT_RST_GPIO_Port GPIOB
#define ESP_TX_Pin GPIO_PIN_9
#define ESP_TX_GPIO_Port GPIOA
#define ESP_RX_Pin GPIO_PIN_10
#define ESP_RX_GPIO_Port GPIOA
#define TFT_DC_Pin GPIO_PIN_12
#define TFT_DC_GPIO_Port GPIOA
#define TFT_CS_Pin GPIO_PIN_15
#define TFT_CS_GPIO_Port GPIOA
#define Beep_Pin GPIO_PIN_6
#define Beep_GPIO_Port GPIOB
#define Input1_Pin GPIO_PIN_7
#define Input1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define DIAGONAL    28          // 24 -> для дисплеев 2,4"; 28 -> для дисплеев 2,8"; 32 -> для дисплеев 3,2"
#define TOUCHMODE   0           // 0 или 1
#define MAX_SENSOR  4
#define MAX_MODE    4
#define MAX_SET     8
#define MAX_OTHER   7
#define MAX_SPEED   8
#define ON          1
#define OFF         0

#define T0    0 // Уставка T1 грд. 
#define T1    1 // Уставка T2 грд. 
#define T2    2 // Уставка T3 грд. (Дым)
#define T3    3 // Уставка T4 грд. (Влажный)
#define TMR0  4 // Длительность режима мин.
#define VENT  5 // Скорость вентилятора %
#define TMON  6 // Таймер ON сек.
#define TMOFF 7 // Таймер OFF сек.
#define TMR1  8 // Длительность продувки сек.
#define ALRM  9 // Авария грд.
#define HIST  10 // Гистерезис грд/10
#define CHILL 11 // Охлаждение

#define ERR1  0x0010  //
#define ERR2  0x0020  //
#define ERR3  0x0040  // ПЕРЕГРЫВ В КАМЕРI
#define ERR4  0x0080  // ПЕРЕГРЫВ В ПРОДУКТI
#define ERR5  0x0100  // ВIДХIЛЕННЯ ТЕМПЕРАТУРИ В КАМЕРI
#define ERR6  0x0200  // ВIДХIЛЕННЯ ТЕМПЕРАТУРИ ДИМA
#define ERR7  0x0400  //
#define ERR8  0x0800  // НЕ ПРАЦЮЭ ВЕНТИЛЯТОР


#include <stdio.h>
#include <stdlib.h>


//#define MANUAL_CHECK

#ifdef MANUAL_CHECK
  #define CHKSMOKE  180 // (3 min.) waiting for smoke temperature check in sec.
#else
  #define CHKSMOKE  1500 // (25 min.) waiting for smoke temperature check in sec.
#endif
#define BEGINCOOL   400 // температура 40 грд. выше которой ЗАПРЕЩЕНО включение охлаждения
#define BEGINHUM    400 // запрет увлажнения при температуре ниже 40 грд.
#define INDEX 12
#define RAMPV_SIZE  4+8+INDEX*2+8

struct Rampv {
    uint8_t model;       // 1 байт ind=0  модель прибора
    uint8_t node;        // 1 байт ind=1  сетевой номер прибора
    uint8_t modeCell;    // 1 байт ind=2  номер режима
    uint8_t portFlag;    // 1 байт ind=3  Flags CHECK; SPEED; WORK; NEWBUTT; VENTIL; PERFECT; RESERVE; PURGING
    int16_t t[4];        // 8 байт ind=4-ind=11  значения датчиков температуры
    uint16_t set[INDEX]; // 24байт ind=12-ind=35 Установки
    uint8_t fanSpeed;    // 1 байт ind=36 скорость вращения вентилятора
    uint8_t pvOut;       // 1 байт ind=37 активные выходы реле
    uint8_t dsplPW;      // 1 байт ind=38 мощность подаваемая на тены
    uint16_t errors;      // 1 байт ind=39 ошибки
    uint8_t currHour;    // 1 байт ind=40 часы
    uint8_t currMin;     // 1 байт ind=41 минуты
    uint8_t currSec;     // 1 байт ind=42 секунды
};

union Upv{
  struct Rampv pv;
  uint8_t receivedData[RAMPV_SIZE]; // Массив для приема
};

/* ---структура с битовыми полями -----*/
struct byte {
    unsigned a0: 1;
    unsigned a1: 1;
    unsigned a2: 1;
    unsigned a3: 1;
    unsigned a4: 1;
    unsigned a5: 1;
    unsigned a6: 1;
    unsigned a7: 1;
};
 
union Byte {
    unsigned char value;
    struct byte bitfield;
};

extern union Upv upv;

union d4v{
  uint8_t data[4];  
  uint16_t val[2]; 
};

typedef struct {
    float Ki, iPart, Kp, pPart;  // Коэффициенты PID
    int32_t dPart, prev_error, output;
    uint16_t Kd;
} PIDController;

extern PIDController pid;

#define CHECK   portFlag.bitfield.a0  // Start of all checks
#define SPEED   portFlag.bitfield.a1  // Speed Ok.
#define WORK 	  portFlag.bitfield.a2  // At work flag
#define NEWBUTT portFlag.bitfield.a3  // New screen flag
#define VENTIL	portFlag.bitfield.a4  // Fan speed flag
#define PERFECT	portFlag.bitfield.a5  // Достигли желаемой температуры
#define RESERVE portFlag.bitfield.a6  // резерв
#define PURGING portFlag.bitfield.a7  // Продувка

#define TRIAC   relayOut.bitfield.a0  // SSR-25DA
#define HEATER  relayOut.bitfield.a1  // НАГРЕВАТЕЛЬ
#define TIMER 	relayOut.bitfield.a2  // ТАЙМЕР
#define HUMIDI	relayOut.bitfield.a3  // УВЛАЖНИТЕЛЬ
#define ELECTRO	relayOut.bitfield.a4  // Электроподжиг
#define SMOKE   relayOut.bitfield.a5  // Клапан дыма
#define WATER 	relayOut.bitfield.a6  // Клапан воды
#define ALARM   relayOut.bitfield.a7  // Тревога

extern union Byte portFlag;
extern union Byte relayOut;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
