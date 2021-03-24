#include "sciv3.h"
#include "string.h"
#include "main.h"
//#include "stdlib.h"
#include "DataConvert.h"
#include "ParamsTable.h"

Uint8* p_txFrame = NULL;

Uint16 GTMP0;
Uint16 GTMP1;

extern ParamsTable_t Params;
extern PControl_t PControl;
extern CrashNotice_t Fault;

//extern Page0_t Page0;

void Test(void);
void SciSlave_GetPRD(Uint32 speed, volatile Uint16* hbaud, volatile Uint16* lbaud);
Uint16 SciSlave_GetTimeInterval(SciSlave_t* slave, Mode_t time);

/*	Обработчик прерывания по приему	*/
void
SciSlave_Receive(SciSlave_t* slave)
{
	Uint8 tmp;
	Uint8 byte;
	
	byte = ((slave->ptSciRegs->SCIRXBUF.all) & 0xff);
	
	//if (slave->Name == SCI_PULT)
	//{
	//	GpioDataRegs.GPATOGGLE.bit.GPIO10 = 1;
	//}
	
	
	/*	Обработка первого байта */
	if (slave->Mode == MODE_NEWFRAME)
	{
		slave->DataReceive[slave->BytesCounter] = byte;
		
		//GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;
		
		if ( slave->DataReceive[slave->BytesCounter] == slave->ID )
		{
			slave->Mode = MODE_15T;
			slave->BytesCounter++;
				
			SciSlave_TimerSetPeriod(slave->ptCpuTimer, 100, SciSlave_GetTimeInterval(slave, MODE_15T));
			slave->ptCpuTimer->RegsAddr->TIM.all = 0;
			slave->ptCpuTimer->RegsAddr->TCR.bit.TSS = 0;
		}
	}
	
	/*	Обработка последующих байт, принятых за интервал времени от начала 
		приема менее 1,5Т */
	else if (slave->Mode == MODE_15T)
	{
		/*	Если таймер все еще работает (а может быть так, что после интервала 1,5Т он уже
			выключен)) */
		if (slave->ptCpuTimer->RegsAddr->TCR.bit.TSS == 0)
		{
			/*	Обнулить и снова запустить в работу */
			slave->ptCpuTimer->RegsAddr->TIM.all = 0;
			slave->ptCpuTimer->RegsAddr->TCR.bit.TSS = 0;
			
			slave->DataReceive[slave->BytesCounter] = byte;
			
			if ( slave->BytesCounter == 1 )
			{
				tmp = slave->DataReceive[slave->BytesCounter];
				
				switch (tmp)
				{
					case 0x03:
					case 0x04:
						slave->FuncCode = tmp;
						slave->BytesNumber = BYTESNUMBER_X04;
						break;
					case 0x06:
						slave->FuncCode = tmp;
						slave->BytesNumber = BYTESNUMBER_X06;
						break;
					case 0x10:
						slave->FuncCode = tmp;
						slave->BytesNumber = BYTESNUMBER_X10;
						break;
					default:
						//slave->ptCpuTimer->RegsAddr->TCR.bit.TSS = 1;
						//SciSlave_Default(slave);
						
						/*	Непраильно число */ 
						slave->BytesNumber = BYTESNUMBER_UNK; 
						break;
				}
			}
			
			/*	Пропущена проверка на 0X10 функцию */
			if ((slave->BytesCounter == 6) & (slave->FuncCode == 0x10))
			{
				slave->BytesNumber = (9 + slave->DataReceive[slave->BytesCounter]);
			}
			
			/*
			if ((byteCount_slave == 6) & (u8CurrentFuncCode == 16)){
				NumOfBytes = (t_NumOfBytes)(9 + rxFrame_slave[byteCount_slave]);
			}
			*/
			
			/*	делаем пустой инкремент */
				slave->BytesCounter++;
		}
		else
		{
			//slave->Mode = MODE_NEWFRAME;
		}
	}
	
	/*	Подсчет байт за интервал времени тишины (более 1,5Т и менее 3,5Т )на линии */
	else if (slave->Mode == MODE_35T)
	{
		//slave->BytesNumIn35T++;
		tmp = byte;
	}
	
	/*	Пустая операция чтения из буфера */
	else
	{
		tmp = byte;
	}
	
}


/*	Обработчик прерывания по передаче	*/
void
SciSlave_Transmit(SciSlave_t* slave)
{
	//static Uint16 byteCnt = 1;
	//slave->ByteCntTx = 1;
	
	//if (slave->Name == SCI_PULT)
	//{
	//	GpioDataRegs.GPATOGGLE.bit.GPIO10 = 1;
	//}
	
	
	if (slave->ByteCntTx == slave->DataTransmitLenght){
		slave->LookForTxEmpty = true;
		SciSlave_DisableTransmitInterrupt(slave);
		slave->ByteCntTx = 1;
	}
	
	if ((slave->ByteCntTx < slave->DataTransmitLenght) & (slave->LookForTxEmpty == _false)){
		slave->ptSciRegs->SCITXBUF = slave->DataTransmit[slave->ByteCntTx];	//idle
		slave->ByteCntTx++;
	}
}


