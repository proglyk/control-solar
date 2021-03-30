#include "ParamsTable.h"
#include "MemoryEEPROM.h"
#include "RegulatorPI.h"
#include "string.h"


//ParamsTable_t Params = {{0,0,0,{0,0,0,0,0,0}}, {0,0,0}, {false}};

//ParamsTable_t Params = {0};
extern const I2C_Device_t I2C_EepromAT24C512;
extern const I2C_Device_t I2C_RtcDS3231;
extern RegulatorPi_t* pPiVout;
extern RegulatorPi_t* pPiMppt1;

//extern RegulatorPi_t* pPiUBat1;
//extern RegulatorPi_t* pPiUMppt1;
//extern RegulatorPi_t* pPiUMppt2;
//extern RegulatorPi_t* pPiUBat2;

extern RegulatorPi_t stPI_UMPPT1;
extern RegulatorPi_t stPI_UBAT1;
extern RegulatorPi_t stPI_UMPPT2;
extern RegulatorPi_t stPI_UBAT2;

PControl_t PControl = {true,true,true,true,true,true,_IQ(0),_IQ(0),_IQ(10), 0};
CrashNotice_t Fault = {0};

ParamsTable_t Params;


void
	Params__ChargeControlInit(ChargeControl * pxStruct, ParamsTable_t * pxParams){
		
	// 0. Зануляем
	memset((void*)pxParams, 0, sizeof(ParamsTable_t));
	
	// 1. Заносим нужные начальные значения регистров обмена с пультом
	pxParams->Page0.Reg_SOLBAT_CH1_U_MPP = 1250;
	pxParams->Page0.Reg_SOLBAT_CH1_U_XX = 525;
	pxParams->Page0.Reg_SOLBAT_CH1_I_KZ = 89;
	pxParams->Page0.Reg_SOLBAT_CH1_I_MPP = 86;
	
	pxParams->Page0.Reg_SOLBAT_CH2_U_MPP = 1250;
	pxParams->Page0.Reg_SOLBAT_CH2_U_XX = 525;
	pxParams->Page0.Reg_SOLBAT_CH2_I_KZ = 89;
	pxParams->Page0.Reg_SOLBAT_CH2_I_MPP = 86;
	
	pxParams->Page0.Reg_INVERTER_UOUT_SET = 220;
	
	// 2. Заполняем
	pxStruct->pxSources[0] = &(pxStruct->xSource1);
	pxStruct->pxSources[1] = &(pxStruct->xSource2);	
	
	// 3. Начальные значения
	pxStruct->pTrig = (Uint16(*)(Uint16)) Latch;
	pxStruct->VLoad = (VLoadType_t) V_LOAD_220;
	
	// 4. Батарея - общая для обоих каналов
	pxStruct->xBattery.eStage = CHARGESTAGE_MPPT;
	//pxStruct->xBattery.stUBatSet.value = _IQ15(VBAT); //временно UNUSED
	pxStruct->xBattery.stUBatSet.threshold = _IQ15(7);
	pxStruct->xBattery.stUBatSet.type = SCHMITT_MINUS;
	pxStruct->xBattery.stUBatSet.stored = false;
	pxStruct->xBattery.psSTBN = &(pxParams->Page0.RegSTBN);
	// уставка должна быть только в одном экземпляре, т.к. и АКБ только один!
	// сейчас Reg_SOLBAT_CH1_U_XX используется и для 1-ого и для 2-ого каналов
	pxStruct->xBattery.psVBAT = &(pxParams->Page0.Reg_SOLBAT_CH1_U_XX);
	
	// 5. Канал MPPT1
	pxStruct->pxSources[CH1]->pbEnable = &(pxParams->Page0.fMPPT1);
	pxStruct->pxSources[CH1]->eStatus = MPP_DIS;
	
	pxStruct->pxSources[CH1]->usPwm = 0;
	
	pxStruct->pxSources[CH1]->pxMppPI = &(stPI_UMPPT1);
	pxStruct->pxSources[CH1]->pxUbatPI = &(stPI_UBAT1);
	pxStruct->pxSources[CH1]->iqMutualInt = 0;
	
	pxStruct->pxSources[CH1]->psIOxN = &(pxParams->Page0.RegIO1N);
	pxStruct->pxSources[CH1]->psSBxN = &(pxParams->Page0.RegSB1N);
	pxStruct->pxSources[CH1]->psIMPP = &(pxParams->Page0.Reg_SOLBAT_CH1_I_MPP);
	pxStruct->pxSources[CH1]->psUMPP = &(pxParams->Page0.Reg_SOLBAT_CH1_U_MPP);
	pxStruct->pxSources[CH1]->psIKZ = &(pxParams->Page0.Reg_SOLBAT_CH1_I_KZ);
	
	// 6. Канал MPPT2
	pxStruct->pxSources[CH2]->pbEnable = &(pxParams->Page0.fMPPT2);
	pxStruct->pxSources[CH2]->eStatus = MPP_DIS;
	
	pxStruct->pxSources[CH2]->usPwm = 0;
	
	// разобраться stPI_UBAT1 использовать для обоих каналов, или по отдельности?
	pxStruct->pxSources[CH2]->pxMppPI = &(stPI_UMPPT2);
	pxStruct->pxSources[CH2]->pxUbatPI = &(stPI_UBAT2);
	pxStruct->pxSources[CH2]->iqMutualInt = 0;
	
	pxStruct->pxSources[CH2]->psIOxN = &(pxParams->Page0.RegIO2N);
	pxStruct->pxSources[CH2]->psSBxN = &(pxParams->Page0.RegSB2N);
	pxStruct->pxSources[CH2]->psIMPP = &(pxParams->Page0.Reg_SOLBAT_CH2_I_MPP);
	pxStruct->pxSources[CH2]->psUMPP = &(pxParams->Page0.Reg_SOLBAT_CH2_U_MPP);
	pxStruct->pxSources[CH2]->psIKZ = &(pxParams->Page0.Reg_SOLBAT_CH2_I_KZ);
}

