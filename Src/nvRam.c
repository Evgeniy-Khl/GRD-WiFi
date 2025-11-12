#include "main.h"
#include "nvRam.h"
#include "tftArcFill.h"

extern uint8_t familycode[MAX_SENSOR][8], modeCell;
extern uint16_t speedData[MAX_SPEED][2];
extern uint16_t set[INDEX];
extern CRC_HandleTypeDef hcrc;

union DataRam dataRAM;

void PID_Init(PIDController *pid, uint16_t Kp, uint16_t Ki, uint16_t Kd) {
    pid->Kp = (float)Kp/10;
    pid->Ki = (float)Ki/10000;
    pid->Kd = Kd;
}

uint32_t calcChecksum(void){
    __HAL_RCC_CRC_CLK_ENABLE(); // Включение clock для CRC модуля
//    CRC->CR = CRC_CR_RESET;     // Сбрасываем CRC модуль
    HAL_CRC_Init(&hcrc);
    // Начальное слово для вычисления CRC
    uint32_t* configPtr = (uint32_t*)&dataRAM.config.magicNum;
    // Вычисляем CRC для всех полей структуры Config, кроме полей notUsed[5], поля checkSum и поля countSave
    uint8_t sizeBuff = (sizeof(struct Config) - sizeof(uint32_t)*(UNUSED_FIELDS+1) - sizeof(uint16_t)) / sizeof(uint32_t);
    uint32_t CRCVal = HAL_CRC_Calculate(&hcrc, configPtr, sizeBuff);
    
    __HAL_RCC_CRC_CLK_DISABLE();// Отключаем clock для CRC модуля
    return CRCVal;// Возвращаем значение CRC
}

void setData(uint8_t m){
  uint8_t i, x;
  switch (m){
  	case 0: for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet0[i];} break;
  	case 1: for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet1[i];} break;
    case 2: for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet2[i];} break;
  	case 3: for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet3[i];} break;
  	default: modeCell = dataRAM.config.mode = 0; for(i=0;i<INDEX;i++){set[i] = dataRAM.config.modeSet0[i];}	break;
  }
//  if(set[VENT]>7) set[VENT]=0;
  for(i=0;i<MAX_SENSOR;i++){
    for(x=0;x<8;x++){
      familycode[i][x] = dataRAM.config.familycode[i][x];
    }
  }
  for(i=0;i<MAX_SPEED;i++){
    for(x=0;x<2;x++){
      speedData[i][x] = dataRAM.config.speedData[i][x];
    }
  }
  PID_Init(&pid, dataRAM.config.koff[m][0], dataRAM.config.koff[m][1], dataRAM.config.koff[m][2]);
  grafDispl[0].sp = set[T0];
  grafDispl[1].sp = set[T1];
}