/*	Обработчик прерывания по переполнению счетчика таймера	*/
void
SciSlave_StopTimer35T(SciSlave_t* slave)
{
	static Error_t err;
	
	//if (slave->Name == SCI_PULT)
	//{
	//	GpioDataRegs.GPATOGGLE.bit.GPIO10 = 1;
	//}
	
	/*	Выключить Таймер */
	slave->ptCpuTimer->RegsAddr->TCR.bit.TSS = 1;
	
	/*	Если таймер сработал раньше приема нового байта */
	if (slave->Mode == MODE_15T)
	{
		/*	Если номер функции был определен, то */
//		if (slave->BytesNumber != BYTESNUMBER_UNK)
		{
			if ( (slave->BytesNumber != BYTESNUMBER_UNK) & (slave->BytesCounter == slave->BytesNumber) )
			{
				memcpy(slave->Buf2, slave->DataReceive, slave->BytesNumber);
				
				slave->ByteCntTx = 1;

				switch (slave->FuncCode)
				{
					case (0x03):
					case (0x04):
						err = SciSlave_FunctionX04(slave);
						break;
					case (0x06):
						err = SciSlave_F06(slave);
						break;
					case (0x10):
						err = SciSlave_FunctionX10(slave);
						break;
					default:
						break;
				}
				
				

				SciSlave_Default(slave);

				/*	Сменить режим ожидания на 3,5Т */
				//slave->Mode = MODE_35T;
					
				/*	Обнулить и снова запустить в работу */
				//SciSlave_TimerSetPeriod(slave->ptCpuTimer, 100, SciSlave_GetTimeInterval(slave, MODE_35T));
				//slave->ptCpuTimer->RegsAddr->TIM.all = 0;
				//slave->ptCpuTimer->RegsAddr->TCR.bit.TSS = 0;
			}
			else
			{
				if (slave->Name == SCI_PC)
				{
					
				}
				SciSlave_Default(slave);
			}
			
			
		}
	}
	
	/*	переходим к обработке только в том случае, если за интервал 3,5Т была соблюдена 
		тишина на линии */
	else if (slave->Mode == MODE_35T)
	{
		/*	обнулить структуру до начальных значений */
		
	}	
}


/*	Инициализация SCI	*/
Error_t
SciSlave_Setup(SciSlave_t* slave, Uint32 baud, Uint8 id, volatile Uint32* REport, Uint16 REpin)
{
	Error_t err;
	
	if (slave->ptSciRegs == NULL)
		return ERR_NULLREF;
		
	slave->ptSciRegs->SCICCR.all = 0x0087;
	slave->ptSciRegs->SCICTL1.bit.RXENA = 1;
	slave->ptSciRegs->SCICTL1.bit.TXENA = 1;
	slave->ptSciRegs->SCICTL2.bit.RXBKINTENA = 1;
	slave->ptSciRegs->SCICTL2.bit.TXINTENA = 1;
	//slave->ptSciRegs->SCIHBAUD = 0x0000;
	SciSlave_GetPRD(baud, &(slave->ptSciRegs->SCIHBAUD), &(slave->ptSciRegs->SCILBAUD));
	//ScibRegs.SCILBAUD = 0x50;
	slave->ptSciRegs->SCIFFTX.all=0x8000;
	slave->ptSciRegs->SCIFFRX.all = 0;
	slave->ptSciRegs->SCIFFCT.all=0x0;
	slave->ptSciRegs->SCICTL1.bit.SWRESET = 1;  // Relinquish SCI from Reset
	
	slave->BaudRate = baud;
	slave->ID = id;
	slave->pGPIOPort = REport;
	slave->GPIOBit = REpin;
	
	SciSlave_TimerInit(slave->ptCpuTimer);
	
	SciSlave_Default(slave);
	
	return ERR_OK;
}


/*	Вспомогательные функции	*/


void
Test(void)
{
	static Uint8* pPointer;
	
	pPointer = malloc(128 * sizeof(Uint8));
	
	
	
	free(pPointer);
}

void
SciSlave_GetPRD(Uint32 speed, volatile Uint16* hbaud, volatile Uint16* lbaud)
{
	Uint32 tmp;
	Uint32 baud;
	Uint32 lspckl;
	Uint32 sysclk;
	
#if defined RT_MODE
	sysclk = CPU_FREQ;
#elif defined DBG_MODE_DIV2
	sysclk = CPU_FREQ / 2;
#endif
	
	
	(SysCtrlRegs.LOSPCP.all == 0) ?
		(tmp = 1) :
		(tmp = 0) ;
	lspckl = sysclk / (tmp + 2 * SysCtrlRegs.LOSPCP.all);
	baud = (lspckl /( speed * 8)) - 1;
	
	*hbaud = (Uint16)(baud & 0xff00);
	*lbaud = (Uint16)(baud & 0x00ff);
}

Uint16
SciSlave_GetTimeInterval(SciSlave_t* slave, Mode_t time)
{
	float fframe = 1.0f / (slave->BaudRate);
	Uint32 u32frame;
	Uint16 interval = 0;
	
	fframe *= 1000000;
	u32frame = fframe * 11;
	
	switch (time)
	{
		case MODE_15T:
			interval = (u32frame * 3) / 2;
			break;
		case MODE_35T:
			interval = u32frame * 2;
			break;
		default:
			break;
	}
	
#if defined RT_MODE
	interval /= 1;
#elif defined DBG_MODE_DIV2
	interval /= 2;
#endif	
	
	return interval;
}

