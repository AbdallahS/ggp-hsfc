//=============================================================================
// Project: High Speed Forward Chaining
// Module: GDL
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcGDL.h"

//=============================================================================
// CLASS: hsfcGDLAtom
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGDLAtom::hsfcGDLAtom(hsfcLexicon* Lexicon) {

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGDLAtom::~hsfcGDLAtom(void) {

	// Delete any terms
	this->DeleteTerms();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcGDLAtom::Initialise() {

	// Delete any terms
	this->DeleteTerms();

	// Initialise the properties
	this->PredicateIndex = 0;

}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------
bool hsfcGDLAtom::Read(hsfcWFTElement* WFT) {

	hsfcGDLAtom* NewTerm;

	// Initialise the atom
	this->Initialise();

	// Does this element have an opening bracket '('
	if (WFT->LexiconIndex == 0) {

		// Does this element have any children
		if (WFT->Child.size() > 0) {
			// The first child is the predicate
			if (WFT->Child[0]->LexiconIndex == 0) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: Double brackets '((' in hsfcGDLAtom::Read\n");
				return false; 
			}
			this->PredicateIndex = WFT->Child[0]->LexiconIndex;
		}

		// Load the terms from the children
		for (unsigned int i = 1; i < WFT->Child.size(); i++) {
			// Create a new term
			NewTerm = new hsfcGDLAtom(this->Lexicon);
			// Read it and check for errors
			if (NewTerm->Read(WFT->Child[i])) {
				this->Term.push_back(NewTerm);
			} else {
				delete NewTerm;
				return false;
			}
		}

	} else {

		// Test the term
		if (this->Lexicon->Text(WFT->LexiconIndex)[0] == '"') {
			this->Lexicon->IO->WriteToLog(0, false, "Error: Literal not term in hsfcGDLAtom::Read\n");
			return false; 
		}
		if (strstr(this->Lexicon->Text(WFT->LexiconIndex), " ") != NULL) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: Term contains space ' ' in hsfcGDLAtom::Read\n");
			return false; 
		}

		// This is just a term
		this->PredicateIndex = WFT->LexiconIndex;

	}

	return true;

}

//-----------------------------------------------------------------------------
// DeleteTerms
//-----------------------------------------------------------------------------
void hsfcGDLAtom::DeleteTerms() {

	// Delete any children
	for (unsigned int i = 0; i < this->Term.size(); i++) {
		delete this->Term[i];
	}
	this->Term.clear();

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcGDLAtom::Print() {

	// Print the predicate
	if (this->Term.size() == 0) {
		this->Lexicon->IO->FormatToLog(0, false, " %s", this->Lexicon->Text(this->PredicateIndex));
	} else {
		this->Lexicon->IO->FormatToLog(0, false, " (%s", this->Lexicon->Text(this->PredicateIndex));
		for (unsigned int i = 0; i < this->Term.size(); i++) {
			// Is this a rule
			if (this->Lexicon->Match(this->PredicateIndex, "<=") && (i > 0)) {
				this->Lexicon->IO->WriteToLog(0, false, "    ");
			}
			this->Term[i]->Print();
			// Is this a rule
			if (this->Lexicon->Match(this->PredicateIndex, "<=") && (i < this->Term.size() - 1)) {
				this->Lexicon->IO->WriteToLog(0, false, "\n");
			}
		}
		this->Lexicon->IO->WriteToLog(0, false, ")");
	}

}


//=============================================================================
// CLASS: hsfcGDL
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGDL::hsfcGDL(hsfcLexicon* Lexicon) {

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGDL::~hsfcGDL(void) {

	// Delete the GDL
	this->DeleteRules();
	this->DeleteStatements();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcGDL::Initialise() {

	// Delete the GDL
	this->DeleteRules();
	this->DeleteStatements();

}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------
bool hsfcGDL::Read(hsfcWFT* WFT) {

	hsfcGDLAtom* NewAtom;
	hsfcGDLAtom* NewAtomTerm;

	// Initialise the GDL
	this->Initialise();
	this->Lexicon->IO->WriteToLog(2, true, "Reading WFT into GDL ...\n");

	// Read the level 0 elements in the WFT
	for (unsigned int i = 0; i < WFT->Structure->RootElement->Child.size(); i++) {

		// Is it just a comment
		if (WFT->Structure->RootElement->Child[i]->LexiconIndex != 0) continue;

		// Create a new atom
		NewAtom = new hsfcGDLAtom(this->Lexicon);
		// Read the WFT element
		if (NewAtom->Read(WFT->Structure->RootElement->Child[i])) {
			// Is this s rule or a statement
			if (this->Lexicon->Match(NewAtom->PredicateIndex, "<=")) {
				// Check the rule integrity
				if (NewAtom->Term.size() == 0) {
					this->Lexicon->IO->WriteToLog(0, false, "Error: empty rule in hsfcGDL::Read\n");
					return false;
				}
				// Is a rule with only an output
				if (NewAtom->Term.size() == 1) {
					NewAtomTerm = NewAtom->Term[0];
					NewAtom->Term.clear();
					delete NewAtom;

					this->Statement.push_back(NewAtomTerm);
				} else {
					// Add the new rule
					this->Rule.push_back(NewAtom);
				}
			} else {
				this->Statement.push_back(NewAtom);
			}
		} else {
			// Atom read failed
			delete NewAtom;
			return false;
		}
	}

	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");
	if (this->Lexicon->IO->Parameters->LogDetail > 2) this->Print();
	return true;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcGDL::Print() {

	this->Lexicon->IO->WriteToLog(0, true, "\n--- GDL ---\n");

	// Print statement
	this->Lexicon->IO->WriteToLog(0, true, "\nStatements\n\n");
	for (unsigned int i = 0; i < this->Statement.size(); i++) {
		this->Statement[i]->Print();
		this->Lexicon->IO->WriteToLog(0, true, "\n");
	}

	// Print rules
	this->Lexicon->IO->WriteToLog(0, true, "\nRules\n");
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		this->Lexicon->IO->WriteToLog(0, true, "\n");
		this->Rule[i]->Print();
		this->Lexicon->IO->WriteToLog(0, true, "\n");
	}

}

//-----------------------------------------------------------------------------
// DeleteStatements
//-----------------------------------------------------------------------------
void hsfcGDL::DeleteStatements() {

	// Delete any children
	for (unsigned int i = 0; i < this->Statement.size(); i++) {
		delete this->Statement[i];
	}
	this->Statement.clear();

}

//-----------------------------------------------------------------------------
// DeleteTerms
//-----------------------------------------------------------------------------
void hsfcGDL::DeleteRules() {

	// Delete any children
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete this->Rule[i];
	}
	this->Rule.clear();

}


