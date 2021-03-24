#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"

//#define	GLOBAL_Q 19
#include "IQmathLib.h"



typedef enum
{
	false = 0,
	true = 1
}
bool;

#define FALSE false
#define TRUE true

#define VBAT 524 			//2.25*6*4 = 540.0 Напяжение-уставка полностью заряженной батареи
#define SCHMITT_NORMAL 0
#define SCHMITT_PLUS 1
#define SCHMITT_MINUS 2

struct Schmitt_st
{
	_iq15 value;
	_iq15 threshold;
	Uint16 type;
	bool stored;
};

typedef enum
{
	TYPE_NULL,
	TYPE_U16,
	TYPE_U8
}
Type_t;

typedef enum
{
	ERR_OK = 0,
	ERR_NULLREF,
	ERR_WRONGREF,
	ERR_CRC1,
	ERR_CRC2,
	ERR_CRC3
}
Error_t;

typedef enum
{
	Q_ERR_OK = 0,
	Q_ERR_OVRFL,
	Q_ERR_EMPTY
}
Qerror_t;

typedef unsigned char Uint8;



typedef enum{
	_true = 1,
	_false = 0,
	_CRCfalse = 2,
	_IllegalFunc = 3,
	_unk = 4
}t_bool;

typedef struct{
	t_bool PC_Connected;
	t_bool Pult_Connected;
//	t_bool RT_Mode;
	t_bool DBG_Mode;
	t_bool fRelay1;
	t_bool fRelay2;
	t_bool fBridge;
	t_bool fMPPT1;
	t_bool fMPPT2;
	t_bool fBuckConv1;
	t_bool fBuckConv2;
	_iq pwBuckConv1;
	_iq pwBuckConv2;
	_iq pwBridge;
	Uint16 currentMPPT1;
	Uint16 currentMPPT2;
	Uint16 currentBridge;
	Uint16 currentBattery;
	Uint16 currentLoad;
	Uint16 voltageMPPT1;
	Uint16 voltageMPPT2;
	Uint16 voltageLoad;
	Uint16 voltageBattery;
	int16 tempBattery;
	int16 tempCooler;
	int16 tempTransformer;
	Uint16 powerOut;
	Uint16 powerIn;
	Uint16 chargeBat;
	Uint16 timeRem;
	
	Uint16 u16_Tmp0;
	Uint16 u16_Tmp1;
	Uint16 u16_Tmp2;
	Uint16 u16_Tmp3;
	
	Uint16 u16_Tmp4;
	Uint16 u16_Tmp5;
	Uint16 u16_Tmp6;
	Uint16 u16_Tmp7;
	
	Uint16 u16_Tmp8;
	Uint16 u16_Tmp9;
	Uint16 u16_Tmp10;
	Uint16 u16_Tmp11;
	
	Uint16 u16_Tmp12;
	Uint16 u16_Tmp13;
}
t_InverterState;

typedef struct{
	bool	Bridge;
	bool 	fBridge;
	bool	MPPT1;
	bool 	fMPPT1;
	bool	MPPT2;
	bool 	fMPPT2;
	bool	Battery;
	bool 	fBattery;
	bool	Load;
	bool 	fLoad;
}
crashNotice_t;

#endif