Error_t
SciSlave_FunctionX04(SciSlave_t* slave)
{
	static Uint16 regStartPos, regNumber, u16CRC;
	Uint8* p_txFrame = NULL;
	Uint8* pInvState = NULL;
	Uint16 t;
	Uint16 Tmp;
	
	/*	Проверка CRC	*/
	u16CRC = SciSlave_CRC16(slave->Buf2, ALLOW_BYTE_NUM_FUNC_X04 - 2);
	if (u16CRC != ((slave->Buf2[7]<<8)|slave->Buf2[6]))
	{
		return ERR_CRC1;
	}
	
	/*	Проверка адреса начального регистра	*/
	regStartPos = (slave->Buf2[2] << 8) + (slave->Buf2[3]);
	if (regStartPos >= SLAVE_4FUNC_DATA_MAX_NUM)
	{
		return ERR_CRC2;
	}
	
	/*	Проверка числа запрашиваемых регистров	*/
	regNumber = (slave->Buf2[4] << 8) + (slave->Buf2[5]);
	if ((regNumber > SLAVE_4FUNC_DATA_MAX_NUM) | (regNumber == 0) |
		((regStartPos+regNumber) > SLAVE_4FUNC_DATA_MAX_NUM))
	{
		return ERR_CRC3;
	}
	
	/*	Подговка буферов байт для передачи	*/
	p_txFrame = malloc((2*regNumber + 5)*sizeof(Uint8));
	pInvState = malloc(2*SLAVE_4FUNC_DATA_MAX_NUM*sizeof(Uint8));
	
	/*	Обнуление до начальных значений	*/
	memset(p_txFrame, 0, (2*regNumber + 5)*sizeof(Uint8));
	memset(pInvState, 0, (2*SLAVE_4FUNC_DATA_MAX_NUM)*sizeof(Uint8));
	
	//if (slave->Name == SCI_PULT)
	//{
	//	GpioDataRegs.GPATOGGLE.bit.GPIO10 = 1;
	//}
	
	// WORD 0 MODBUS_REG_TIME1
	Tmp = DEC_to_BCD(Params.DateTime.Second, 0);
	Tmp |= DEC_to_BCD(Params.DateTime.Minute, 1);
		
	pInvState[0] = (Uint8)((Tmp & 0xff00) >> 8);
	pInvState[1] = (Uint8)(Tmp & 0x00ff);
		
	// WORD 1 MODBUS_REG_TIME2
	Tmp = DEC_to_BCD(Params.DateTime.Hour, 0);
		
	pInvState[2] = (Uint8)((Tmp & 0xff00) >> 8);
	pInvState[3] = (Uint8)(Tmp & 0x00ff);
	
	// WORD 2 MODBUS_REG_DATE1
	Tmp = DEC_to_BCD(Params.DateTime.Day, 0);
	Tmp |= DEC_to_BCD(Params.DateTime.Month, 1);
	
	pInvState[4] = (Uint8)((Tmp & 0xff00) >> 8);
	pInvState[5] = (Uint8)(Tmp & 0x00ff);
		
	// WORD 3 MODBUS_REG_DATE2
	Tmp = DEC_to_BCD(Params.DateTime.Year, 0);
	
	pInvState[6] = (Uint8)((Tmp & 0xff00) >> 8);
	pInvState[7] = (Uint8)(Tmp & 0x00ff);
	
	// WORD 4 MODBUS_REG_0
	pInvState[8] = (Uint8)((Params.Page0.Reg0 & 0xff00) >> 8);
	pInvState[9] = (Uint8)(Params.Page0.Reg0 & 0x00ff);
		
	// WORD 5 MODBUS_REG_1
	pInvState[10] = (Uint8)((Params.Page0.Reg1 & 0xff00) >> 8);
	pInvState[11] = (Uint8)(Params.Page0.Reg1 & 0x00ff);
		
	// WORD 6 MODBUS_REG_2
	pInvState[12] = (Uint8)((Params.Page0.Reg2 & 0xff00) >> 8);
	pInvState[13] = (Uint8)(Params.Page0.Reg2 & 0x00ff);
	
	// WORD 7 MODBUS_REG_INVERTER
	pInvState[14] = (Uint8)((Params.Page0.fBridge & 0xff00) >> 8);
	pInvState[15] = (Uint8)(Params.Page0.fBridge & 0x00ff);
	
	// WORD 8 MODBUS_REG_MPPT1
	pInvState[16] = (Uint8)((Params.Page0.fMPPT1 & 0xff00) >> 8);
	pInvState[17] = (Uint8)(Params.Page0.fMPPT1 & 0x00ff);
	
	// WORD 9 MODBUS_REG_MPPT2
	pInvState[18] = (Uint8)((Params.Page0.fMPPT2 & 0xff00) >> 8);
	pInvState[19] = (Uint8)(Params.Page0.fMPPT2 & 0x00ff);
	
	// WORD 10 STARTSTOP
	pInvState[20] = (Uint8)((Params.Page0.Mode & 0xff00) >> 8);
	pInvState[21] = (Uint8)(Params.Page0.Mode & 0x00ff);
	
	// WORD 11 MODBUS_ADC_TSTB
	pInvState[(2*MODBUS_ADC_TSTB)] = (Uint8)((Params.Page0.RegTSTB & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_TSTB)+1] = (Uint8)(Params.Page0.RegTSTB & 0x00ff);
	
	// WORD 12 MODBUS_ADC_TRAD
	pInvState[(2*MODBUS_ADC_TRAD)] = (Uint8)((Params.Page0.RegTRAD & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_TRAD)+1] = (Uint8)(Params.Page0.RegTRAD & 0x00ff);
	
	// WORD 13 MODBUS_ADC_220VN
	pInvState[(2*MODBUS_ADC_220VN)] = (Uint8)((Params.Page0.Reg220VN & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_220VN)+1] = (Uint8)(Params.Page0.Reg220VN & 0x00ff);
	
	// WORD 14 MODBUS_ADC_ISTBN
	pInvState[(2*MODBUS_ADC_ISTBN)] = (Uint8)((Params.Page0.RegISTBN & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_ISTBN)+1] = (Uint8)(Params.Page0.RegISTBN & 0x00ff);
	
	// WORD 15 MODBUS_ADC_TTR
	pInvState[(2*MODBUS_ADC_TTR)] = (Uint8)((Params.Page0.RegTTR & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_TTR)+1] = (Uint8)(Params.Page0.RegTTR & 0x00ff);
	
	// WORD 16 MODBUS_ADC_STBN
	pInvState[(2*MODBUS_ADC_STBN)] = (Uint8)((Params.Page0.RegSTBN & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_STBN)+1] = (Uint8)(Params.Page0.RegSTBN & 0x00ff);
	
	// WORD 17 MODBUS_ADC_IINVN
	pInvState[(2*MODBUS_ADC_IINVN)] = (Uint8)((Params.Page0.RegIINVN & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_IINVN)+1] = (Uint8)(Params.Page0.RegIINVN & 0x00ff);
	
	// WORD 18 MODBUS_ADC_I220N
	pInvState[(2*MODBUS_ADC_I220N)] = (Uint8)((Params.Page0.RegI220N & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_I220N)+1] = (Uint8)(Params.Page0.RegI220N & 0x00ff);
	
	// WORD 19 MODBUS_ADC_IO2N
	pInvState[(2*MODBUS_ADC_IO2N)] = (Uint8)((Params.Page0.RegIO2N & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_IO2N)+1] = (Uint8)(Params.Page0.RegIO2N & 0x00ff);
	
	// WORD 20 MODBUS_ADC_SB2N
	pInvState[(2*MODBUS_ADC_SB2N)] = (Uint8)((Params.Page0.RegSB2N & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_SB2N)+1] = (Uint8)(Params.Page0.RegSB2N & 0x00ff);
	
	// WORD 21 MODBUS_ADC_SB1N
	pInvState[(2*MODBUS_ADC_SB1N)] = (Uint8)((Params.Page0.RegSB1N & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_SB1N)+1] = (Uint8)(Params.Page0.RegSB1N & 0x00ff);
	
	// WORD 22 MODBUS_ADC_IO1N
	pInvState[(2*MODBUS_ADC_IO1N)] = (Uint8)((Params.Page0.RegIO1N & 0xff00) >> 8);
	pInvState[(2*MODBUS_ADC_IO1N)+1] = (Uint8)(Params.Page0.RegIO1N & 0x00ff);
	
	// WORD 23 MODBUS_SOLBAT_CH1_U_MPP
	pInvState[(2*MODBUS_SOLBAT_CH1_U_MPP)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH1_U_MPP & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH1_U_MPP)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH1_U_MPP & 0x00ff);
	
	// WORD 24 MODBUS_SOLBAT_CH1_U_XX
	pInvState[(2*MODBUS_SOLBAT_CH1_U_XX)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH1_U_XX & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH1_U_XX)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH1_U_XX & 0x00ff);
	
	// WORD 25 MODBUS_SOLBAT_CH1_NUM_SERIAL
	pInvState[(2*MODBUS_SOLBAT_CH1_NUM_SERIAL)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH1_NUM_SERIAL & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH1_NUM_SERIAL)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH1_NUM_SERIAL & 0x00ff);
	
	// WORD 26 MODBUS_SOLBAT_CH1_NUM_PARAL
	pInvState[(2*MODBUS_SOLBAT_CH1_NUM_PARAL)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH1_NUM_PARAL & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH1_NUM_PARAL)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH1_NUM_PARAL & 0x00ff);
	
	// WORD 27 MODBUS_SOLBAT_CH1_I_KZ
	pInvState[(2*MODBUS_SOLBAT_CH1_I_KZ)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH1_I_KZ & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH1_I_KZ)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH1_I_KZ & 0x00ff);
	
	// WORD 28 MODBUS_SOLBAT_CH1_I_MPP
	pInvState[(2*MODBUS_SOLBAT_CH1_I_MPP)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH1_I_MPP & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH1_I_MPP)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH1_I_MPP & 0x00ff);
	
	// WORD 29 MODBUS_SOLBAT_CH1_MODE
	pInvState[(2*MODBUS_SOLBAT_CH1_MODE)] = (Uint8)((Params.ChargeControl.xBattery.eStage & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH1_MODE)+1] = (Uint8)(Params.ChargeControl.xBattery.eStage & 0x00ff);
	
	// WORD 30 MODBUS_REG_DEBUG_TEMP0
	pInvState[(2*MODBUS_REG_DEBUG_TEMP0)] = (Uint8)((Params.Page0.RegTemp0 & 0xff00) >> 8);
	pInvState[(2*MODBUS_REG_DEBUG_TEMP0)+1] = (Uint8)(Params.Page0.RegTemp0 & 0x00ff);
	
	// WORD 31 MODBUS_REG_DEBUG_TEMP1
	pInvState[(2*MODBUS_REG_DEBUG_TEMP1)] = (Uint8)((Params.ChargeControl.pxSources[CH1]->usPwm & 0xff00) >> 8);
	pInvState[(2*MODBUS_REG_DEBUG_TEMP1)+1] = (Uint8)(Params.ChargeControl.pxSources[CH1]->usPwm & 0x00ff);
	
	// WORD 32 MODBUS_REG_DEBUG_TEMP2
	pInvState[(2*MODBUS_REG_DEBUG_TEMP2)] = (Uint8)((Params.ChargeControl.pxSources[CH2]->usPwm & 0xff00) >> 8);
	pInvState[(2*MODBUS_REG_DEBUG_TEMP2)+1] = (Uint8)(Params.ChargeControl.pxSources[CH2]->usPwm & 0x00ff);
	
	// WORD 33 MODBUS_INVERTER_UOUT_SET
	pInvState[(2*MODBUS_INVERTER_UOUT_SET)] = (Uint8)((Params.Page0.Reg_INVERTER_UOUT_SET & 0xff00) >> 8);
	pInvState[(2*MODBUS_INVERTER_UOUT_SET)+1] = (Uint8)(Params.Page0.Reg_INVERTER_UOUT_SET & 0x00ff);
	
	// WORD 34 MODBUS_REG_DEBUG_TEMP3
	//pInvState[(2*MODBUS_REG_DEBUG_TEMP3)] = (Uint8)((Params.Page0.RegTemp3 & 0xff00) >> 8);
	//pInvState[(2*MODBUS_REG_DEBUG_TEMP3)+1] = (Uint8)(Params.Page0.RegTemp3 & 0x00ff);
	pInvState[(2*MODBUS_REG_DEBUG_TEMP3)] = (Uint8)((Params.ChargeControl.xSource1.eStatus & 0xff00) >> 8);
	pInvState[(2*MODBUS_REG_DEBUG_TEMP3)+1] = (Uint8)(Params.ChargeControl.xSource1.eStatus & 0x00ff);
	
	// WORD 35 MODBUS_RELAY_P2
	pInvState[(2*MODBUS_RELAY_P2)] = (Uint8)((Params.Page0.Reg_RELAY_P2 & 0xff00) >> 8);
	pInvState[(2*MODBUS_RELAY_P2)+1] = (Uint8)(Params.Page0.Reg_RELAY_P2 & 0x00ff);
	
	// WORD 36 MODBUS_RELAY_P3
	pInvState[(2*MODBUS_RELAY_P3)] = (Uint8)((Params.Page0.Reg_RELAY_P3 & 0xff00) >> 8);
	pInvState[(2*MODBUS_RELAY_P3)+1] = (Uint8)(Params.Page0.Reg_RELAY_P3 & 0x00ff);
	
	
	//
	
	// WORD 37 MODBUS_SOLBAT_CH2_U_MPP
	pInvState[(2*MODBUS_SOLBAT_CH2_U_MPP)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH2_U_MPP & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH2_U_MPP)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH2_U_MPP & 0x00ff);
	
	// WORD 38 MODBUS_SOLBAT_CH2_U_XX
	pInvState[(2*MODBUS_SOLBAT_CH2_U_XX)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH2_U_XX & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH2_U_XX)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH2_U_XX & 0x00ff);
	
	// WORD 39 MODBUS_SOLBAT_CH2_NUM_SERIAL
	pInvState[(2*MODBUS_SOLBAT_CH2_NUM_SERIAL)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH2_NUM_SERIAL & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH2_NUM_SERIAL)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH2_NUM_SERIAL & 0x00ff);
	
	// WORD 40 MODBUS_SOLBAT_CH2_NUM_PARAL
	pInvState[(2*MODBUS_SOLBAT_CH2_NUM_PARAL)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH2_NUM_PARAL & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH2_NUM_PARAL)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH2_NUM_PARAL & 0x00ff);
	
	// WORD 41 MODBUS_SOLBAT_CH2_I_KZ
	pInvState[(2*MODBUS_SOLBAT_CH2_I_KZ)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH2_I_KZ & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH2_I_KZ)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH2_I_KZ & 0x00ff);
	
	// WORD 42 MODBUS_SOLBAT_CH2_I_MPP
	pInvState[(2*MODBUS_SOLBAT_CH2_I_MPP)] = (Uint8)((Params.Page0.Reg_SOLBAT_CH2_I_MPP & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH2_I_MPP)+1] = (Uint8)(Params.Page0.Reg_SOLBAT_CH2_I_MPP & 0x00ff);
	
	// WORD 43 MODBUS_SOLBAT_CH2_MODE
	pInvState[(2*MODBUS_SOLBAT_CH2_MODE)] = (Uint8)((Params.ChargeControl.xBattery.eStage & 0xff00) >> 8);
	pInvState[(2*MODBUS_SOLBAT_CH2_MODE)+1] = (Uint8)(Params.ChargeControl.xBattery.eStage & 0x00ff);
	
	/*	Заголовок сообщения	*/
	p_txFrame[0] = slave->ID;
	p_txFrame[1] = slave->FuncCode;
	p_txFrame[2] = 2*regNumber;
	
	/*	Заполнение байт данных	*/
	for (t = 2*regStartPos; t <= (2*regStartPos + 2*regNumber); t += 2)
	{
		p_txFrame[t - 2*regStartPos+3] = pInvState[t];
		p_txFrame[t + 1 - 2*regStartPos+3] = pInvState[t+1];
	}
	
	/*	CRC в конец	*/
	u16CRC = SciSlave_CRC16(p_txFrame, 2*regNumber + 3);
	p_txFrame[2*regNumber+3] = (Uint8)(u16CRC & 0x00ff);
	p_txFrame[2*regNumber+4] = (Uint8)((u16CRC & 0xff00)>>8);
	p_txFrame[2*regNumber+5] = 0;
	
	/*	Инициализация отправки	*/
	SciSlave_InitTransmission(slave, p_txFrame, (2*regNumber + 5));
	
	free(pInvState);
	free(p_txFrame);
	
	return ERR_OK;
}


