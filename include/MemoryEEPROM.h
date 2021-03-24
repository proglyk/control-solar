#ifndef _MEMORYEEPROM_H_
#define _MEMORYEEPROM_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"

#define MACRO1(a) (Uint16)(a >> 1)

#define I2C_TIMEOUT_WRITE 1000

#define I2C_MEM_ADDR 0xA0
#define I2C_MEM_NUMBYTES 10
#define I2C_MEM_HIGH_ADDR 0x00
#define I2C_MEM_LOW_ADDR 0x00

#define I2C_RTC_ADDR 0xD0
#define I2C_RTC_NUMBYTES 10
#define I2C_RTC_HIGH_ADDR 0x00
#define I2C_RTC_LOW_ADDR 0x00

#define I2C_MSG_TO_WRITE 32
#define I2C_MSG_CARRIAGE I2C_MAX_BUFFER_SIZE-2
//#define I2C_MSG_CARRIAGE 10

typedef enum
{
	I2C_ADDR_8BIT,
	I2C_ADDR_16BIT
}
I2C_AdressType_t;

typedef enum
{
	I2C_WORD_8BIT,
	I2C_WORD_16BIT
}
I2C_WordType_t;

struct I2cMsgReceiveControl_st
{
	Uint16 Status;
	Uint16 Count;
	//Uint16 Lenght;
	Uint16 CountRequestToSend;
	Uint16 CountSent;
	Uint16 Fifo[I2C_MAX_BUFFER_SIZE];
	
	Uint16 CountReqToReceive;
	Uint16 CountReceived;
	
	
};

typedef struct
{
	Uint16 SlaveId;
	I2C_AdressType_t AdrBitRange;
	I2C_WordType_t DataBitRange;
}
I2C_Device_t;

struct I2cMsgUser_st
{
	Uint16 SlaveId;
	Uint16 Lenght;
	Uint16 Adress;
	const I2C_Device_t * Device;
	//I2C_AdressType_t AddrType;
	//I2C_AdressType_t WordType;
	Uint16 ReqToRead;
	
	Uint16 * pBuffer;
	Uint16 Buf[128];
};

typedef enum
{
	RTC_UNK = 0,
	RTC_SEC,
	RTC_MIN,
	RTC_HOUR,
	RTC_DAT,
	RTC_MONTH,
	RTC_YEAR
	
}
RTC_Type_t;

typedef struct
{
	struct I2cMsgReceiveControl_st Control;
	struct I2cMsgUser_st User;
}
I2cMsgReceive_t;

int TestMem(void);
void MemEEPROM_Init(void);
void MemEEPROM_Isr(void);
void MemEEPROM_Isr2(void);
Uint16 MemEEPROM_ReadData(struct I2CMSG *msg);
Uint16 MemEEPROM_WriteData(struct I2CMSG *msg);
Uint16 WriteData(struct I2CMSG *msg,Uint16 *MsgBuffer,Uint16 MemoryAdd,Uint16 NumOfBytes);
Uint16 ReadData(struct I2CMSG *msgin, struct I2CMSG *msgout,Uint16 *MsgBuffer,Uint16 MemoryAdd,Uint16 NumOfBytes);
Uint16 WriteData_v1(I2cMsgReceive_t* control, struct I2cMsgUser_st * msg);
Uint16 WriteData2_v1(I2cMsgReceive_t* control);
Uint16 WriteData3_v1(I2cMsgReceive_t* control);

Uint16 I2C_ReadWrite(I2cMsgReceive_t* con, Uint16 opcode, struct I2cMsgUser_st * msg);
void I2C_Driver(I2cMsgReceive_t* con);
void I2C_Sheduler(void);
#endif


