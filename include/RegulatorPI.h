#ifndef _REGULATORPI_
#define _REGULATORPI_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
#include "Global.h"

#define KDIV 50

#define S16_MAX (32767)
#define S16_MIN (-32768)

#define Kp (0.25)
#define KiVout (0.25)
#define KiImppt1 (75)
#define KiImppt2 (75)

#define LIMIT_MAX_VOUT 350	//умноженное на x1
#define LIMIT_MIN_VOUT 0
//#define LIMIT_MAX_IMPPT1 20	//умноженное на x10
//#define LIMIT_MIN_IMPPT1 0
#define LIMIT_MAX_UMPPT1 150	//умноженное на x10
#define LIMIT_MIN_UMPPT1 0
#define LIMIT_MAX_UMPPT2 150	//умноженное на x10
#define LIMIT_MIN_UMPPT2 0
#define LIMIT_MAX_UBAT 60	//умноженное на x10
#define LIMIT_MIN_UBAT 0


#define PITYPE_DIRECT 0
#define PITYPE_INVERS 1

typedef struct
{
  _iq15 Reference;		// уставка
  _iq15 Kp_Gain;		// пропорцилональная часть P
  _iq15 Ki_Gain;		// интегральная часть
  _iq15 Integral_Sum;	// интегратор
  _iq15 KDiv;			// делитель/частота вызова в секунду (например если проц.вызывается в таймере с
						// частотой 50 Гц, то KDiv = 50 )
  
  _iq15 Upper_Limit;	// Верхнее ограничение интегратора (не больше для данного типа, 32767 для Int16)
  _iq15 Lower_Limit;	// Нижнее ограничение интегратора (не меньше для данного типа, -32768 для Int16)
  
  _iq15 OutLimitMax;	// Ограничение выхода (аналог элемента Limiter в PSIM)
  _iq15 OutLimitMin;	// Ограничение выхода (аналог элемента Limiter в PSIM)
  
  bool Max_PI_Output;	// Флаг ограничения интегратора
  bool Min_PI_Output;	// Флаг ограничения интегратора
  
  bool Reset;			//Сброс интегратора в выключенном режиме //УБРАТЬ
  Uint16 PiType;		//Тип регулятор (Знак уставки, что из чего вычитается)	
  
} RegulatorPi_t;

typedef struct
{
	_iq15 Reference1;	// уставка 1-го параметра
	_iq15 Reference2;	// уставка 2-го параметра
	_iq15 Kp_Gain;		// пропорцилональная часть P
	_iq15 Ki_Gain;		// интегральная часть
	_iq15 Integral_Sum;	// интегратор
	_iq15 KDiv;			// делитель/частота вызова в секунду (например если проц.вызывается в таймере с
						// частотой 50 Гц, то KDiv = 50 )
						
	_iq15 Upper_Limit1;	// Верхнее ограничение интегратора (не больше для данного типа, 32767 для Int16)
	_iq15 Lower_Limit1;	// Нижнее ограничение интегратора (не меньше для данного типа, -32768 для Int16)
	_iq15 Upper_Limit2;	// Верхнее ограничение интегратора (не больше для данного типа, 32767 для Int16)
	_iq15 Lower_Limit2;	// Нижнее ограничение интегратора (не меньше для данного типа, -32768 для Int16)
	
	_iq15 OutLimitMax1;	// Ограничение выхода (число для дальнейшего ограничения)
	_iq15 OutLimitMin1;	// Ограничение выхода (аналог элемента Limiter в PSIM)
	_iq15 OutLimitMax2;	// Ограничение выхода (число для дальнейшего ограничения)
	_iq15 OutLimitMin2;	// Ограничение выхода (аналог элемента Limiter в PSIM)
  
	bool Max_PI_Output;	// Флаг ограничения интегратора
	bool Min_PI_Output;	// Флаг ограничения интегратора
  
} RegulatorPi2Params_t;

_iq15 RegulatorPI(RegulatorPi_t* regul, _iq15 input);

#endif
