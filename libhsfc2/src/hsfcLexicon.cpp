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
hsfcLexicon::hsfcLexicon(void) {

	this->IO = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcLexicon::~hsfcLexicon(void) {

	// Free resources
	if (this->IO != NULL) delete this->IO;

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcLexicon::Initialise() {

	// Create interface
	if (this->IO == NULL) this->IO = new hsfcIO();
	this->IO->Initialise();

	// Reset the terms
	this->Term.clear();
	this->TermIndex.clear();

	// Reset the relation names
	this->RelationName.clear();
	this->RelationNameID.clear();

	// Set the zeroth term
	this->Term.push_back("NULL");
	this->TermIndex.push_back(0);
	this->RelationName.push_back("Lexicon");
	this->RelationNameID.push_back(0);

}

//-----------------------------------------------------------------------------
// Index
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::Index(const char* Value) {

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
// RelationIndex
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::RelationIndex(const char* Value, bool Add) {

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare = -1;

    // Binary search; zeroth term is NULL
	LowerBound = 1;
	UpperBound = this->RelationName.size() - 1;

	// Look for the term according to its value
	while (LowerBound <= UpperBound) {
		Target = (LowerBound + UpperBound) / 2;
		Compare = strcmp(Value, this->RelationName[this->RelationNameID[Target]].c_str());
		if (Compare == 0) return this->RelationNameID[Target];
		if (Compare < 0) UpperBound = Target - 1;
		if (Compare > 0) LowerBound = Target + 1;
	}

	// Not found
	if (Add) {
		return this->AddName(Value);
	} else {
		return UNDEFINED;
	}

}

//-----------------------------------------------------------------------------
// RelationIndex
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::RelationIndex(unsigned int NameID) {

	return this->RelationIndex(this->Text(NameID), false);

}

//-----------------------------------------------------------------------------
// GDLIndex
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::GDLIndex(unsigned int ID) {

	unsigned int Result = 0;
	char* Text;
	char* GDLPredicate;
	int Length;

	// The SCL Name is in the form "predicate/arity:number:predicate/arity"
	// We want the last predicate
	Text = new char[strlen(this->Term[ID].c_str()) + 1];
	strcpy(Text, this->Term[ID].c_str());

	// Find the last ':'
	GDLPredicate = strrchr(Text, ':');
	if (GDLPredicate == NULL) {
		GDLPredicate = Text;
	} else {
		GDLPredicate++;
	}

	// Now extrqact the predicate
	Length = strlen(GDLPredicate);
	for (int i = 0; i < Length; i++) {
		if (GDLPredicate[i] == '/') GDLPredicate[i] = 0;
		break;
	}

	// Find the index for the GDL
	Result = this->Index(GDLPredicate);
	delete[] Text;

	return Result;

}

//-----------------------------------------------------------------------------
// Parse
//-----------------------------------------------------------------------------
void hsfcLexicon::Parse(const char* Text, vector<hsfcTuple>& Reference) {

	char* Parsed;
	int Length;
	hsfcTuple NewReference;

	// Clear the TermIndex
	Reference.clear();

	// Allocate the temporary memory
	Length = strlen(Text);
	Parsed = new char[Length + 2];
	Parsed[0] = 0;

	// Copy the text blanking out the spaces and brackets
	for (int i = 0; i <= Length; i++) {
		if ((Text[i] == '(') || (Text[i] == ')') || (Text[i] <= ' ')) {
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
			NewReference.Index = UNDEFINED;
			NewReference.ID = this->Index(&Parsed[i]);  // Add the term if its not already there
			Reference.push_back(NewReference); 
		}
	}

	// Free the memory
	delete Parsed;

}

//-----------------------------------------------------------------------------
// Match
//-----------------------------------------------------------------------------
bool hsfcLexicon::Match(unsigned int ID, const char* Text) {

	// Is the text empty
	if (strlen(Text) == 0) return false;

	// Is the index valid
	if (ID >= this->Term.size()) return false;

	// Make the comparison
	return (strcmp(this->Term[ID].c_str(), Text) == 0);

}

//-----------------------------------------------------------------------------
// PartialMatch
//-----------------------------------------------------------------------------
bool hsfcLexicon::PartialMatch(unsigned int ID, const char* Text) {

	unsigned int Length;
	
	// Is the text empty
	if (strlen(Text) == 0) return false;

	// Is the index valid
	if (ID >= this->Term.size()) return false;

	// Make the comparison
	Length = strlen(Text);
	return (strncmp(this->Term[ID].c_str(), Text, Length) == 0);

}

//-----------------------------------------------------------------------------
// MatchText
//-----------------------------------------------------------------------------
bool hsfcLexicon::MatchText(unsigned int ID, const char* Text) {

	unsigned int Length;
	
	// Is the text empty
	if (strlen(Text) == 0) return false;

	// Is the index valid
	if (ID >= this->Term.size()) return false;

	// Make the comparison
	Length = strlen(Text);
	return (strncmp(this->Term[ID].c_str(), Text, Length) == 0);

}

//-----------------------------------------------------------------------------
// IsVariable
//-----------------------------------------------------------------------------
bool hsfcLexicon::IsVariable(unsigned int ID) {

	// Is the index valid
	if (ID >= this->Term.size()) return false;

	// Make the comparison
	return (this->Term[ID].c_str()[0] == '?');

}

//-----------------------------------------------------------------------------
// IsVariable
//-----------------------------------------------------------------------------
bool hsfcLexicon::IsUsed(const char* Letter, bool IgnoreComments) {

	char FirstLetter[2] = "/";

	// Is the index valid
	for (unsigned int i = 0; i < this->Term.size(); i++) {
		if (IgnoreComments) {
			FirstLetter[0] = this->Term[i].c_str()[0];
			if (strstr("/#;:'", FirstLetter) == NULL) {
				if (strstr(this->Term[i].c_str(), Letter) != NULL) return true;
			}
		} else {
			if (strstr(this->Term[i].c_str(), Letter) != NULL) return true;
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// Text
//-----------------------------------------------------------------------------
const char* hsfcLexicon::Text(unsigned int ID) {

	// Is the index valid
	if (ID >= this->Term.size()) return this->Term[0].c_str();

    // Get the text
	return this->Term[ID].c_str();

}

//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------
char* hsfcLexicon::Copy(unsigned int ID, bool WithArity) {

	char* Result;
	char* Slash;

	// Is the index ok
	if (ID >= this->Term.size()) ID = 0;

	// Create the new string
	Result = new char[strlen(this->Term[ID].c_str()) + 1];

	// Copy
	strcpy(Result, this->Term[ID].c_str());

	// Remove the '/n' arity
	if (!WithArity) {
		Slash = strchr(Result, '/');
		if (Slash != NULL) Slash[0] = 0;
	}

	return Result;

}

//-----------------------------------------------------------------------------
// Relation
//-----------------------------------------------------------------------------
const char* hsfcLexicon::Relation(unsigned int ID) {

	// Is the index valid
	if (ID >= this->RelationName.size()) return this->RelationName[0].c_str();

    // Get the text
	return this->RelationName[ID].c_str();

}

//-----------------------------------------------------------------------------
// Size
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::Size() {

	return this->Term.size();

}

//-----------------------------------------------------------------------------
// TrueFromInit
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::TrueFrom(unsigned int ID) {

	char* Name;
	int Target;
	int LowerBound;
	int UpperBound;
	int Compare = -1;


	// Check the index
	if (ID >= this->RelationName.size()) return 0;

	// Construct the name of the new relation
	Name = new char[strlen(this->RelationName[ID].c_str()) + 1];
	sprintf(Name, "true:%s", &(this->RelationName[ID].c_str()[5]));

    // Binary search; zeroth term is Lexicon
	LowerBound = 1;
	UpperBound = this->RelationName.size() - 1;

	// Look for the term according to its value
	while (LowerBound <= UpperBound) {
		Target = (LowerBound + UpperBound) / 2;
		Compare = strcmp(Name, this->RelationName[this->RelationNameID[Target]].c_str());
		if (Compare == 0) {
			delete[] Name;
			return this->RelationNameID[Target];
		}
		if (Compare < 0) UpperBound = Target - 1;
		if (Compare > 0) LowerBound = Target + 1;
	}

	// Not found
	delete[] Name;
	return 0;

}

//-----------------------------------------------------------------------------
// NextFromTrue
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::NextFrom(unsigned int ID) {

	char* Name;
	int Target;
	int LowerBound;
	int UpperBound;
	int Compare = -1;


	// Check the index
	if (ID >= this->RelationName.size()) return 0;

	// Construct the name of the new relation
	Name = new char[strlen(this->RelationName[ID].c_str()) + 1];
	sprintf(Name, "next:%s", &(this->RelationName[ID].c_str()[5]));

    // Binary search; zeroth term is Lexicon
	LowerBound = 1;
	UpperBound = this->RelationName.size() - 1;

	// Look for the term according to its value
	while (LowerBound <= UpperBound) {
		Target = (LowerBound + UpperBound) / 2;
		Compare = strcmp(Name, this->RelationName[this->RelationNameID[Target]].c_str());
		if (Compare == 0) {
			delete[] Name;
			return this->RelationNameID[Target];
		}
		if (Compare < 0) UpperBound = Target - 1;
		if (Compare > 0) LowerBound = Target + 1;
	}

	// Not found
	delete[] Name;
	return 0;

}

//-----------------------------------------------------------------------------
// InitFromNext
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::InitFrom(unsigned int ID) {

	char* Name;
	int Target;
	int LowerBound;
	int UpperBound;
	int Compare = -1;


	// Check the index
	if (ID >= this->RelationName.size()) return 0;

	// Construct the name of the new relation
	Name = new char[strlen(this->RelationName[ID].c_str()) + 1];
	sprintf(Name, "init:%s", &(this->RelationName[ID].c_str()[5]));

    // Binary search; zeroth term is Lexicon
	LowerBound = 1;
	UpperBound = this->RelationName.size() - 1;

	// Look for the term according to its value
	while (LowerBound <= UpperBound) {
		Target = (LowerBound + UpperBound) / 2;
		Compare = strcmp(Name, this->RelationName[this->RelationNameID[Target]].c_str());
		if (Compare == 0) {
			delete[] Name;
			return this->RelationNameID[Target];
		}
		if (Compare < 0) UpperBound = Target - 1;
		if (Compare > 0) LowerBound = Target + 1;
	}

	// Not found
	delete[] Name;
	return 0;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcLexicon::Print() {

	this->IO->WriteToLog(1, false, "\n--- Lexicon ---\n");
	this->IO->LogIndent = 4;
    
    // Display the List
	this->IO->WriteToLog(1, true, "Terms\n");
	for (unsigned int i = 0; i < this->Term.size(); i++) {
		this->IO->FormatToLog(1, true, "%4d - %s\n", i, this->Term[i].c_str()); 
    }
	this->IO->WriteToLog(1, true, "Relations\n");
	for (unsigned int i = 0; i < this->RelationName.size(); i++) {
		this->IO->FormatToLog(1, true, "%4d - %s\n", i, this->RelationName[i].c_str()); 
    }

}

//-----------------------------------------------------------------------------
// AddTerm
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::AddTerm(const char* Value) {

	unsigned int Target;
	unsigned int Index;

	// Append the term at the end
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

//-----------------------------------------------------------------------------
// AddName
//-----------------------------------------------------------------------------
unsigned int hsfcLexicon::AddName(const char* Value) {

	unsigned int Target;
	unsigned int Index;

	// Append the term at the end
	this->RelationName.push_back(Value);
	this->RelationNameID.push_back(0);

	// Index the new term
	Index = this->RelationName.size() - 1;
	for (Target = 1; Target < this->RelationName.size(); Target++) {
		if (strcmp(Value, this->RelationName[this->RelationNameID[Target]].c_str()) < 0) break;
	}
	for (unsigned int j = this->RelationName.size() - 1; j > Target; j--) {
		this->RelationNameID[j] = this->RelationNameID[j - 1];
	}

	// Add the new term to the index
	if (Target > this->RelationName.size() - 1) Target = this->RelationName.size() - 1;
	this->RelationNameID[Target] = this->RelationName.size() - 1;

	return Index;

}

