#include "Mppt.h"
#include "Epwm.h"
#include "faults.h"

//extern PControl_t PControl;
extern CrashNotice_t Fault;
extern ParamsTable_t Params;
extern struct sFaults Faults;

static void Mppt_Clear(SourcePtr pxSource);
static void Mppt_IntAssign(RegulatorPi_t *, _iq15, RegulatorPi_t *);

static
	void Mppt_Clear(SourcePtr pxSource) {
	
	pxSource->eStatus = MPP_DIS;
	*(pxSource->psPwm) = 0;
	
	pxSource->pxMppPI->Integral_Sum = 0;
	pxSource->pxUbatPI->Integral_Sum = 0;
	pxSource->iqMutualInt = 0;	
}


/*------------------------------------------------------------------------------
	Comment: Данная процедура используется при смене режима MPP->DCCONST для того,
	чтобы скопировать значение "общего" интегратора iqMutualInt, значение которого
	обновляется для текущего режима с приведением верхнего предела. 
	param: dest  - регулятор нового режима (куда отправить value),
	param: value - значение общего интегратора, которое нужно скопировать,
	param: src - регулятор предыдущего режима (откуда взять value).
	
------------------------------------------------------------------------------*/
static void
	Mppt_IntAssign(RegulatorPi_t * dest, _iq15 value, RegulatorPi_t * src){
	
	dest->Integral_Sum = 
		_IQ15mpy(value, _IQ15div(dest->Upper_Limit, src->Upper_Limit));
}

void
	Charge_MPPT_v3_1(ChargeControl * pxStruct, CurrentMode_t mode) {
	
	//SourcePtr pxPtr = pxStruct->pxSources[CH1];
	//SourcePtr pxPtr = &pxStruct->xSource1;
	//SourcePtr pxPtr = &(Params.ChargeControl.xSource1);
	SourcePtr pxPtr = pxStruct->pxSources[CH1];
	_iq15 regul = 0;
	
	if (mode == RUN) {
		//1. Для инверсного ПИ-рег уставку оставляем прежней...
		pxPtr->pxMppPI->Reference = _IQ15(*(pxPtr->psUMPP));
		//pxPtr->pxMppPI->Reference = _IQ15(1250);
		
		//2. ... но входной сигнал зануляем или уравниваем с уставкой.
		// В результате получается ошибка с минусом и регулятор быстро выводится
		// в нуль или стоит на месте.
		//2.3. обычный режим MPP, когда рег. догоняет уставку
		regul = RegulatorPI(pxPtr->pxMppPI, _IQ15(*(pxPtr->psSBxN)));
		pxPtr->eStatus = MPP_NORM;
		
		//3. Масштаб
		regul = _IQ15mpy(regul, _IQ15div(_IQ15(425), pxPtr->pxMppPI->OutLimitMax));
		
		* (pxPtr->psPwm) = _IQ15int(regul);
		//pxPtr->iqMutualInt = pxPtr->pxMppPI->Integral_Sum;
	} else {
		// Очищаем оба канала
		Mppt_Clear(pxStruct->pxSources[CH1]);
	}
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
			// присвоение локального указателя для сокращения писанины
			pxPtr = pxStruct->pxSources[i];
			// Если канал работает в данный момент
			if (*(pxPtr->pbEnable) == true) {
				switch (pxStruct->xBattery.eStage) {
					//работа режима mppt
					case (CHARGESTAGE_MPPT): {
						
						//1. Для инверсного ПИ-рег уставку оставляем прежней...
						//p_con->p_pi_mpp->Reference = _IQ15(*(p_con->p_reg_SOLBAT_U_MPP));
						pxPtr->pxMppPI->Reference = _IQ15(*(pxPtr->psUMPP));
						
						//2. ... но входной сигнал зануляем или уравниваем с уставкой.
						// В результате получается ошибка с минусом и регулятор быстро выводится
						// в нуль или стоит на месте.
						if ( *(pxPtr->psIOxN) >= *(pxPtr->psIKZ) ) {
							//2.1. вывод в 0 при достижении тока КЗ батарей
							regul = RegulatorPI(pxPtr->pxMppPI, _IQ15(0));
							pxPtr->eStatus = MPP_KZ;
						}
						else if ((*(pxPtr->psIOxN) >= *(pxPtr->psIMPP)) & 
							(*(pxPtr->psIOxN) < *(pxPtr->psIKZ))) {
							//2.2. стоянка на месте в точке MPP если ток равен или больше тока MPP
							regul = RegulatorPI(pxPtr->pxMppPI, _IQ15(*(pxPtr->psUMPP)));
							pxPtr->eStatus = MPP_STAB;
						}
						else {
							//2.3. обычный режим MPP, когда рег. догоняет уставку
							regul = RegulatorPI(pxPtr->pxMppPI, _IQ15(*(pxPtr->psSBxN)));
							pxPtr->eStatus = MPP_NORM;
						}
						
						//3. Масштаб
						regul = _IQ15mpy(regul, _IQ15div(_IQ15(425), pxPtr->pxMppPI->OutLimitMax));
						
						* (pxPtr->psPwm) = _IQ15int(regul);
						pxPtr->iqMutualInt = pxPtr->pxMppPI->Integral_Sum;
					} break;
					
					//работа режима стабилизации напряжения
					case (CHARGESTAGE_DCVOLTAGE): {
						
						//1. Для прямого ПИ-рег уставку наоборот зануляем...
						if ( *(pxPtr->psIOxN) >= *(pxPtr->psIKZ) ) {
							//1.1. вывод в 0 при достижении тока КЗ батарей
							pxPtr->pxUbatPI->Reference = _IQ15(0);
							pxPtr->eStatus = MPP_KZ;
						} else if ((*(pxPtr->psIOxN) >= *(pxPtr->psIMPP)) & 
							(*(pxPtr->psIOxN) < *(pxPtr->psIKZ))) {
							//1.2. стоянка на месте в точке MPP если ток равен или больше тока MPP
							pxPtr->pxUbatPI->Reference = _IQ15(*(pxStruct->xBattery.psSTBN));
							pxPtr->eStatus = MPP_STAB;
						} else {
							//1.3. обычный режим MPP, когда рег. догоняет уставку
							//p_con->p_pi_u->Reference = p_con->stUBatSet.value;
							pxPtr->pxUbatPI->Reference = _IQ15(*(pxStruct->xBattery.psVBAT));
							pxPtr->eStatus = MPP_NORM;
						}
						
						//2. ... а вх.сигнал оставляем без изм. В результате получается ошибка с 
						// минусом и регулятор быстро выводится в нуль.
						regul = RegulatorPI(pxPtr->pxUbatPI, _IQ15(*(pxStruct->xBattery.psSTBN)));
						
						//3. Масштаб
						regul = _IQ15mpy(regul, _IQ15div(_IQ15(425), pxPtr->pxUbatPI->OutLimitMax));
						
						* (pxPtr->psPwm) = _IQ15int(regul);
						pxPtr->iqMutualInt = pxPtr->pxUbatPI->Integral_Sum;
					} break;
				}
			} else {
				// Очищаем текущий канал
				Mppt_Clear(pxPtr);
			}
		}
		
	} else {
		// Очищаем оба канала
		Mppt_Clear(pxStruct->pxSources[CH1]);
		Mppt_Clear(pxStruct->pxSources[CH2]);
	}
}