/*
void
	Params__Init(ParamsTable_t * pxParams) {
	
	// 1. Зануляем
	memset((void*)pxParams, 0, sizeof(ParamsTable_t));
	
	//pxParams->Control.pxSources[0] = &(pxParams->Control.Mppt_CH1);
	//pxParams->Control.pxSources[1] = &(pxParams->Control.Mppt_CH2);
	
	// 2. Заносим нужные начальные значения регистров обмена с пультом
	pxParams->Page0.Reg_SOLBAT_CH1_U_MPP = 1250;
	pxParams->Page0.Reg_SOLBAT_CH1_U_XX = 525;
	pxParams->Page0.Reg_SOLBAT_CH1_I_KZ = 89;
	pxParams->Page0.Reg_SOLBAT_CH1_I_MPP = 86;
	
	pxParams->Page0.Reg_SOLBAT_CH2_U_MPP = 1250;
	pxParams->Page0.Reg_SOLBAT_CH2_U_XX = 525;
	pxParams->Page0.Reg_SOLBAT_CH2_I_KZ = 89;
	pxParams->Page0.Reg_SOLBAT_CH2_I_MPP = 86;
	
	pxParams->Page0.Reg_INVERTER_UOUT_SET = 220;
	
	
	// 3. Начальные значения
	pxParams->Control.pTrig = (Uint16(*)(Uint16)) Latch;
	pxParams->Control.VLoad = (VLoadType_t) V_LOAD_220;
	
	
	// Канал MPPT1
	// 4. Начальные значения и адреса полей, контролирующих заряд через MPPT1
	pxParams->Control.Mppt_CH1.iqVbatNominal = _IQ15(VBAT);
	pxParams->Control.Mppt_CH1.iqIbatNominal = _IQ15(10);	
	pxParams->Control.Mppt_CH1.Stage = MPPT_CHARGESTAGE_MPPT;
	
	pxParams->Control.Mppt_CH1.stUBatSet.value = _IQ15(VBAT);	// фактически не исп
	pxParams->Control.Mppt_CH1.stUBatSet.threshold = _IQ15(7);
	pxParams->Control.Mppt_CH1.stUBatSet.type = SCHMITT_MINUS;
	pxParams->Control.Mppt_CH1.stUBatSet.stored = false;
	
	pxParams->Control.Mppt_CH1.qMainIntegrator = 0;
	pxParams->Control.Mppt_CH1.StageMPP = MPP_DIS;
	
	pxParams->Control.Mppt_CH1.p_channel = &(pxParams->Page0.fMPPT1);
	pxParams->Control.Mppt_CH1.p_pi_mpp = &(stPI_UMPPT1);
	pxParams->Control.Mppt_CH1.p_pi_u = &(stPI_UBAT1);
	pxParams->Control.Mppt_CH1.p_reg_pwm = &(pxParams->Page0.RegTemp1);
	pxParams->Control.Mppt_CH1.psSTBN = &(pxParams->Page0.RegSTBN);
	pxParams->Control.Mppt_CH1.psIOxN = &(pxParams->Page0.RegIO1N);
	pxParams->Control.Mppt_CH1.psSBxN = &(pxParams->Page0.RegSB1N);
	pxParams->Control.Mppt_CH1.p_reg_SOLBAT_I_MPP = &(pxParams->Page0.
		Reg_SOLBAT_CH1_I_MPP);
	pxParams->Control.Mppt_CH1.p_reg_SOLBAT_U_MPP = &(pxParams->Page0.
		Reg_SOLBAT_CH1_U_MPP);
	pxParams->Control.Mppt_CH1.p_reg_SOLBAT_I_KZ = &(pxParams->Page0.
		Reg_SOLBAT_CH1_I_KZ);
	pxParams->Control.Mppt_CH1.psVBAT = &(pxParams->Page0.
		Reg_SOLBAT_CH1_U_XX);
	
	
	// Канал MPPT2
	// 4. Начальные значения и адреса полей, контролирующих заряд через MPPT1
	pxParams->Control.Mppt_CH2.iqVbatNominal = _IQ15(VBAT);
	pxParams->Control.Mppt_CH2.iqIbatNominal = _IQ15(10);	
	pxParams->Control.Mppt_CH2.Stage = MPPT_CHARGESTAGE_MPPT;
	
	pxParams->Control.Mppt_CH2.stUBatSet.value = _IQ15(VBAT);	// фактически не исп
	pxParams->Control.Mppt_CH2.stUBatSet.threshold = _IQ15(7);
	pxParams->Control.Mppt_CH2.stUBatSet.type = SCHMITT_MINUS;
	pxParams->Control.Mppt_CH2.stUBatSet.stored = false;
	
	pxParams->Control.Mppt_CH2.qMainIntegrator = 0;
	pxParams->Control.Mppt_CH2.StageMPP = MPP_DIS;
	
	pxParams->Control.Mppt_CH2.p_channel = &(pxParams->Page0.fMPPT2);
	pxParams->Control.Mppt_CH2.p_pi_mpp = &(stPI_UMPPT2);
// разобраться stPI_UBAT1 использовать для обоих каналов, или по отдельности?
	pxParams->Control.Mppt_CH2.p_pi_u = &(stPI_UBAT2);
	pxParams->Control.Mppt_CH2.p_reg_pwm = &(pxParams->Page0.RegTemp2);
	pxParams->Control.Mppt_CH2.psSTBN = &(pxParams->Page0.RegSTBN);
	pxParams->Control.Mppt_CH2.psIOxN = &(pxParams->Page0.RegIO2N);
	pxParams->Control.Mppt_CH2.psSBxN = &(pxParams->Page0.RegSB2N);
	pxParams->Control.Mppt_CH2.p_reg_SOLBAT_I_MPP = &(pxParams->Page0.
		Reg_SOLBAT_CH2_I_MPP);
	pxParams->Control.Mppt_CH2.p_reg_SOLBAT_U_MPP = &(pxParams->Page0.
		Reg_SOLBAT_CH2_U_MPP);
	pxParams->Control.Mppt_CH2.p_reg_SOLBAT_I_KZ = &(pxParams->Page0.
		Reg_SOLBAT_CH2_I_KZ);
	// уставка должна быть только в одном экземпляре, т.к. и АКБ только один!
	// сейчас Reg_SOLBAT_CH1_U_XX используется и для 1-ого и для 2-ого каналов
	pxParams->Control.Mppt_CH2.psVBAT = pxParams->Control.Mppt_CH1.psVBAT;
	
	
	// Вызов новой процедуры
	Params__ChargeControlInit(&(pxParams->ChargeControl), pxParams);
}*/


