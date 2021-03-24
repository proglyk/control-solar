#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
#include "stdlib.h"

#define MPPT1_TIMER_INTSEL	(1<<1) //Enable event time-base counter equal to period
#define MPPT2_TIMER_INTSEL	(1<<1) //Enable event time-base counter equal to period
#define MPPT1_TIMER_INTPRD	(1<<1) //Enable event time-base counter equal to period
#define MPPT2_TIMER_INTPRD	(1<<1) //Enable event time-base counter equal to period

#define EPWM_TBPRD 690
#define EPWM_CMPA (EPWM_TBPRD / 2)

#define MPPT_TBPRD 410
#define MPPT_CMPA (MPPT_TBPRD / 2)

void EPwm1Timer_Config(void);
void EPwm2Timer_Config(void);
void EPwm3Timer_Config(void);
void EPwm4Timer_Config(void);
void EPwm5Timer_Config(void);
void Stop_PWM1(void);
void Stop_PWM2(void);
void Stop_PWM3(void);
void Stop_PWM4(void);
void Start_PWM1(void);
void Start_PWM2(void);
void Start_PWM3(void);
void Start_PWM4(void);
/**/
