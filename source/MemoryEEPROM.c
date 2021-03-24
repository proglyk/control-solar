#include "MemoryEEPROM.h"
#include "string.h"
#include "DataConvert.h"

//extern DateTime_t DateTime;
extern Page0_t Page0;

static void MemEEPROM_RegsInit(void);
static Uint16 I2C_ReadWrite_Drv(I2cMsgReceive_t* con);
extern Eeprom_t Memory2[EEPROM_LENGHT];

const I2C_Device_t I2C_EepromAT24C512 = {MACRO1(I2C_MEM_ADDR), I2C_ADDR_16BIT, I2C_WORD_16BIT};
const I2C_Device_t I2C_RtcDS3231 = {MACRO1(I2C_RTC_ADDR), I2C_ADDR_8BIT, I2C_WORD_8BIT};

/*
	rtc
*/
/*
static struct I2CMSG I2cMsgOut1=
{
						  I2C_MSGSTAT_INACTIVE,
                          MACRO1(I2C_RTC_ADDR),
                          14,
                          0x00,
                          0x02,
 //                         0,  	// Msg Byte 0
 //                         0,	// Msg Byte 1
                          0,	// Msg Byte 2
                          1,	// Msg Byte 3
                          1,	// Msg Byte 4
                          1,	// Msg Byte 5
                          7,	// Msg Byte 6
                          0,	// Msg Byte 7
                          0,	// Msg Byte 8
                          0,	// Msg Byte 9
                          0,	// Msg Byte 0x0a
                          0,	// Msg Byte 0x0b
                          0,	// Msg Byte 0x0c
                          0,	// Msg Byte 0x0d
						  0,	// Msg Byte 0x0e
						  0,
						  0,
						  0
};                  
*/

static struct I2CMSG I2cMsgOut1=
{
	I2C_MSGSTAT_INACTIVE,
	MACRO1(I2C_MEM_ADDR),
	5,
	0,
	0,
	
