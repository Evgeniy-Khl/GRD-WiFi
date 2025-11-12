#ifndef _PROCEDURE_H
#define _PROCEDURE_H

#include <main.h>

#define DRYING_PROCESS_FLAG_MAGIC   0xCAFE
#define BKP_REG_DRYING_FLAG         RTC_BKP_DR1
#define BKP_REG_REMAINING_TIME      RTC_BKP_DR2

// 3. Объявляем наши глобальные переменные с ключевым словом "extern"
// Это говорит другим файлам, что эти переменные существуют, но определены где-то еще (в нашем случае, в main.c)
extern RTC_HandleTypeDef hrtc; // Также делаем видимым хэндлер RTC

void startBackUp(void);
void stopBackUp(void);
void startPrg(uint8_t repair);
void newMitutesBackUp(uint32_t minutres);
uint8_t Relay(int16_t err, uint8_t hst);
uint8_t humidifier(uint8_t value);
uint8_t UpdatePID(PIDController *pid, uint8_t cn);
void permutation (char a, char b);
uint8_t sendToI2c(uint16_t val);
uint8_t ignition(uint8_t value);

#endif /* _PROCEDURE_H */