uint8_t initData(void){
  // Чтение данных из FLASH памяти
  uint8_t i, x, err=0;
  uint32_t l_Address = FLASH_CONFIG_START_ADDR;
  uint32_t l_End = FLASH_CONFIG_START_ADDR + sizeof(struct Config);
  uint32_t l_Index = 0x00;
  
  while (l_Address < l_End){    // FLASH_CONFIG_END_ADDR
    // ((__IO uint32_t *)l_Address)
    // приведение адреса l_Address к указателю на 32-битное (4-байтное) беззнаковое целое число с атрибутом __IO, 
    // который обозначает, что это "вход/выход" (Input/Output) переменная.
    // *(...) - разыменование этого указателя, чтобы получить значение по этому адресу.
    dataRAM.data32[l_Index] = *(__IO uint32_t *)l_Address;
    l_Index = l_Index + 1;
    l_Address = l_Address + 4;
  }
//  for(uint8_t i=0;i<5;i++){dataRAM.config.notUsed[i]=0;}
  // Если CRC не совпадают ...
  l_Index = calcChecksum();
  if(l_Index != dataRAM.config.checkSum) err |= 2;
  
  // Если после чтения ключ не найденб то это первый запуск
  if(dataRAM.config.magicNum != MAGIC_KEY_DEFINE || err){
  /*
  //----------------    T1,T2,T3,Tmr,cTmr,ON,OFF,al,hysteresis
  uint16_t modeSet0[9]={70,60,50,180,160,  60,180,10,5}  // грд, грд, грд, мин, сек., сек., сек. грд, грд/10,
  uint16_t modeSet1[9]={71,61,51,181,161,  60,180,10,5}  // грд, грд, грд, мин, сек., сек., сек. грд, грд/10,
  uint16_t modeSet2[9]={72,62,52,182,162,  60,180,10,5}  // грд, грд, грд, мин, сек., сек., сек. грд, грд/10,
  uint16_t modeSet3[9]={73,63,53,183,163,  60,180,10,5}  // грд, грд, грд, мин, сек., сек., сек. грд, грд/10,
  //------------------auto
  int8_t relaySet[8]={-1,-1,-1,-1,-1,-1,-1,-1};
  //------------------auto
  int8_t analogSet[2]={-1,-1};
  */  
    err |= 1;
    for(uint8_t i=0;i<UNUSED_FIELDS;i++){dataRAM.config.notUsed[i]=0;}
    
    dataRAM.config.magicNum=MAGIC_KEY_DEFINE;
    dataRAM.config.mode = 0;
// ----------- "СУШЫННЯ"
    dataRAM.config.modeSet0[0]=70;  // Уставка T1 грд. 
    dataRAM.config.modeSet0[1]=60;  // Уставка T2 грд.
    dataRAM.config.modeSet0[2]=0;   // Уставка T3 грд.
    dataRAM.config.modeSet0[3]=0;   // Уставка T4 грд.
    dataRAM.config.modeSet0[4]=180; // Длительность режима мин.
    dataRAM.config.modeSet0[5]=0;   // Скорость вентилятора %
    dataRAM.config.modeSet0[6]=0;   // Таймер ON не используется
    dataRAM.config.modeSet0[7]=0;   // Таймер OFF не используется
    dataRAM.config.modeSet0[8]=120; // Длительность продувки сек.
    dataRAM.config.modeSet0[9]= 5;  // Авария ( 5 грд.)
    dataRAM.config.modeSet0[10]=5;  // Гистерезис (0.5 грд.)
    dataRAM.config.modeSet0[11]=0;  // Нагрев 0-прямое, 1-инвесное управление, 2-отключены аварийные звуковые сигналы
// ----------- "ОБЖАРКА"
    dataRAM.config.modeSet1[0]=71;  // Уставка T1 рд
    dataRAM.config.modeSet1[1]=61;  // Уставка T2 грд
    dataRAM.config.modeSet1[2]=0;   // Уставка T3 грд
    dataRAM.config.modeSet1[3]=0;   // Уставка T4 грд
    dataRAM.config.modeSet1[4]=170; // Длительность режима мин
    dataRAM.config.modeSet1[5]=1;   // Скорость вентилятора %
    dataRAM.config.modeSet1[6]=0;   // Таймер ON не используется
    dataRAM.config.modeSet1[7]=0;   // Таймер OFF не используется
    dataRAM.config.modeSet1[8]=121; // Длительность продувки сек
    dataRAM.config.modeSet1[9]= 5;  // Авария ( 5 грд.)
    dataRAM.config.modeSet1[10]=5;  // Гистерезис (0.5 грд.)
    dataRAM.config.modeSet1[11]=0;  // Нагрев 0-прямое, 1-инвесное управление, 2-отключены аварийные звуковые сигналы
// ----------- "ВАРЫННЯ"
    dataRAM.config.modeSet2[0]=72;  // Уставка T1 грд
    dataRAM.config.modeSet2[1]=62;  // Уставка T2 грд
    dataRAM.config.modeSet2[2]=0;   // Уставка T3 грд
    dataRAM.config.modeSet2[3]=52;  // Уставка T4 грд (Влажный)
    dataRAM.config.modeSet2[4]=160; // Длительность режима мин
    dataRAM.config.modeSet2[5]=2;   // Скорость вентилятора %
    dataRAM.config.modeSet2[6]=6;   // Увлажнитель ON 0.6 сек.
    dataRAM.config.modeSet2[7]=18;  // Увлажнитель OFF 1.8сек.
    dataRAM.config.modeSet2[8]=122; // Длительность продувки сек
    dataRAM.config.modeSet2[9]= 5;  // Авария ( 5 грд.)
    dataRAM.config.modeSet2[10]=5;  // Гистерезис (0.5 грд.)
    dataRAM.config.modeSet2[11]=0;  // Нагрев 0-прямое, 1-инвесное управление, 2-отключены аварийные звуковые сигналы
// ----------- "КОПЧЕННЯ"
    dataRAM.config.modeSet3[0]=73;  // Уставка T1 грд
    dataRAM.config.modeSet3[1]=63;  // Уставка T2 грд
    dataRAM.config.modeSet3[2]=53;  // Уставка T3 грд (Дым)
    dataRAM.config.modeSet3[3]=0;   // Уставка T4 грд
    dataRAM.config.modeSet3[4]=150; // Длительность режима мин
    dataRAM.config.modeSet3[5]=3;   // Скорость вентилятора %
    dataRAM.config.modeSet3[6]=180; // Розжиг ON 180сек.
    dataRAM.config.modeSet3[7]=2;   // пауза перед розжигом OFF 2 сек.
    dataRAM.config.modeSet3[8]=123; // Длительность продувки сек
    dataRAM.config.modeSet3[9]= 5;  // Авария ( 5 грд.)
    dataRAM.config.modeSet3[10]=5;  // Гистерезис (0.5 грд.)
    dataRAM.config.modeSet3[11]=0;  // Нагрев 0-прямое, 1-инвесное управление, 2-отключены аварийные звуковые сигналы
    
    dataRAM.config.koff[0][0]=50;      // пропорциональный
    dataRAM.config.koff[0][1]=70;      // интегральный
    dataRAM.config.koff[0][2]=30;      // дифференциальный
    
    dataRAM.config.koff[1][0]=40;      // пропорциональный
    dataRAM.config.koff[1][1]=60;      // интегральный
    dataRAM.config.koff[1][2]=20;      // дифференциальный
    
    dataRAM.config.koff[2][0]=30;      // пропорциональный
    dataRAM.config.koff[2][1]=50;      // интегральный
    dataRAM.config.koff[2][2]=40;      // дифференциальный
    
    dataRAM.config.koff[3][0]=20;      // пропорциональный
    dataRAM.config.koff[3][1]=40;      // интегральный
    dataRAM.config.koff[3][2]=50;      // дифференциальный
    
    for(i=0;i<8;i++){dataRAM.config.relaySet[i]=-1;}  // автоматическое управление
    for(i=0;i<2;i++){dataRAM.config.analogSet[i]=-1;} // автоматическое управление
    for(i=0;i<MAX_SENSOR;i++){
      for(x=0;x<8;x++){
        dataRAM.config.familycode[i][x]=0;
      }
    }
    dataRAM.config.speedData[0][0]=1000;  //0x3E8
    dataRAM.config.speedData[0][1]=0x2E0; //0x2F4
    dataRAM.config.speedData[1][0]=1200;  //0x4B0
    dataRAM.config.speedData[1][1]=0x492; //0x4A6
    dataRAM.config.speedData[2][0]=1400;  //0x578
    dataRAM.config.speedData[2][1]=0x63C; //0x655
    dataRAM.config.speedData[3][0]=1600;  //0x640
    dataRAM.config.speedData[3][1]=0x7E6; //0x804
    dataRAM.config.speedData[4][0]=1800;  //0x708
    dataRAM.config.speedData[4][1]=0x998; //0x9B6
    dataRAM.config.speedData[5][0]=2000;  //0x7D0
    dataRAM.config.speedData[5][1]=0xB3D; //0xB65
    dataRAM.config.speedData[6][0]=2200;  //0x898
    dataRAM.config.speedData[6][1]=0xCEC; //0xD14
    dataRAM.config.speedData[7][0]=2400;  //0x960
    dataRAM.config.speedData[7][1]=0xFFF; //0xFFF
    
    dataRAM.config.checkSum = calcChecksum();
    dataRAM.config.countSave = 0;
  }
  modeCell = dataRAM.config.mode;
  setData(modeCell);
  return err;
}