Error_t
SciSlave_F06(SciSlave_t* slave)
{
	Uint16 u16CRC;
	Uint16 regStartPos, regNumber;
	Uint16 regPage;
	Uint8* p_txFrame = NULL;
	Uint16 recword = 0;
	
	
	
	/* И еще чото */
	u16CRC = SciSlave_CRC16(slave->Buf2, slave->BytesNumber - 2);
	if (u16CRC != ((slave->Buf2[7]<<8) | slave->Buf2[6]))
	{
		return ERR_CRC1;
	}
	
	
	
	/*	Проверка адреса начального регистра	*/
	regPage = slave->Buf2[2];
	
	if (regPage > MODBUS_PAGE_COUNT-1)
	{
		return ERR_CRC2;
	}
	
	/*	Проверка адреса начального регистра	*/
	regStartPos = slave->Buf2[3];
	
	if (regStartPos >= MODBUS_FUNC06_ADDRLEN)
	{
		return ERR_CRC2;
	}
	
	
	
	recword = ((slave->Buf2[4]) << 8) | (slave->Buf2[5]);
	
	if (slave->Name == SCI_PC)
	{
		if (regStartPos == 2)
		{
			PControl.pwBridge = _IQ(recword);
		}
	}
	else if (slave->Name == SCI_PULT)
	{
		
		
		Params_WriteFromSerial(regPage, regStartPos, 1, &recword);
	}
	
	
	
	/* И еще чото */
	//pPoolRegs = malloc(MODBUS_FUNC06_REQBYTENUM*sizeof(Uint16));
	//memset(pPoolRegs, 0, (MODBUS_FUNC06_REQBYTENUM)*sizeof(Uint16));
	
	/* И еще чото */
	p_txFrame = malloc(8*sizeof(Uint8));
	memset(p_txFrame, 0, 8*sizeof(Uint8));
		
	
	/*	Заголовок сообщения	*/
	p_txFrame[0] = slave->ID;
	p_txFrame[1] = slave->FuncCode;
	p_txFrame[2] = slave->Buf2[2];
	p_txFrame[3] = slave->Buf2[3];
	p_txFrame[4] = slave->Buf2[4];
	p_txFrame[5] = slave->Buf2[5];
	p_txFrame[6] = slave->Buf2[6];
	p_txFrame[7] = slave->Buf2[7];
	
	/*	CRC в конец	*/
	//u16CRC = SciSlave_CRC16(p_txFrame, 2*regNumber + 3);
	//p_txFrame[6] = (Uint8)(u16CRC & 0x00ff);
	//p_txFrame[7] = (Uint8)((u16CRC & 0xff00)>>8);
	//p_txFrame[2*regNumber+5] = 0;
	
	/*	Инициализация отправки	*/
	SciSlave_InitTransmission(slave, p_txFrame, 8);
	
	//free(pPoolRegs);
	free(p_txFrame);
	
	return ERR_OK;
}


