//=============================================================================
// Project: High Speed Forward Chaining
// Module: Well Formed Text
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcWFT.h"

//=============================================================================
// CLASS: hsfcWFTElement
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcWFTElement::hsfcWFTElement(hsfcLexicon* Lexicon) {

	// Create the Element
	this->Lexicon = Lexicon;
	this->Parent = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcWFTElement::~hsfcWFTElement(void) {

	// Delete any children
	this->DeleteChildren();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcWFTElement::Initialise(const char* Script, int Length) {

	char* Text;

	// Delete any children
	this->DeleteChildren();
	this->Parent = NULL;
	this->Level = 0;

	// Add the text to the lexicon
	if (Length <= 0) {
		this->LexiconIndex = 0;
	} else {
		Text = new char[Length + 1];
		strncpy(Text, Script, Length);
		Text[Length] = 0;
		this->LexiconIndex = this->Lexicon->Index(Text);
		delete[] Text;
	}

}

//-----------------------------------------------------------------------------
// AddChild
//-----------------------------------------------------------------------------
hsfcWFTElement* hsfcWFTElement::AddChild(const char* Script, int Length) {

	hsfcWFTElement* NewElement;

	// Create the new element
	NewElement = new hsfcWFTElement(this->Lexicon);
	NewElement->Initialise(Script, Length);
	NewElement->Level = this->Level + 1;
	NewElement->Parent = this;

	// Add the new element to the end of the children
	this->Child.push_back(NewElement);

	return NewElement;

}

//-----------------------------------------------------------------------------
// RemoveComments
//-----------------------------------------------------------------------------
void hsfcWFTElement::RemoveComments(const char* Prefix) {

	// Check each child
	for (unsigned int i = 0; i < this->Child.size(); i++) {
		// Is it a comment
		if (strchr(Prefix, this->Lexicon->Text(this->Child[i]->LexiconIndex)[0]) != NULL) {
			delete this->Child[i];
			this->Child.erase(this->Child.begin() + i);
			i--;
		}
	}

	// Check all of the children
	for (unsigned int i = 0; i < this->Child.size(); i++) {
		this->Child[i]->RemoveComments(Prefix);
	}

}

//-----------------------------------------------------------------------------
// TextLength
//-----------------------------------------------------------------------------
int hsfcWFTElement::TextLength() {

	int Result = 0;

	// Calculate the length of the text including '()'
	if (Level != -1) {
		if (this->LexiconIndex == 0) Result += 3;
		if (this->LexiconIndex != 0) Result += strlen(this->Lexicon->Text(this->LexiconIndex)) + 1;
	}
	for (unsigned int i = 0; i < this->Child.size(); i++) {
		Result += this->Child[i]->TextLength();
	}

	return Result;

}
//-----------------------------------------------------------------------------
// AsText
//-----------------------------------------------------------------------------
char* hsfcWFTElement::AsText(char* Text, bool Space) {

	char* Index;

	// Print the text
	Index = Text;
	if ((Level != -1) && (this->LexiconIndex == 0)) {
		if (Space) Index += sprintf(Index, " ");
		Index += sprintf(Index, "%s", "(");
	}
	if ((Level != -1) && (this->LexiconIndex != 0)) {
		if (Space) Index += sprintf(Index, " ");
		Index += sprintf(Index, "%s", this->Lexicon->Text(this->LexiconIndex));
	}
	for (unsigned int i = 0; i < this->Child.size(); i++) {
		if (i == 0) {
			Index = this->Child[i]->AsText(Index, false);
		} else {
			Index = this->Child[i]->AsText(Index, true);
		}
	}
	if ((Level != -1) && (this->LexiconIndex == 0)) {
		Index += sprintf(Index, "%s", ")");
	}
	
	return Index;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcWFTElement::Print() {

	// Print the opening bracket
	if ((this->Level != -1) && (this->LexiconIndex == 0)) {
		this->Lexicon->IO->LogIndent = 2 * this->Level; 
		this->Lexicon->IO->WriteToLog(0, true, "(\n"); 
	}
	// Print the element
	if (this->LexiconIndex != 0) {
		this->Lexicon->IO->LogIndent = 2 * this->Level; 
		this->Lexicon->IO->FormatToLog(0, true, "%s\n", this->Lexicon->Text(this->LexiconIndex)); 
	}
	// Print the children
	for (unsigned int i = 0; i < this->Child.size(); i++) {
		this->Child[i]->Print();
	}
	// Print the closing bracket
	if ((this->Level != -1) && (this->LexiconIndex == 0)) {
		this->Lexicon->IO->LogIndent = 2 * this->Level; 
		this->Lexicon->IO->WriteToLog(0, true, ")\n"); 
	}

}

//-----------------------------------------------------------------------------
// DeleteChildren
//-----------------------------------------------------------------------------
void hsfcWFTElement::DeleteChildren() {

	// Delete any children
	for (unsigned int i = 0; i < this->Child.size(); i++) {
		if (this->Child[i] != NULL) delete this->Child[i];
	}
	this->Child.clear();

}

//=============================================================================
// CLASS: hsfcTextStructure
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcTextStructure::hsfcTextStructure(hsfcLexicon* Lexicon) {

	// Create the Structure
	this->Lexicon = Lexicon;
	this->RootElement = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcTextStructure::~hsfcTextStructure(void) {

	if (this->RootElement != NULL) delete this->RootElement;

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcTextStructure::Initialise() {

	// Delete any elements 
	if (this->RootElement == NULL) this->RootElement = new hsfcWFTElement(this->Lexicon);
	this->RootElement->Initialise(NULL, 0);
	this->RootElement->Level = -1;

}

//-----------------------------------------------------------------------------
// RemoveComments
//-----------------------------------------------------------------------------
void hsfcTextStructure::RemoveComments(const char* Prefix) {

	// Check each element
	this->RootElement->RemoveComments(Prefix);

}

//-----------------------------------------------------------------------------
// AddElement
//-----------------------------------------------------------------------------
hsfcWFTElement* hsfcTextStructure::AddElement(hsfcWFTElement* Parent, const char* Script, int Length) {

	return Parent->AddChild(Script, Length);

}

//-----------------------------------------------------------------------------
// AsText
//-----------------------------------------------------------------------------
char* hsfcTextStructure::AsText() {

	char* Text;
	int Length;

	// Print the text
	Length = this->RootElement->TextLength();
	Text = new char[Length + 1];
	this->RootElement->AsText(Text, false);
	Text[Length] = 0;

	return Text;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcTextStructure::Print() {

	// Print the element text
	this->RootElement->Print();

}


//=============================================================================
// CLASS: hsfcWFT
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcWFT::hsfcWFT(hsfcLexicon* Lexicon) {

	// Create the Element
	this->Lexicon = Lexicon;
	this->Structure = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcWFT::~hsfcWFT(void) {

	// Destroy any children
	if (this->Structure != NULL) {
		delete(this->Structure);
		this->Structure = NULL;
	}

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcWFT::Initialise() {

	// Create the Structure
	if (this->Structure == NULL) this->Structure = new hsfcTextStructure(this->Lexicon);

	// Remove any links
	this->Structure->Initialise();

}

//-----------------------------------------------------------------------------
// Load
//-----------------------------------------------------------------------------
void hsfcWFT::Load(const char* Script, const char* CommentPrefix) {

	int Index;
	const char* Start;
	int Length;
	hsfcWFTElement* Parent;

	// Rules for parsing
	// {/ # ; : '} . . . . . {\r \n} = comment
	// "literal"
	// {space spacespace} delimiter
	// ( indent
	// ) outdent

	// Process the script
	Parent = this->Structure->RootElement;
	Start = Script;
	while (Start[0] != 0) {

		// Ignore leading white space or control characters, but not end of text marker
		while (((Start[0] <= ' ') || (Start[0] > '~')) && (Start[0] != 0)) {
			Start++;
		}

		// Have we reached the end of the text
		if (Start[0] == 0) break;

		// Parse the script 
		Index = 0;
		Length = 0;

		// Does the first character signal a comment
		if (strchr(CommentPrefix, Start[0]) != NULL) {
			// Its a comment so look for an end of line marker
			while ((Start[Index] != 10) && (Start[Index] != 13) && (Start[Index] != 0)) {
				Index++;
			}
			Length = Index;
			goto ProcessSubString;
		}

		// Does the first character signal a literal
		if (Start[0] == '"') {
			// Its a literal so look for an end of line marker
			Index++;
			while ((Start[Index] != '"') && (Start[Index] != 0)) {
				Index++;
			}
			Length = Index + 1;
			goto ProcessSubString;
		}

		// Its something meaningful, is it a word or a bracket
		if ((Start[Index] == '(') || (Start[Index] == ')')) {
			Length = 1;
		} else {
			// Its a word, so find the end
			while ((Start[Index] != '(') && (Start[Index] != ')') && (Start[Index] > ' ') && (Start[Index] != 0)) {
				Index++;
			}
			Length = Index;
		}

ProcessSubString:

		// Choose what to do
		switch (Start[0]) {
			case ')':
				if (Parent != this->Structure->RootElement) Parent = Parent->Parent;
				break;
			case '(':
				Parent = this->Structure->AddElement(Parent, NULL, 0);
				break;
			default:
				this->Structure->AddElement(Parent, Start, Length);
		}

		// Reposition the start
		Start = Start + Length;

	}

	// Print the WFT
	if (this->Lexicon->IO->Parameters->LogDetail > 3) {
		this->Lexicon->IO->LogIndent = 0;
		this->Print();
	}


}

//-----------------------------------------------------------------------------
// ReadFile
//-----------------------------------------------------------------------------
void hsfcWFT::ReadFile(const char* FileName, const char* CommentPrefix) {

	int Length;
	char Letter;
	FILE* InputFile;
	char* Script;
	int FileSize;

	// Initialise the WFT
	this->Initialise();
	this->Lexicon->IO->FormatToLog(2, true, "Reading File '%s'\n", FileName);

	// Open the input file
	InputFile = fopen(FileName, "r");
	if (InputFile == NULL) {
		this->Lexicon->IO->FormatToLog(0, false, "Error: File does not exist '%s'\n", FileName);
		// Load an empty string
	    Script = new char[1];
		Script[0] = 0;
		this->Load(Script, CommentPrefix);
		// Clean up
		delete[] Script;
		return;
	}

    // Find the filesize
    fseek(InputFile, 0, SEEK_END);
    FileSize = ftell(InputFile);
	this->Lexicon->IO->FormatToLog(2, true, "File size = %d\n", FileSize);
    rewind(InputFile);

    // Load the file into memory
    Script = new char[FileSize + 1];
	// Get the description from the file
	Length = 0;
	while ((!feof(InputFile)) && (Length < FileSize)) {

		// Read a letter from the file
		fscanf(InputFile, "%c", &Letter);
		Script[Length] = Letter;
		Length++;

	}

	Script[Length] = 0;
	fclose(InputFile);
	this->Lexicon->IO->FormatToLog(2, true, "%d bytes read\n", Length);

	// Load the script
	this->Load(Script, CommentPrefix);
	delete[] Script;

}

//-----------------------------------------------------------------------------
// RemoveComments
//-----------------------------------------------------------------------------
void hsfcWFT::RemoveComments(const char* Prefix) {

	// Check each element
	this->Structure->RemoveComments(Prefix);

	// Print the WFT
	if (this->Lexicon->IO->Parameters->LogDetail > 3) {
		this->Lexicon->IO->LogIndent = 0;
		this->Print();
	}

}

//-----------------------------------------------------------------------------
// AsText
//-----------------------------------------------------------------------------
char* hsfcWFT::AsText() {

	return this->Structure->AsText();

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcWFT::Print() {

	// Print the element text
	this->Lexicon->IO->WriteToLog(0, false, "\n--- WFT ---\n"); 
	this->Structure->Print();

}

