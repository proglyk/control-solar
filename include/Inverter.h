#ifndef _INVERTER_H_
#define _INVERTER_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
//#include "IQmathLib.h"
#include "global.h"
#include "epwm.h"
#include "stdlib.h"

#define IQ_INVERTER_PERIOD 		_IQ(EPWM_CMPA)
#define INVERTER_PERIOD 		EPWM_CMPA
#define DOUBLE_PI 				6.283185
#define IQ_DOUBLE_PI			_IQ(DOUBLE_PI)
#define DOUBLE_PI_DIVIDED 		0.017453
#define IQ_DOUBLE_PI_DIVIDED	_IQ(DOUBLE_PI_DIVIDED)



void Inverter_ServiceRoutine(void);
Uint16 SineInverting(_iq iqPulseWight);

#endif