Error_t
SciSlave_FunctionX10(SciSlave_t* slave)
{
	Uint16 u16CRC;
	Uint16 regStartPos, regNumber, regPage;
	Uint16 k;
	Uint8* p_txFrame = NULL;
	Uint16* pPoolRegs = NULL;
	
	/* И еще чото */
	u16CRC = SciSlave_CRC16(slave->Buf2, slave->BytesNumber - 2);
	if (u16CRC != ((slave->Buf2[slave->BytesNumber - 2]) | (slave->Buf2[slave->BytesNumber - 1]<<8)))
	{
		return ERR_CRC1;
	}
	
	/*	Проверка адреса начального регистра	*/
	regPage = slave->Buf2[2];
	
	if (regPage > MODBUS_PAGE_COUNT-1)
	{
		return ERR_CRC2;
	}
	
	/*	Проверка адреса начального регистра	*/
	regStartPos = slave->Buf2[3];
	
	if (regStartPos >= POOL_REGS_MAX_VALUE)
	{
		return ERR_CRC2;
	}
	
	/*	Проверка числа запрашиваемых регистров	*/
	regNumber = (slave->Buf2[4] << 8) | (slave->Buf2[5]);
	
	if	( (regNumber > POOL_REGS_MAX_VALUE) |
		  (regNumber == 0) |
		  ((regStartPos+regNumber) > POOL_REGS_MAX_VALUE) )
	{
		return ERR_CRC3;
	}
	
	/* И еще чото */
	if ( (2*regNumber != slave->Buf2[6]) |
		 ((slave->BytesNumber-9) != slave->Buf2[6]) )
	{
		return ERR_CRC3;
	}
	
	/* И еще чото */
	pPoolRegs = malloc(POOL_REGS_MAX_VALUE*sizeof(Uint16));
	memset(pPoolRegs, 0, (POOL_REGS_MAX_VALUE)*sizeof(Uint16));
	
	/* И еще чото */
	p_txFrame = malloc(8*sizeof(Uint8));
	memset(p_txFrame, 0, 8*sizeof(Uint8));
	
	/* И еще чото */
	for (k=regStartPos; k<(regStartPos + regNumber); k++)
	{
		*(pPoolRegs + k) = (slave->Buf2[7 + 2*(k - regStartPos)] << 8) | slave->Buf2[7 + 1 + 2*(k - regStartPos)];
	}
	
	/*
	Page0.DateTime.Day = pPoolRegs[0] & 0xff;
	Page0.DateTime.Month = pPoolRegs[0] >> 8;
	Page0.DateTime.Year = pPoolRegs[1] & 0xff;
	*/
	
	if (slave->Name == SCI_PULT)
	{
		Params_WriteFromSerial(regPage, regStartPos, regNumber, pPoolRegs);
	}
	else if (slave->Name == SCI_PC)
	{
		/*
		if (pPoolRegs[0] & (1<<0)){
			if (crashNotice.Bridge == false){
				PControl.fBridge = true;
				PControl.pwBridge = _IQ(pPoolRegs[3]);
			}
		}
		else{
			PControl.fBridge = false;
			crashNotice.Bridge = false;
		}
		*/
	}
	
	
	
	
	/*	Заголовок сообщения	*/
	p_txFrame[0] = slave->ID;
	p_txFrame[1] = slave->FuncCode;
	p_txFrame[2] = slave->Buf2[2];
	p_txFrame[2] = slave->Buf2[3];
	p_txFrame[2] = slave->Buf2[4];
	p_txFrame[2] = slave->Buf2[5];
	
	/*	CRC в конец	*/
	u16CRC = SciSlave_CRC16(p_txFrame, 2*regNumber + 3);
	p_txFrame[2*regNumber+3] = (Uint8)(u16CRC & 0x00ff);
	p_txFrame[2*regNumber+4] = (Uint8)((u16CRC & 0xff00)>>8);
	//p_txFrame[2*regNumber+5] = 0;
	
	/*	Инициализация отправки	*/
	SciSlave_InitTransmission(slave, p_txFrame, 8);
	
	free(pPoolRegs);
	free(p_txFrame);
	
	return ERR_OK;
}

