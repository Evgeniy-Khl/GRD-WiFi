#include "uart.h"

uint8_t rxBuffer[SEND_SIZE+3];  // [Старт][Данные][CRC][Конец]

void espCallback(void){
  uint8_t checkSum = 0;
  usd.sd.bot[1]++;    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  usd.sd.bot[0] = 0;  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  if(rxBuffer[0] == START_MARKER){
    for(uint8_t i=1; i<SEND_SIZE+1; i++) {
        checkSum ^= rxBuffer[i]; // Using XOR to calculate the checksum
    }
    if(checkSum == rxBuffer[SEND_SIZE+1]){
      if(rxBuffer[SEND_SIZE+2] == END_MARKER){
        memcpy(usd.dataUnion, &rxBuffer[1], SEND_SIZE);
        usd.sd.bot[0] = 200;  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      }
      else usd.sd.bot[0] = 3;
    }
    else usd.sd.bot[0] = 2;
  }
  else usd.sd.bot[0] = 1;
  
}
