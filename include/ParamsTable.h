#ifndef _PARAMSTABLE_H_
#define _PARAMSTABLE_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
#include "stdlib.h"
#include "global.h"
#include "DataConvert.h"

#include "RegulatorPI.h"

#define PAGE_0 0
#define PAGE_1 1

#define CH1 0
#define CH2 1

#define MODBUS_REG_TIME1 			0
#define MODBUS_REG_TIME2 			1
#define MODBUS_REG_DATE1 			2
#define MODBUS_REG_DATE2 			3
#define MODBUS_REG_0 				4
#define MODBUS_REG_1 				5
#define MODBUS_REG_2 				6
#define MODBUS_REG_INVERTER 		7
#define MODBUS_REG_MPPT1 			8
#define MODBUS_REG_MPPT2 			9

#define STARTSTOP 					10
#define MODBUS_ADC_TSTB 			11	// A0:	tempBattery		TSTB	v
#define MODBUS_ADC_TRAD 			12	// A2:	tempCooler		TRAD	v
#define MODBUS_ADC_220VN			13	// A3:	voltageLoad		~220VN
#define MODBUS_ADC_ISTBN			14	// A4:	currentBattery	ISTBN
#define MODBUS_ADC_TTR 				15	// A5:	tempTransformer	TTR		v	
#define MODBUS_ADC_STBN 			16	// A6:  VoltageBattery	+STBN
#define MODBUS_ADC_IINVN			17	// A7:	currentBridge	IINVN
#define MODBUS_ADC_I220N 			18	// B0:	currentLoad		I220N
#define MODBUS_ADC_IO2N				19	// B1:	currentMPPT2	IO2N

#define MODBUS_ADC_SB2N					20	// B4:	voltageMPPT2	+SB2N
#define MODBUS_ADC_SB1N 				21	// B5:	voltageMPPT1	+SB1N
#define MODBUS_ADC_IO1N 				22	// B6:	currentMPPT1	IO1N
#define MODBUS_SOLBAT_CH1_U_MPP			23
#define MODBUS_SOLBAT_CH1_U_XX			24
#define MODBUS_SOLBAT_CH1_NUM_SERIAL	25
#define MODBUS_SOLBAT_CH1_NUM_PARAL		26
#define MODBUS_SOLBAT_CH1_I_KZ			27
#define MODBUS_SOLBAT_CH1_I_MPP			28
#define MODBUS_SOLBAT_CH1_MODE			29

#define MODBUS_REG_DEBUG_TEMP0			30
#define MODBUS_REG_DEBUG_TEMP1			31
#define MODBUS_REG_DEBUG_TEMP2			32

#define MODBUS_INVERTER_UOUT_SET		33
#define MODBUS_REG_DEBUG_TEMP3			34
#define MODBUS_RELAY_P2					35
#define MODBUS_RELAY_P3					36

#define MODBUS_SOLBAT_CH2_U_MPP			37
#define MODBUS_SOLBAT_CH2_U_XX			38
#define MODBUS_SOLBAT_CH2_NUM_SERIAL	39
#define MODBUS_SOLBAT_CH2_NUM_PARAL		40
#define MODBUS_SOLBAT_CH2_I_KZ			41
#define MODBUS_SOLBAT_CH2_I_MPP			42
#define MODBUS_SOLBAT_CH2_MODE			43



//#define REG_TEMP	23
//#define REG_U_MPPT_SET	24

#define MEMADDR(a) (Uint16)(2 * a)

#define DS3231_MEMADDR_SEC 	 (Uint8)0x0
#define DS3231_MEMADDR_MIN 	 (Uint8)0x1
#define DS3231_MEMADDR_HOUR	 (Uint8)0x2
#define DS3231_MEMADDR_DAY 	 (Uint8)0x4
#define DS3231_MEMADDR_MONTH (Uint8)0x5
#define DS3231_MEMADDR_YEAR  (Uint8)0x6

typedef enum
{
	CM_UNK = 0,
	RUN,
	STOP,
	FAULT,
	NO485
}
CurrentMode_t;

typedef enum
{
	V_LOAD_UNK = 0,
	V_LOAD_180,
	V_LOAD_220,
	V_LOAD_230,
	V_LOAD_250
}
VLoadType_t;

typedef enum {
	CHARGESTAGE_UNK = 0,
	CHARGESTAGE_DISABLED,
	CHARGESTAGE_START,		
	CHARGESTAGE_MPPT,		//3
	CHARGESTAGE_DCVOLTAGE	//4
} ChargeStage;

struct InverterControlBlock_st {
	bool b_Trig_PWM_Set;		//триггер
};

struct MpptControlBlock_st {
	bool bFirstAfterON; 		//триггер первый вызов после включения
	
};

typedef enum {
	MPP_DIS = 0,
	MPP_NORM,
	MPP_STAB,
	MPP_KZ
} MPP_t;

typedef MPP_t MpptStatus;


