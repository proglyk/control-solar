#include "DataConvert.h"
#include "string.h"
#include "ParamsTable.h"

extern I2cMsgReceive_t* pMsg1;
extern const I2C_Device_t I2C_EepromAT24C512;
extern const I2C_Device_t I2C_RtcDS3231;

extern ParamsTable_t Params;

Eeprom_t Memory[EEPROM_LENGHT+5] = 
{
	{&I2C_EepromAT24C512, 0x0000, 0x80}, 
	{&I2C_EepromAT24C512, 0x0002, 0x81}, 
	{&I2C_EepromAT24C512, 0x0004, 0x82}, 
	{&I2C_EepromAT24C512, 0x0006, 0x83}, 
	{&I2C_EepromAT24C512, 0x0008, 0x84},
	{&I2C_EepromAT24C512, 0x000a, 0x85}, 
	{&I2C_EepromAT24C512, 0x000c, 0x86}, 
	{&I2C_EepromAT24C512, 0x000e, 0x87}, 
	{&I2C_EepromAT24C512, 0x0010, 0x88}, 
	{&I2C_EepromAT24C512, 0x0012, 0x89}
};

Eeprom_t Memory2[EEPROM_LENGHT] = 
{
	{&I2C_EepromAT24C512, 0x0000, 0}, 
	{&I2C_EepromAT24C512, 0x0002, 0}, 
	{&I2C_EepromAT24C512, 0x0004, 0}, 
	{&I2C_EepromAT24C512, 0x0006, 0}, 
	{&I2C_EepromAT24C512, 0x0008, 0}
};


Queue_t Q = 
{
	{NULL, NULL, NULL, NULL, NULL},
	{{0,0}, {0,0}, {0,0}, {0,0}, {0,0}},
	0,
	0
};
extern I2cMsgReceive_t* pMsg1;

Qerror_t
Q_Store1(Eeprom_t * q)
{
	/*	Переполнение */
	if ((Q.Spos + 1 == Q.Rpos) | 
	   ((Q.Spos + 1 == QUEUE_LENGHT) & Q.Rpos == 0))
	{
		return Q_ERR_OVRFL;
	}
	
	Q.Buf[Q.Spos] = q;
	Q.Spos++;
	if (Q.Spos == QUEUE_LENGHT)
	{
		Q.Spos = 0;
	}
	
	return Q_ERR_OK;
}

Qerror_t
Q_Store2(Uint16 Addr, Uint16 Value)
{
	/*	Переполнение */
	if ((Q.Spos + 1 == Q.Rpos) | 
	   ((Q.Spos + 1 == QUEUE_LENGHT) & Q.Rpos == 0))
	{
		return Q_ERR_OVRFL;
	}
	
	/*	Локальный буфер, привязанный к структуре очереди */
	Q.BufData[Q.Spos].Adr = Addr;
	Q.BufData[Q.Spos].Data = Value;
	
	/* Указатель, ссылающийся на свой же буфер. Криво, но переделывать Лень */
	Q.Buf[Q.Spos] = &(Q.BufData[Q.Spos]);
	Q.Spos++;
	if (Q.Spos == QUEUE_LENGHT)
	{
		Q.Spos = 0;
	}
	
	return Q_ERR_OK;
}

Qerror_t
Q_Store3(const I2C_Device_t * dev, Uint16 Addr, Uint16 Value)
{
	/*	Переполнение */
	if ((Q.Spos + 1 == Q.Rpos) | 
	   ((Q.Spos + 1 == QUEUE_LENGHT) & Q.Rpos == 0))
	{
		return Q_ERR_OVRFL;
	}
	
	/*	Локальный буфер, привязанный к структуре очереди */
	Q.BufData[Q.Spos].Device = dev;
	Q.BufData[Q.Spos].Adr = Addr;
	Q.BufData[Q.Spos].Data = Value;
	
	/* Указатель, ссылающийся на свой же буфер. Криво, но переделывать Лень */
	Q.Buf[Q.Spos] = &(Q.BufData[Q.Spos]);
	Q.Spos++;
	if (Q.Spos == QUEUE_LENGHT)
	{
		Q.Spos = 0;
	}
	
	return Q_ERR_OK;
}