Uint16
Latch(Uint16 action)
{
	static Uint16 val = 0;
	
	if (action == 0)
	{
		
	}
	else if (action == 1)
	{
		
	}
	
	return val;
}

void
Params_WriteFromSerial(Uint16 page, Uint16 startpos, Uint16 len, Uint16 * pdata)
{
	Uint16 i;
	
	
	
	for (i=startpos; i<(startpos+len); i++)
	{
		switch (page)
		{
			case (PAGE_0):
			{
				
				
				switch (i)
				{
					case (MODBUS_REG_DATE1):
						//Params.Page0.DateTime.Day = pdata[0] & 0xff;
						//Params.Page0.DateTime.Month = pdata[0] >> 8;
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(REG_DATE1), pdata[0]);
						Q_Store3(&I2C_RtcDS3231, DS3231_MEMADDR_DAY, DataConvert_DS3231_Set(pdata[0] & 0xff, BCDF_6BIT));
						Q_Store3(&I2C_RtcDS3231, DS3231_MEMADDR_MONTH, DataConvert_DS3231_Set(pdata[0] >> 8, BCDF_5BIT));
						break;
						
					case (MODBUS_REG_DATE2):
						//Params.Page0.DateTime.Year = pdata[1] & 0xff;
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(REG_DATE2), pdata[1]);
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(REG_DATE2), pdata[1]);
						Q_Store3(&I2C_RtcDS3231, DS3231_MEMADDR_YEAR, DataConvert_DS3231_Set(pdata[1] & 0xff, BCDF_8BIT));
						break;
						
					case (MODBUS_REG_TIME1):
						Q_Store3(&I2C_RtcDS3231, DS3231_MEMADDR_SEC, DataConvert_DS3231_Set(pdata[2] & 0xff, BCDF_7BIT));
						Q_Store3(&I2C_RtcDS3231, DS3231_MEMADDR_MIN, DataConvert_DS3231_Set(pdata[2] >> 8, BCDF_7BIT));
						break;
						
					case (MODBUS_REG_TIME2):
						Q_Store3(&I2C_RtcDS3231, DS3231_MEMADDR_HOUR, DataConvert_DS3231_Set(pdata[3] & 0xff, BCDF_5BIT));
						break;
						
					case (MODBUS_REG_0):
						Params.Page0.Reg0 = pdata[0];
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(MODBUS_REG_0), pdata[0]);
						break;
					
					case (MODBUS_REG_1):
						Params.Page0.Reg1 = pdata[0];
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(MODBUS_REG_1), pdata[0]);
						break;
					
					case (MODBUS_REG_2):
						Params.Page0.Reg2 = pdata[0];
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(MODBUS_REG_2), pdata[0]);
						break;
						
					case (MODBUS_REG_INVERTER):
						Params.Page0.fBridge = (bool)pdata[0];
						//Params.Page0.Reg2 = pdata[0];
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(REG_2), pdata[0]);
						break;
					case (MODBUS_REG_MPPT1):
						Params.Page0.fMPPT1 = (bool)pdata[0];
						//Params.Page0.Reg2 = pdata[0];
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(REG_2), pdata[0]);
						break;
					case (MODBUS_REG_MPPT2):
						Params.Page0.fMPPT2 = (bool)pdata[0];
						//Params.Page0.Reg2 = pdata[0];
						//Q_Store3(&I2C_EepromAT24C512, MEMADDR(REG_2), pdata[0]);
						break;
						
					case (STARTSTOP):
						switch (pdata[0])
						{
							case (1):
								Params.Page0.Mode = RUN;
								break;
							case (2):
								Params.Page0.Mode = STOP;
								break;
							default:
							break;
						}
						break;
						
						
					case (MODBUS_SOLBAT_CH1_U_MPP):
					
						Params.Page0.Reg_SOLBAT_CH1_U_MPP = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH1_U_XX):
						Params.Page0.Reg_SOLBAT_CH1_U_XX = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH1_I_MPP):
						Params.Page0.Reg_SOLBAT_CH1_I_MPP = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH1_I_KZ):
						Params.Page0.Reg_SOLBAT_CH1_I_KZ = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH1_NUM_SERIAL):
						Params.Page0.Reg_SOLBAT_CH1_NUM_SERIAL = (bool)pdata[0];
						break;
					case (MODBUS_INVERTER_UOUT_SET):
						Params.Page0.Reg_INVERTER_UOUT_SET = (bool)pdata[0];
						break;
					case (MODBUS_RELAY_P2):
						Params.Page0.Reg_RELAY_P2 = (bool)pdata[0];
						break;
					case (MODBUS_RELAY_P3):
						Params.Page0.Reg_RELAY_P3 = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH2_U_MPP):
						
						Params.Page0.Reg_SOLBAT_CH2_U_MPP = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH2_U_XX):
						Params.Page0.Reg_SOLBAT_CH2_U_XX = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH2_I_MPP):
						Params.Page0.Reg_SOLBAT_CH2_I_MPP = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH2_I_KZ):
						Params.Page0.Reg_SOLBAT_CH2_I_KZ = (bool)pdata[0];
						break;
					case (MODBUS_SOLBAT_CH2_NUM_SERIAL):
					
						Params.Page0.Reg_SOLBAT_CH2_NUM_SERIAL = (bool)pdata[0];
						break;
						
					default:
						break;
				}
				// case (PAGE_0) //
				break;
			}
				
			case (PAGE_1):
			{
				
				// case (PAGE_1) //
				break;
			}
			default:
				break;
		}
	}
}