/*
	Зарядка
*/
/*
void
	Charge_MPPT_v2(struct BatteryControlBlock_st * p_con) {
	
	//static _iq15 qMainIntegrator = 0;
	_iq15 regul;
	
	if (*(p_con->p_channel) & (Params.Page0.Mode == RUN)) {
		//выбор режима работы
		if (Schmitt(&(p_con->stUBatSet), _IQ15(*(p_con->psSTBN)), p_con->psVBAT)) {
			if (p_con->Stage != MPPT_CHARGESTAGE_DCVOLTAGE) {
				p_con->Stage = MPPT_CHARGESTAGE_DCVOLTAGE;
				p_con->p_pi_u->Integral_Sum = _IQ15mpy(p_con->qMainIntegrator, 
					_IQ15div(p_con->p_pi_u->Upper_Limit, p_con->p_pi_mpp->Upper_Limit));
			}
		} else {
			if (p_con->Stage != MPPT_CHARGESTAGE_MPPT) {
				p_con->Stage = MPPT_CHARGESTAGE_MPPT;
				p_con->p_pi_mpp->Integral_Sum = _IQ15mpy(p_con->qMainIntegrator, 
					_IQ15div(p_con->p_pi_mpp->Upper_Limit, p_con->p_pi_u->Upper_Limit));
			}
		}
		
		switch (p_con->Stage) {
			//работа режима mppt
			case (MPPT_CHARGESTAGE_MPPT): {
				
				//1. Для инверсного ПИ-рег уставку оставляем прежней...
				p_con->p_pi_mpp->Reference = _IQ15(*(p_con->p_reg_SOLBAT_U_MPP));
				
				//2. ... но входной сигнал зануляем или уравниваем с уставкой.
				// В результате получается ошибка с минусом и регулятор быстро выводится
				// в нуль или стоит на месте.
				if ( *(p_con->psIOxN) >= *(p_con->p_reg_SOLBAT_I_KZ) ) {
					//2.1. вывод в 0 при достижении тока КЗ батарей
					regul = RegulatorPI(p_con->p_pi_mpp, _IQ15(0));
					p_con->StageMPP = MPP_KZ;
				}
				else if ((*(p_con->psIOxN) >= *(p_con->p_reg_SOLBAT_I_MPP)) & 
					(*(p_con->psIOxN) < *(p_con->p_reg_SOLBAT_I_KZ))) {
					//2.2. стоянка на месте в точке MPP если ток равен или больше тока MPP
					regul = RegulatorPI(p_con->p_pi_mpp, _IQ15(*(p_con->p_reg_SOLBAT_U_MPP)));
					p_con->StageMPP = MPP_STAB;
				}
				else {
					//2.3. обычный режим MPP, когда рег. догоняет уставку
					regul = RegulatorPI(p_con->p_pi_mpp, _IQ15(*(p_con->psSBxN)));
					p_con->StageMPP = MPP_NORM;
				}
				
				//3. Масштаб
				regul = _IQ15mpy(regul, _IQ15div(_IQ15(425), p_con->p_pi_mpp->OutLimitMax));
				
				* (p_con->p_reg_pwm) = _IQ15int(regul);
				p_con->qMainIntegrator = p_con->p_pi_mpp->Integral_Sum;
			} break;
			
			//работа режима стабилизации напряжения
			case (MPPT_CHARGESTAGE_DCVOLTAGE): {
				
				//1. Для прямого ПИ-рег уставку наоборот зануляем...
				if ( *(p_con->psIOxN) >= *(p_con->p_reg_SOLBAT_I_KZ) ) {
					//1.1. вывод в 0 при достижении тока КЗ батарей
					p_con->p_pi_u->Reference = _IQ15(0);
					p_con->StageMPP = MPP_KZ;
				} else if ((*(p_con->psIOxN) >= *(p_con->p_reg_SOLBAT_I_MPP)) & 
					(*(p_con->psIOxN) < *(p_con->p_reg_SOLBAT_I_KZ))) {
					//1.2. стоянка на месте в точке MPP если ток равен или больше тока MPP
					p_con->p_pi_u->Reference = _IQ15(*(p_con->psSTBN));
					p_con->StageMPP = MPP_STAB;
				} else {
					//1.3. обычный режим MPP, когда рег. догоняет уставку
					//p_con->p_pi_u->Reference = p_con->stUBatSet.value;
					p_con->p_pi_u->Reference = _IQ15(*(p_con->psVBAT));
					p_con->StageMPP = MPP_NORM;
				}
				
				//2. ... а вх.сигнал оставляем без изм. В результате получается ошибка с 
				// минусом и регулятор быстро выводится в нуль.
				regul = RegulatorPI(p_con->p_pi_u, _IQ15(*(p_con->psSTBN)));
				
				//3. Масштаб
				regul = _IQ15mpy(regul, _IQ15div(_IQ15(425), p_con->p_pi_u->OutLimitMax));
				
				* (p_con->p_reg_pwm) = _IQ15int(regul);
				p_con->qMainIntegrator = p_con->p_pi_u->Integral_Sum;
			} break;
		}
	}
	else
	{
		p_con->qMainIntegrator = 0;
		*(p_con->p_reg_pwm) = 0;
		p_con->p_pi_u->Integral_Sum = 0;
		p_con->p_pi_mpp->Integral_Sum = 0;
		p_con->Stage = MPPT_CHARGESTAGE_DISABLED;
		p_con->StageMPP = MPP_DIS;
	}
}*/


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
	
	//tmp0 = MPPT1(tempISense_MPPT1, tempUSense_MPPT1);
	//BuckConv_RegulateMPPT_vers1(0, tmp0, &i32PulseWight);
	
	//i32PulseWight= 100; //пускаем шим с кф.заполения 100/EPWM34_TBPRD
	
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
					/*
					Pwm = Params.Page0.Reg1 * MPPT_TBPRD / 100;
					if (Pwm > MPPT_TBPRD) Pwm = MPPT_TBPRD;
					//if (Pwm > 100) Pwm = 100;
					EPwm3Regs.CMPA.half.CMPA = Pwm;
					*/
					
					Pwm = _IQ15int(_IQ15mpy(_IQ15div(_IQ15(Params.Page0.RegTemp1), _IQ15(500)), _IQ15(MPPT_TBPRD)));
					if (Pwm > MPPT_TBPRD) Pwm = MPPT_TBPRD;
					EPwm3Regs.CMPA.half.CMPA = Pwm;
					
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
				asm("	NOP");	//здесь будет крутиться при обнаружении аварии			
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
}