Eeprom_t *
Q_Retrive(void)
{
	/*	Установить в начало */
	if (Q.Rpos == QUEUE_LENGHT)
	{
		Q.Rpos = 0;
	}
	
	/*	Очередь пуста */
	if (Q.Rpos == Q.Spos)
	{
		return NULL;
	}
	
	Q.Rpos++;
	
	return Q.Buf[Q.Rpos - 1];
}

void
Memory_Write(Eeprom_t * mem)
{
	Q_Store1(mem);
}

void
Memory_Read(Uint16 Address, Uint16 Lenght)
{
	struct I2cMsgUser_st I2cUser;
	
	I2cUser.SlaveId = MACRO1(I2C_MEM_ADDR);
	I2cUser.Lenght = Lenght;
	I2cUser.Adress = Address;
	I2cUser.pBuffer = I2cUser.Buf;
	
	I2C_ReadWrite(pMsg1, I2C_RECEIVE, &I2cUser);
}

struct I2cMsgUser_st *
Memory_ReadNew(const I2C_Device_t * dev, Uint16 Address, Uint16 Lenght)
{
	static struct I2cMsgUser_st I2cUser;
	
	if (dev != NULL)
	{
		I2cUser.SlaveId = MACRO1(I2C_MEM_ADDR);
		I2cUser.Lenght = Lenght;
		I2cUser.Adress = Address;
		I2cUser.pBuffer = I2cUser.Buf;
		I2cUser.Device = dev;
	}
	
	return &I2cUser;
	
	
}

void
Rtc_Read(const I2C_Device_t * dev, Uint16 Address, Uint16 Lenght)
//Rtc_Read(Uint16 Address, Uint16 Lenght)
{
	struct I2cMsgUser_st I2cUser;
	
	I2cUser.SlaveId = MACRO1(I2C_RTC_ADDR);
	I2cUser.Lenght = Lenght;
	I2cUser.Adress = Address;
	I2cUser.pBuffer = I2cUser.Buf;
	I2cUser.Device = dev;
	
	I2C_ReadWrite(pMsg1, I2C_RECEIVE, &I2cUser);
}

void
Memory_Convert1(Eeprom_t * rom, struct I2cMsgUser_st * buffer)
{
	buffer->SlaveId = MACRO1(I2C_MEM_ADDR);
	buffer->Lenght = 2;
	buffer->Adress = rom->Adr;
	buffer->pBuffer = &(rom->Data);
	
	/*	Временная вставка, которую надо переделать */
	buffer->Buf[0] = (rom->Data >> 8);
	buffer->Buf[1] = (rom->Data & 0xff);
}

void
Memory_Convert2(I2cMsgReceive_t* con, Eeprom_t * memory)
{
	Uint16 i;
	Uint16 data;
	
	for (i = 0; i < con->User.Lenght; i += 2)
	{
		data = con->User.Buf[i] << 8;
		data += con->User.Buf[i+1];
		
		(memory + (i/2))->Data = data;
	}
}

void
DataConvert_EepromToParamsTable(I2cMsgReceive_t* con)
{
	Uint16 i;
	Uint16 data;
	
	for (i = 0; i < con->User.Lenght; i += 2)
	{
		data = con->User.Buf[i] << 8;
		data += con->User.Buf[i+1];
		
		switch (i/2)
		{
			case (0):
				//Params.Page0.Reg0 = data;
				//Params.Page0.DateTime.Day = data;
				//Params.Page0.DateTime.Month = data;
				
				Params.DateTime.Day = data & 0xff;
				Params.DateTime.Month = data >> 8;
				break;
				
			case (1):
				//Params.Page0.Reg1 = data;
				//Params.Page0.DateTime.Month = data;
				Params.DateTime.Year = data & 0xff;
				break;
				
			case (2):
				Params.DateTime.Second = data & 0xff;
				Params.DateTime.Minute = data >> 8;
				break;
			
			case (3):
				Params.DateTime.Hour = data & 0xff;
				break;
			
			default:
				break;
		}
		//(memory + (i/2))->Data = data;
	}
}

