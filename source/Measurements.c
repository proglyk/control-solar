#include "Measurements.h"

#include "Temperature.h"
#include "ParamsTable.h"

static int16 Relay_type2_TEST2(_iq13 in_value, _iq13* in_average_value);

_iq13 iq_tempSensorValues[12] = {_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0)};
static _iq13 iq_avVal[12] = {_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0)};
static _iq13 iq_tmp_val[12] = {_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0)};
static _iq13 iq_prev_val[12] = {_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0)};
static _iq13 tmp_val_q13[12] = {_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0),_IQ13(0)};

extern PControl_t PControl;
extern ParamsTable_t Params;

void
Measurement(void)
{
	volatile Uint16	tmp0 = 0;
	volatile _iq13	tmp_val_q13s;
	volatile int16	not_used_tmp;	
	volatile int16	s16tmp;
	static Uint8 	cnt = 0;
	static Uint16 cnt1 = 0;
	
	if (cnt == 20)
	{
		iq_tmp_val[0] = _IQ13div(iq_avVal[0], _IQ13(20));
		iq_tmp_val[1] = _IQ13div(iq_avVal[1], _IQ13(20));
		iq_tmp_val[2] = _IQ13div(iq_avVal[2], _IQ13(20));
		iq_tmp_val[3] = _IQ13div(iq_avVal[3], _IQ13(20));
		iq_tmp_val[4] = _IQ13div(iq_avVal[4], _IQ13(20));
		iq_tmp_val[5] = _IQ13div(iq_avVal[5], _IQ13(20));
		iq_tmp_val[6] = _IQ13div(iq_avVal[6], _IQ13(20));
		iq_tmp_val[7] = _IQ13div(iq_avVal[7], _IQ13(20));
		iq_tmp_val[8] = _IQ13div(iq_avVal[8], _IQ13(20)) - _IQ13(2);
		if (iq_tmp_val[8] < _IQ13(0)) iq_tmp_val[8] = _IQ13(0);
		iq_tmp_val[9] = _IQ13div(iq_avVal[9], _IQ13(20));
		if (iq_tmp_val[9] < _IQ13(0)) iq_tmp_val[9] = _IQ13(0);
		iq_tmp_val[10] = _IQ13div(iq_avVal[10], _IQ13(20));
		if (iq_tmp_val[10] < _IQ13(0)) iq_tmp_val[10] = _IQ13(0);
		iq_tmp_val[11] = _IQ13div(iq_avVal[11], _IQ13(20)) - _IQ13(2);
		if (iq_tmp_val[11] < _IQ13(0)) iq_tmp_val[11] = _IQ13(0);
		
		/*************************************************************************************************/			
		
		// A0:	tempBattery		TSTB
		tmp_val_q13[0] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[0]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[0]);
		if (cnt1 == 0)
			Params.Page0.RegTSTB = give_temperature(_IQ13int(tmp_val_q13[0]), 57891);

/*************************************************************************************************/	
		
		// A2:	tempCooler		TRAD
		tmp_val_q13[1] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[1]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[1]);
		if (cnt1 == 0)
			Params.Page0.RegTRAD = give_temperature(_IQ13int(tmp_val_q13[1]), 57045);

/*************************************************************************************************/	
		
		// A3:	voltageLoad		~220VN
		tmp_val_q13s = _IQ13sqrt(iq_tmp_val[2]);
		
		tmp_val_q13[2] = _IQ13mpy(_IQ13(0.5), tmp_val_q13s) + _IQ13mpy(_IQ13(0.5), tmp_val_q13[2]);
		
		//tmp_val_q13s = _IQ13mpy(tmp_val_q13[2], _IQ13(11.8));
		//tmp_val_q13s = _IQ13mpy(tmp_val_q13[2], _IQ13(11.22));
		tmp_val_q13s = _IQ13mpy(tmp_val_q13[2], _IQ13(15.0));
		tmp_val_q13s = _IQ13div(tmp_val_q13s, _IQ13(71.12));	//кф
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));		//масштабирующий кф для пульта
		
		if (cnt1 == 0)
			Params.Page0.Reg220VN = (int16)(_IQ13int(tmp_val_q13s));
		
