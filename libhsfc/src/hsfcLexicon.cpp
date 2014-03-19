//=============================================================================
// Project: High Speed Forward Chaining
// Module: Lexicon
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include "hsfcLexicon.h"

using namespace std;

//=============================================================================
// CLASS: hsfcLexicon
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcLexicon::hsfcLexicon(void){

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcLexicon::~hsfcLexicon(void){

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcLexicon::Initialise(){

	// Reset the terms
	this->Term.clear();
	this->TermIndex.clear();

	// Set the zeroth term
	this->Term.push_back("NULL");
	this->TermIndex.push_back(0);

}

//-----------------------------------------------------------------------------
// Index
//-----------------------------------------------------------------------------
int hsfcLexicon::Index(const char* Value) {

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare = -1;

    // Binary search; zeroth term is NULL
	LowerBound = 1;
	UpperBound = this->Term.size() - 1;

	// Look for the term according to its value
	while (LowerBound <= UpperBound) {
		Target = (LowerBound + UpperBound) / 2;
		Compare = strcmp(Value, this->Term[this->TermIndex[Target]].c_str());
		if (Compare == 0) return this->TermIndex[Target];
		if (Compare < 0) UpperBound = Target - 1;
		if (Compare > 0) LowerBound = Target + 1;
	}

	// Not found
	return this->AddTerm(Value);

}

//-----------------------------------------------------------------------------
// Parse
//-----------------------------------------------------------------------------
void hsfcLexicon::Parse(char* Text, vector<hsfcTuple>& Reference) {

	char* Parsed;
	int Length;
	hsfcTuple NewReference;

	// Clear the TermIndex
	Reference.clear();

	// Allocate the memory
	Length = strlen(Text);
	Parsed = new char[Length + 2];
	Parsed[0] = 0;

	// Copy the text blanking out the spaces and brackets
	for (int i = 0; i <= Length; i++) {
		if ((Text[i] == '(') || (Text[i] == ')') || (Text[i] == ' ')) {
			Parsed[i+1] = 0;
		} else {
			Parsed[i+1] = Text[i];
		}
	}
	Length = Length + 2;

	// Now get the terms and fill the argument array
	for (int i = 1; i < Length; i++) {
		// Is it the start of a term
		if ((Parsed[i-1] == 0) && (Parsed[i] != 0)) {
			NewReference.RelationIndex = -1;
			NewReference.ID = this->Index(&Parsed[i]);
			Reference.push_back(NewReference); // Add the term if its not already there
		}
	}

	// Free the memory
	delete Parsed;

}

//-----------------------------------------------------------------------------
// Match
//-----------------------------------------------------------------------------
bool hsfcLexicon::Match(int Index, char* Text) {

	// Is the text empty
	if (strlen(Text) == 0) return false;

	// Is the index valid
	if (Index >= (int)this->Term.size()) return false;
	if (Index < 0) return false;

	// Make the comparison
	return (strcmp(this->Term[Index].c_str(), Text) == 0);

}

//-----------------------------------------------------------------------------
// PartialMatch
//-----------------------------------------------------------------------------
bool hsfcLexicon::PartialMatch(int Index, char* Text) {

	unsigned int Length;
	
	// Is the text empty
	if (strlen(Text) == 0) return false;

	// Is the index valid
	if (Index >= (int)this->Term.size()) return false;
	if (Index < 0) return false;

	// Make the comparison
	Length = strlen(Text);
	if (Length > strlen(this->Term[Index].c_str())) Length = strlen(this->Term[Index].c_str());
	return (strncmp(this->Term[Index].c_str(), Text, Length) == 0);

}

//-----------------------------------------------------------------------------
// PartialMatch
//-----------------------------------------------------------------------------
bool hsfcLexicon::MatchText(int Index, char* Text) {

	unsigned int Length;
	
	// Is the text empty
	if (strlen(Text) == 0) return false;

	// Is the index valid
	if (Index >= (int)this->Term.size()) return false;
	if (Index < 0) return false;

	// Make the comparison
	Length = strlen(Text);
	return (strncmp(this->Term[Index].c_str(), Text, Length) == 0);

}

//-----------------------------------------------------------------------------
// IsVariable
//-----------------------------------------------------------------------------
bool hsfcLexicon::IsVariable(int Index) {

	// Is the index valid
	if (Index >= (int)this->Term.size()) return false;
	if (Index < 0) return false;

	// Make the comparison
	return (this->Term[Index].c_str()[0] == '?');

}

//-----------------------------------------------------------------------------
// Text
//-----------------------------------------------------------------------------
const char* hsfcLexicon::Text(int Index) {

	// Is the index valid
	if (Index >= (int)this->Term.size()) return false;
	if (Index < 0) return false;

    // Get the text
	return this->Term[Index].c_str();

}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::Size() {

	return this->Term.size();

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcLexicon::Print() {

	printf("\n--- Lexicon ------------------------------------------------\n");
    
    // Display the List
	for (unsigned int i = 0; i < this->Term.size(); i++) {
		printf(" %4d - %s\n", i, this->Term[i].c_str()); 
    }

}

//-----------------------------------------------------------------------------
// AddTerm
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::AddTerm(const char* Value) {

	unsigned int Target;
	unsigned int Index;

    // Look for the term according to its value
	//Target = this->Index(Value);
	//if (Target > 0) return Target;
	
	// Insert the term at the end
	this->Term.push_back(Value);
	this->TermIndex.push_back(0);

	// Index the new term
	Index = this->Term.size() - 1;
	for (Target = 1; Target < this->Term.size(); Target++) {
		if (strcmp(Value, this->Term[this->TermIndex[Target]].c_str()) < 0) break;
	}
	for (unsigned int j = this->Term.size() - 1; j > Target; j--) {
		this->TermIndex[j] = this->TermIndex[j - 1];
	}

	// Add the new term to the index
	if (Target > this->Term.size() - 1) Target = this->Term.size() - 1;
	this->TermIndex[Target] = this->Term.size() - 1;

	return Index;

}