void
RTC_Convert1(Eeprom_t * rom, struct I2cMsgUser_st * buffer)
{
	
	//I2C_ConvertUserReq(rom, buffer);
}

void
I2C_ConvertUserReq(I2cMsgReceive_t* con, Eeprom_t * rom, struct I2cMsgUser_st * buffer)
{
	/*	Адрес I2C-устройства */
	buffer->SlaveId = rom->Device->SlaveId;
	buffer->Device = rom->Device;
	/*	Раряднось слова I2C-устройства */
	switch (rom->Device->DataBitRange)
	//switch (con->User.Device->DataBitRange)
	{
		case (I2C_WORD_8BIT):
			buffer->Lenght = 1;
			/*	Временная вставка, которую надо переделать */
			buffer->Buf[0] = (rom->Data);
			
			break;
					
		case (I2C_WORD_16BIT):
			buffer->Lenght = 2;
			/*	Временная вставка, которую надо переделать */
			buffer->Buf[0] = (rom->Data >> 8);
			buffer->Buf[1] = (rom->Data & 0xff);
			break;
					
		default:
			break;
	}
	/*	Раряднось слова I2C-устройства */
	buffer->Adress = rom->Adr;
	/*	Раряднось слова I2C-устройства */
	buffer->pBuffer = &(rom->Data);
	/*	Временная вставка, которую надо переделать */
}



void
RTC_Convert(I2cMsgReceive_t* con)
{
	//Uint16 i;
	Uint16 data;
	
	/* Декодирование даных в формат Uint16 для дальнейшего использования */
	data = con->User.Buf[0] & 0x00FF;
	Params.DateTime.Second = BCD_to_DEC_16Bit(data, RTC_SEC);
	
	data = con->User.Buf[1] & 0x00FF;
	Params.DateTime.Minute = BCD_to_DEC_16Bit(data, RTC_MIN);
	
	data = con->User.Buf[2] & 0x00FF;
	Params.DateTime.Hour = BCD_to_DEC_16Bit(data, RTC_HOUR);
	
	data = con->User.Buf[4] & 0x00FF;
	Params.DateTime.Day = BCD_to_DEC_16Bit(data, RTC_DAT);
	
	data = con->User.Buf[5] & 0x00FF;
	Params.DateTime.Month = BCD_to_DEC_16Bit(data, RTC_MONTH);
	
	data = con->User.Buf[6] & 0x00FF;
	Params.DateTime.Year = BCD_to_DEC_16Bit(data, RTC_YEAR);
	
	/*
	for (i = 0; i < con->User.Lenght; i += 2)
	{
		data = con->User.Buf[i] << 8;
		data += con->User.Buf[i+1];
		
		(memory + (i/2))->Data = data;
	}
	*/
}

void
ConvertInit(void)
{
	
	/*
	Page0.Value1 = 0;
	Page0.Value2 = 0;
	Page0.Password = 0;
		
	Page0.DateTime.Year = 01;
	Page0.DateTime.Month = 01;
	Page0.DateTime.Day = 01;
	Page0.DateTime.Hour = 01;
	Page0.DateTime.Minute = 01;
	Page0.DateTime.Second = 01;
	*/
}

