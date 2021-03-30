#ifndef _MAIN_H_
#define _MAIN_H_

#include "DSP280x_Device.h"
#include "DSP280x_Examples.h"
#include "stdlib.h"

#define RT_MODE
//#define DBG_MODE_DIV2

//#define DEBUG

/*
void System_Init(void);
void GPIO_Settings(void);
void ISR_Settings(void);
void GPIO34On(void);
void GPIO34Off(void);
void GPIO34TOG(void);

void GPIO9_On(void);
void GPIO9_Off(void);
void GPIO9_Tog(void);
void GPIO10_On(void);
void GPIO10_Off(void);
void GPIO10_Tog(void);
//void StartInit(void);
*/

void Timer0Int(void);
void Timer1Int(void);
void Timer2Int(void);
void DELAY_40US(void);
void DELAY_1MS(void);
void Invertor(void);

#endif
