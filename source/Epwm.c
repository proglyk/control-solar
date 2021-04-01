#include "global.h"
#include "epwm.h"
#include "main.h"

void EPwm1Timer_Config(void){
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
	EDIS;
	
	Stop_PWM1();
	
	EALLOW;
   	EPwm1Regs.TZCTL.bit.TZA = TZ_FORCE_LO;		// первоначально в 0 
   	EPwm1Regs.TZCTL.bit.TZB = TZ_FORCE_LO;		// первоначально в 0 
	EPwm1Regs.TZFRC.bit.OST = 1;				//запрещаем ШИМ
   	EDIS;
	
	EPwm1Regs.TBPRD = EPWM_TBPRD;                        // Set timer period
	EPwm1Regs.TBPHS.half.TBPHS = 0;           // Phase is 0
	EPwm1Regs.TBCTR = 0;                      // Clear counter

	// Setup TBCLK
	EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up
	EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
	#if defined RT_MODE
		EPwm1Regs.TBCTL.bit.HSPCLKDIV = 1;
	#elif defined DBG_MODE_DIV2
		EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;
	#endif       // Clock ratio to SYSCLKOUT

	EPwm1Regs.TBCTL.bit.CLKDIV = 1;

	EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;
	EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // Sync down-stream module

   EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;    // Load registers every ZERO
   EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
   EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
   EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;   

   // Setup compare 
   EPwm1Regs.CMPA.half.CMPA = EPWM_CMPA;
   
	// Set actions
	EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;          // Set PWM1A on Zero
	EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;

	EPwm1Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
	EPwm1Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
	EPwm1Regs.DBCTL.bit.IN_MODE = DBA_ALL;
	EPwm1Regs.DBRED = 120;	//	5 US
	EPwm1Regs.DBFED = 120;	//	5 US
	
	EPwm1Regs.ETSEL.bit.INTSEL = 2;     //	Enable event time-base counter equal to period
	EPwm1Regs.ETSEL.bit.INTEN = 1;		//	Interrupt Enabled
	EPwm1Regs.ETSEL.bit.SOCAEN = 1;        // Enable SOC on A group
	EPwm1Regs.ETSEL.bit.SOCASEL = 2;       // Select SOC from from CPMA on upcount
	
	EPwm1Regs.ETPS.bit.INTPRD = 3;
	EPwm1Regs.ETPS.bit.SOCAPRD = 1;
	
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	EDIS;
}
/**/

void EPwm2Timer_Config(void){
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
	EDIS;
	
	Stop_PWM2();
	
	EALLOW;
   	EPwm2Regs.TZCTL.bit.TZA = TZ_FORCE_LO;		// первоначально в 0 
   	EPwm2Regs.TZCTL.bit.TZB = TZ_FORCE_LO;		// первоначально в 0 
	EPwm2Regs.TZFRC.bit.OST = 1;				//запрещаем ШИМ
   	EDIS;
	
	EPwm2Regs.TBPRD = EPWM_TBPRD;                        // Set timer period
	EPwm2Regs.TBPHS.half.TBPHS = 0;           // Phase is 0
	EPwm2Regs.TBCTR = 0;                      // Clear counter

	// Setup TBCLK
	EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; // Count up
	EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;        // Disable phase loading
	#if defined RT_MODE
		EPwm2Regs.TBCTL.bit.HSPCLKDIV = 1;
	#elif defined DBG_MODE_DIV2
		EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;
	#endif       // Clock ratio to SYSCLKOUT
	
	EPwm2Regs.TBCTL.bit.CLKDIV = 1;

	EPwm2Regs.TBCTL.bit.PRDLD = TB_SHADOW;
	EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; // Sync down-stream module

	EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;    // Load registers every ZERO
	EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
	EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;   

   // Setup compare 
	EPwm2Regs.CMPA.half.CMPA = EPWM_CMPA;
   
	// Set actions
	EPwm2Regs.AQCTLA.bit.CAU = AQ_CLEAR;          // Set PWM1A on Zero
	EPwm2Regs.AQCTLA.bit.CAD = AQ_SET;

	EPwm2Regs.DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;
	EPwm2Regs.DBCTL.bit.POLSEL = DB_ACTV_HIC;
	EPwm2Regs.DBCTL.bit.IN_MODE = DBA_ALL;
	EPwm2Regs.DBRED = 120;
	EPwm2Regs.DBFED = 120;
	
	EPwm2Regs.ETSEL.bit.INTSEL = 2;     //	Enable event time-base counter equal to period
	EPwm2Regs.ETSEL.bit.INTEN = 0;		//	Interrupt Enabled
	EPwm2Regs.ETPS.bit.INTPRD = 3;
	
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	EDIS;
}
/**/

