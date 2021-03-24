#ifndef _DATACONVERT_H_
#define _DATACONVERT_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
#include "stdlib.h"
#include "DataConvert.h"
#include "global.h"
#include "MemoryEEPROM.h"

#define EEPROM_LENGHT 5
#define QUEUE_LENGHT 24



typedef struct
{
	Uint16	Second;
	Uint16	Minute;
	Uint16	Hour;
	Uint16	Day;
	Uint16	Month;
	Uint16	Year;
}
DateTime_t;

typedef enum
{
	BCDF_8BIT,
	BCDF_7BIT,
	BCDF_6BIT,
	BCDF_5BIT
}
BCDFormat_t;

typedef struct
{
	Uint16 Value1;
	Uint16 Value2;
	Uint16 Password;
	DateTime_t DateTime;
}
Page0_t;

typedef union 
{
	Uint16 u16;
}
AnyType_t;



typedef struct 
{
	const I2C_Device_t * Device;
	Uint16 Adr;
	Uint16 Data;
}
Eeprom_t;

typedef struct
{
	Eeprom_t * Buf[QUEUE_LENGHT];
	Eeprom_t BufData[QUEUE_LENGHT];
	Uint16 Spos;
	Uint16 Rpos;
}
Queue_t;

void ConvertInit(void);
Uint16 DEC_to_BCD(Uint16 in, Uint16 param);
Uint16 BCD_to_DEC_16Bit(Uint16 In, RTC_Type_t type);

Qerror_t Q_Store1(Eeprom_t * q);
Qerror_t Q_Store2(Uint16 Addr, Uint16 Value);
Eeprom_t * Q_Retrive(void);
void Memory_Write(Eeprom_t * mem);
void Memory_Convert1(Eeprom_t * rom, struct I2cMsgUser_st * buffer);
void Memory_Convert2(I2cMsgReceive_t* con, Eeprom_t * memory);
void Rtc_Read(const I2C_Device_t * dev, Uint16 Address, Uint16 Lenght);
//void Rtc_Read(Uint16 Address, Uint16 Lenght);
void RTC_Convert(I2cMsgReceive_t* con);
void RTC_Convert1(Eeprom_t * rom, struct I2cMsgUser_st * buffer);
void I2C_ConvertUserReq(I2cMsgReceive_t* con, Eeprom_t * rom, struct I2cMsgUser_st * buffer);
Qerror_t Q_Store3(const I2C_Device_t * dev, Uint16 Addr, Uint16 Value);
void Memory_Read(Uint16 Address, Uint16 Lenght);
struct I2cMsgUser_st * Memory_ReadNew(const I2C_Device_t * dev, Uint16 Address, Uint16 Lenght);
void DataConvert_EepromToParamsTable(I2cMsgReceive_t* con);
Uint16 DataConvert_DS3231_Set(Uint16 in, BCDFormat_t bits);

#endif

