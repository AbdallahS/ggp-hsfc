//=============================================================================
// Project: High Speed Forward Chaining
// Module: IO
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>
#include <vector>

#include "hsfcDefinition.h"

//=============================================================================
// CLASS: hsfcIO
//=============================================================================
class hsfcIO {

public:
	hsfcIO(void);
	~hsfcIO(void);

	void Initialise();
	void WriteToLog(int DetailLevel, bool Indent, const char* Text);
	void FormatToLog(int DetailLevel, bool Indent, const char* Format, const char* Arg1);
	void FormatToLog(int DetailLevel, bool Indent, const char* Format, const char* Arg1, const char* Arg2);
	void FormatToLog(int DetailLevel, bool Indent, const char* Format, const int Arg1, const char* Arg2);
	void FormatToLog(int DetailLevel, bool Indent, const char* Format, const int Arg1, const int Arg2);
	void FormatToLog(int DetailLevel, bool Indent, const char* Format, const double Arg1);

	hsfcParameters* Parameters;
	unsigned int LogIndent;

protected:

private:

};

