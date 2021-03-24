#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
#include "stdlib.h"
#include "Global.h"

int16 give_temperature(int16 in_adc_val, Uint16 type);

#endif
