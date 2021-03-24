#include "Inverter.h"
#include "ParamsTable.h"
#include "Epwm.h"
#include "faults.h"


//extern PControl_t PControl;
extern CrashNotice_t Fault;
extern ParamsTable_t Params;

bool Temp = true;
extern struct sFaults Faults; 




/*	Routines */


Uint16 SineInverting(_iq iqPulseWight)
{
	static _iq phi = _IQ(0);
	Uint16 siTmp;
	_iq iqSine, iqCmp;


	iqSine = _IQsin(phi);
	iqCmp = _IQmpy(iqSine, IQ_INVERTER_PERIOD);
	siTmp = INVERTER_PERIOD + _IQint(_IQmpy(iqCmp, iqPulseWight));
	
	phi += IQ_DOUBLE_PI_DIVIDED;
	if (phi > IQ_DOUBLE_PI)
	{
		phi -= IQ_DOUBLE_PI;
	}
	
	return siTmp;
}
/**/

void
Inverter_ServiceRoutine(void)
{
	volatile Uint16 sine;
	static bool xor0 = false, xor1 = false;
	
/*----------------------------------------------------------------------------*/
	GpioDataRegs.GPBSET.bit.GPIO34 = 1;
	
	sine = SineInverting(_IQdiv(_IQ(Params.Page0.RegTemp0), _IQ(500)));
	//sine = SineInverting(_IQdiv(_IQ(Params.Page0.Reg0), _IQ(100)));
	
	main_FaultInvCheck();
	/*if (Faults.uDrvFlt.bit.INV_A1 | Faults.uDrvFlt.bit.INV_A2 | Faults.uDrvFlt.bit.INV_A3 |
		Faults.uDrvFlt.bit.INV_A4) {
		Fault.Bridge = true;
	}*/
	
	switch (Params.Page0.Mode)
	{
		case RUN:
		{
			if (xor1 == true)xor1 = false;
				
			if (Params.Page0.fBridge == true)
			{
				if (xor0 == false)
				{
					//xor0 ^= (t_bool)1;
					xor0 = true;
					Fault.Bridge = false;
					Start_PWM1();
					Start_PWM2();
				}
				else if (Fault.Bridge == true)
				{
	//				if (xor2 == false){
	//					xor2 = true;	//временно не используется
						Stop_PWM1();
						Stop_PWM2();
						Params.Page0.fBridge = false;
	//				}
	//				else{
	//					asm("	NOP");	//здесь будет крутиться при обнаружении аварии
	//				}
				}
				else
				{
					DINT;
					if (sine > EPWM_TBPRD) sine = EPWM_TBPRD;
					EPwm2Regs.CMPA.half.CMPA = sine;
					EPwm1Regs.CMPA.half.CMPA = sine;
					EINT;
				}
					
			}
			else
			{
				if (xor0 == true)
				{
					//xor0 ^= (t_bool)1;
					xor0 = false;
					Stop_PWM1();
					Stop_PWM2();
				}
				asm("	NOP");	//здесь будет крутиться при обнаружении аварии
			}
			
			break; //case RUN
		}
		
		default:
		{
			if (xor1 == false){
				xor0 = false;
				xor1 = true;
				EPwm1Regs.CMPA.half.CMPA = 0;
				EPwm2Regs.CMPA.half.CMPA = 0;
				Stop_PWM1();
				Stop_PWM2();
			}
			
			break; // default
		}
	}
	
		GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
/*----------------------------------------------------------------------------*/
}
/**/
