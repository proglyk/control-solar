#include "Adc.h"
#include "IQmathLib.h"
#include "Main.h"
//#include "ParamsTable.h"

int16 adcvalues[13] = {0};
_iq tempISense_MPPT1 = 0, tempISense_MPPT2 = 0;
_iq tempUSense_MPPT1 = 0, tempUSense_MPPT2 = 0;

extern _iq13 iq_tempSensorValues[12];

//extern CrashNotice_t Fault;

void
Sensor(void)
{
	static Uint16 tmp = 0;
	static int16 s16tmp0 = 0;
	int16 s16tmp = 0;
	
//	AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;
	
	// A0:	tempBattery		TSTB
	iq_tempSensorValues[0] = _IQ13(AdcRegs.ADCRESULT0 >> 4);
//	tmp = AdcRegs.ADCRESULT0 >> 4;
//	GPIO10_On();
//	DELAY_US(_IQ13int(_IQ13div(_IQ13((AdcRegs.ADCRESULT0 >> 4)), _IQ13(100))));
//	GPIO10_Off();
	
	
	// A2:	tempCooler		TRAD
	iq_tempSensorValues[1] = _IQ13(AdcRegs.ADCRESULT2 >> 4);
	
	// A3:	voltageLoad		~220VN
	//iq_tempSensorValues[2] = _IQ13(AdcRegs.ADCRESULT3 >> 4);
	s16tmp0 = (AdcRegs.ADCRESULT3 >> 4) - 2027;
	//s16tmp0 = 2047;
	iq_tempSensorValues[2] = _IQ13(s16tmp0);

	// A4:	currentBattery	ISTBN
	//iq_tempSensorValues[3] = _IQ13(AdcRegs.ADCRESULT4 >> 4);
	s16tmp0 = (AdcRegs.ADCRESULT4 >> 4) - 2035;
	iq_tempSensorValues[3] = _IQ13(s16tmp0);

	// A5:	tempTransformer	TTR
	iq_tempSensorValues[4] = _IQ13(AdcRegs.ADCRESULT5 >> 4);
	
	// A6:	VoltageBattery	+STBN
	iq_tempSensorValues[5] = _IQ13(AdcRegs.ADCRESULT6 >> 4);

	// A7:	currentBridge	IINVN
	s16tmp0 = (AdcRegs.ADCRESULT7 >> 4) - 2047;
	iq_tempSensorValues[6] = _IQ13(s16tmp0);
	
	// B0:	currentLoad		I220N
	//iq_tempSensorValues[7] = _IQ13(AdcRegs.ADCRESULT8 >> 4);
	s16tmp0 = (AdcRegs.ADCRESULT8 >> 4) - 2047;
	iq_tempSensorValues[7] = _IQ13(s16tmp0);

	// B1:	currentMPPT2	IO2N
	tmp = (Uint16)(AdcRegs.ADCRESULT9 >> 4);
	s16tmp = (int16)tmp - 4;
	if (s16tmp < 0) s16tmp = 0;
	iq_tempSensorValues[8] = _IQ13((Uint16)s16tmp);

	// B4:	voltageMPPT2	+SB2N
	iq_tempSensorValues[9] = _IQ13(AdcRegs.ADCRESULT12 >> 4);
//	s16tmp = _IQ13int(iq_tempSensorValues[9]);

	// B5:	voltageMPPT1	+SB1N
	iq_tempSensorValues[10] = _IQ13(AdcRegs.ADCRESULT13 >> 4);

	// B6:	currentMPPT1	IO1N
	iq_tempSensorValues[11] = _IQ13(AdcRegs.ADCRESULT14 >> 4);
}