/*************************************************************************************************/
		
		// A4:	currentBattery	ISTBN
		tmp_val_q13[3] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[3]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[3]);
		
		tmp_val_q13s = _IQ13div(tmp_val_q13[3], _IQ13(16.4));	//кф
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));		//масштабирующий кф для пульта

		if (cnt1 == 0)
			Params.Page0.RegISTBN = (int16)(_IQ13int(tmp_val_q13s));
		
/*************************************************************************************************/			
		
		// A5:	tempTransformer	TTR	
		tmp_val_q13[4] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[4]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[4]);
		if (cnt1 == 0)
			Params.Page0.RegTTR = give_temperature(_IQ13int(tmp_val_q13[4]), 57891);

/*************************************************************************************************/	
		
		// A6: VoltageBattery	+STBN
		
		tmp_val_q13[5] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[5]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[5]);
		
		tmp_val_q13s = _IQ13div(tmp_val_q13[5], _IQ13(47.6));	// основной кф
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));		// масштабирующий кф для пульта
		
		// TEMPORARY
		if (cnt1 == 0)
			Params.Page0.RegSTBN = (int16)(_IQ13int(tmp_val_q13s));
		//Params.Page0.RegSTBN = 520;
		
/*************************************************************************************************/		
		// A7:	currentBridge	IINVN

		tmp_val_q13s = _IQ13mpy(_IQ13sqrt(iq_tmp_val[6]), _IQ13(15));
		
		tmp_val_q13[6] = _IQ13mpy(_IQ13(0.1), tmp_val_q13s) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[6]);
		
		//исходя из 2,5раза больше чем I_LOAD
		tmp_val_q13s = _IQ13div(tmp_val_q13[6] , _IQ13(17.2));	
		
		//функция искривления измерения с целью подогнать под AC щупы
		if (_IQ13int(tmp_val_q13s) > 80)
			tmp_val_q13s += _IQ13mpy(_IQ13mpy(tmp_val_q13s - _IQ13(80), _IQ13(0.01)), tmp_val_q13s); 
		
		//масштабирующий кф для пульта
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));		

		//замедлить обновление на экране
		if (cnt1 == 0)
			Params.Page0.RegIINVN = (int16)(_IQ13int(tmp_val_q13s));

/*************************************************************************************************/	
		
		// B0:	currentLoad		I220N
		
		tmp_val_q13s = _IQ13mpy(_IQ13sqrt(iq_tmp_val[7]), _IQ13(15));
		
		tmp_val_q13[7] = _IQ13mpy(_IQ13(0.1), tmp_val_q13s) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[7]);
		
		tmp_val_q13s = _IQ13div(tmp_val_q13[7], _IQ13(43.7));
		
		//функция искривления измерения с целью подогнать под AC щупы
		if (_IQ13int(tmp_val_q13s) > 80)
			tmp_val_q13s += _IQ13mpy(_IQ13mpy(tmp_val_q13s - _IQ13(80), _IQ13(0.01)), tmp_val_q13s); 
		
		//масштабирующий кф для пульта
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));
		
		//замедлить обновление на экране
		if (cnt1 == 0)
			Params.Page0.RegI220N = (int16)(_IQ13int(tmp_val_q13s));

/*************************************************************************************************/	
		
		// B1:	currentMPPT2	IO2N
		
		// фильтр
		tmp_val_q13[8] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[8]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[8]);
		
		//при 2,62 В вход АЦП = 3580
		tmp_val_q13s = _IQ13div(tmp_val_q13[8], _IQ13(189.5));	
		
		//масштабирующий кф для пульта
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));
		
		//замедлить обновление на экране
		if (cnt1 == 0)
			Params.Page0.RegIO2N = (int16)(_IQ13int(tmp_val_q13s));
		
/*************************************************************************************************/
		
		// B4:	voltageMPPT2	+SB2N
		
		// фильтр
		tmp_val_q13[9] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[9]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[9]);
		
		//при 2,62 В вход АЦП = 3580
		tmp_val_q13s = _IQ13div(tmp_val_q13[9], _IQ13(17));	
		
		//масштабирующий кф для пульта
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));
		
		// TEMPORARY
		if (cnt1 == 0) {
#if defined DEBUG
			Params.Page0.RegSB2N = 1500;
#else
			Params.Page0.RegSB2N = (int16)(_IQ13int(tmp_val_q13s));
#endif
		}
		//	Params.Page0.RegSB2N = (int16)(_IQ13int(tmp_val_q13s));
		//Params.Page0.RegSB2N = 1500;

