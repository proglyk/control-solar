#include "DSP280x_Device.h"     // DSP280x Headerfile Include File
#include "DSP280x_Examples.h"   // DSP280x Examples Include File
#include "DSP280x_SWPrioritizedIsrLevels.h" 
#include "sciv3.h"
#include "MemoryEEPROM.h"
#include "DataConvert.h"
#include "string.h"
#include "Epwm.h"
#include "Adc.h"
#include "Measurements.h"
#include "Inverter.h"
#include "ParamsTable.h"
#include "RegulatorPI.h"
#include "Mppt.h"
#include "faults.h"

SciSlave_t stSciSlave1;
SciSlave_t* ptSciSlave1 = &stSciSlave1;

SciSlave_t stSciSlave2;
SciSlave_t* ptSciSlave2 = &stSciSlave2;

extern RegulatorPi_t* pPiVout;
extern RegulatorPi_t* pPiUBat1;
extern RegulatorPi_t* pPiUMppt1;
extern ParamsTable_t Params;

extern RegulatorPi_t* pPiUMppt2;
extern RegulatorPi_t* pPiUBat2;

Uint16 tmp;

Error_t Err;
Uint16 Error;
extern struct I2CMSG * pMsgOut1;
extern struct I2CMSG * pMsgIn1;
extern struct I2CMSG * CurrentMsgPtr;

Uint16 u16Msg[20] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33};
Uint16 u16Msg128Write[128];
Uint16 u16Msg128Read[128];

extern Eeprom_t Memory[EEPROM_LENGHT];
extern Queue_t Q;

extern I2cMsgReceive_t* pMsg1;
extern I2C_Device_t I2C_EepromAT24C512;
extern I2C_Device_t I2C_RtcDS3231;
extern struct sFaults Faults; 

Qerror_t qerr;
void Charge(void);
void Relay(void);
void Charge_MPPT2(void);


Uint16 usCountLed = 0;