void EPwm3Timer_Config(void){
	Stop_PWM3();
	
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      	// Stop all the TB clocks
	EDIS;

	EPwm3Regs.TBPRD = MPPT_TBPRD;                	// Set timer period
	EPwm3Regs.TBPHS.half.TBPHS = 0x0000;           	// Phase is 0
	EPwm3Regs.TBCTR = 0x0000;                      	// Clear counter

	EPwm3Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;	// Count up
	EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;        	// Disable phase loading

	#if defined RT_MODE
		EPwm3Regs.TBCTL.bit.HSPCLKDIV = 1;
	#elif defined DBG_MODE_DIV2
		EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;
	#endif      		// Clock ratio to SYSCLKOUT

	EPwm3Regs.TBCTL.bit.CLKDIV = 1;
	
	EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;    	// Load registers every ZERO
	EPwm3Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
	EPwm3Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;   

	EPwm3Regs.CMPA.half.CMPA = 0;
   
	EPwm3Regs.AQCTLA.bit.CAU = AQ_CLEAR;             // Set PWM1A on Zero
	EPwm3Regs.AQCTLA.bit.CAD = AQ_SET;
//	EPwm4Regs.AQCTLB.bit.ZRO = AQ_TOGGLE;
	
	EPwm3Regs.DBCTL.bit.OUT_MODE = 2;
	EPwm3Regs.DBCTL.bit.POLSEL = 0;
	EPwm3Regs.DBCTL.bit.IN_MODE = 0;
	EPwm3Regs.DBRED = 25;
	EPwm3Regs.DBFED = 25;
   
	EPwm3Regs.ETSEL.bit.INTSEL = 2;     //	Enable event time-base counter equal to period
	EPwm3Regs.ETSEL.bit.INTEN = 1;		//	Interrupt Enabled
	EPwm3Regs.ETPS.bit.INTPRD = 3;		//	Generate an interrupt on the first event

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	EDIS;
}
/**/

void EPwm4Timer_Config(void){
	Stop_PWM4();
	
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      	// Stop all the TB clocks
	EDIS;

	EPwm4Regs.TBPRD = MPPT_TBPRD;                	// Set timer period
	EPwm4Regs.TBPHS.half.TBPHS = 0x0000;           	// Phase is 0
	EPwm4Regs.TBCTR = 0x0000;                      	// Clear counter

	EPwm4Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN; 	// Count up
	EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;        	// Disable phase loading

	#if defined RT_MODE
		EPwm4Regs.TBCTL.bit.HSPCLKDIV = 1;
	#elif defined DBG_MODE_DIV2
		EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;
	#endif 

	EPwm4Regs.TBCTL.bit.CLKDIV = 1;
	
	EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;    	// Load registers every ZERO
	EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;
	EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
	EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;   

	EPwm4Regs.CMPA.half.CMPA = 0;
   
	EPwm4Regs.AQCTLA.bit.CAU = AQ_CLEAR;             	// Set PWM1A on Zero
	EPwm4Regs.AQCTLA.bit.CAD = AQ_SET;             	// Set PWM1A on Zero
//	EPwm4Regs.AQCTLA.bit.ZRO = AQ_SET;
//	EPwm4Regs.AQCTLB.bit.ZRO = AQ_TOGGLE;
	
	EPwm4Regs.DBCTL.bit.OUT_MODE = 2;
	EPwm4Regs.DBCTL.bit.POLSEL = 0;
	EPwm4Regs.DBCTL.bit.IN_MODE = 0;
	EPwm4Regs.DBRED = 25;
	EPwm4Regs.DBFED = 25;
   
	EPwm4Regs.ETSEL.bit.INTSEL = 2;     //	Enable event time-base counter equal to period
	EPwm4Regs.ETSEL.bit.INTEN = 1;		//	Interrupt Enabled
	EPwm4Regs.ETPS.bit.INTPRD = 3;		//	Generate an interrupt on the first event

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;
	EDIS;
}
/**/