void
SciSlave_InitTransmission(SciSlave_t* slave, Uint8* pBuf, Uint8 len)
{
	SciSlave_SwitchTransceiver(slave, MODETRANSCEIVER_TX);
	
	/*  */
	//if (strlen((const char*)pBuf) >= SLAVE_TRANSMIT_BUFFER_LENGHT)
	//	return;
	
	/* НАДО ДОБАВЛЯТЬ НУЛЬ В КОНЕЦ pBuf */
	memcpy((char*)slave->DataTransmit, (const char*)pBuf, len);
	
	/* */
	slave->DataTransmitLenght = len;
	
	/*	Включаем прерывание по TX */
	SciSlave_EnableTransmitInterrupt(slave);
	
	/* 	Ждем флаг готовности о передаче	*/
	while 
	( 
		!(slave->ptSciRegs->SCICTL2.bit.TXRDY) & 
		!(slave->ptSciRegs->SCICTL2.bit.TXEMPTY) 
	)
	{
		asm("	NOP");
	}
	
	/*	Отправка бервого байта. Обработка каждого посл. байта ведется в прерывании  
		SciSlave_Transmit(SciSlave_t* slave)	*/
	slave->ptSciRegs->SCITXBUF = slave->DataTransmit[0];
}

void
SciSlave_Default(SciSlave_t* var)
{
	var->BytesNumber = BYTESNUMBER_UNK;
	var->Mode = MODE_NEWFRAME;
	var->BytesCounter = 0;
	//var->ByteCntTx = 0;
	var->FuncCode = 0;
	var->LookForTxEmpty = false;
	var->ModeTransceiver = MODETRANSCEIVER_RX;
	memset(var->DataReceive, 0, SLAVE_RECEIVE_BUFFER_LENGHT*sizeof(Uint8));
	memset(var->Buf2, 0, SLAVE_RECEIVE_BUFFER_LENGHT*sizeof(Uint8));
	//memset(var->DataTransmit, 0, SLAVE_TRANSMIT_BUFFER_LENGHT*sizeof(Uint8));
}

