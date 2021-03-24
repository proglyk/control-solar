#ifndef _SCIV2_H_
#define _SCIV2_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
//#include "stdlib.h"
#include "global.h"
#include "main.h"

#define CPU_FREQ 						(Uint32)99532800
#define LSPCLK_FREQ 					(CPU_FREQ / 6)
#define SCI_PRD 						(LSPCLK_FREQ/(SCI_FREQ*8))-1

#define SLAVE_4FUNC_DATA_MAX_NUM 		64

#define SLAVE_TRANSMIT_BUFFER_LENGHT 	128
#define SLAVE_RECEIVE_BUFFER_LENGHT 	SLAVE_4FUNC_DATA_MAX_NUM
#define ALLOW_BYTE_NUM_FUNC_X04 		8

#define POOL_REGS_MAX_VALUE				8


#define MODBUS_PAGE_COUNT 				3
#define MODBUS_FUNC06_ADDRLEN 			(SLAVE_4FUNC_DATA_MAX_NUM-1)
#define MODBUS_FUNC06_REQBYTENUM 		1


typedef enum
{
	BYTESNUMBER_UNK = 0,
	BYTESNUMBER_X04 = 8,
	BYTESNUMBER_X06	= 8,
	BYTESNUMBER_X10 = 13,
	BYTESNUMBER_X03 = 8
}
BytesNumber_t;

typedef enum
{
	MODE_UNK = -1,
	MODE_NEWFRAME = 1,
	MODE_15T = 2,
	MODE_35T = 3
}
Mode_t;

typedef enum
{
	MODETRANSCEIVER_RX = 1,
	MODETRANSCEIVER_TX = 0
}
ModeTransceiver_t;

enum SciName
{
	SCI_UNK,
	SCI_PULT,
	SCI_PC
};

typedef struct
{
	/* Идентификатор */
	enum SciName Name;
	
	/* Ожидаемое число байт */
	BytesNumber_t BytesNumber;
	
	/* Режим ожидания нового байта */
	Mode_t Mode;
	
	/* Счетчик принятых байт */
	Uint8 BytesCounter;
	
	/* Буфер данных на прием */
	Uint8 DataReceive[SLAVE_RECEIVE_BUFFER_LENGHT];
	/* Буфер данных на прием */
	Uint8 Buf2[SLAVE_RECEIVE_BUFFER_LENGHT];
	
	/* Буфер данных на передачу */
	Uint8 DataTransmit[SLAVE_TRANSMIT_BUFFER_LENGHT];
	
	/* Длина буфера данных на передачу ??? */
	Uint8 DataTransmitLenght;
	
	/* Счетчик байт на передачу */
	Uint8 DataTransmitCount;
	
	/* Код запрашиваемой функции */
	Uint8 FuncCode;
	
	/* ??? */
	bool LookForTxEmpty;
	
	/* Режим ADM2483 */
	ModeTransceiver_t ModeTransceiver;
	
	volatile struct SCI_REGS* ptSciRegs;
	struct CPUTIMER_VARS* ptCpuTimer;
	
	Uint32 BaudRate;
	
	/* Сетевой адрес */
	Uint8 ID;
	
	/* Порт GPIO. Формат: GpioDataRegs.GPBDAT.all - здесь all 32-bit число и как раз неа него указатель */
	volatile Uint32* pGPIOPort;
	
	/*	Маска разряда бита порта GPIO, адресованного pGPIOPort. Правильность значения маска контроллируется только программистом. Для разных портов, например GPA и 
		GPB, значения масок отличаются и надо смотреть заголовочник DSP280x_Gpio.h. Например GPIO34 пойдёт на порт GPIB с маской 2 */
	Uint16 GPIOBit;
	
	Uint16 ByteCntTx;
}
SciSlave_t;

Error_t SciSlave_Link(SciSlave_t* slave, volatile struct SCI_REGS* sciregs, struct CPUTIMER_VARS* timerregs);
void SciSlave_UnLink(SciSlave_t* slave);
void SciSlave_Default(SciSlave_t* var);
Error_t SciSlave_Setup(SciSlave_t* slave, Uint32 baud, Uint8 id, volatile Uint32* REport, Uint16 REpin);
void SciSlave_StopTimer35T(SciSlave_t* slave);
void SciSlave_Receive(SciSlave_t* slave);
void SciSlave_InitTransmission(SciSlave_t* slave, Uint8* pBuf, Uint8 len);
void SciSlave_Transmit(SciSlave_t* slave);
void SciSlave_DisableTransmitInterrupt(SciSlave_t* slave);
void SciSlave_EnableTransmitInterrupt(SciSlave_t* slave);
void SciSlave_SwitchTransceiver(SciSlave_t* slave, ModeTransceiver_t mode);
Uint16 SciSlave_CRC16(const Uint8 *nData, Uint16 wLength);
Error_t SciSlave_FunctionX04(SciSlave_t* slave);
Error_t SciSlave_F06(SciSlave_t* slave);
Error_t SciSlave_FunctionX10(SciSlave_t* slave);
Error_t SciSlave_TimerInit(struct CPUTIMER_VARS *Timer);
void SciSlave_TimerSetPeriod( struct CPUTIMER_VARS *Timer, Uint16 Freq, Uint16 Period );



#endif
