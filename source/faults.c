#include "faults.h"

struct sFaults Faults;


void
	main_FaultInvCheck(void) {
	
	if (GpioDataRegs.GPADAT.bit.GPIO23)
		Faults.uDrvFlt.bit.INV_A1 = 1;
	else
		Faults.uDrvFlt.bit.INV_A1 = 0;
		
	if (GpioDataRegs.GPADAT.bit.GPIO8)
		Faults.uDrvFlt.bit.INV_A2 = 1;
	else
		Faults.uDrvFlt.bit.INV_A2 = 0;
	
	if (GpioDataRegs.GPADAT.bit.GPIO9)
		Faults.uDrvFlt.bit.INV_A3 = 1;
	else
		Faults.uDrvFlt.bit.INV_A3 = 0;
		
	if (GpioDataRegs.GPADAT.bit.GPIO22)
		Faults.uDrvFlt.bit.INV_A4 = 1;
	else
		Faults.uDrvFlt.bit.INV_A4 = 0;
}


void
	main_FaultMpptCheck(void) {
	
	if (GpioDataRegs.GPADAT.bit.GPIO13)
		Faults.uDrvFlt.bit.MPPT_1 = 1;
	else
		Faults.uDrvFlt.bit.MPPT_1 = 0;
		
	if (GpioDataRegs.GPADAT.bit.GPIO20)
		Faults.uDrvFlt.bit.MPPT_2 = 1;
	else
		Faults.uDrvFlt.bit.MPPT_2 = 0;
}