void
AdcIntServiceRoutine(void)
{
	//static Uint32	bridge_cnt = 0;
	
	GpioDataRegs.GPATOGGLE.bit.GPIO10 = 1;
	
//	adcvalues[0] = AdcRegs.ADCRESULT0;
//	adcvalues[1] = 0;
//	adcvalues[2] = AdcRegs.ADCRESULT2;
//	adcvalues[3] = AdcRegs.ADCRESULT3;
//	iq_adcvalues[4] = _IQ13(AdcRegs.ADCRESULT4  >> 4);								//currentBattery
//	adcvalues[5] = AdcRegs.ADCRESULT5;
//	adcvalues[6] = AdcRegs.ADCRESULT6;
//	adcvalues[7] = 	(adcvalues[7] * 7 + ((AdcRegs.ADCRESULT7 >> 4) - 2047)) >> 3;	//currentBridge
	adcvalues[7] = ((AdcRegs.ADCRESULT7 >> 4) - 0x0800);
//	adcvalues[7] = (adcvalues[7]*7 + ((AdcRegs.ADCRESULT7 >> 4) - 0x0800))>>3;
//	iq_adcvalues[8] = _IQ13(AdcRegs.ADCRESULT8 >> 4);								//currentLoad
	adcvalues[9] = (adcvalues[9] * 7 + (AdcRegs.ADCRESULT9 >> 4)) >> 3;				//currentMPPT2
//	adcvalues[9] += 5;
	adcvalues[10] = (adcvalues[10] * 7 + (AdcRegs.ADCRESULT10 >> 4)) >> 3;			//voltageMPPT2
//	adcvalues[10] = 21;
	adcvalues[11] = (adcvalues[11] * 7 + (AdcRegs.ADCRESULT11 >> 4)) >> 3;			//voltageMPPT1
	adcvalues[12] = (adcvalues[12] * 7 + (AdcRegs.ADCRESULT12 >> 4)) >> 3;			//currentMPPT1
	
	tempISense_MPPT2 = _IQdiv(_IQ(adcvalues[9]), _IQ(68.5));
	tempUSense_MPPT2 = _IQdiv(_IQ(adcvalues[10]), _IQ(20.8));
	tempISense_MPPT1 = _IQdiv(_IQ(adcvalues[12]), _IQ(68.5));
	tempUSense_MPPT1 = _IQdiv(_IQ(adcvalues[11]), _IQ(20.8));
	
//	if (adcvalues[4]) >
//	Protection(_IQ13(adcvalues[7]));

//	if (_IQ13mpy(iq_adcvalues[4], _IQ13(16.12)) > BATTERY_CURRENT_SHUT_OFF(1.5)){
//		crashNotice.Battery = true;
//	}

	if (adcvalues[7] > _BRIDGE_CURRENT_SHUT_OFF(1.5, 100)){
	//	crashNotice.Bridge = true;
	}
	else if (adcvalues[7] < -_BRIDGE_CURRENT_SHUT_OFF(1.5, 100)){
	//	crashNotice.Bridge = true;
	}
	
//	if (iq_adcvalues[8] > LOAD_CURRENT_SHUT_OFF(1.5)){
//		crashNotice.fBridge = true;
//		crashNotice.Bridge = true;
//	}
	
	if (adcvalues[9] > _BUCK_CONV_CURRENT_SHUT_OFF(3, 3)){
		//Fault.MPPT2 = true;
	}
	
	if (adcvalues[12] > _BUCK_CONV_CURRENT_SHUT_OFF(3, 3)){
		//Fault.MPPT1 = true;
	}
/*	
	else if (iq_adcvalues[7] > BRIDGE_CURRENT_SHUT_OFF(1.3)){
		if (bridge_cnt > (1200000)){
//			crashNotice.fBridge = true;
//			crashNotice.Bridge = true;
			bridge_cnt = 0;
		}
		else bridge_cnt++;
	}
*/
	
}


