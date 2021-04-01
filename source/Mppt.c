#include "Mppt.h"
#include "Epwm.h"
#include "faults.h"

extern CrashNotice_t Fault;
extern ParamsTable_t Params;
extern struct sFaults Faults;

static void Mppt_Clear(SourcePtr pxSource);
static void Mppt_IntAssign(RegulatorPi_t *, _iq15, RegulatorPi_t *);

static
	void Mppt_Clear(SourcePtr pxSource) {
	
	pxSource->eStatus = MPP_DIS;
	pxSource->usPwm = 0;
	
	pxSource->pxMppPI->Integral_Sum = 0;
	pxSource->pxUbatPI->Integral_Sum = 0;
	pxSource->iqMutualInt = 0;	
}


/*------------------------------------------------------------------------------
	Comment: ƒанна€ процедура используетс€ при смене режима MPP->DCCONST дл€ того,
	чтобы скопировать значение "общего" интегратора iqMutualInt, значение которого
	обновл€етс€ дл€ текущего режима с приведением верхнего предела. 
	param: dest  - регул€тор нового режима (куда отправить value),
	param: value - значение общего интегратора, которое нужно скопировать,
	param: src - регул€тор предыдущего режима (откуда вз€ть value).
	
------------------------------------------------------------------------------*/
static void
	Mppt_IntAssign(RegulatorPi_t * dest, _iq15 value, RegulatorPi_t * src){
	
	dest->Integral_Sum = 
		_IQ15mpy(value, _IQ15div(dest->Upper_Limit, src->Upper_Limit));
}