typedef struct {
	
	// Указатель на флаг работы текущего канала MPPT
	bool * 					pbEnable;
	// состояние текущего режима. (Норм, К.З. и т.д.)
	MpptStatus 			eStatus;
	
	// Указатель на регистр с кф.ШИМ
	int16 * 				psPwm;
	
	// Указатель на ПИ-регул. режима MPP
	RegulatorPi_t * pxMppPI;
	// Указатель на ПИ-регул. режима стабилизации напряж.
	RegulatorPi_t * pxUbatPI;
	// общий для обоих регуляторов интегратор
	_iq15 					iqMutualInt;
	
	// Указатель на регистр со значением текущего тока массива солн.батарей
	int16 * 				psIOxN;
	// Указатель на регистр со значением текущего напряжения массива солн.батарей
	int16 * 				psSBxN;
	// Указатель на регистр со значением тока MPP
	int16 * 				psIMPP;
	// Указатель на регистр со значением напряжения MPP
	int16 * 				psUMPP;
	// Указатель на регистр со значением тока К.З.
	int16 * 				psIKZ;
	// Указатель на регистр со значением тока MPP
	//int16 * p_reg_SOLBAT_I_MPP;
} SolarSource;


typedef SolarSource * SourcePtr;


typedef struct {
	//стадия заряда
	ChargeStage eStage;
	
	//Триггер шмидта
	struct Schmitt_st stUBatSet;
	
	// Указатель на регистр со значением напряжения аккумулятора
	int16 * 				psSTBN;
	// используется временно как ссылка на текущую уставку номинального напряжения
	// полностью заряженного АКБ. В ходе испытаний выяснилось, что удобно менять
	// real-time данную уставку, чтобы отследить работу режима стаблилизации Ubat
	// инициализируется каким-то регистром, который входит в обмен пульта
	// В данный момент этот регистр Params.Page0.Reg_SOLBAT_CH1_U_XX
	int16 * 				psVBAT;	
} DcBattery;


typedef struct {
	//public members
	Uint16 (* pTrig)(Uint16);
	VLoadType_t VLoad;
	DcBattery 	xBattery;
	SourcePtr		pxSources[2];		// need to be initiated by links to xSource1,2
	//private members
	SolarSource xSource1;
	SolarSource xSource2;
} ChargeControl;


typedef struct {
	
	// Page0
	// Основное
	struct {
		Uint16 Reg0;										//0
		Uint16 Reg1;
		Uint16 Reg2;
		CurrentMode_t Mode;
		bool fBridge;
		bool fMPPT1;
		bool fMPPT2;
		int16 RegTSTB;
		int16 RegTRAD;
		int16 Reg220VN;									//9
		
		int16 RegISTBN;									//10
		int16 RegTTR;
		int16 RegSTBN;
		int16 RegIINVN;
		int16 RegI220N;
		int16 RegIO2N;
		int16 RegSB2N;
		int16 RegSB1N;
		int16 RegIO1N;
		int16 RegTemp0;									//19
		
		
		int16 RegTemp1;									//20
		int16 RegTemp2;									//21
		int16 RegChargeStage;						//22
		int16 Reg_SOLBAT_CH1_U_MPP;			//23
		int16 Reg_SOLBAT_CH1_U_XX;			//24 // временно ном. напряжение заряда АКБ 
		int16 Reg_SOLBAT_CH1_NUM_SERIAL;//25
		int16 Reg_SOLBAT_CH1_NUM_PARAL;	//26
		int16 Reg_SOLBAT_CH1_I_KZ;			//27
		int16 Reg_SOLBAT_CH1_I_MPP;			//28
		int16 Reg_INVERTER_UOUT_SET;		//29
		
		int16 RegTemp3;						
		int16 Reg_RELAY_P2;
		int16 Reg_RELAY_P3;					
		int16 Reg_SOLBAT_CH2_U_MPP;
		int16 Reg_SOLBAT_CH2_U_XX;
		int16 Reg_SOLBAT_CH2_NUM_SERIAL;
		int16 Reg_SOLBAT_CH2_NUM_PARAL;
		int16 Reg_SOLBAT_CH2_I_KZ;
		int16 Reg_SOLBAT_CH2_I_MPP;			//38
	} Page0;
	
	// Page1 
	// Х.З.
	struct {
		Uint16 Reg0;
		Uint16 Reg1;
		Uint16 Reg2;
	} Page1;
	
	// DateTime
	// Для RTC часов
	DateTime_t DateTime;
	
	// Новая структура. Соответсвует процедурам с пометкой "V3"
	ChargeControl ChargeControl;
	
} ParamsTable_t;


typedef struct
{
//	b PC_Connected;
//	t_bool Pult_Connected;
//	t_bool RT_Mode;
	bool DBG_Mode;

	bool fRelay1;
	bool fRelay2;
	bool fBridge;
	bool fMPPT1;
	bool fMPPT2;
	bool fBuckConv1;
	bool fBuckConv2;
	
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

}
PControl_t;

typedef struct
{
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
CrashNotice_t;

Uint16 Latch(Uint16 action);
void Params_WriteFromSerial(Uint16 page, Uint16 startpos, Uint16 len, Uint16 * pdata);

void
	Params__ChargeControlInit(ChargeControl *, ParamsTable_t *);

#endif