/*
	Второй канал MPPT2
*/
void
EPWM4_ServiceRoutine(void)
{
	Uint16 Pwm;
	int32 i32PulseWight = 0;
	static bool xor0 = false, xor1 = false;
	
	//tmp0 = MPPT1(tempISense_MPPT1, tempUSense_MPPT1);
	//BuckConv_RegulateMPPT_vers1(0, tmp0, &i32PulseWight);
	
	//i32PulseWight= 100; //пускаем шим с кф.заполения 100/EPWM34_TBPRD
	
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
					
					/*Pwm = Params.Page0.Reg2 * MPPT_TBPRD / 100;
					if (Pwm > MPPT_TBPRD) Pwm = MPPT_TBPRD;
					//if (Pwm > 100) Pwm = 100;
					EPwm4Regs.CMPA.half.CMPA = Pwm;*/
					
					
					Pwm = _IQ15int(_IQ15mpy(_IQ15div(_IQ15(Params.Page0.RegTemp2), _IQ15(500)), _IQ15(MPPT_TBPRD)));
					if (Pwm > MPPT_TBPRD) Pwm = MPPT_TBPRD;
					EPwm4Regs.CMPA.half.CMPA = Pwm;
					
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
				asm("	NOP");	//здесь будет крутиться при обнаружении аварии			
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