void
	Charge_MPPT_v3(ChargeControl * pxStruct, CurrentMode_t mode) {
	
	int i;
	_iq15 regul = 0;
	SourcePtr pxPtr = NULL;
	
	if (mode == RUN) {
		
		//выбор режима работы
		if (Schmitt(&(pxStruct->xBattery.stUBatSet), 
			_IQ15(*(pxStruct->xBattery.psSTBN)), pxStruct->xBattery.psVBAT)) {
			if (pxStruct->xBattery.eStage != CHARGESTAGE_DCVOLTAGE) {
				pxStruct->xBattery.eStage = CHARGESTAGE_DCVOLTAGE;
				
				// приравнивание интегратора CH1
				if (*(pxStruct->xSource1.pbEnable) == true) {
					Mppt_IntAssign(pxStruct->xSource1.pxUbatPI, pxStruct->xSource1.iqMutualInt, 
						pxStruct->xSource1.pxMppPI);
				}
				// приравнивание интегратора CH2
				if (*(pxStruct->xSource2.pbEnable) == true) {
					Mppt_IntAssign(pxStruct->xSource2.pxUbatPI, pxStruct->xSource2.iqMutualInt, 
						pxStruct->xSource2.pxMppPI);
				}
			}
		} else {
			if (pxStruct->xBattery.eStage != CHARGESTAGE_MPPT) {
				pxStruct->xBattery.eStage = CHARGESTAGE_MPPT;
				
				// приравнивание интегратора CH1
				if (*(pxStruct->xSource1.pbEnable) == true) {
					Mppt_IntAssign(pxStruct->xSource1.pxMppPI, pxStruct->xSource1.iqMutualInt, 
						pxStruct->xSource1.pxUbatPI);
				}
				// приравнивание интегратора CH2
				if (*(pxStruct->xSource2.pbEnable) == true) {
					Mppt_IntAssign(pxStruct->xSource2.pxMppPI, pxStruct->xSource2.iqMutualInt, 
						pxStruct->xSource2.pxUbatPI);
				}
			}
		}
		
		for (i=0; i<2; i++){
			// присвоение локального указател€ дл€ сокращени€ писанины
			pxPtr = pxStruct->pxSources[i];
			// ≈сли канал работает в данный момент
			if (*(pxPtr->pbEnable) == true) {
				switch (pxStruct->xBattery.eStage) {
					//работа режима mppt
					case (CHARGESTAGE_MPPT): {
						
						//1. ƒл€ инверсного ѕ»-рег уставку оставл€ем прежней...
						//p_con->p_pi_mpp->Reference = _IQ15(*(p_con->p_reg_SOLBAT_U_MPP));
						pxPtr->pxMppPI->Reference = _IQ15(*(pxPtr->psUMPP));
						
						//2. ... но входной сигнал занул€ем или уравниваем с уставкой.
						// ¬ результате получаетс€ ошибка с минусом и регул€тор быстро выводитс€
						// в нуль или стоит на месте.
						if ( *(pxPtr->psIOxN) >= *(pxPtr->psIKZ) ) {
							//2.1. вывод в 0 при достижении тока  « батарей
							regul = RegulatorPI(pxPtr->pxMppPI, _IQ15(0));
							pxPtr->eStatus = MPP_KZ;
						}
						else if ((*(pxPtr->psIOxN) >= *(pxPtr->psIMPP)) & 
							(*(pxPtr->psIOxN) < *(pxPtr->psIKZ))) {
							//2.2. сто€нка на месте в точке MPP если ток равен или больше тока MPP
							regul = RegulatorPI(pxPtr->pxMppPI, _IQ15(*(pxPtr->psUMPP)));
							pxPtr->eStatus = MPP_STAB;
						}
						else {
							//2.3. обычный режим MPP, когда рег. догон€ет уставку
							regul = RegulatorPI(pxPtr->pxMppPI, _IQ15(*(pxPtr->psSBxN)));
							pxPtr->eStatus = MPP_NORM;
						}
						
						//3. ћасштаб
						regul = _IQ15mpy(regul, _IQ15div(_IQ15(425), pxPtr->pxMppPI->OutLimitMax));
						
						pxPtr->usPwm = _IQ15int(_IQ15mpy(_IQ15div(regul, _IQ15(500)), _IQ15(MPPT_TBPRD)));
						if (pxPtr->usPwm > MPPT_TBPRD) 
							pxPtr->usPwm = MPPT_TBPRD;
						pxPtr->iqMutualInt = pxPtr->pxMppPI->Integral_Sum;
					} break;
					
					//работа режима стабилизации напр€жени€
					case (CHARGESTAGE_DCVOLTAGE): {
						
						//1. ƒл€ пр€мого ѕ»-рег уставку наоборот занул€ем...
						if ( *(pxPtr->psIOxN) >= *(pxPtr->psIKZ) ) {
							//1.1. вывод в 0 при достижении тока  « батарей
							pxPtr->pxUbatPI->Reference = _IQ15(0);
							pxPtr->eStatus = MPP_KZ;
						} else if ((*(pxPtr->psIOxN) >= *(pxPtr->psIMPP)) & 
							(*(pxPtr->psIOxN) < *(pxPtr->psIKZ))) {
							//1.2. сто€нка на месте в точке MPP если ток равен или больше тока MPP
							pxPtr->pxUbatPI->Reference = _IQ15(*(pxStruct->xBattery.psSTBN));
							pxPtr->eStatus = MPP_STAB;
						} else {
							//1.3. обычный режим MPP, когда рег. догон€ет уставку
							//p_con->p_pi_u->Reference = p_con->stUBatSet.value;
							pxPtr->pxUbatPI->Reference = _IQ15(*(pxStruct->xBattery.psVBAT));
							pxPtr->eStatus = MPP_NORM;
						}
						
						//2. ... а вх.сигнал оставл€ем без изм. ¬ результате получаетс€ ошибка с 
						// минусом и регул€тор быстро выводитс€ в нуль.
						regul = RegulatorPI(pxPtr->pxUbatPI, _IQ15(*(pxStruct->xBattery.psSTBN)));
						
						//3. ћасштаб
						regul = _IQ15mpy(regul, _IQ15div(_IQ15(425), pxPtr->pxUbatPI->OutLimitMax));
						
						pxPtr->usPwm = _IQ15int(_IQ15mpy(_IQ15div(regul, _IQ15(500)), _IQ15(MPPT_TBPRD)));
						if (pxPtr->usPwm > MPPT_TBPRD) 
							pxPtr->usPwm = MPPT_TBPRD;
						pxPtr->iqMutualInt = pxPtr->pxUbatPI->Integral_Sum;
					} break;
				}
			} else {
				// ќчищаем текущий канал
				Mppt_Clear(pxPtr);
			}
		}
		
	} else {
		// ќчищаем оба канала
		Mppt_Clear(pxStruct->pxSources[CH1]);
		Mppt_Clear(pxStruct->pxSources[CH2]);
	}
}


