
#pragma once

#include "Structures.h"


class Main_Alg
{
public:
	void	Init (void) ;														// allocate memory
	void	ShutDown (void) ;													// free memory
	void	Detect (const Input_Data& tInput, Output_Data& tOutput, int iID, const char* sCSV_Output, const char* sMain_CSV_Output) ;	// detect
} ;

