#ifndef _ADC_H_
#define _ADC_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"

#define	_BRIDGE_CURRENT_SHUT_OFF(A,B)		_IQ13int(_IQ13mpy(_IQ13mpy(_IQ13(A),_IQ13(B)), _IQ13(16.4)))
#define	_BUCK_CONV_CURRENT_SHUT_OFF(A,B)	_IQ13int(_IQ13mpy(_IQ13mpy(_IQ13(A),_IQ13(B)), _IQ13(68.5)))
#define	_MAX_LOAD_CURRENT_VALUE				_IQ13(10000)
#define	_LOAD_CURRENT_SHUT_OFF(A)			_IQ13mpy(_IQ13(A),_MAX_LOAD_CURRENT_VALUE)
#define	_MAX_BATTERY_CURRENT_VALUE			_IQ13(20000)
#define	_BATTERY_CURRENT_SHUT_OFF(A)		_IQ13mpy(_IQ13(A),_MAX_BATTERY_CURRENT_VALUE)

#define ADC_SHCLK  0xf   // S/H width in ADC module periods = 16 ADC clocks
#define ADC_CKPS   0x1   // ADC module clock = HSPCLK/2*ADC_CKPS = 12.4416 MHz / (1*2) = 6.2208 Mhz

void ADC_Config_SOC(void);
void AdcIntServiceRoutine(void);
void Sensor(void);

#endif