bool
Schmitt( struct Schmitt_st * var, _iq15 in, int16 * psVbat )
{	
	if (var->type == SCHMITT_NORMAL)
	{
		/*if (in >= (var->value + var->threshold))
			var->stored = true;
		if (in < (var->value - var->threshold))
			var->stored = false;*/
		if (in >= (_IQ15(*(psVbat)) + var->threshold))
			var->stored = true;
		if (in < (_IQ15(*(psVbat)) - var->threshold))
			var->stored = false;
	}
	
	if (var->type == SCHMITT_MINUS)
	{
		/*if (in >= var->value)
			var->stored = true;
		if (in < (var->value - var->threshold))
			var->stored = false;*/
		if (in >= _IQ15(*(psVbat)))
			var->stored = true;
		if (in < (_IQ15(*(psVbat)) - var->threshold))
			var->stored = false;
	}

	
	if (var->type == SCHMITT_PLUS)
	{
		/*if (in >= (var->value + var->threshold))
			var->stored = true;
		if (in < var->value)
			var->stored = false;*/
		if (in >= (_IQ15(*(psVbat)) + var->threshold))
			var->stored = true;
		if (in < _IQ15(*(psVbat)))
			var->stored = false;
	}
	
	
	return var->stored;
}

void
EPWM3_ServiceRoutine(void)
{
	Uint16 Pwm;
	int32 i32PulseWight = 0;
	static bool xor0 = false, xor1 = false;
/*----------------------------------------------------------------------------*/
	//GpioDataRegs.GPBSET.bit.GPIO34 = 1;
	
	//tmp0 = MPPT1(tempISense_MPPT1, tempUSense_MPPT1);
	//BuckConv_RegulateMPPT_vers1(0, tmp0, &i32PulseWight);
	
	//i32PulseWight= 100; //пускаем шим с кф.заполени€ 100/EPWM34_TBPRD
	
	main_FaultMpptCheck();
	if (Faults.uDrvFlt.bit.MPPT_1) {
		Fault.MPPT1 = true;
	}
	
	switch (Params.Page0.Mode)
	{
		case RUN:
		{
			if (xor1 == true)xor1 = false;
			
			if (Params.Page0.fMPPT1 == true)
			{
				if (xor0 == false)
				{
					//xor0 ^= (bool)1;
					xor0 = true;
					Start_PWM3();
				}
				/*else if (0) {
					//Stop_PWM3();
					//Params.Page0.fMPPT1 = 0;
				}*/
				else {
					
					Pwm = Params.Page0.Reg1 * MPPT_TBPRD / 100;
					if (Pwm > MPPT_TBPRD) Pwm = MPPT_TBPRD;
					//if (Pwm > 100) Pwm = 100;
					EPwm3Regs.CMPA.half.CMPA = Pwm;
					
					
					//EPwm3Regs.CMPA.half.CMPA = Params.ChargeControl.pxSources[CH1]->usPwm;
					
				}
			}
			else
			{
				if (xor0 == true)
				{
					//xor0 ^= (bool)1;
					xor0 = false;
					Stop_PWM3();
					EPwm3Regs.CMPA.half.CMPA = 0;
				}
				asm("	NOP");	//здесь будет крутитьс€ при обнаружении аварии			
			}
	
			break;
		}
		
		default:
		{
			if (xor1 == false){
				xor1 = true;
				xor0 = false;
				EPwm3Regs.CMPA.half.CMPA = 0;
				Stop_PWM3();
			}
			break;
		}
	}
	
	//GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
/*----------------------------------------------------------------------------*/
}