int
main(void)
{
	Uint16 i;
	Uint16 cnt1 = 0;
	
	
	InitSysCtrl();
	
	EALLOW;	
	GpioCtrlRegs.GPAPUD.bit.GPIO10 = 0;		//No pull-up
	GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 0;   	//Configure Pin as GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;		//Configure as Output
	GpioCtrlRegs.GPAPUD.bit.GPIO11 = 0;		//No pull-up
	GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 0;   	//Configure Pin as GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;		//Configure as Output
	GpioCtrlRegs.GPAPUD.bit.GPIO7 = 0;		//No pull-up
	GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 0;   	//Configure Pin as GPIO
	GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;		//Configure as Output
	
	GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;
	
	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // Enable pull-up for GPIO28
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = 0;	   // Enable pull-up for GPIO29
	GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // Asynch input GPIO28
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // Configure GPIO28 for SCIRXDA operation
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // Configure GPIO29 for SCITXDA operation
	
	GpioCtrlRegs.GPAPUD.bit.GPIO14 = 0;
	GpioCtrlRegs.GPAPUD.bit.GPIO15 = 0;
	GpioCtrlRegs.GPAQSEL1.bit.GPIO15 = 3;
	GpioCtrlRegs.GPAMUX1.bit.GPIO14 = 2;
	GpioCtrlRegs.GPAMUX1.bit.GPIO15 = 2;
	
	GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;	//REDE SCIA
	GpioCtrlRegs.GPADIR.bit.GPIO26 = 1;
	GpioCtrlRegs.GPAMUX2.bit.GPIO25 = 0;	//REDE SCIB
	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;
	
	GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 0;	//Настройка ног упр. РЕЛЕ на выход
	GpioCtrlRegs.GPADIR.bit.GPIO12 = 1;
	GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO21 = 1;
	
	GpioCtrlRegs.GPAMUX2.bit.GPIO23 = 0;	// D1_FLT
	GpioCtrlRegs.GPADIR.bit.GPIO23 = 0;
	GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 0;		// D2_FLT
	GpioCtrlRegs.GPADIR.bit.GPIO8 = 0;
	GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;		// D3_FLT
	GpioCtrlRegs.GPADIR.bit.GPIO9 = 0;
	GpioCtrlRegs.GPAMUX2.bit.GPIO22 = 0;	// D4_FLT
	GpioCtrlRegs.GPADIR.bit.GPIO22 = 0;
	GpioCtrlRegs.GPAMUX1.bit.GPIO13 = 0;	// D5_FLT
	GpioCtrlRegs.GPADIR.bit.GPIO13 = 0;
	GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 0;	// D6_FLT
	GpioCtrlRegs.GPADIR.bit.GPIO20 = 0;
	EDIS;
	
	
	
	memset(&Faults, 0, sizeof(struct sFaults));
	
	InitEPwm1Gpio();
	InitEPwm2Gpio();
	InitEPwm3Gpio();
	InitEPwm4Gpio();
	
	DINT;
	
	InitPieCtrl();
	
	IER = 0x0000;
	IFR = 0x0000; 
   
	InitPieVectTableSWPrio();
	
	MemCopy(&RamfuncsLoadStart, &RamfuncsLoadEnd, &RamfuncsRunStart);
	InitFlash();
	
	GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
	GpioDataRegs.GPACLEAR.bit.GPIO10 = 1;
	
	Params__ChargeControlInit(&(Params.ChargeControl), &Params);
	
	EPwm1Timer_Config();
	EPwm2Timer_Config();
	EPwm3Timer_Config();
	EPwm4Timer_Config();
	
//	InitI2CGpio();
	
//	MemEEPROM_Init();
	
	//ConvertInit();
	
	/*	Таймеры	*/
	InitCpuTimers();
	
	ConfigCpuTimer(&CpuTimer0, 99.5328, 1000);
	//ConfigCpuTimer(&CpuTimer1, 99.5328, 5000);
	//ConfigCpuTimer(&CpuTimer2, 99.5328, 11000);
	
	/*	Подключение тактирования к Timer 1	*/
	XIntruptRegs.XNMICR.bit.POLARITY = 1;	//RISING_EDGE
	XIntruptRegs.XNMICR.bit.SELECT = 0;		//TIMER1
	XIntruptRegs.XNMICR.bit.ENABLE = 0;		//DISABLE_XNMI_CTR
	
	
	SciSlave_Link(ptSciSlave1, &ScibRegs, &CpuTimer1);
	SciSlave_Setup(ptSciSlave1, 115200, 1, &(GpioDataRegs.GPADAT.all), 25);
	SciSlave_SwitchTransceiver(ptSciSlave1, MODETRANSCEIVER_RX);
	ptSciSlave1->Name = SCI_PC;
	
	
	SciSlave_Link(ptSciSlave2, &SciaRegs, &CpuTimer2);
	SciSlave_Setup(ptSciSlave2, 115200, 1, &(GpioDataRegs.GPADAT.all), 26);
	SciSlave_SwitchTransceiver(ptSciSlave2, MODETRANSCEIVER_RX);
	ptSciSlave2->Name = SCI_PULT;
	
	
	PieCtrlRegs.PIEIER1.all = 0;
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
	PieCtrlRegs.PIEIER1.bit.INTx6 = 1;	//ADCINT
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;	//TINT0
	PieCtrlRegs.PIEIER3.bit.INTx1 = 1;	//EPWM1_INT
	PieCtrlRegs.PIEIER3.bit.INTx3 = 1;	//EPWM3_INT
	PieCtrlRegs.PIEIER3.bit.INTx4 = 1;	//EPWM4_INT
//	PieCtrlRegs.PIEIER8.bit.INTx1 = 1;	//I2CINT1A
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;	//SCIRXINTA
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;	//SCITXINTA
	PieCtrlRegs.PIEIER9.bit.INTx3 = 1;	//SCIRXINTB
	PieCtrlRegs.PIEIER9.bit.INTx4 = 1;	//SCITXINTB
	
	PieCtrlRegs.PIEACK.all = 0xffff;
	
	ADC_Config_SOC();
	
//	IER = M_INT1 | M_INT3 | M_INT8 | M_INT9 | M_INT13 | M_INT14;
	IER = M_INT1 | M_INT3 | M_INT9 | M_INT13 | M_INT14;
	//IER = M_INT1 | M_INT3 | M_INT9 | M_INT14;
	//IER = M_INT1 | M_INT3;
	
	EINT;
	ERTM;
	
//	Memory_ReadNew(&I2C_EepromAT24C512, 0, 128);
	pMsg1->User.ReqToRead = 1;

	StartCpuTimer0();
	//StartCpuTimer1(); - SCIA включит сам, когда надо
	//StartCpuTimer2(); - SCIB включит сам, когда надо
	
	for (;;)
	{
		GpioDataRegs.GPATOGGLE.bit.GPIO11 = 1;
		
		if (ptSciSlave1->LookForTxEmpty == true)
		{
			i = ptSciSlave1->ptSciRegs->SCICTL2.all;
			if (i && ptSciSlave1->ptSciRegs->SCICTL2.bit.TXEMPTY)
			{
				cnt1++;
#if defined RT_MODE
				if (cnt1 > 30)
#elif defined DBG_MODE_DIV2
				if (cnt1 > 15)
#endif
				{
					SciSlave_SwitchTransceiver(ptSciSlave1, MODETRANSCEIVER_RX);
					ptSciSlave1->LookForTxEmpty = false;
					SciSlave_Default(ptSciSlave1);
					cnt1 = 0;
				}
			}
		}
		
		
		if (ptSciSlave2->LookForTxEmpty == true)
		{
			i = ptSciSlave2->ptSciRegs->SCICTL2.all;
			if (i && ptSciSlave2->ptSciRegs->SCICTL2.bit.TXEMPTY)
			{
				cnt1++;
#if defined RT_MODE
				if (cnt1 > 30)
#elif defined DBG_MODE_DIV2
				if (cnt1 > 15)
#endif
				{
					SciSlave_SwitchTransceiver(ptSciSlave2, MODETRANSCEIVER_RX);
					ptSciSlave2->LookForTxEmpty = false;
					SciSlave_Default(ptSciSlave2);
					cnt1 = 0;
				}
			}
		}
	}
}