void
ADC_Config_SOC(void)
{
	/**/
	
//	EALLOW;
//	SysCtrlRegs.HISPCP.all = 0x4;  // HSPCLK = SYSCLKOUT/8
//	EDIS;


	//AdcRegs.ADCTRL1.bit.RESET = 1;
	//DELAY_US(50);	
	/*
	
	AdcRegs.ADCREFSEL.all = 0;
	AdcRegs.ADCTRL3.all = (1<<7) | (1<<6);
	DELAY_40US();
	DELAY_40US();
	//DELAY_US(50);
	AdcRegs.ADCTRL3.all |= (1<<5);
	//DELAY_US(5000);
	DELAY_40US();
	DELAY_40US();
	DELAY_40US();
	
	*/
	
	AdcRegs.ADCTRL3.bit.ADCBGRFDN = 0x3;// Power up bandgap/reference circuitry
	//DELAY_US(20000); // 10 mSec 150 MGz Delay before powering up rest of ADC
	DELAY_1MS();
	DELAY_1MS();
	DELAY_1MS();
	DELAY_1MS();
	DELAY_1MS();
	DELAY_1MS();
	DELAY_1MS();
	DELAY_1MS();
	DELAY_1MS();
	DELAY_1MS();
	
	AdcRegs.ADCTRL3.bit.ADCPWDN = 1;	// Power up rest of ADC
	//DELAY_US(10000); // 1 mSec 150 MGz Delay after powering up ADC
	DELAY_1MS();
	
/*	
#ifdef RT_MODE
	AdcRegs.ADCTRL1.bit.CPS = 1;
	AdcRegs.ADCTRL3.bit.ADCCLKPS = 4;
#elif defined DBG_MODE_DIV2
	AdcRegs.ADCTRL1.bit.CPS = 0;
	AdcRegs.ADCTRL3.bit.ADCCLKPS = 4;
#endif
*/
	//InitAdc();

	AdcRegs.ADCTRL1.bit.ACQ_PS = ADC_SHCLK;
	AdcRegs.ADCTRL3.bit.ADCCLKPS = ADC_CKPS;
   
	AdcRegs.ADCTRL3.bit.SMODE_SEL = 0; 	// Setup simultaneous sampling mode
	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1; 	// Setup cascaded sequencer mode
	AdcRegs.ADCMAXCONV.all = 15;			// (16 total)
	AdcRegs.ADCTRL1.bit.CONT_RUN = 0;
	AdcRegs.ADCTRL1.bit.SEQ_OVRD = 0;
	
	AdcRegs.ADCTRL2.bit.EPWM_SOCA_SEQ1 = 1;// Enable SOCA from ePWM to start SEQ1
	AdcRegs.ADCTRL2.bit.INT_ENA_SEQ1 = 1;  // Enable SEQ1 interrupt (every EOS)

	AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0; 	// Setup conv from ADCINA0
	AdcRegs.ADCCHSELSEQ1.bit.CONV01 = 1;	// Setup conv from ADCINA1
	AdcRegs.ADCCHSELSEQ1.bit.CONV02 = 2; 	// Setup conv from ADCINA2
	AdcRegs.ADCCHSELSEQ1.bit.CONV03 = 3; 	// Setup conv from ADCINA3

	AdcRegs.ADCCHSELSEQ2.bit.CONV04 = 4; 	// Setup conv from ADCINA4
	AdcRegs.ADCCHSELSEQ2.bit.CONV05 = 5;	// Setup conv from ADCINA5
	AdcRegs.ADCCHSELSEQ2.bit.CONV06 = 6; 	// Setup conv from ADCINA6
	AdcRegs.ADCCHSELSEQ2.bit.CONV07 = 7; 	// Setup conv from ADCINA7

	AdcRegs.ADCCHSELSEQ3.bit.CONV08 = 8; 	// Setup conv from ADCINB0
	AdcRegs.ADCCHSELSEQ3.bit.CONV09 = 9;	// Setup conv from ADCINB1
	AdcRegs.ADCCHSELSEQ3.bit.CONV10 = 10; 	// Setup conv from ADCINB2
	AdcRegs.ADCCHSELSEQ3.bit.CONV11 = 11; 	// Setup conv from ADCINB3
	
	AdcRegs.ADCCHSELSEQ4.bit.CONV12 = 12; 	// Setup conv from ADCINB4
	AdcRegs.ADCCHSELSEQ4.bit.CONV13 = 13; 	// Setup conv from ADCINB5
	AdcRegs.ADCCHSELSEQ4.bit.CONV14 = 14; 	// Setup conv from ADCINB6
	AdcRegs.ADCCHSELSEQ4.bit.CONV15 = 15; 	// Setup conv from ADCINB7
	
	//AdcRegs.ADCTRL2.all = 0x2000; // Не надо
}