/*************************************************************************************************/			
		
		// B5:	voltageMPPT1	+SB1N
		
		// фильтр
		tmp_val_q13[10] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[10]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[10]);
		
		//при 2,62 В вход АЦП = 3580
		tmp_val_q13s = _IQ13div(tmp_val_q13[10], _IQ13(17));	
		
		//масштабирующий кф для пульта
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));
		
		// TEMPORARY
		if (cnt1 == 0) {
#if defined DEBUG
			Params.Page0.RegSB1N = 1500;
#else
			Params.Page0.RegSB1N = (int16)(_IQ13int(tmp_val_q13s));
#endif
		}
		
/*************************************************************************************************/
		
		// B6:	currentMPPT1	IO1N
		
		// фильтр
		tmp_val_q13[11] = _IQ13mpy(_IQ13(0.1), iq_tmp_val[11]) + _IQ13mpy(_IQ13(0.90), tmp_val_q13[11]);
		
		//при 2,62 В вход АЦП = 3580
		tmp_val_q13s = _IQ13div(tmp_val_q13[11], _IQ13(189.5));	
		
		//масштабирующий кф для пульта
		tmp_val_q13s = _IQ13mpy(tmp_val_q13s, _IQ13(10));
		
		//замедлить обновление на экране
		if (cnt1 == 0)
			Params.Page0.RegIO1N = (int16)(_IQ13int(tmp_val_q13s));
		
/*************************************************************************************************/
		
		iq_avVal[0] = 0;
		iq_avVal[1] = 0;
		iq_avVal[2] = 0;
		iq_avVal[3] = 0;
		iq_avVal[4] = 0;
		iq_avVal[5] = 0;
		iq_avVal[6] = 0;
		iq_avVal[7] = 0;
		iq_avVal[8] = 0;
		iq_avVal[9] = 0;
		iq_avVal[10] = 0;
		iq_avVal[11] = 0;
		cnt = 0;
		
		if (cnt1 > 12)
			cnt1 = 0;
		else
			cnt1 += 1;
	}
	else
	{
		// Делители здесь исключительно чтобы не зайти в переполнение при вычислении среднеквадратичного 
		// (только для переменного тока). Если в уже усредненной величине
		// помножить на тот же коэфф., то получим на экране значение АЦП. Удобно для отладки
		
		iq_avVal[0] +=		iq_tempSensorValues[0];
		iq_avVal[1] +=		iq_tempSensorValues[1];
		//iq_avVal[2] +=		iq_tempSensorValues[2];
		
		tmp_val_q13s = 		_IQ13div(iq_tempSensorValues[2], _IQ13(15));
		iq_avVal[2] +=		_IQ13mpy(tmp_val_q13s, tmp_val_q13s);
		
		iq_avVal[3] +=		iq_tempSensorValues[3];
		//iq_avVal[3] += _IQ13div((_IQ13mpy(iq_avVal[3], _IQ13(31)) + iq_tempSensorValues[3]), _IQ13(32)); //вариант №1 Виталий Пиксаев. Ничо так, лучше чем усреднение /2 которое у меня было
		
		iq_avVal[4] +=		iq_tempSensorValues[4];
		iq_avVal[5] += 		iq_tempSensorValues[5];
		
		tmp_val_q13s = 		_IQ13div(iq_tempSensorValues[6], _IQ13(15));
		iq_avVal[6] +=		_IQ13mpy(tmp_val_q13s, tmp_val_q13s);
		
		//iq_avVal[7] +=		iq_tempSensorValues[7];
		tmp_val_q13s = 		_IQ13div(iq_tempSensorValues[7], _IQ13(15)); 
		iq_avVal[7] +=		_IQ13mpy(tmp_val_q13s, tmp_val_q13s);
		
		
		iq_avVal[8] += 		iq_tempSensorValues[8];
		iq_avVal[9] += 		iq_tempSensorValues[9];
		iq_avVal[10] +=		iq_tempSensorValues[10];
		iq_avVal[11] +=		iq_tempSensorValues[11];
		cnt++;
	}
}