void
DELAY_40US(void)
{
	Uint16 i;
	
	for (i=0; i < 100; i++)
	{
		asm("	NOP");
	}
}

void
DELAY_1MS(void)
{
	Uint16 i;
	
	for (i=0; i < 3500; i++)
	{
		asm("	NOP");
	}
}

void
Timer0Int(void)
{	
	static Uint16 cnt = 0;
	static int16 temp = 0;

/*----------------------------------------------------------------------------*/
	//GpioDataRegs.GPASET.bit.GPIO10 = 1;
	
	Sensor();
	Measurement();
//	I2C_Sheduler();

	if (cnt > 20)	{
//	if (1)	{
		//Заряжалка
		Charge_MPPT_v3(&(Params.ChargeControl), Params.Page0.Mode);
		
		// Реле
		Relay();
		
		cnt = 0;
	} else {
		cnt += 1;
	}
	
	// Выходное
	Invertor();
	
	//GpioDataRegs.GPACLEAR.bit.GPIO10 = 1;
/*----------------------------------------------------------------------------*/

#if defined RT_MODE
	if (usCountLed > 1000) {
#elif defined DBG_MODE_DIV2
	if (usCountLed > 500) {
#endif
		//GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;
		//GpioDataRegs.GPATOGGLE.bit.GPIO10 = 1;
		usCountLed=0;
	} else {
		usCountLed++;
	}
	
	
}

void
Relay(void)
{
	if (Params.Page0.Reg_RELAY_P2 == 1){
		GpioDataRegs.GPACLEAR.bit.GPIO12 = 1;
	}
	else{
		GpioDataRegs.GPASET.bit.GPIO12 = 1;
	}
	
	if (Params.Page0.Reg_RELAY_P3 == 1){
		GpioDataRegs.GPACLEAR.bit.GPIO21 = 1;
	}
	else{
		GpioDataRegs.GPASET.bit.GPIO21 = 1;
	}
}

void
Invertor(void)
{
	_iq15 set;
	_iq15 fb;
	_iq15 regul;
	
	if (Params.Page0.fBridge & (Params.Page0.Mode == RUN))
	{
		set = _IQ15div(_IQ15(Params.Page0.Reg_INVERTER_UOUT_SET), _IQ15(220));
		fb = _IQ15div(_IQ15(Params.Page0.Reg220VN_ENT), _IQ15(220));
		
		pPiVout->Reference = set;
		regul = RegulatorPI(pPiVout, fb);
		regul = _IQ15mpy(regul, _IQ15div(_IQ15(425), pPiVout->OutLimitMax));
		Params.Page0.RegTemp0 = _IQ15int(regul);
	}
	else
	{
		pPiVout->Reference = 0;
		pPiVout->Integral_Sum = 0;
		Params.Page0.RegTemp0 = 0;
	}
	
	
}





void
Timer1Int(void)
{
	Uint16 i;
	
	for (i=0; i < 1000; i++)
	{
		asm("	NOP");
	}
	
}

void
Timer2Int(void)
{
	Uint16 i;
	
	for (i=0; i < 1000; i++)
	{
		asm("	NOP");
	}
}