	10,
	11,
	12,
	13,
	14,
	15,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

static struct I2CMSG I2cMsgIn1=
{ 
	I2C_MSGSTAT_INACTIVE,
	MACRO1(I2C_MEM_ADDR),
	I2C_MEM_NUMBYTES,
	I2C_MEM_HIGH_ADDR,
	I2C_MEM_LOW_ADDR
};

static struct I2CMSG Msg=
{ 
	I2C_MSGSTAT_INACTIVE,
	MACRO1(I2C_MEM_ADDR),
	0,
	0,
	0
};

I2cMsgReceive_t Msg1;
Uint16 MsgToWrite[I2C_MSG_TO_WRITE];


struct I2CMSG * pMsgOut1 = &I2cMsgOut1;
struct I2CMSG * pMsgIn1 = &I2cMsgIn1;
struct I2CMSG * CurrentMsgPtr;
I2cMsgReceive_t* pMsg1 = &Msg1;

int
TestMem(void)
{
	int a =0;
	int b = 1;

	return a + b;
}



void
MemEEPROM_Init(void)
{
	int a =0;
	int b = 1;

	a = a + b;
	
	CurrentMsgPtr = &I2cMsgOut1;
	MemEEPROM_RegsInit();
	memset(I2cMsgIn1.MsgBuffer, 0, (I2C_MAX_BUFFER_SIZE)*sizeof(Uint16));

	pMsg1->User.SlaveId = 0;
	pMsg1->User.Lenght = 0;
	pMsg1->User.Adress = 0;
	pMsg1->User.pBuffer = NULL;
	//pMsg1->User.pBuffer = &MsgToWrite[0];
	memset(pMsg1->User.Buf, 0, (128)*sizeof(Uint16));
	
	pMsg1->Control.Status = I2C_MSGSTAT_INACTIVE;
	pMsg1->Control.Count = 0;
	pMsg1->Control.CountSent = 0;
	pMsg1->Control.CountRequestToSend = 0;
	//pMsg1->Control.Lenght = 0;
	memset(pMsg1->Control.Fifo, 0, (I2C_MAX_BUFFER_SIZE)*sizeof(Uint16));
	
}



static void
MemEEPROM_RegsInit(void)
{
   // Initialize I2C
   I2caRegs.I2CSAR = 0x0050;		// Slave address - EEPROM control code

   #if (CPU_FRQ_100MHZ)
     I2caRegs.I2CPSC.all = 9;		// Prescaler - need 7-12 Mhz on module clk
   #endif
   #if (CPU_FRQ_60MHZ)
     I2caRegs.I2CPSC.all = 6;		// Prescaler - need 7-12 Mhz on module clk
   #endif
   I2caRegs.I2CCLKL = 10;			// NOTE: must be non zero
   I2caRegs.I2CCLKH = 5;			// NOTE: must be non zero
   //I2caRegs.I2CIER.all = 0x24;		// Enable SCD & ARDY interrupts
   I2caRegs.I2CIER.all = 0x67;

   I2caRegs.I2CMDR.all = 0x0020;	// Take I2C out of reset
   									// Stop I2C when suspended

   I2caRegs.I2CFFTX.all = 0x6000;	// Enable FIFO mode and TXFIFO
   I2caRegs.I2CFFRX.all = 0x2040;	// Enable RXFIFO, clear RXFFINT,
}



void
I2C_Sheduler(void)
{
	static Uint16 val = 0;
	Eeprom_t * pROM;
	struct I2cMsgUser_st Buf;
	
	
	if (pMsg1->User.ReqToRead)
	{
		I2C_ReadWrite(pMsg1, I2C_RECEIVE, Memory_ReadNew(0,0,0));
		pMsg1->User.ReqToRead = 0;
	}
	else
	{
		switch (val)
		{
			case 0:
				
				Rtc_Read(&I2C_RtcDS3231, 0, 19);
				break;
				
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			{
				/*	Для записи. Работает */
				
				pROM = Q_Retrive();
				if (pROM != NULL)
				{
					
					I2C_ConvertUserReq(pMsg1,pROM, &Buf);
					I2C_ReadWrite(pMsg1, I2C_TRANSMIT, &Buf);
				}
				
				break;
			}
				
			default:
				break;
		}
	}
	
	/*	Обязательный вызов */
	I2C_Driver(pMsg1);
	
	if (val >= 5)
		val = 0;
	else
		val++;
}



void
MemEEPROM_Isr(void)
{
	/*	Список локальных переменных	*/
	Uint16 i;
	Uint16 IntSource = I2caRegs.I2CISRC.all;
	
	
	
	
	if(IntSource == I2C_SCD_ISRC)
	{
		if (pMsg1->Control.Status == I2C_MSGSTAT_WRITE)
		{			
			pMsg1->Control.CountSent += pMsg1->Control.CountRequestToSend;
			if (pMsg1->Control.CountSent >= pMsg1->User.Lenght)
			{
				pMsg1->Control.Status = I2C_MSGSTAT_INACTIVE;
			}
			
		}
		else
		{
			/*	Тут должно быть что-то важное */
			if (0)
			{
				
			}
			
			/*	If completed message was reading EEPROM data, reset msg to inactive state and read data from FIFO */
			else if (pMsg1->Control.Status == I2C_MSGSTAT_READ_BUSY)
			{
				/*	Копирование во временный буфер */
				for(i=0; i < pMsg1->Control.CountReqToReceive; i++)
				{
				  pMsg1->Control.Fifo[i] = I2caRegs.I2CDRR;
				}
				
				/*	Копирование во внешний буфер */
				memcpy( pMsg1->User.pBuffer + pMsg1->Control.CountReceived, 
						pMsg1->Control.Fifo, 
						pMsg1->Control.CountReqToReceive );
						
				pMsg1->Control.CountReceived += pMsg1->Control.CountReqToReceive;
				
				if (pMsg1->Control.CountReceived >= pMsg1->User.Lenght)
				{
					//Memory_Convert2(pMsg1, Memory2);
					pMsg1->Control.Status = I2C_MSGSTAT_INACTIVE;
					
					if (pMsg1->User.Device == &I2C_EepromAT24C512)
					{
						
						DataConvert_EepromToParamsTable(pMsg1);
					}
					
					if (pMsg1->User.Device == &I2C_RtcDS3231)
					{
						
						RTC_Convert(pMsg1);
					}
					
					
					
					//RTC_Convert(pMsg1);
					
					//Page0.DateTime.Minute = BCD_to_DEC_16Bit(CurrentMsgPtr->MsgBuffer[1], 0);
					//Page0.DateTime.Year = BCD_to_DEC_16Bit(CurrentMsgPtr->MsgBuffer[7], 1);
				}
				/*
				else
				{
					pMsg1->Control.Status = I2C_MSGSTAT_READ_BUSY;	//заменил формат чтения с Random на CurrentAddress Read
					//pMsg1->Control.Status = I2C_MSGSTAT_READ;
				}
				*/
				
				I2C_ReadWrite_Drv(pMsg1);
			}
		}
	}
	else if(IntSource == I2C_ARDY_ISRC)
	{
		if(I2caRegs.I2CSTR.bit.NACK == 1)
		{
			I2caRegs.I2CMDR.bit.STP = 1;
			I2caRegs.I2CSTR.all = I2C_CLR_NACK_BIT;
		}
		else if (pMsg1->Control.Status == I2C_MSGSTAT_READ)
		{
			pMsg1->Control.Status = I2C_MSGSTAT_READ_BUSY;
			I2C_ReadWrite_Drv(pMsg1);
		}
	}
}


/*
	Функция приемо-передачи по I2C с использованием прерываний. 
	
	arg con - управляющий блок,
	arg opcode - тип операции: ПРОЧИТАТЬ или ЗАПИСАТЬ,
	arg msg - блок с информацией для приема-передачи
*/
Uint16 
I2C_ReadWrite(I2cMsgReceive_t* con, Uint16 opcode, struct I2cMsgUser_st * msg)
{
	/*	Данная функция не может быть вызвана, если не завершлся предыдущий процес приема или передачи */
	
	
	if (opcode == I2C_RECEIVE)
	{
		if (con->Control.Status == I2C_MSGSTAT_WRITE)
			return I2C_ERROR;
	}
	else if (opcode == I2C_TRANSMIT)
	{
		if (con->Control.Status == I2C_MSGSTAT_READ)
			return I2C_ERROR;
		if (con->Control.Status == I2C_MSGSTAT_READ_BUSY)
			return I2C_ERROR;
		if (con->Control.Status == I2C_MSGSTAT_READ_COPY)
			return I2C_ERROR;
	}
	else
	{
		return I2C_ERROR;
	}
	
	/*	аргумент msg должен иметь верный адрес */
	if (msg == NULL)
		return I2C_ERROR;
	
	/*	аргумент opcode должен иметь разрешенные значения */
	//if ((opcode != I2C_RECEIVE) | (opcode != I2C_TRANSMIT))
	//	return I2C_ERROR;
	
	/*	Обнуление полей управляющего блока */
	con->Control.Count = 0;
	con->Control.CountRequestToSend = 0;
	con->Control.CountSent = 0;
	con->Control.CountReqToReceive = 0;
	con->Control.CountReceived = 0;
	con->User.SlaveId = msg->SlaveId;
	con->User.Device = msg->Device;
	con->User.Lenght = msg->Lenght;
	con->User.Adress = msg->Adress;
	
	/*	Временная вставка, которую надо переделать */
	con->User.pBuffer = con->User.Buf;
	
	/*	Поле Status задает тип операции для I2C_ReadWrite_Drv */
	if (opcode == I2C_RECEIVE)
	{
		
		con->Control.Status = I2C_MSGSTAT_READ;
	}
	else if (opcode == I2C_TRANSMIT)
	{
		/*	Временная вставка, которую надо переделать */
		con->User.Buf[0] = msg->Buf[0];
		con->User.Buf[1] = msg->Buf[1];
		
		con->Control.Status = I2C_MSGSTAT_WRITE;
	}
	
	
	
	return I2C_SUCCESS;
}



void
I2C_Driver(I2cMsgReceive_t* con)
{
	I2C_ReadWrite_Drv(con);
}



static Uint16 
I2C_ReadWrite_Drv(I2cMsgReceive_t* con)
{
	/*	Список локальных переменных	*/
	Uint16 i;
	
	
	
	// Wait until the STP bit is cleared from any previous master communication.
	// Clearing of this bit by the module is delayed until after the SCD bit is
	// set. If this bit is not checked prior to initiating a new message, the
	// I2C could get confused.
	if (I2caRegs.I2CMDR.bit.STP == 1)
	{
		return I2C_STP_NOT_READY_ERROR;
	}
	
	
	
	switch (con->Control.Status)
	{
		/*	Два случая	*/
		case I2C_MSGSTAT_READ:
		{
						
			
			
			if (I2caRegs.I2CSTR.bit.BB == 1)
			{
				//I2caRegs.I2CSAR = con->User.SlaveId;
				
				I2caRegs.I2CMDR.all = 0x6EA0;
				return I2C_BUS_BUSY_ERROR;
				
				/*
				i = I2caRegs.I2CDRR;
				i = I2caRegs.I2CDRR;
				I2caRegs.I2CMDR.bit.STP = 1;
				I2caRegs.I2CDXR = (con->User.Adress + con->Control.CountReceived) & 0xFF;
			/*	Send data to setup EEPROM address */
			//I2caRegs.I2CMDR.all = 0x2620
				//I2caRegs.I2CSTR.all |= I2C_CLR_SCD_BIT;
				//I2caRegs.I2CSTR.all |= I2C_CLR_NACK_BIT;
			}
			
			/*	Заголовок */
			I2caRegs.I2CSAR = con->User.SlaveId;
			
			/*	Тело */
			switch (con->User.Device->AdrBitRange)
			{
				case (I2C_ADDR_8BIT):
				{
					I2caRegs.I2CCNT = 1;
					break;
				}
					
				case (I2C_ADDR_16BIT):
				{
					I2caRegs.I2CCNT = 2;
					I2caRegs.I2CDXR = 0;
					break;
				}
					
				default:
					break;
			}
			
			I2caRegs.I2CDXR = (con->User.Adress + con->Control.CountReceived) & 0xFF;
			
			/*	Send data to setup EEPROM address */
			I2caRegs.I2CMDR.all = 0x2620;
			
			
			
			
			break;
		}
		
		/*	Два случая	*/
		case I2C_MSGSTAT_READ_BUSY:
		{
			/*	Сдвиг каретки */
			if (con->User.Lenght - con->Control.CountReceived > I2C_MAX_BUFFER_SIZE)
			{
				con->Control.CountReqToReceive = I2C_MAX_BUFFER_SIZE;
			}
			else
			{
				con->Control.CountReqToReceive = con->User.Lenght - con->Control.CountReceived;
			}
			
			/*	Заголовок */			
			I2caRegs.I2CSAR = con->User.SlaveId;
			
			/*	Setup how many bytes to expect */	
			I2caRegs.I2CCNT = con->Control.CountReqToReceive;
			
			/*	Send restart as master receiver */
			I2caRegs.I2CMDR.all = 0x2C20;
			
			break;
		}
		
		/*	Два случая	*/
		case I2C_MSGSTAT_READ_COPY:
		{
			for(i=0; i < con->Control.CountRequestToSend; i++)
            {
              con->Control.Fifo[i] = I2caRegs.I2CDRR;
            }
			
			/*	Копирование во временный буфер */
			memcpy( con->User.pBuffer + con->Control.CountReceived, 
					con->Control.Fifo, 
					con->Control.CountReqToReceive );
			
			
			break;
		}
		
		/*	Два случая	*/
		case I2C_MSGSTAT_WRITE:
		{
			/*	Сдвиг каретки */
			if (con->User.Lenght - con->Control.CountSent > I2C_MSG_CARRIAGE)
			{
				con->Control.CountRequestToSend = I2C_MSG_CARRIAGE;
			}
			else
			{
				con->Control.CountRequestToSend = con->User.Lenght - con->Control.CountSent;
			}
			
			/*	Копирование во временный буфер */
			memcpy( con->Control.Fifo, 
					con->User.pBuffer + con->Control.CountSent, 
					con->Control.CountRequestToSend );
					
			/*	Заголовок */			
			I2caRegs.I2CSAR = con->User.SlaveId;
			
			/* Check if bus busy */
			if (I2caRegs.I2CSTR.bit.BB == 1)
				return I2C_BUS_BUSY_ERROR;
			
			/*	Тело */
			switch (con->User.Device->AdrBitRange)
			{
				case (I2C_ADDR_8BIT):
				{
					I2caRegs.I2CCNT = con->Control.CountRequestToSend + 1;
					break;
				}
					
				case (I2C_ADDR_16BIT):
				{
					I2caRegs.I2CCNT = con->Control.CountRequestToSend + 2;
					I2caRegs.I2CDXR = 0;
					break;
				}
					
				default:
				{
					break;
				}
			}
			
			I2caRegs.I2CDXR = (con->User.Adress + con->Control.CountSent) & 0xFF;
			for (i=0; i<con->Control.CountRequestToSend; i++)
			{
				I2caRegs.I2CDXR = *(con->Control.Fifo+i);
			}
			//control->Control.Status = I2C_MSGSTAT_WRITE_BUSY;
			
			/*	Send start as master transmitter */
			I2caRegs.I2CMDR.all = 0x6E20;
			
			
			break;
		}
		
		default:
			break;
	}
	
	return I2C_SUCCESS;
	
}