Uint16
DataConvert_DS3231_Set(Uint16 in, BCDFormat_t bits)
{
	Uint16 	u16_Tmp1 = 0;
	Uint16	u16_Return = 0;
	
	switch (bits)
	{
		case (BCDF_8BIT):
		{
			u16_Tmp1 = in / 10;
			u16_Tmp1 = u16_Tmp1 << 4;
			u16_Return = u16_Tmp1 & 0x00f0;
		
			u16_Tmp1 = in - ((in / 10)*10);
			u16_Return += u16_Tmp1 & 0x000f;
			
			break;
		}
		
		case (BCDF_7BIT):
		{
			u16_Tmp1 = in / 10;
			u16_Tmp1 = u16_Tmp1 << 4;
			u16_Return = u16_Tmp1 & 0x0070;
		
			u16_Tmp1 = in - ((in / 10)*10);
			u16_Return += u16_Tmp1 & 0x000f;
			
			break;
		}
		
		case (BCDF_6BIT):
		{
			u16_Tmp1 = in / 10;
			u16_Tmp1 = u16_Tmp1 << 4;
			u16_Return = u16_Tmp1 & 0x0030;
		
			u16_Tmp1 = in - ((in / 10)*10);
			u16_Return += u16_Tmp1 & 0x000f;
			
			break;
		}
		
		case (BCDF_5BIT):
		{
			u16_Tmp1 = in / 10;
			u16_Tmp1 = u16_Tmp1 << 4;
			u16_Return = u16_Tmp1 & 0x0010;
		
			u16_Tmp1 = in - ((in / 10)*10);
			u16_Return += u16_Tmp1 & 0x000f;
			
			break;
		}
	}
	
	return u16_Return;
}
/**/

Uint16
DEC_to_BCD(Uint16 in, Uint16 param)
{
	Uint16 	u16_Tmp = 0;
	Uint16 	u16_Tmp1 = 0;
	Uint16	u16_Return = 0;
	
	switch (param)
	{
		case (0):
		{
			u16_Tmp1 = in / 10;
			u16_Tmp1 = u16_Tmp1 << 4;
			u16_Return = u16_Tmp1 & 0x00f0;
		
			u16_Tmp1 = in - ((in / 10)*10);
			u16_Return += u16_Tmp1 & 0x000f;
			
			break;
		}
		
		case (1):
		{
			u16_Tmp = in / 10;
			u16_Tmp = u16_Tmp << 12;
			u16_Return = u16_Tmp & 0xf000;
		
			u16_Tmp = in - ((in / 10)*10);
			u16_Tmp = u16_Tmp << 8;
			u16_Return += u16_Tmp & 0x0f00;
			break;
		}
	}
	
	return u16_Return;
}
/**/

Uint16
BCD_to_DEC_16Bit(Uint16 In, RTC_Type_t type)
{
	Uint16 u16Var = 0;
	Uint16 u16_Tmp = 0;
	
	switch (type)
	{
		
		case (RTC_SEC):	//Секунда, Минута
		case (RTC_MIN):
			u16_Tmp = In & 0x000f;
			u16Var = u16_Tmp;
			
			u16_Tmp = In & 0x0070;
			u16_Tmp = u16_Tmp >> 4;
			u16_Tmp *= 10;
			u16Var += u16_Tmp;
			
			break;
			
		case (RTC_HOUR):	//Час
			u16_Tmp = In & 0x000f;
			u16Var = u16_Tmp;
			
			u16_Tmp = In & 0x0010;
			u16_Tmp = u16_Tmp >> 4;
			u16_Tmp *= 10;
			u16Var += u16_Tmp;
			
			break;
			
		case (RTC_DAT):	//День месяца
			u16_Tmp = In & 0x000f;
			u16Var = u16_Tmp;
			
			u16_Tmp = In & 0x0030;
			u16_Tmp = u16_Tmp >> 4;
			u16_Tmp *= 10;
			u16Var += u16_Tmp;
			
			break;
			
		case (RTC_MONTH):	//Месяц
			u16_Tmp = In & 0x000f;
			u16Var = u16_Tmp;
			
			u16_Tmp = In & 0x0010;
			u16_Tmp = u16_Tmp >> 4;
			u16_Tmp *= 10;
			u16Var += u16_Tmp;
			
			break;
			
		case (RTC_YEAR):	//Год
			u16_Tmp = In & 0x000f;
			u16Var = u16_Tmp;
			
			u16_Tmp = In & 0x00f0;
			u16_Tmp = u16_Tmp >> 4;
			u16_Tmp *= 10;
			u16Var += u16_Tmp;
			
			break;
			
		
			
		default:
			break;
	}
	
	return u16Var;
}