void EPwm5Timer_Config(void){
	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;      // Stop all the TB clocks
	EDIS;

	EPwm5Regs.TBPRD = 180;
	EPwm5Regs.CMPA.half.CMPA = 177;
	EPwm5Regs.TBCTL.bit.CTRMODE = TB_COUNT_UP;
	EPwm5Regs.TBCTL.bit.PHSEN = TB_DISABLE;
	EPwm5Regs.TBCTL.bit.PRDLD = TB_SHADOW;
	EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_DISABLE;
	#if defined RT_MODE
		EPwm5Regs.TBCTL.bit.HSPCLKDIV = 2;
	#elif defined DBG_MODE_DIV2
		EPwm5Regs.TBCTL.bit.HSPCLKDIV = 1;
	#endif
	EPwm5Regs.TBCTL.bit.CLKDIV = 7;
	EPwm5Regs.ETSEL.bit.INTSEL = 4;
	EPwm5Regs.ETSEL.bit.INTEN = 1;
	EPwm5Regs.TBCTR = 0;
	EPwm5Regs.ETPS.bit.INTPRD = ET_1ST;

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1;         // Start all the timers synced
	EDIS;
	EPwm5Regs.TBCTL.bit.CTRMODE = 3;
}
/**/

void Stop_PWM1(void){
	EALLOW;
	EPwm1Regs.TZFRC.bit.OST = 1;
	EPwm1Regs.TZCTL.bit.TZA = TZ_FORCE_LO;
	EPwm1Regs.TZCTL.bit.TZB = TZ_FORCE_LO;
   	EDIS;
}
/**/

void Stop_PWM2(void){
	EALLOW;
	EPwm2Regs.TZFRC.bit.OST = 1;
	EPwm2Regs.TZCTL.bit.TZA = TZ_FORCE_LO;
	EPwm2Regs.TZCTL.bit.TZB = TZ_FORCE_LO;
   	EDIS;
}
/**/

void Stop_PWM3(void){
	EALLOW;
	EPwm3Regs.TZFRC.bit.OST = 1;
	EPwm3Regs.TZCTL.bit.TZA = TZ_FORCE_LO;
	EPwm3Regs.TZCTL.bit.TZB = TZ_FORCE_LO;
   	EDIS;
}
/**/

void Stop_PWM4(void){
	EALLOW;
	EPwm4Regs.TZCTL.bit.TZA = TZ_FORCE_LO; 
	EPwm4Regs.TZCTL.bit.TZB = TZ_FORCE_LO;	 
	EPwm4Regs.TZFRC.bit.OST = 1;
   	EDIS;
}
/**/

void Start_PWM1 (void){
   	EALLOW;
	EPwm1Regs.TZCLR.bit.OST = 1;
   	EDIS;
}
/**/

void Start_PWM2 (void){
   	EALLOW;
	EPwm2Regs.TZCLR.bit.OST = 1;
   	EDIS;
}
/**/

void Start_PWM3 (void){
   	EALLOW;
	EPwm3Regs.TZCLR.bit.OST = 1;
   	EDIS;
}
/**/

void Start_PWM4 (void){
   	EALLOW;
	EPwm4Regs.TZCLR.bit.OST = 1;
   	EDIS;
}
/**/
