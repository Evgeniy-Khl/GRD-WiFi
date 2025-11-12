#include "main.h"
#include "ds18b20.h"
#include "nvRam.h"

//--------------------------------------------------
uint8_t LastDeviceFlag;
uint8_t LastDiscrepancy;
uint8_t LastFamilyDiscrepancy;
uint8_t ROM_NO[8];
//uint8_t my_dt[8], my_ret;
extern char buffTFT[];
extern uint8_t familycode[MAX_SENSOR][8], ds18b20_amount, Y_str, Y_top;
extern int16_t pvTH, pvRH;
extern uint16_t fillScreen;

//--------------------------------------------------
__STATIC_INLINE void DelayMicro(__IO uint32_t micros){
micros *= (SystemCoreClock / 1000000) / 8;
/* Wait till done */
while (micros--) ;
}
//--------------------------------------------------
void ds18b20_port_init(void){
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);
  GPIOB->CRH |= GPIO_CRH_MODE11;
  GPIOB->CRH |= GPIO_CRH_CNF11_0;
  GPIOB->CRH &= ~GPIO_CRH_CNF11_1;
}
//-----------------------------------------------
uint8_t ds18b20_SearhRom(uint8_t *Addr){
  uint8_t id_bit_number;
  uint8_t last_zero, rom_byte_number, search_result;
  uint8_t id_bit, cmp_id_bit;
  uint8_t rom_byte_mask, search_direction;
  //проинициализируем переменные
  id_bit_number = 1;
  last_zero = 0;
  rom_byte_number = 0;
  rom_byte_mask = 1;
  search_result = 0;
	if (!LastDeviceFlag){
		ds18b20_Reset();
		ds18b20_WriteByte(0xF0);
	}
	do{
		id_bit = ds18b20_ReadBit();
		cmp_id_bit = ds18b20_ReadBit();
		if ((id_bit == 1) && (cmp_id_bit == 1))	break;
		else{
			if (id_bit != cmp_id_bit)
				search_direction = id_bit; // bit write value for search
			else{
				if (id_bit_number < LastDiscrepancy)
					search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
				else
					search_direction = (id_bit_number == LastDiscrepancy);
				if (search_direction == 0){
					last_zero = id_bit_number;
					if (last_zero < 9) LastFamilyDiscrepancy = last_zero;
				}
			}
			if (search_direction == 1) ROM_NO[rom_byte_number] |= rom_byte_mask;
			else ROM_NO[rom_byte_number] &= ~rom_byte_mask;
			ds18b20_WriteBit(search_direction);
			id_bit_number++;
			rom_byte_mask <<= 1;
			if (rom_byte_mask == 0){
				rom_byte_number++;
				rom_byte_mask = 1;
			}
		}
  } while(rom_byte_number < 8); // считываем байты с 0 до 7 в цикле
	if (!(id_bit_number < 65)){
	  // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
		LastDiscrepancy = last_zero;
		// check for last device
		if (LastDiscrepancy == 0)	LastDeviceFlag = 1;
		search_result = 1;	
  }
	if (!search_result || !ROM_NO[0]){
		LastDiscrepancy = 0;
		LastDeviceFlag = 0;
		LastFamilyDiscrepancy = 0;
		search_result = 0;
	}
	else{
    for (int i = 0; i < 8; i++) Addr[i] = ROM_NO[i];
  }	
  return search_result;
}
//-----------------------------------------------
uint8_t ds18b20_Reset(void){
  uint8_t status;
  GPIOB->BSRR =GPIO_BSRR_BR11;//низкий уровень (Сбросили 11 бит порта B )
  //GPIOB->ODR &= ~GPIO_ODR_ODR11;//низкий уровень
  DelayMicro(300);//задержка как минимум на 480 микросекунд
  GPIOB->BSRR =GPIO_BSRR_BS11;//высокий уровень (Установили 11 бит порта B )
  //GPIOB->ODR |= GPIO_ODR_ODR11;//высокий уровень
  DelayMicro(40);//задержка как минимум на 60 микросекунд
  status = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
  DelayMicro(300);//задержка как минимум на 480 микросекунд
  //(на всякий случай подождём побольше, так как могут быть неточности в задержке)
  return (status ? 1 : 0);//вернём результат
}
//--------------------------------------------------
uint8_t ds18b20_ReadBit(void){
  uint8_t bit = 0;
  __disable_irq();  // Запрет прерываний
  GPIOB->BSRR =GPIO_BSRR_BR11;//низкий уровень
  DelayMicro(1);
  GPIOB->BSRR =GPIO_BSRR_BS11;//высокий уровень
  DelayMicro(10);
  bit = (GPIOB->IDR & GPIO_IDR_IDR11 ? 1 : 0);//проверяем уровень
  DelayMicro(30);
  __enable_irq();  // Разрешение прерываний
  return bit;
}
//-----------------------------------------------
uint8_t ds18b20_ReadByte(void){
  uint8_t data = 0;
  for (uint8_t i = 0; i < 8; i++)
  data += ds18b20_ReadBit() << i;
  return data;
}
//-----------------------------------------------
void ds18b20_WriteBit(uint8_t bit){
  __disable_irq();  // Запрет прерываний
  GPIOB->BSRR =GPIO_BSRR_BR11;//низкий уровень
  DelayMicro(bit ? 1 : 45);
  GPIOB->BSRR =GPIO_BSRR_BS11;//высокий уровень
  DelayMicro(bit ? 45 : 1);
  __enable_irq();  // Разрешение прерываний
}
//-----------------------------------------------
void ds18b20_WriteByte(uint8_t dt){
  for (uint8_t i = 0; i < 8; i++){
    ds18b20_WriteBit(dt >> i & 1);
    //Delay Protection
    DelayMicro(2);
  }
}
//-----------------------------------------------
uint8_t ds18b20_addCode(uint8_t num){
  uint8_t ret=0, dt[8];
  if(ds18b20_SearhRom(dt)){
    ret = dallas_crc8(dt, 7);
    if(dt[7] == ret){
      memcpy(familycode[num], dt, 8);
      ret = 1;
    }
  }
  return ret;
}
//-----------------------------------------------
uint8_t ds18b20_count(uint8_t amount){
  uint8_t i, dt[8];
  ds18b20_amount = 0;
  for(i = 0; i < amount; i++){
    if(ds18b20_SearhRom(dt)){
      memcpy(familycode[ds18b20_amount],dt,sizeof(dt));
      ds18b20_amount++;
    }
    else break;
  }
  return 0;
}
//-----------------------------------------------
void ds18b20_Convert_T(){
  ds18b20_Reset();
  ds18b20_WriteByte(0xCC);  //SKIP ROM
  ds18b20_WriteByte(0x44);  //CONVERT T
}
//----------------------------------------------------------
uint8_t ds18b20_ReadStratcpad(uint8_t DevNum){
  uint8_t i, crc, dt[8];
  ds18b20_Reset();            // 1 Wire Bus initialization
	ds18b20_WriteByte(0x55);  // Load MATCH ROM [55H] comand
	for(i = 0; i < 8; i++){ds18b20_WriteByte(familycode[DevNum][i]);}
  ds18b20_WriteByte(0xBE);  //READ SCRATCHPAD
  for(i=0;i<8;i++){dt[i] = ds18b20_ReadByte();}
  crc = ds18b20_ReadByte(); // Read CRC byte
  i = dallas_crc8(dt, 8);
  if (i==crc) i=1; else i=0;
  return i;
}
//----------------------------------------------------------
void ds18b20_WriteScratchpad(uint8_t DevNum, uint8_t th, int8_t tl)   // ЗАПИСЬ в configuration register
{
 uint8_t i;
  ds18b20_Reset();          // 1 Wire Bus initialization
  ds18b20_WriteByte(0x55);  // Load MATCH ROM [55H] comand
  for(i = 0; i < 8; i++){
			ds18b20_WriteByte(familycode[DevNum][i]);
		}
  ds18b20_WriteByte(0x4E);  // Load WRITE SCRATCHPAD [4EH] command
  ds18b20_WriteByte(th);    // TH
  ds18b20_WriteByte(tl);    // TL
  ds18b20_WriteByte(0x7F);  // configuration register
  ds18b20_Reset();          // 1 Wire Bus initialization
  ds18b20_WriteByte(0xCC);  // Load Skip ROM [CCH] command
  ds18b20_WriteByte(0x48);  // Load COPY SCRATCHPAD [48h] command
}
//----------------------------------------------------------
uint8_t dallas_crc8(uint8_t * data, uint8_t size){
 uint8_t crc = 0;
  for (uint8_t i = 0; i < size; ++i){
    uint8_t inbyte = data[i];
    for (uint8_t j = 0; j < 8; ++j){
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if ( mix ) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}

//----------------------------------------------------------
void temperature_check(){
  uint8_t item, byte, crc, try_cnt=0;
  union ds {uint8_t data[8]; int16_t val;} buffer;
  for (item=0; item < ds18b20_amount;){
    ds18b20_Reset(); // 1 Wire Bus initialization
    ds18b20_WriteByte(0x55); // Load MATCH ROM [55H] comand
    for (byte=0; byte < 8; byte++) ds18b20_WriteByte(familycode[item][byte]); // Load cont. byte
    ds18b20_WriteByte(0xBE); // Read Scratchpad command [BE]
    for (byte=0; byte < 8; byte++){
        buffer.data[byte] = ds18b20_ReadByte(); // Read cont. byt
    }
    crc = ds18b20_ReadByte(); // Read CRC byte
    byte = dallas_crc8(buffer.data, 8);
    if (byte==crc){
      try_cnt = 0; ds.err[item]=0;
      if (buffer.val<0){
        buffer.val = -buffer.val;
        ds.pvT[item] = buffer.val*10/16;
        ds.pvT[item] = -ds.pvT[0];
      }
      else ds.pvT[item] =  buffer.val*10/16;
      crc = buffer.data[2]&TUNING;
      if (crc==TUNING){int8_t correction=buffer.data[3]; ds.pvT[item] +=correction;}// корекция показаний датчика
    }
    else if (++try_cnt > 1){    // (199) если ошибка более X раз то больше не опрашиваем
      try_cnt = 0;
      if(++ds.err[item]>4) {ds.pvT[item] = 1990; upv.pv.errors |= (1<<item);}
    }; 
    if (try_cnt==0) item++;
   }
  ds18b20_Convert_T();
}

//----------------------------------------------------------
void ds18b20_checkSensor(uint8_t sensors){
  uint8_t errors=0, i, num, x, ok;
  uint16_t point_color=WHITE;
  for(i=0;i<sensors;i++){
    if(familycode[i][7]==0) errors++;   // пустой CRC
    else {                              // код заполнен
      if(ds18b20_ReadStratcpad(i)) ds18b20_amount++;
      else {
        sprintf(buffTFT,"Датчик N%u выдсутный!",i+1);
        GUI_WriteString(5, Y_str, buffTFT, Font_11x18, RED, YELLOW);
        Y_str = Y_str+18+5;
        familycode[i][7]=0; errors++;   // но датчик не подключен
      }
    }
  }
  if(errors){
    GUI_WriteString(5+10, Y_str, "Потрыбно додати датчикыв!", Font_11x18, MAGENTA, BLACK);
    Y_str = Y_str+18+5;
    GUI_WriteString(3, Y_str, "Зараз выдключыть усы датчики", Font_11x18, WHITE, BLACK);
    HAL_Delay(5000);
    GUI_Clear(fillScreen);
    Y_str = 5;
    for(num=0;num<sensors;){
      if(familycode[num][7]==0){            // пустой CRC
        sprintf(buffTFT,"Пыдключыть тыкы датчик N%u",num+1);
        GUI_WriteString(5, Y_str, buffTFT, Font_11x18, point_color, BLACK);
        HAL_Delay(3000);
        if(ds18b20_addCode(num)){           // код cчитался
          Y_str = Y_str+18+5; x=0;
          sprintf(buffTFT,"Датчик N%u прочитаний.",num+1);
          GUI_WriteString(5, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
          Y_str = Y_str+18+5;
          sprintf(buffTFT,"%02X %02X %02X %02X %02X %02X %02X %02X",
                           familycode[num][0], familycode[num][1], familycode[num][2], familycode[num][3],
                           familycode[num][4], familycode[num][5], familycode[num][6], familycode[num][7]);
          GUI_WriteString(5, Y_str, buffTFT, Font_11x18, WHITE, BLACK);
          //-- проверяю эксклюзивность этого датчика
          ok = 1;
          for(i=0;i<sensors;i++){
            if(num!=i){   // исключаем проверку своего же кода
              if(familycode[num][7]==familycode[i][7]){
                familycode[num][7]=0; ok = 0;
                Y_str = Y_str+18+5;
                sprintf(buffTFT,"Це датчик N%u потрыбен ынший!",i+1);
                GUI_WriteString(5, Y_str, buffTFT, Font_11x18, MAGENTA, BLACK);
                HAL_Delay(3000);
                GUI_Clear(fillScreen);
                Y_str = 5;
                break;
              }
            }
          }
          if(ok){
            ds18b20_amount++;
            Y_str = Y_str+18+5;
            sprintf(buffTFT,"Датчик N%u готовий до роботи. ",num+1);
            GUI_WriteString(5, Y_str, buffTFT, Font_11x18, GREEN, BLACK);
            Y_str = Y_str+18+5;
            GUI_WriteString(3, Y_str, "Тепер выд'эднайте цей датчик", Font_11x18, WHITE, BLACK);
            num++; x=0; point_color=WHITE;
            HAL_Delay(5000);
            GUI_Clear(fillScreen);
            Y_str = 5;
          }
        }
        else {           // код не читается
          x++;
          switch (x){
            case 1: point_color=YELLOW; break;  // начинаем сердится
            case 2: point_color=MAGENTA; break; // очень сердимся
            case 3: point_color=RED; break;     // последнее предупреждение
            default: point_color=WHITE; x=0;    // релакс
                      if(num==3){ // если уже записаны 3 датчика, а четвертого не подключают то ...
                        for(uint8_t i=0;i<8;i++){
                          familycode[num][i]=familycode[num-1][i];     // копирруем familycode 3 датчика
                        }
                        
                        num++;  // и представляем что это четвертый датчик
                      }
            break;
          }
        }
      }
      else num++;
    }  
    for(uint8_t i=0;i<sensors;i++){
      for(uint8_t x=0;x<8;x++){
        dataRAM.config.familycode[i][x] = familycode[i][x];
      }
    }
    GUI_Clear(fillScreen);
    GUI_WriteString(lcddev.width/2-90,lcddev.height/2-60, "ВИКОНАЮ ЗАПИС!", Font_11x18, GREEN, BLACK);
    uint32_t er = writeData();        // запишем значения во FLASH
    if(er) GUI_WriteString(lcddev.width/2-40,lcddev.height/2-20, "ПОМИЛКА!", Font_11x18, YELLOW, RED);
    else GUI_WriteString(lcddev.width/2-10,lcddev.height/2-20, "OK", Font_11x18, GREEN, BLACK);
    HAL_Delay(1000);
    GUI_Clear(fillScreen);
    GUI_WriteString(3, Y_str+60, "Тепер пыдключыть усы датчики", Font_11x18, WHITE, BLACK);
    HAL_Delay(5000);
    GUI_Clear(fillScreen);
    Y_str = 5;
  }
  else {
    GUI_WriteString(5, Y_str, "Усы датчики пыдключены.", Font_11x18, GREEN, BLACK);
    Y_str = Y_str+18+5;
  }
}
