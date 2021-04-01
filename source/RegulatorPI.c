#include "RegulatorPI.h"

RegulatorPi_t stPI_VOUT = 
{
	_IQ15(0),
	_IQ15(Kp),
	_IQ15(KiVout),
	_IQ15(0),
	_IQ15(KDIV_INV),
	
	_IQ15(LIMIT_MAX_VOUT * KDIV_INV),
	_IQ15(LIMIT_MIN_VOUT * KDIV_INV),
	
	_IQ15(LIMIT_MAX_VOUT),
	_IQ15(LIMIT_MIN_VOUT),
	
	FALSE,
	FALSE,
	
	FALSE,
	PITYPE_DIRECT
};
RegulatorPi_t* pPiVout = &stPI_VOUT;

RegulatorPi_t stPI_UMPPT1 = 
{
	_IQ15(0),
	_IQ15(Kp),
	_IQ15(KiImppt1),
	_IQ15(0),
	_IQ15(KDIV_MPPT),
	
	_IQ15(LIMIT_MAX_UMPPT1 * KDIV_MPPT),
	_IQ15(0),
	
	_IQ15(LIMIT_MAX_UMPPT1),
	_IQ15(0),
	
	FALSE,
	FALSE,
	
	TRUE,
	PITYPE_INVERS
};
RegulatorPi_t* pPiUMppt1 = &stPI_UMPPT1;

RegulatorPi_t stPI_UBAT1 = 
{
	_IQ15(VBAT),
	_IQ15(Kp),
	_IQ15(KiImppt1),
	_IQ15(0),
	_IQ15(KDIV_MPPT),
	
	_IQ15(LIMIT_MAX_UBAT * KDIV_MPPT),
	_IQ15(0),
	
	_IQ15(LIMIT_MAX_UBAT),
	_IQ15(0),
	
	FALSE,
	FALSE,
	
	TRUE,
	PITYPE_DIRECT
};
RegulatorPi_t* pPiUBat1 = &stPI_UBAT1;

RegulatorPi_t stPI_UMPPT2 = 
{
	_IQ15(0),
	_IQ15(Kp),
	_IQ15(KiImppt2),
	_IQ15(0),
	_IQ15(KDIV_MPPT),
	
	_IQ15(LIMIT_MAX_UMPPT2 * KDIV_MPPT),
	_IQ15(0),
	
	_IQ15(LIMIT_MAX_UMPPT2),
	_IQ15(0),
	
	FALSE,
	FALSE,
	
	TRUE,
	PITYPE_INVERS
};
RegulatorPi_t* pPiUMppt2 = &stPI_UMPPT2;

RegulatorPi_t stPI_UBAT2 = 
{
	_IQ15(VBAT),
	_IQ15(Kp),
	_IQ15(KiImppt2),
	_IQ15(0),
	_IQ15(KDIV_MPPT),
	
	_IQ15(LIMIT_MAX_UBAT * KDIV_MPPT),
	_IQ15(0),
	
	_IQ15(LIMIT_MAX_UBAT),
	_IQ15(0),
	
	FALSE,
	FALSE,
	
	TRUE,
	PITYPE_DIRECT
};
RegulatorPi_t* pPiUBat2 = &stPI_UBAT2;

_iq15
RegulatorPI(RegulatorPi_t* regul, _iq15 input)
{
	_iq15 Proportional_Term, Integral_Term, output;
	_iq15 Error;
	
	// 1.) Error
	if (regul->PiType == PITYPE_DIRECT)
	{
		Error = ( regul->Reference - input );
	}
	else
	{
		Error = ( input - regul->Reference );
	}
	
	
	// 2.) P
	Proportional_Term = _IQ15mpy(regul->Kp_Gain, Error);
	
	// 3.) I
	//if ((regul->Ki_Gain == 0) | (regul->Reset))
	if (regul->Ki_Gain == 0)
	{
		regul->Integral_Sum = 0;
	}
	else
	{
		Integral_Term = _IQ15div(Error, regul->Ki_Gain);
		
		if ( (regul->Integral_Sum >= 0) && (Integral_Term >= 0) && (regul->Max_PI_Output == FALSE) )  // freeze integral term in case of over/underflow
		{
			if ( (regul->Integral_Sum + Integral_Term) > regul->Upper_Limit )
			{
				regul->Integral_Sum = regul->Upper_Limit;
			}
			else
			{
				regul->Integral_Sum += Integral_Term;  
			}
		}
		else if ( (regul->Integral_Sum <= 0) && (Integral_Term <= 0) && (regul->Min_PI_Output == FALSE) )
		{
			if ( (regul->Integral_Sum + Integral_Term) < regul->Lower_Limit ) 
			{
				regul->Integral_Sum = regul->Lower_Limit;
			}
			else  
			{
				regul->Integral_Sum += Integral_Term;  
			}
		}
		else if ( (regul->Integral_Sum <= 0) && (Integral_Term >= 0) )
		{
			regul->Integral_Sum += Integral_Term;  
		}
		else if ( (regul->Integral_Sum >= 0) && (Integral_Term <= 0) )
		{
			regul->Integral_Sum += Integral_Term;  
		}
	}
	
	// 4.) Result
	output = ( _IQ15div(Proportional_Term, regul->KDiv) + _IQ15div(regul->Integral_Sum, regul->KDiv) );
	
	
	// 5.) Limitation
	if ( output >= regul->Upper_Limit )
    {
		regul->Max_PI_Output = TRUE;
		return(regul->Upper_Limit);		  			 	
    }
    else if ( output < regul->Lower_Limit )
    {
		regul->Min_PI_Output = TRUE;
		return(regul->Lower_Limit);
    }
    else 
    {
		regul->Min_PI_Output = FALSE;
		regul->Max_PI_Output = FALSE;
		return(output); 		
    }
}
