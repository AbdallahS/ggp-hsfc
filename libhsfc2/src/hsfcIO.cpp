//=============================================================================
// Project: High Speed Forward Chaining
// Module: IO
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include "hsfcIO.h"

using namespace std;

//=============================================================================
// CLASS: hsfcIO
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcIO::hsfcIO(void) {

	this->Parameters = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcIO::~hsfcIO(void) {

	// Free everything
	if (this->Parameters != NULL) delete this->Parameters;

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcIO::Initialise() {

	// Create the Parameters
	if (this->Parameters == NULL) this->Parameters = new hsfcParameters;

	// Set the default values
	this->Parameters->LogFileName = NULL;
	this->LogIndent = 0;
	this->Parameters->LogDetail = 1;
		// 0 - Silent
		// 1 - Basic progress messages
		// 2 -  + Detail messages
		// 3 -  + Debug messages
		// 4 -  + Full Audit

	this->Parameters->MaxRelationSize = 1000000;
	this->Parameters->MaxReferenceSize = 100000000;
	this->Parameters->MaxStateSize = 100000000;

	this->Parameters->LowSpeedOnly = false;
	this->Parameters->SCLOnly = false;
	this->Parameters->SchemaOnly = false;

}

//-----------------------------------------------------------------------------
// WriteToLog
//-----------------------------------------------------------------------------
void hsfcIO::WriteToLog(int DetailLevel, bool Indent, const char* Text) {

	FILE* LogFile;
	char Spaces[] = "                                 ";

	// Should we write this
	if (DetailLevel > this->Parameters->LogDetail) return;

	// Set up the indent
	if (this->LogIndent > 32) this->LogIndent = 32;
	Spaces[this->LogIndent] = 0;

	// Write to a file or the screen
	if (this->Parameters->LogFileName == NULL) {

		// Write to the screen
		if (Indent) printf("%s", Spaces);
		printf("%s", Text);

	} else {

		// Write to a file
		LogFile = fopen(this->Parameters->LogFileName, "a");
		if (LogFile != NULL) {
			if (Indent) fprintf(LogFile, "%s", Spaces);
			fprintf(LogFile, "%s", Text);
			fflush(LogFile);
			fclose(LogFile);
		}

		// Was this an error message
		if ((strstr(Text, "Error") != NULL) || (strstr(Text, "Warning") != NULL)) {
			if (Indent) printf("%s", Spaces);
			printf("%s", Text);
		}

	}

}

//-----------------------------------------------------------------------------
// FormatToLog
//-----------------------------------------------------------------------------
void hsfcIO::FormatToLog(int DetailLevel, bool Indent, const char* Format, const int Arg1, const char* Arg2) {

	char Text[1024];
	int MaxLength;

	// Should we write this
	if (DetailLevel > this->Parameters->LogDetail) return;

	// Are we likely to overrun
	MaxLength = 1;
	if (Format != NULL) MaxLength += strlen(Format);
	MaxLength += 16;
	if (Arg2 != NULL) MaxLength += strlen(Arg2);
	if (MaxLength > 1022) return;

	// Create a line of text
	sprintf(Text, Format, Arg1, Arg2);

	// Write to the log
	this->WriteToLog(DetailLevel, Indent, Text);

}

//--- Overload ----------------------------------------------------------------
void hsfcIO::FormatToLog(int DetailLevel, bool Indent, const char* Format, const char* Arg1) {

	char Text[1024];
	int MaxLength;

	// Should we write this
	if (DetailLevel > this->Parameters->LogDetail) return;

	// Are we likely to overrun
	MaxLength = 1;
	if (Format != NULL) MaxLength += strlen(Format);
	if (Arg1 != NULL) MaxLength += strlen(Arg1);
	if (MaxLength > 1022) return;

	// Create a line of text
	sprintf(Text, Format, Arg1);

	// Write to the log
	this->WriteToLog(DetailLevel, Indent, Text);

}

//--- Overload ----------------------------------------------------------------
void hsfcIO::FormatToLog(int DetailLevel, bool Indent, const char* Format, const char* Arg1, const char* Arg2) {

	char Text[1024];
	int MaxLength;

	// Should we write this
	if (DetailLevel > this->Parameters->LogDetail) return;

	// Are we likely to overrun
	MaxLength = 1;
	if (Format != NULL) MaxLength += strlen(Format);
	if (Arg1 != NULL) MaxLength += strlen(Arg1);
	if (Arg2 != NULL) MaxLength += strlen(Arg2);
	if (MaxLength > 1022) return;

	// Create a line of text
	sprintf(Text, Format, Arg1, Arg2);

	// Write to the log
	this->WriteToLog(DetailLevel, Indent, Text);

}

//--- Overload ----------------------------------------------------------------
void hsfcIO::FormatToLog(int DetailLevel, bool Indent, const char* Format, const int Arg1, const int Arg2) {

	char Text[1024];
	int MaxLength;

	// Should we write this
	if (DetailLevel > this->Parameters->LogDetail) return;

	// Are we likely to overrun
	MaxLength = 1;
	if (Format != NULL) MaxLength += strlen(Format);
	MaxLength += 16;
	MaxLength += 16;
	if (MaxLength > 1022) return;

	// Create a line of text
	sprintf(Text, Format, Arg1, Arg2);

	// Write to the log
	this->WriteToLog(DetailLevel, Indent, Text);

}

//--- Overload ----------------------------------------------------------------
void hsfcIO::FormatToLog(int DetailLevel, bool Indent, const char* Format, const double Arg1) {

	char Text[1024];
	int MaxLength;

	// Should we write this
	if (DetailLevel > this->Parameters->LogDetail) return;

	// Are we likely to overrun
	MaxLength = 1;
	if (Format != NULL) MaxLength += strlen(Format);
	MaxLength += 16;
	if (MaxLength > 1022) return;

	// Create a line of text
	sprintf(Text, Format, Arg1);

	// Write to the log
	this->WriteToLog(DetailLevel, Indent, Text);

}

