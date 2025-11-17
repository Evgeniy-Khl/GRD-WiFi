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
#include <string.h>
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
#define DIAGONAL    28          // 24 -> ��� �������� 2,4"; 28 -> ��� �������� 2,8"; 32 -> ��� �������� 3,2"
#define TOUCHMODE   0           // 0 ��� 1
#define MAX_SENSOR  4
#define MAX_MODE    4
#define MAX_SET     8
#define MAX_OTHER   9
#define MAX_SPEED   8
#define ON          1
#define OFF         0

#define T0    0 // ������� T1 ���. 
#define T1    1 // ������� T2 ���. 
#define T2    2 // ������� T3 ���. (���)
#define T3    3 // ������� T4 ���. (�������)
#define TMR0  4 // ������������ ������ ���.
#define VENT  5 // �������� ����������� %
#define TMON  6 // ������ ON ���.
#define TMOFF 7 // ������ OFF ���.
#define TMR1  8 // ������������ �������� ���.
#define ALRM  9 // ������ ���.
#define HIST  10 // ���������� ���/10
#define CHILL 11 // ����������

#define ERR1  0x0010  //
#define ERR2  0x0020  //
#define ERR3  0x0040  // �������� � �����I
#define ERR4  0x0080  // �������� � �������I
#define ERR5  0x0100  // �I��I����� ����������� � �����I
#define ERR6  0x0200  // �I��I����� ����������� ���A
#define ERR7  0x0400  //
#define ERR8  0x0800  // �� ������ ����������


#include <stdio.h>
#include <stdlib.h>


#define MANUAL_CHECK

#ifdef MANUAL_CHECK
  #define CHKSMOKE  180 // (3 min.) waiting for smoke temperature check in sec.
#else
  #define CHKSMOKE  1500 // (25 min.) waiting for smoke temperature check in sec.
#endif
#define BEGINCOOL   400 // ����������� 40 ���. ���� ������� ��������� ��������� ����������
#define BEGINHUM    400 // ������ ���������� ��� ����������� ���� 40 ���.
#define INDEX       12
#define START_MARKER  0xAA  // ��������� ����
#define END_MARKER    0x55  // �������� ����

struct __attribute__((packed)) Rampv {
    uint8_t id;          // 1 ���� ID �������
    uint8_t wifi;        // 1 ���� ����������� � ����
    uint8_t modeCell;    // 1 ���� ����� ������
    uint8_t portFlag;    // 1 ���� Flags CHECK; SPEED; WORK; NEWBUTT; VENTIL; PERFECT; RESERVE; PURGING
    uint8_t currHour;    // 1 ���� ����
    uint8_t currMin;     // 1 ���� ������
    uint8_t currSec;     // 1 ���� �������
    uint16_t errors;     // 2 ���� ������
    int16_t t[4];        // 8 ���� �������� �������� �����������
    uint16_t set[INDEX]; //24 ���� ���������
};
#define RAMPV_SIZE 7+2+8+INDEX*2 // ����������� �������
union Upv{
  struct Rampv pv;
  uint8_t dataUnion[RAMPV_SIZE]; // ������ ��� ������
};

struct __attribute__((packed)) Senddata {
    uint8_t command;     // 1 ���� ��������
    uint8_t myIp[4];     // 4 ���� IP
    uint8_t bot[2];      // 2 ���� strlen(botToken); strlen(chatID);
    uint16_t set[INDEX]; //24 ���� ���������
};
#define SEND_SIZE 7+INDEX*2 // ����������� �������
union Usd{
  struct Senddata sd;
  uint8_t dataUnion[SEND_SIZE]; // ������ ��� ������
};


/* ---��������� � �������� ������ -----*/
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
extern union Usd usd;
extern uint8_t rxBuffer[SEND_SIZE+3];// [�����][������][CRC][�����]

union d4v{
  uint8_t data[4];  
  uint16_t val[2]; 
};

typedef struct {
    float Ki, iPart, Kp, pPart;  // ������������ PID
    int32_t dPart, prev_error, output;
    uint16_t Kd;
} PIDController;

extern PIDController pid;

#define CHECK   portFlag.bitfield.a0  // Start of all checks
#define SPEED   portFlag.bitfield.a1  // Speed Ok.
#define WORK 	  portFlag.bitfield.a2  // At work flag
#define NEWBUTT portFlag.bitfield.a3  // New screen flag
#define VENTIL	portFlag.bitfield.a4  // Fan speed flag
#define PERFECT	portFlag.bitfield.a5  // �������� �������� �����������
#define RESERVE portFlag.bitfield.a6  // ������
#define PURGING portFlag.bitfield.a7  // ��������

#define TRIAC   relayOut.bitfield.a0  // SSR-25DA
#define HEATER  relayOut.bitfield.a1  // �����������
#define TIMER 	relayOut.bitfield.a2  // ������
#define HUMIDI	relayOut.bitfield.a3  // �����������
#define ELECTRO	relayOut.bitfield.a4  // �������������
#define SMOKE   relayOut.bitfield.a5  // ������ ����
#define WATER 	relayOut.bitfield.a6  // ������ ����
#define ALARM   relayOut.bitfield.a7  // �������

extern union Byte portFlag;
extern union Byte relayOut;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