Error_t
SciSlave_Link(SciSlave_t* slave, volatile struct SCI_REGS* sciregs, struct CPUTIMER_VARS* timer)
{
	slave->ptSciRegs = sciregs;
	slave->ptCpuTimer = timer;
	
	return ERR_OK;
}

void
SciSlave_UnLink(SciSlave_t* slave)
{
	slave->ptSciRegs = NULL;
	slave->ptCpuTimer = NULL;
}

void
SciSlave_DisableTransmitInterrupt(SciSlave_t* slave)
{
	slave->ptSciRegs->SCICTL2.bit.TXINTENA = 0;
}

void
SciSlave_EnableTransmitInterrupt(SciSlave_t* slave)
{
	slave->ptSciRegs->SCICTL1.bit.SWRESET = 0;
	slave->ptSciRegs->SCICTL1.bit.SWRESET = 1;
//	slave->ptSciRegs->SCICTL1.bit.TXENA = 1;
	while (!slave->ptSciRegs->SCICTL1.bit.TXENA)
	{
		asm("	NOP");
	}
	slave->ptSciRegs->SCICTL2.bit.TXINTENA = 1;
}

void
SciSlave_SwitchTransceiver(SciSlave_t* slave, ModeTransceiver_t mode)
{
	Uint32 tmp = 1;
	Uint32 tmp2 = 1;
	Uint16 i;
	
	if (slave->GPIOBit > 0)
	{
		for (i = 0; i < slave->GPIOBit; i++)
		{
			tmp *= 2;
		}
	}
	
	// КОСЯК ДЛЯ ПЕРЕДАЧИ НЕ НАДО ИНВЕРТИРОВАТЬ!!
	
	
	switch (mode)
	{
		case MODETRANSCEIVER_RX:
			slave->ModeTransceiver = MODETRANSCEIVER_RX;
			//GpioDataRegs.GPACLEAR.bit.GPIO25 = 1;		//Receive
			//*(slave->pGPIOPort) &= ~(1 << slave->GPIOBit);
			//Toggle(&(GpioDataRegs.GPADAT.all));
			//tmp = (1<<15);
			//tmp2 = ~tmp;
			tmp2 = ~tmp;
			*(slave->pGPIOPort) &= tmp2;
			break;
		case MODETRANSCEIVER_TX:
			slave->ModeTransceiver = MODETRANSCEIVER_TX;
			//GpioDataRegs.GPASET.bit.GPIO25 = 1;			//Transmit
			//*(slave->pGPIOPort) |= (1 << slave->GPIOBit);
			//Toggle(&(GpioDataRegs.GPADAT.all));
			tmp2 = tmp;
			*(slave->pGPIOPort) |= tmp2;
			break;
		default:
			break;
	}
}

