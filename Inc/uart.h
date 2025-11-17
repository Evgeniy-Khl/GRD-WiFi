#ifndef _UART_H
#define _UART_H

#include <main.h>

#define CMD_LINK        81  // 0x51 Индикация
#define CMD_SP_WRITE	  51 	// 0x33 Запись опорных значений
#define CMD_CLR_WIFI	  53	// 0x35 Запись номера прибора и прочих установок
//#define CMD_PRG_WRITE	  55	//      Запись в микросхему программы

void espCallback(void);

#endif /* _UART_H */