uint32_t writeData(void){
  static FLASH_EraseInitTypeDef EraseInitStruct;
  // Структура данных для очистки памяти
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_CONFIG_START_ADDR;
  EraseInitStruct.NbPages     = 0x01;

  uint32_t l_Address = FLASH_CONFIG_START_ADDR;
  uint32_t l_End = FLASH_CONFIG_START_ADDR + sizeof(struct Config);
  uint32_t l_Error = 0x00;
  uint32_t l_Index = 0x00;
  // Заполняем масив
  dataRAM.config.mode = modeCell;
  switch (modeCell){
  	case 0: for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet0[i] = set[i];} break;
  	case 1: for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet1[i] = set[i];} break;
    case 2: for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet2[i] = set[i];} break;
  	case 3: for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet3[i] = set[i];} break;
  	default: dataRAM.config.mode=modeCell=0;  for(uint8_t i=0;i<INDEX;i++){dataRAM.config.modeSet0[i] = set[i];}	break;
  }
  dataRAM.config.checkSum = calcChecksum();
  dataRAM.config.countSave = dataRAM.config.countSave + 1;
  // Готовимся к записи в память
  HAL_FLASH_Unlock();
  // Очищаем страницу памяти
  HAL_FLASHEx_Erase(&EraseInitStruct, &l_Error);
  // Пишем данные в память
  while (l_Address < l_End){   //  FLASH_CONFIG_END_ADDR
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, l_Address, dataRAM.data32[l_Index]) == HAL_OK){
      l_Index = l_Index + 1;
      l_Address = l_Address + 4;
    }
  }
  // Закрываем доступ на запись
  HAL_FLASH_Lock();
  
  // Проверяем соотвествие данных
  l_Address = FLASH_CONFIG_START_ADDR;
  l_Error = 0x00;
  l_Index = 0x00; // 5 полей портятся процессором по неизвестной причине!!
  while (l_Address < l_End){
    if(dataRAM.data32[l_Index] != *(__IO uint32_t *)l_Address){
      if(l_Index>4) l_Error++;
    }
    l_Index = l_Index + 1;
    l_Address = l_Address + 4;
  }
  setData(modeCell);
  return l_Error;
}
/*
void checkData(void){
  static FLASH_EraseInitTypeDef EraseInitStruct;
  // Структура данных для очистки памяти
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = FLASH_CONFIG_START_ADDR;
  EraseInitStruct.NbPages     = 0x01;
  
  // Проверяем соотвествие данных
  uint32_t l_Address = FLASH_CONFIG_START_ADDR;
  uint32_t l_Error = 0x00;
  uint32_t l_Index = 0x00;
  while (l_Address < FLASH_CONFIG_END_ADDR){
    if(dataRAM.data32[l_Index] != *(__IO uint32_t *)l_Address){
      l_Error++;
    }
    l_Index = l_Index + 1;
    l_Address = l_Address + 4;
  }
  if(l_Error){
    // Готовимся к записи в память
    HAL_FLASH_Unlock();
    // Очищаем страницу памяти
    HAL_FLASHEx_Erase(&EraseInitStruct, &l_Error);
    // Пишем данные в память
    l_Address = FLASH_CONFIG_START_ADDR;
    l_Error = 0x00;
    l_Index = 0x00;
    dataRAM.config.countSave = dataRAM.config.countSave + 1;
    while (l_Address < FLASH_CONFIG_END_ADDR){     
      if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, l_Address, dataRAM.data32[l_Index]) == HAL_OK){
        l_Index = l_Index + 1;
        l_Address = l_Address + 4;
      }
    }
    // Закрываем доступ на запись
    HAL_FLASH_Lock();
  }
}*/