void
SciSlave_TimerSetPeriod( struct CPUTIMER_VARS *Timer, Uint16 Freq, Uint16 Period )
{
	Timer->CPUFreqInMHz = Freq;
    Timer->PeriodInUSec = Period;
	Timer->RegsAddr->PRD.all = (long)(_IQ13int(_IQ13mpy(_IQ13(Freq), _IQ13(Period))));
}

Error_t 
SciSlave_TimerInit( struct CPUTIMER_VARS *Timer )
{
	if (Timer == NULL)
		return ERR_NULLREF;
	
    // Set pre-scale counter to divide by 1 (SYSCLKOUT):
    Timer->RegsAddr->TPR.all  = 0;
    Timer->RegsAddr->TPRH.all  = 0;

    // Initialize timer control register:
	Timer->RegsAddr->TCR.bit.TSS = 1;		// 1 = Stop timer, 0 = Start/Restart Timer
    Timer->RegsAddr->TCR.bit.TRB = 1;      // 1 = reload timer
    Timer->RegsAddr->TCR.bit.SOFT = 1;
    Timer->RegsAddr->TCR.bit.FREE = 1;     // Timer Free Run
    Timer->RegsAddr->TCR.bit.TIE = 1;      // 0 = Disable/ 1 = Enable Timer Interrupt

    Timer->InterruptCount = 0;
	
	return ERR_OK;
}

Uint16
SciSlave_CRC16(const Uint8 *nData, Uint16 wLength)
{
    Uint8 nTemp;
    Uint16 wCRCWord = 0xFFFF;
	static const Uint16 wCRCTable[] = 
	{
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
	};

    while (wLength--)
    {
        nTemp = *nData++ ^ wCRCWord;
        wCRCWord >>= 8;
        wCRCWord  ^= wCRCTable[(nTemp & 0xFF)];
    }
    return wCRCWord;
}