/*
	¬торой канал MPPT2
*/
void
EPWM4_ServiceRoutine(void)
{
	Uint16 Pwm;
	int32 i32PulseWight = 0;
	static bool xor0 = false, xor1 = false;
	
	//tmp0 = MPPT1(tempISense_MPPT1, tempUSense_MPPT1);
	//BuckConv_RegulateMPPT_vers1(0, tmp0, &i32PulseWight);
	
	//i32PulseWight= 100; //пускаем шим с кф.заполени€ 100/EPWM34_TBPRD
	
	main_FaultMpptCheck();
	if (Faults.uDrvFlt.bit.MPPT_2) {
		Fault.MPPT2 = true;
	}
	
	switch (Params.Page0.Mode)
	{
		case RUN:
		{
			if (xor1 == true)xor1 = false;
			
			if (Params.Page0.fMPPT2 == true)
			{
				if (xor0 == false)
				{
					//xor0 ^= (bool)1;
					xor0 = true;
					Start_PWM4();
				}
				/*else if (Fault.MPPT2 == true) {
					Stop_PWM4();
					Params.Page0.fMPPT2 = 0;
				} */
				else {
					
					Pwm = Params.Page0.Reg2 * MPPT_TBPRD / 100;
					if (Pwm > MPPT_TBPRD) Pwm = MPPT_TBPRD;
					//if (Pwm > 100) Pwm = 100;
					EPwm4Regs.CMPA.half.CMPA = Pwm;
					
					//EPwm4Regs.CMPA.half.CMPA = Params.ChargeControl.pxSources[CH2]->usPwm;
				}
			}
			else
			{
				if (xor0 == true)
				{
					//xor0 ^= (bool)1;
					xor0 = false;
					Stop_PWM4();
					EPwm4Regs.CMPA.half.CMPA = 0;
				}
				asm("	NOP");	//здесь будет крутитьс€ при обнаружении аварии			
			}
	
			break;
		}
		
		default:
		{
			if (xor1 == false){
				xor1 = true;
				xor0 = false;
				EPwm4Regs.CMPA.half.CMPA = 0;
				Stop_PWM4();
			}
			break;
		}
	}
}


_iq
MPPT1(_iq batCurrent, _iq batVoltage)
{
	static _iq out_ref = BAT_PMAX_VOLTAGE, delta_out = _IQ(1), pr_power = 0;
	_iq power, tmp, mpptFlyback;
	/**/
	
	power = _IQmpy(batCurrent, batVoltage);
	tmp = power - pr_power;
	
//	tempSrach = _IQ(1);

	if (tmp > 0){
		mpptFlyback = batVoltage - (out_ref + delta_out);
	}
	else{
		mpptFlyback = batVoltage - (out_ref - delta_out);
	}
	pr_power = power;
	
	return mpptFlyback;
}
/**/

_iq
MPPT2(_iq batCurrent, _iq batVoltage)
{
	static _iq out_ref = BAT_PMAX_VOLTAGE, delta_out = _IQ(1), pr_power = 0;
	_iq power, tmp, mpptFlyback;
	/**/
	
	power = _IQmpy(batCurrent, batVoltage);
	tmp = power - pr_power;
	
//	tempSrach = _IQ(1);

	if (tmp > 0){
		mpptFlyback = batVoltage - (out_ref + delta_out);
	}
	else{
		mpptFlyback = batVoltage - (out_ref - delta_out);
	}
	pr_power = power;
	
	return mpptFlyback;
}
/**/

void
BuckConv_RegulateMPPT_vers1(_iq inValSense, _iq inValCalc, int32* pPulseWight)
{
	static _iq regI = _IQ(0), regPI = _IQ(0);
	
//	GPIO10_Tog();
//	GPIO10_Tog();
//	GPIO10_Tog();
	_iq error;														//0.2 us
	
//	GPIO10_Tog();
	error = inValCalc - inValSense;									//1.1 us //iq19 256ns
	
//	GPIO10_Tog();
	regI += _IQmpy(error, IQ_ISODROM_VALUE);						//3.4 us //iq19 328ns
	if (regI > IQ_REG_UPLIMIT_I) regI = IQ_REG_UPLIMIT_I;
	if (regI < IQ_REG_DOWNLIMIT_I) regI = IQ_REG_DOWNLIMIT_I;		//0.5 us //IQ19 552ns
	
//	GPIO10_Tog();
	regPI = _IQmpy(error, _IQ(REG1_P)) + regI;						//3.4 us //iq19 352nS
	
//	GPIO10_Tog();
	if (regPI > IQ_REG_UPLIMIT_PI) regPI = IQ_REG_UPLIMIT_PI;
	if (regPI < IQ_REG_DOWNLIMIT_PI) regPI = IQ_REG_DOWNLIMIT_PI;	//0.5 us //IQ19 552ns
	
//	GPIO10_Tog();
	*pPulseWight = _IQint(_IQmpy(regPI, IQ_RESOLUT_CF));			//0.72 us //iq19 680ns
	
//	GPIO10_Tog();
//	GPIO10_Tog();
//	GPIO10_Tog();
}
/**/
