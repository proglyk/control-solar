#ifndef _FAULTS_H_
#define _FAULTS_H_

#ifdef __cplusplus
 extern "C" {
#endif 

#include "DSP280x_Device.h"     // DSP280x Headerfile Include File
#include "DSP280x_Examples.h"   // DSP280x Examples Include File


struct DRVFAULTS_BITS {
	// сигналы с платы драйверов
	Uint16 INV_A1:1;		//0
	Uint16 INV_A2:1;		//1
	Uint16 INV_A3:1;		//2
	Uint16 INV_A4:1;		//3
	Uint16 MPPT_1:1;		//4
	Uint16 MPPT_2:1;		//5
};

union DRVFAULTS 
{
	Uint16 								all;
	struct DRVFAULTS_BITS	bit;
};

struct sFaults {
	union DRVFAULTS uDrvFlt;
} ;

void
	main_FaultInvCheck(void);
	
void
	main_FaultMpptCheck(void);


#ifdef __cplusplus
}
#endif

#endif /* _FAULTS_H_ */

