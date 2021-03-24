#ifndef _MPPT_H_
#define _MPPT_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
#include "stdlib.h"
//#include "IQmathLib.h"
#include "Global.h"
#include "ParamsTable.h"

#define BAT_PMAX_VOLTAGE				_IQ(10)
#define ISR_FREQUENCY					20000
#define PSIM_I_VALUE					0.0005 
#define IQ_ISODROM_VALUE				_IQ((1/(float32)(ISR_FREQUENCY * PSIM_I_VALUE)))
#define REG1_P							0.5
#define	IQ_REG_UPLIMIT_PI				_IQ(30.1)
#define	IQ_REG_DOWNLIMIT_PI				_IQ(0)
#define	IQ_REG_UPLIMIT_I				_IQ(30.1)
#define	IQ_REG_DOWNLIMIT_I				_IQ(0)
#define IQ_RESOLUT_CF					_IQ(20.33)



void EPWM3_ServiceRoutine(void);
void EPWM4_ServiceRoutine(void);
_iq MPPT1(_iq, _iq);
_iq MPPT2(_iq, _iq);
void BuckConv_RegulateMPPT_vers1(_iq, _iq, int32*);
bool Schmitt( struct Schmitt_st * var, _iq15 in, int16 * psVbat);
void 
	Charge_MPPT_v2(struct BatteryControlBlock_st * p_con);

void
	Charge_MPPT_v3(ChargeControl * pxStruct, CurrentMode_t mode);

#endif
