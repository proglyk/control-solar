#include "Temperature.h"

static Uint16
mas_voltage_b57891[23] =
{
	1291,	//0
	1276,
	1257,
	1233,
	1203,
	1167,
	1123,
	1073,
	1016,
	952,
	884,
	813,
	741,
	668,
	598,
	531,
	470,
	415,
	363,
	316,
	276,
	241,
	209		//22
};


static Uint16
mas_voltage_b57045[23] = 
{
	1810,	//0
	1782,
	1745,
	1698,
	1640,
	1570,
	1488,
	1395,
	1293,
	1184,
	1071,
	957,
	847,
	743,
	647,
	559,
	482,
	413,
	354,
	303,
	259,
	221,
	189		//22
};


static const int16
mas_temperature[23] = 
{
	-55, -50,	//0-1
	-45, -40,	//2-3
	-35, -30,	//4-5
	-25, -20,	//6-7
	
	-15, -10,	//8-9
	-5, 0,		//10-11
	5, 10, 		//12-13
	15, 20, 	//14-15
	
	25, 30, 	//16-17
	35, 40, 	//18-19
	45, 50, 	//20-21
	55			//22
};


int16
give_temperature(int16 in_adc_val, Uint16 type)
{
	int16 tmp_v = 0, tmp_t = 0, tmp0, tmp1;
	Uint8 i;
	static bool flag = false;
	Uint16* p_voltg = NULL;
	
	if (type == 57891){
		p_voltg = &(mas_voltage_b57891[0]);
	}
	else if (type == 57045){
		p_voltg = &(mas_voltage_b57045[0]);
	}
	else{
		return 0;
	}
	
	tmp_v = _IQ13int(_IQ13mpy(_IQ13div(_IQ13(3000), _IQ13(4095)), _IQ13(in_adc_val)));
	
	for (i = 0; i < 23; i++){
		if (tmp_v < p_voltg[i]){
			if (i == 22){
				tmp_t = mas_temperature[22];
			}
		}
		else{
			if (i == 0){
				tmp_t = mas_temperature[0];
				
			}
			else if (flag == false){
				tmp0 = p_voltg[i-1] - p_voltg[i];
				tmp1 = tmp_v - p_voltg[i];
				tmp_t = mas_temperature[i] - (tmp1*5 / tmp0);
				flag = true;
			}
		}
	}
	flag = false;
	return tmp_t;
}

