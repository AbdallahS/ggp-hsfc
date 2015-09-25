//=============================================================================
// Project: High Speed Forward Chaining
// Module: SCL
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcSCL.h"

//=============================================================================
// CLASS: hsfcSCLAtom
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcSCLAtom::hsfcSCLAtom(hsfcLexicon* Lexicon) {

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcSCLAtom::~hsfcSCLAtom(void) {

	// Delete any terms
	this->DeleteTerms();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcSCLAtom::Initialise() {

	// Delete any terms
	this->DeleteTerms();

	// Initialise the properties
	this->PredicateIndex = 0;
	this->NameID = 0;
	this->Not = false;
	this->Distinct = false;
	this->Cursor = 0;

}

//-----------------------------------------------------------------------------
// FromSCLAtom
//-----------------------------------------------------------------------------
void hsfcSCLAtom::FromSCLAtom(hsfcSCLAtom* Source) {

	hsfcSCLAtom* NewAtom;

	// Initialise this atom
	this->Initialise();

	// Copy source information
	this->PredicateIndex = Source->PredicateIndex;
	this->NameID = Source->NameID;
	this->Not = Source->Not;
	this->Distinct = Source->Distinct;

	// Copy the children
	for (unsigned int i = 0; i < Source->Term.size(); i++) {
		NewAtom = new hsfcSCLAtom(this->Lexicon);
		NewAtom->FromSCLAtom(Source->Term[i]);
		this->Term.push_back(NewAtom);
	}

}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------
bool hsfcSCLAtom::Read(hsfcGDLAtom* GDLAtom) {

	hsfcSCLAtom* NewTerm;

	// Initialise the atom
	this->Initialise();

	// Copy the elements of the gdl atom
	this->PredicateIndex = GDLAtom->PredicateIndex;

	// Load the terms from the children
	for (unsigned int i = 0; i < GDLAtom->Term.size(); i++) {
		// Create a new term
		NewTerm = new hsfcSCLAtom(this->Lexicon);
		// Read it and check for errors
		if (NewTerm->Read(GDLAtom->Term[i])) {
			this->Term.push_back(NewTerm);
		} else {
			delete NewTerm;
			return false;
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// SetQualifiedName
//-----------------------------------------------------------------------------
void hsfcSCLAtom::SetQualifiedName(unsigned int ParentNameID, unsigned int ArgumentIndex) {

	char* QualifiedName;
	char* ParentName;

	// Create the new qualified name
	// Is this a zero arity relation like (terimnal)
	if ((ParentNameID == 0) && (this->Term.size() == 0)) {
		QualifiedName = new char[strlen(this->Lexicon->Text(this->PredicateIndex)) + 3];
		sprintf(QualifiedName, "%s/0", this->Lexicon->Text(this->PredicateIndex));
		this->NameID = this->Lexicon->Index(QualifiedName);
		delete[] QualifiedName;
		return;
	}

	// Is it a term with no children
	if (this->Term.size() == 0) {
		this->NameID = this->PredicateIndex;
		return;
	}

	// Is it a state relation or an embedded relation
	if (ParentNameID == 0) {
		QualifiedName = new char[strlen(this->Lexicon->Text(this->PredicateIndex)) + 12];
		sprintf(QualifiedName, "%s/%lu", this->Lexicon->Text(this->PredicateIndex), this->Term.size());
		this->NameID = this->Lexicon->Index(QualifiedName);
		delete[] QualifiedName;
	} else {
		QualifiedName = new char[strlen(this->Lexicon->Text(ParentNameID)) + strlen(this->Lexicon->Text(this->PredicateIndex)) + 12];
		//sprintf(QualifiedName, "%s:%d:%s/%d", this->Lexicon->Text(ParentNameID), ArgumentIndex, this->Lexicon->Text(this->PredicateIndex), this->Term.size());
		// This is easier in Schema to create joins in rule inputs
		sprintf(QualifiedName, "%s/%lu", this->Lexicon->Text(this->PredicateIndex), this->Term.size());
		this->NameID = this->Lexicon->Index(QualifiedName);
		delete[] QualifiedName;
	}

	// Process any children
	for (unsigned int i = 0; i < this->Term.size(); i++) {
		if (this->Lexicon->PartialMatch(this->NameID, "next:")) {
			// (next ...) takes the same embedded functions as (true ...)
			ParentName = new char[strlen(this->Lexicon->Text(this->NameID)) + 1];
			sprintf(ParentName, "true:%s", &this->Lexicon->Text(this->NameID)[5]);
			this->Term[i]->SetQualifiedName(this->Lexicon->Index(ParentName), i);
			delete[] ParentName;
			continue;
		}
		if (this->Lexicon->PartialMatch(this->NameID, "init:")) {
			// (init ...) takes the same embedded functions as (true ...)
			ParentName = new char[strlen(this->Lexicon->Text(this->NameID)) + 1];
			sprintf(ParentName, "true:%s", &this->Lexicon->Text(this->NameID)[5]);
			this->Term[i]->SetQualifiedName(this->Lexicon->Index(ParentName), i);
			delete[] ParentName;
			continue;
		}
		this->Term[i]->SetQualifiedName(this->NameID, i);
	}

}

//-----------------------------------------------------------------------------
// RequiredFor
//-----------------------------------------------------------------------------
bool hsfcSCLAtom::RequiredFor(vector<hsfcSCLAtom*>& Rule) {

	// Check each rule in the collection
	for (unsigned int i = 0; i < Rule.size(); i++) {
		// Is it this rule; don't check self referencing
		if (Rule[i] != this) {
			// Check all of the inputs against this outputs
			for (unsigned int j = 1; j < Rule[i]->Term.size(); j++) {
				if (Rule[i]->Term[j]->PredicateIndex == this->Term[0]->PredicateIndex) {
					return true;
				}
			}
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// ResetEnumeration
//-----------------------------------------------------------------------------
void hsfcSCLAtom::ResetEnumeration() {

	// Reset the cursor
	this->Cursor = 0;

	// Reset any children
	for (unsigned int i = 0; i < this->Term.size(); i++) {
		this->Term[i]->ResetEnumeration();
	}

}

//-----------------------------------------------------------------------------
// EnumerateTerms
//-----------------------------------------------------------------------------
hsfcSCLAtom* hsfcSCLAtom::EnumerateTerms() {

	hsfcSCLAtom* Atom;

	// Is this the term
	if (this->Cursor == 0) {
		this->Cursor++;
		return this;
	} else {
		while (this->Cursor <= this->Term.size()) {
			Atom = this->Term[this->Cursor - 1]->EnumerateTerms();
			if (Atom != NULL) return Atom;
			this->Cursor++;
		}
		return NULL;
	}

}

//-----------------------------------------------------------------------------
// DeleteTerms
//-----------------------------------------------------------------------------
void hsfcSCLAtom::DeleteTerms() {

	// Delete any children
	for (unsigned int i = 0; i < this->Term.size(); i++) {
		delete this->Term[i];
	}
	this->Term.clear();

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcSCLAtom::Print() {

	// Print the predicate
	if (this->Term.size() == 0) {
		if (this->Not) {
			this->Lexicon->IO->FormatToLog(0, false, " !%s", this->Lexicon->Text(this->NameID));
		} else {
			this->Lexicon->IO->FormatToLog(0, false, " %s", this->Lexicon->Text(this->NameID));
		}
	} else {
		if (this->Not) {
			this->Lexicon->IO->FormatToLog(0, false, " !(%s", this->Lexicon->Text(this->NameID));
		} else {
			this->Lexicon->IO->FormatToLog(0, false, " (%s", this->Lexicon->Text(this->NameID));
		}
		for (unsigned int i = 0; i < this->Term.size(); i++) {
			this->Term[i]->Print();
		}
		this->Lexicon->IO->WriteToLog(0, false, ")");
	}

}


//=============================================================================
// CLASS: hsfcSCLStratum
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcSCLStratum::hsfcSCLStratum(hsfcLexicon* Lexicon) {

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcSCLStratum::~hsfcSCLStratum(void) {

	// Delete any terms
	//this->DeleteRules();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcSCLStratum::Initialise() {

	// Delete any terms
	//this->DeleteRules();
	this->Rule.clear();

	// Initialise the properties
	this->Input.clear();
	this->Output.clear();
	this->LinearPaths = false;
	this->SelfReferenceCount = 0;

}

//-----------------------------------------------------------------------------
// AddRule
//-----------------------------------------------------------------------------
void hsfcSCLStratum::AddRule(hsfcSCLAtom* Rule) {

	unsigned int Index;

	// Add the rule to the list
	this->Rule.push_back(Rule);

	// Add its output to the sorted list of outputs
	for (Index = 0; Index < this->Output.size(); Index++) {
		if (Rule->Term[0]->NameID == this->Output[Index]) goto Inputs;
		if (Rule->Term[0]->NameID < this->Output[Index]) break;
	}
	this->Output.insert(this->Output.begin() + Index, Rule->Term[0]->NameID);

Inputs:
	// Add its inputs to the sorted list of inputs
	for (unsigned int i = 1; i < Rule->Term.size(); i++) {
		for (Index = 0; Index < this->Input.size(); Index++) {
			if (Rule->Term[i]->NameID == this->Input[Index]) goto NextInput;
			if (Rule->Term[i]->NameID < this->Input[Index]) break;
		}
		this->Input.insert(this->Input.begin() + Index, Rule->Term[i]->NameID);
NextInput:;
	}

}

//-----------------------------------------------------------------------------
// HasOutput
//-----------------------------------------------------------------------------
bool hsfcSCLStratum::HasOutput(int NameID) {

	// Look for the name in the outputs
	for (unsigned int i = 0; i < this->Output.size(); i++) {
		if (this->Output[i] == NameID) return true;
	}

	return false;

}

//-----------------------------------------------------------------------------
// TracePath
//-----------------------------------------------------------------------------
void hsfcSCLStratum::TracePath(vector<hsfcInputPath>& Path, vector<hsfcSCLStratum*>& Stratum) {

	hsfcInputPath NewPath;

	// Is the path empty
	while (Path.size() > 0) {

		// Look for the first child of the last input
		for (unsigned int i = 0; i < Stratum.size(); i++) {
			// Find the stratum that produces the last input
			if (Stratum[i]->HasOutput(Path[Path.size()-1].InputNameID)) {

				// Is this path already known to be linear
				if (Stratum[i]->LinearPaths) {
					break;
				}
				// Avoid self referencing 
				if (i == Path[Path.size()-1].StratumIndex) {
					break;
				}
				// Add the first input to the path
				if (Stratum[i]->Input.size() == 0) {
					break;
				}

				// Add the details of the new path
				NewPath.StratumIndex = i;
				NewPath.InputIndex = 0;
				NewPath.InputNameID = Stratum[i]->Input[0];
				Path.push_back(NewPath);
				return;
			}
		}

		// Look for the next sibling of the last input
		if (Path[Path.size()-1].InputIndex < Stratum[Path[Path.size()-1].StratumIndex]->Input.size() - 1) {
			// Add the details of the new path
			Path[Path.size()-1].InputIndex++;
			Path[Path.size()-1].InputNameID = Stratum[Path[Path.size()-1].StratumIndex]->Input[Path[Path.size()-1].InputIndex];
			return;
		}

		// Backtrack and advance
		while (true) {
			// Backtrack
			Path.pop_back();
			if (Path.size() <= 1) return;
			// Look for the next sibling of the last input
			if (Path[Path.size()-1].InputIndex < Stratum[Path[Path.size()-1].StratumIndex]->Input.size() - 1) {
				// Add the details of the new path
				Path[Path.size()-1].InputIndex++;
				Path[Path.size()-1].InputNameID = Stratum[Path[Path.size()-1].StratumIndex]->Input[Path[Path.size()-1].InputIndex];
				break;
			}
		}
	}

}

//-----------------------------------------------------------------------------
// PathIsCircular
//-----------------------------------------------------------------------------
bool hsfcSCLStratum::PathIsCircular(vector<hsfcInputPath>& Path, vector<unsigned int>& CircularPath) {

	// Check each stratum in the path
	for (unsigned int i = 1; i < Path.size(); i++) {
		// Check every previous step in the path
		for (unsigned int j = 0; j < i; j++) {
			// Are the two stratum the same
			if (Path[j].StratumIndex == Path[i].StratumIndex) {
				// Record the path
				CircularPath.clear();
				for (unsigned int k = j; k < i; k++) {
					CircularPath.push_back(Path[k].StratumIndex);
				}
				return true;
			}
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// Combine
//-----------------------------------------------------------------------------
void hsfcSCLStratum::Combine(hsfcSCLStratum* Source) {

	// Add the rules from the source stratum to this one
	for (unsigned int i = 0; i < Source->Rule.size(); i++) {
		this->AddRule(Source->Rule[i]);
	}

}

//-----------------------------------------------------------------------------
// PartialMatch
//-----------------------------------------------------------------------------
bool hsfcSCLStratum::PartialMatch(const char* Text) {

	// Check each output for a match
	for (unsigned int i = 0; i < this->Output.size(); i++) {
		if (this->Lexicon->PartialMatch(this->Output[i], Text)) return true;
	}

	return false;

}

//-----------------------------------------------------------------------------
// Match
//-----------------------------------------------------------------------------
bool hsfcSCLStratum::Match(const char* Text) {

	// Check each output for a match
	for (unsigned int i = 0; i < this->Output.size(); i++) {
		if (this->Lexicon->Match(this->Output[i], Text)) return true;
	}

	return false;

}

//-----------------------------------------------------------------------------
// RequiredFor
//-----------------------------------------------------------------------------
bool hsfcSCLStratum::RequiredFor(vector<hsfcSCLStratum*>& Stratum, unsigned int Fisrt, unsigned int Last) {

	// Check each rule in the collection
	for (unsigned int i = Fisrt; i <= Last; i++) {
		// Is it this stratum; don't check self referencing
		if (Stratum[i] != this) {
			// Check all of the inputs against all the outputs
			for (unsigned int j = 0; j < this->Output.size(); j++) {
				for (unsigned int k = 0; k < Stratum[i]->Input.size(); k++) {
					if (Stratum[i]->Input[k] == this->Output[j]) {
						return true;
					}
				}
			}
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// DeleteRules
//-----------------------------------------------------------------------------
void hsfcSCLStratum::DeleteRules() {

	// Delete any children
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete this->Rule[i];
	}
	this->Rule.clear();

}

//=============================================================================
// CLASS: hsfcSCL
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcSCL::hsfcSCL(hsfcLexicon* Lexicon) {

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcSCL::~hsfcSCL(void) {

	// Delete the SCL
	this->DeleteRules();
	this->DeleteStatements();
	this->DeleteStrata();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcSCL::Initialise() {

	// Delete the SCL
	this->DeleteRules();
	this->DeleteStatements();
	this->DeleteStrata();

}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------
bool hsfcSCL::Read(hsfcGDL* GDL) {

	hsfcSCLAtom* NewAtom;

	// Initialise the SCL
	this->Initialise();

	// Read the statement
	for (unsigned int i = 0; i < GDL->Statement.size(); i++) {
		// Create a new atom
		NewAtom = new hsfcSCLAtom(this->Lexicon);
		// Read the GDL atom
		if (NewAtom->Read(GDL->Statement[i])) {
			// Add the new statement
			this->Statement.push_back(NewAtom);
		} else {
			// Statement read failed
			delete NewAtom;
			return false;
		}
	}

	// Read the rule
	for (unsigned int i = 0; i < GDL->Rule.size(); i++) {
		// Create a new atom
		NewAtom = new hsfcSCLAtom(this->Lexicon);
		// Read the GDL atom
		if (NewAtom->Read(GDL->Rule[i])) {
			this->Rule.push_back(NewAtom);
		} else {
			// Statement read failed
			delete NewAtom;
			return false;
		}
	}

	// Check the concatenation character
	if (this->Lexicon->IsUsed(":", true)) {
		this->Lexicon->IO->WriteToLog(0, false, "Warning: concatenation character already in use in hsfcSCL::Normalise\n");
	}

	// Normalise the SCL
	this->Lexicon->IO->WriteToLog(2, true, "Normailsing ... \n");
	if (!this->Normalise()) {
		this->Lexicon->IO->WriteToLog(2, true, "failed\n");
		return false;
	}
	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");

	// Stratify the SCL
	this->Lexicon->IO->WriteToLog(2, true, "Stratifying ... \n");
	if (!this->Stratify()) {
		this->Lexicon->IO->WriteToLog(2, true, "failed\n");
		return false;
	}
	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");

	// Stratify the SCL
	this->Lexicon->IO->WriteToLog(2, true, "Identify Rigids ... \n");
	if (!this->IdentifyRigids()) {
		this->Lexicon->IO->WriteToLog(2, true, "failed\n");
		return false;
	}
	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");

	//// Identify the complex rigids
	//this->Lexicon->IO->WriteToLog(2, true, "Identify Complex Rigids ...\n");
	//if (!this->IdentifyComplexRigids()) {
	//	return false;
	//}
	//this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");

	// Print the details
	if (this->Lexicon->IO->Parameters->LogDetail > 2) this->Print();

	return true;

}

//-----------------------------------------------------------------------------
// ReadStatement
//-----------------------------------------------------------------------------
bool hsfcSCL::ReadStatement(hsfcGDL* GDL) {

	hsfcSCLAtom* NewAtom;

	// Initialise the SCL
	this->Initialise();

	// Read the statement
	for (unsigned int i = 0; i < GDL->Statement.size(); i++) {
		// Create a new atom
		NewAtom = new hsfcSCLAtom(this->Lexicon);
		// Read the GDL atom
		if (NewAtom->Read(GDL->Statement[i])) {
			// Add the new statement
			this->Statement.push_back(NewAtom);
		} else {
			// Statement read failed
			delete NewAtom;
			return false;
		}
	}

	if (!this->Normalise()) return false;

	return true;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcSCL::Print() {

	this->Lexicon->IO->WriteToLog(0, false, "\n--- SCL ---\n");

	// Print statement
	this->Lexicon->IO->WriteToLog(0, false, "\nStatements\n\n");
	for (unsigned int i = 0; i < this->Statement.size(); i++) {
		this->Statement[i]->Print();
		this->Lexicon->IO->WriteToLog(0, false, "\n");
	}

	// Print rules in strata
	this->Lexicon->IO->WriteToLog(0, false, "\nRules\n");
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		for (unsigned int j = 0; j < this->Stratum[i]->Rule.size(); j++) {
			this->Lexicon->IO->WriteToLog(0, false, "\n(<= ");
			this->Stratum[i]->Rule[j]->Term[0]->Print();
			for (unsigned int k = 1; k < this->Stratum[i]->Rule[j]->Term.size(); k++) {
				this->Lexicon->IO->WriteToLog(0, false, "\n    ");
				this->Stratum[i]->Rule[j]->Term[k]->Print();
			}
			this->Lexicon->IO->WriteToLog(0, false, ")\n");
		}
		this->Lexicon->IO->WriteToLog(0, false, "------------------------------------------\n");
	}

}

//-----------------------------------------------------------------------------
// DeleteStatements
//-----------------------------------------------------------------------------
void hsfcSCL::DeleteStatements() {

	// Delete any children
	for (unsigned int i = 0; i < this->Statement.size(); i++) {
		delete this->Statement[i];
	}
	this->Statement.clear();

}

//-----------------------------------------------------------------------------
// DeleteRules
//-----------------------------------------------------------------------------
void hsfcSCL::DeleteRules() {

	// Delete any children
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete this->Rule[i];
	}
	this->Rule.clear();

}

//-----------------------------------------------------------------------------
// DeleteStratum
//-----------------------------------------------------------------------------
void hsfcSCL::DeleteStrata() {

	// Delete any children
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		delete this->Stratum[i];
	}
	this->Stratum.clear();

}

//-----------------------------------------------------------------------------
// DeleteTerm
//-----------------------------------------------------------------------------
void hsfcSCL::DeleteTerm(vector<hsfcSCLAtom*>& Terms, int Index, bool DeleteChildren) {

	// Save the children
	if (!DeleteChildren) {
		Terms.at(Index)->Term.clear();
	}
	// Delete the term
	delete Terms.at(Index);
	Terms.erase(Terms.begin() + Index);

}

//-----------------------------------------------------------------------------
// NameIDisRigid
//-----------------------------------------------------------------------------
bool hsfcSCL::NameIDisRigid(int NameID) {

	// Look for the name id
	for (unsigned int i = 0; i < this->RigidNameID.size(); i++) {
		if (NameID == this->RigidNameID[i]) return true;
	}

	return false;

}

//-----------------------------------------------------------------------------
// NameIDisNotRigid
//-----------------------------------------------------------------------------
void hsfcSCL::NameIDisNotRigid(int NameID) {

	// Look for the name id
	for (unsigned int i = 0; i < this->RigidNameID.size(); i++) {
		if (NameID == this->RigidNameID[i]) {
			this->RigidNameID.erase(this->RigidNameID.begin() + i);
			return;
		}
	}

}

//-----------------------------------------------------------------------------
// WrapLegals
//-----------------------------------------------------------------------------
bool hsfcSCL::WrapLegals() {


	bool Found;
	hsfcSCLAtom* NewTerm;
	hsfcSCLAtom* NewRule;
	unsigned int OldIndex;
	unsigned int NewIndex;

	this->Lexicon->IO->WriteToLog(2, true, "  Wrapping Legal\n");

	// Check each rule for legal as input
	Found = false;
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		// Check each (legal ...) input for a ?variable as an argument
		for (unsigned int j = 1; j < this->Rule[i]->Term.size(); j++) {
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "legal")) {
				Found = true;
				break;
			}
		}

		if (Found) break;

	}

	// Any legal input found?
	if (!Found) {
		this->Lexicon->IO->WriteToLog(2, true, "     not required\n");
		return true;
	}

	// Remedy:
	// Change every legal to an auxilliary and add a new rule
	// (legal ?r ?m) ==> (hsfcLegal ?r ?m)
	// (<= (legal ?r ?m) (hsfcLegal ?r ?m))

	// Change the lexical reference
	OldIndex = this->Lexicon->Index("legal");
	NewIndex = this->Lexicon->Index("hsfcLegal");

	// Check each rule for fluents
	for (unsigned int i = 0; i < this->Rule.size(); i++) {

		// Process each statement
		for (unsigned int j = 0; j < this->Rule[i]->Term.size(); j++) {
			if (this->Rule[i]->Term[j]->PredicateIndex == OldIndex) {
				this->Rule[i]->Term[j]->PredicateIndex = NewIndex;
			}
		}

	}

	// Check each statement as well
	for (unsigned int i = 0; i < this->Statement.size(); i++) {

		if (this->Statement[i]->PredicateIndex == OldIndex) {
			this->Statement[i]->PredicateIndex = NewIndex;
		}

	}

	// Add in the new rule
	NewRule = new hsfcSCLAtom(this->Lexicon);
	NewRule->Initialise();
	NewRule->PredicateIndex = this->Lexicon->Index("<=");
	// Add the legal output
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("legal");
	NewRule->Term.push_back(NewTerm);
	this->Rule.push_back(NewRule);
	// Add the variables
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("?r");
	NewRule->Term[0]->Term.push_back(NewTerm);
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("?m");
	NewRule->Term[0]->Term.push_back(NewTerm);
	// Add the fshcLegal injput
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("hsfcLegal");
	NewRule->Term.push_back(NewTerm);
	// Add the variables
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("?r");
	NewRule->Term[1]->Term.push_back(NewTerm);
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("?m");
	NewRule->Term[1]->Term.push_back(NewTerm);

	// Success
	this->Lexicon->IO->WriteToLog(2, true, "     succeeded\n");
	return true;

}

//-----------------------------------------------------------------------------
// WrapGoals
//-----------------------------------------------------------------------------
bool hsfcSCL::WrapGoals() {


	bool Found;
	hsfcSCLAtom* NewTerm;
	hsfcSCLAtom* NewRule;
	unsigned int OldIndex;
	unsigned int NewIndex;

	this->Lexicon->IO->WriteToLog(2, true, "  Wrapping Goal\n");

	// Check each rule for legal as input
	Found = false;
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		// Check each (legal ...) input for a ?variable as an argument
		for (unsigned int j = 1; j < this->Rule[i]->Term.size(); j++) {
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "goal")) {
				Found = true;
				break;
			}
		}

		if (Found) break;

	}

	// Any legal input found?
	if (!Found) {
		this->Lexicon->IO->WriteToLog(2, true, "     not required\n");
		return true;
	}

	// Remedy:
	// Change every legal to an auxilliary and add a new rule
	// (legal ?r ?m) ==> (hsfcLegal ?r ?m)
	// (<= (legal ?r ?m) (hsfcLegal ?r ?m))

	// Change the lexical reference
	OldIndex = this->Lexicon->Index("goal");
	NewIndex = this->Lexicon->Index("hsfcGoal");

	// Check each rule for fluents
	for (unsigned int i = 0; i < this->Rule.size(); i++) {

		// Process each statement
		for (unsigned int j = 0; j < this->Rule[i]->Term.size(); j++) {
			if (this->Rule[i]->Term[j]->PredicateIndex == OldIndex) {
				this->Rule[i]->Term[j]->PredicateIndex = NewIndex;
			}
		}

	}

	// Check each statement as well
	for (unsigned int i = 0; i < this->Statement.size(); i++) {

		if (this->Statement[i]->PredicateIndex == OldIndex) {
			this->Statement[i]->PredicateIndex = NewIndex;
		}

	}

	// Add in the new rule
	NewRule = new hsfcSCLAtom(this->Lexicon);
	NewRule->Initialise();
	NewRule->PredicateIndex = this->Lexicon->Index("<=");
	// Add the legal output
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("goal");
	NewRule->Term.push_back(NewTerm);
	this->Rule.push_back(NewRule);
	// Add the variables
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("?r");
	NewRule->Term[0]->Term.push_back(NewTerm);
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("?g");
	NewRule->Term[0]->Term.push_back(NewTerm);
	// Add the fshcLegal injput
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("hsfcGoal");
	NewRule->Term.push_back(NewTerm);
	// Add the variables
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("?r");
	NewRule->Term[1]->Term.push_back(NewTerm);
	NewTerm = new hsfcSCLAtom(this->Lexicon);
	NewTerm->Initialise();
	NewTerm->PredicateIndex = this->Lexicon->Index("?g");
	NewRule->Term[1]->Term.push_back(NewTerm);

	// Success
	this->Lexicon->IO->WriteToLog(2, true, "     succeeded\n");
	return true;

}

//-----------------------------------------------------------------------------
// WrapFluents
//-----------------------------------------------------------------------------
bool hsfcSCL::WrapFluents() {

	bool Found;
	hsfcSCLAtom* NewTerm;

	this->Lexicon->IO->WriteToLog(2, true, "  Wrapping Fluents\n");

	// Check each rule for fluent variables
	Found = false;
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		// Check each (next ...) rule for a ?variable as an argument
		if (this->Lexicon->Match(this->Rule[i]->Term[0]->PredicateIndex, "next")) {
			if (this->Lexicon->PartialMatch(this->Rule[i]->Term[0]->Term[0]->PredicateIndex, "?")) {
				Found = true;
				break;
			}
		}
		// Check each (init ...) rule for a ?variable as an argument
		if (this->Lexicon->Match(this->Rule[i]->Term[0]->PredicateIndex, "init")) {
			if (this->Lexicon->PartialMatch(this->Rule[i]->Term[0]->Term[0]->PredicateIndex, "?")) {
				Found = true;
				break;
			}
		}
		// Check each (true ...) input for a ?variable as an argument
		for (unsigned int j = 1; j < this->Rule[i]->Term.size(); j++) {
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "true")) {
				if (this->Lexicon->PartialMatch(this->Rule[i]->Term[j]->Term[0]->PredicateIndex, "?")) {
					Found = true;
					break;
				}
			}
		}

		if (Found) break;

	}

	// Any fluent variqables found?
	if (!Found) {
		this->Lexicon->IO->WriteToLog(2, true, "     not required\n");
		return true;
	}

	// Remedy:
	// Wrap every fluent in a metafluent
	// (next (fluent)) ==> (next (hscf:fluent (fluent)))
	// (next (?f)) ==> (next (hscf:fluent (?f)))
	// (true (fluent)) ==> (true (hscf:fluent (fluent)))
	// (init (fluent)) ==> (init (hscf:fluent (fluent)))

	// Check each rule for fluents
	for (unsigned int i = 0; i < this->Rule.size(); i++) {

		// Process each (next ...) rule 
		if (this->Lexicon->Match(this->Rule[i]->Term[0]->PredicateIndex, "next")) {
			// Create the new term
			NewTerm = new hsfcSCLAtom(this->Lexicon);
			NewTerm->Initialise();
			NewTerm->PredicateIndex = this->Lexicon->Index("hsfcFluent");
			// Link in the new term
			NewTerm->Term.push_back(this->Rule[i]->Term[0]->Term[0]);
			this->Rule[i]->Term[0]->Term[0] = NewTerm;
		}

		// Process each (init ...) rule 
		if (this->Lexicon->Match(this->Rule[i]->Term[0]->PredicateIndex, "init")) {
			// Create the new term
			NewTerm = new hsfcSCLAtom(this->Lexicon);
			NewTerm->Initialise();
			NewTerm->PredicateIndex = this->Lexicon->Index("hsfcFluent");
			// Link in the new term
			NewTerm->Term.push_back(this->Rule[i]->Term[0]->Term[0]);
			this->Rule[i]->Term[0]->Term[0] = NewTerm;
		}

		// Process each (true ...) input
		for (unsigned int j = 1; j < this->Rule[i]->Term.size(); j++) {
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "true")) {
				// Create the new term
				NewTerm = new hsfcSCLAtom(this->Lexicon);
				NewTerm->Initialise();
				NewTerm->PredicateIndex = this->Lexicon->Index("hsfcFluent");
				// Link in the new term
				NewTerm->Term.push_back(this->Rule[i]->Term[j]->Term[0]);
				this->Rule[i]->Term[j]->Term[0] = NewTerm;
			}
		}

	}

	// Check each statement for fluents
	for (unsigned int i = 0; i < this->Statement.size(); i++) {

		// Process each (init ...) statement 
		if (this->Lexicon->Match(this->Statement[i]->PredicateIndex, "init")) {
			// Create the new term
			NewTerm = new hsfcSCLAtom(this->Lexicon);
			NewTerm->Initialise();
			NewTerm->PredicateIndex = this->Lexicon->Index("hsfcFluent");
			// Link in the new term
			NewTerm->Term.push_back(this->Statement[i]->Term[0]);
			this->Statement[i]->Term[0] = NewTerm;
		}

	}

	// Success
	this->Lexicon->IO->WriteToLog(2, true, "     succeeded\n");
	return true;


}

//-----------------------------------------------------------------------------
// Normalise
//-----------------------------------------------------------------------------
bool hsfcSCL::Normalise() {

	hsfcSCLAtom* NewRule;
	hsfcSCLAtom* NewStatement;
	char* Predicate;

	// Rules are guaranteed to have more than one statement from hsfcGDL::Read
	
	// Remove any logical functions
	this->Lexicon->IO->WriteToLog(2, true, "  Flattening Functions\n");

	// Check each statement in each rule; not the output 
	for (unsigned int i = 0; i < this->Rule.size(); i++) {

		this->Lexicon->IO->FormatToLog(3, true, "    Rule %u\n", i);

		for (unsigned int j = 1; j < this->Rule[i]->Term.size(); j++) {

			// Remove any (and (...) (...)) statements
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "and")) {
				// Is it really (not and ()())
				if (this->Rule[i]->Term[j]->Not) {
					// ((not and (...) (...)) ==> (or (not (...)) (not((...)))
					this->Rule[i]->Term[j]->PredicateIndex = this->Lexicon->Index("or");
					this->Rule[i]->Term[j]->Not = false;
					for (unsigned int k = 0; k < this->Rule[i]->Term[j]->Term.size(); k++) {
						this->Rule[i]->Term[j]->Term[k]->Not = !this->Rule[i]->Term[j]->Term[k]->Not;
					}
				} else {
					// Add each child statement to the end of the rule
					for (unsigned int k = 0; k < this->Rule[i]->Term[j]->Term.size(); k++) {
						this->Rule[i]->Term.push_back(this->Rule[i]->Term[j]->Term[k]);
					}
					// Delete the statement; but not its children
					this->DeleteTerm(this->Rule[i]->Term, j, false);
				}
				// Repeat the rule 
				goto RepeatRule;
			}

			// Remove any (or (...) (...)) statements
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "or")) {
				// Is it really (not or ()())
				if (this->Rule[i]->Term[j]->Not) {
					// ((not or (...) (...)) ==> (and (not (...)) (not((...)))
					this->Rule[i]->Term[j]->PredicateIndex = this->Lexicon->Index("and");
					this->Rule[i]->Term[j]->Not = false;
					for (unsigned int k = 0; k < this->Rule[i]->Term[j]->Term.size(); k++) {
						this->Rule[i]->Term[j]->Term[k]->Not = !this->Rule[i]->Term[j]->Term[k]->Not;
					}
				} else {
					// Add copies of the rule to the end of the scl with each child statement
					for (unsigned int k = 0; k < this->Rule[i]->Term[j]->Term.size(); k++) {
						// Copy the rule to the end of the scl
						NewRule = new hsfcSCLAtom(this->Lexicon);
						NewRule->FromSCLAtom(this->Rule[i]);
						this->Rule.push_back(NewRule);
						// Copy and add the kth child to the end of the new rule
						NewStatement = new hsfcSCLAtom(this->Lexicon);
						NewStatement->FromSCLAtom(NewRule->Term[j]->Term[k]);
						NewRule->Term.push_back(NewStatement);
						// Delete the (or ) statement
						this->DeleteTerm(NewRule->Term, j, true);
					}
					// Delete the rule
					this->DeleteTerm(this->Rule, i, true);
				}
				//continue;
				goto RepeatRule;
			}

			// Remove any (not (...)) statements
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "not")) {
				// Check for the correct number of arguments
				if (this->Rule[i]->Term[j]->Term.size() != 1) {
					this->Lexicon->IO->WriteToLog(0, false, "Error: wrong number of arguments in '(not )' in hsfcSCL::Normalise\n");
					return false;
				}
				// Implement the not
				this->Rule[i]->Term[j]->Term[0]->Not = !this->Rule[i]->Term[j]->Not;
				// Remove the term and promote the child
				this->Rule[i]->Term.push_back(this->Rule[i]->Term[j]->Term[0]);
				this->DeleteTerm(this->Rule[i]->Term, j, false);
				// Continue;
				goto RepeatRule;
			}

			// Identify any (distinct ....) statements
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "distinct")) {
				// Check for the correct number of arguments
				if (this->Rule[i]->Term[j]->Term.size() != 2) {
					this->Lexicon->IO->WriteToLog(0, false, "Error: wrong number of arguments in '(distinct )' in hsfcSCL::Normalise\n");
					return false;
				}
				// Tag the term
				this->Rule[i]->Term[j]->Distinct = true;
			}

		}

		// Next Rule
		continue;

		// Repeat this rule
		RepeatRule:
		i--;

	}

	// Check to see if there are any fluent variables
	if (!this->WrapFluents()) return false;

	// Check to see if there are any legal as inputs to rules
	if (!this->WrapLegals()) return false;

	// Check to see if there are any goal as inputs to rules
	if (!this->WrapGoals()) return false;

	this->Lexicon->IO->WriteToLog(2, true, "  Flattening Fluents\n");

	// Check each statement in each rule 
	for (unsigned int i = 0; i < this->Rule.size(); i++) {

		this->Lexicon->IO->FormatToLog(3, true, "    Rule %u\n", i);

		// Remove any (next (...)) statements
		if (this->Lexicon->Match(this->Rule[i]->Term[0]->PredicateIndex, "next")) {
			// Check for the correct number of arguments
			if (this->Rule[i]->Term[0]->Term.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Warning: improper use of '(next )' in hsfcSCL::Normalise\n");
			}
			if (this->Rule[i]->Term[0]->Term.size() == 1) {
				// Convert child to (true:predicate ...)
				Predicate = new char[strlen(this->Lexicon->Text(this->Rule[i]->Term[0]->Term[0]->PredicateIndex)) + 6];
				sprintf(Predicate, "next:%s", this->Lexicon->Text(this->Rule[i]->Term[0]->Term[0]->PredicateIndex));
				this->Rule[i]->Term[0]->Term[0]->PredicateIndex = this->Lexicon->Index(Predicate);
				delete[] Predicate;
				// Remove the term and promote the child
				this->Rule[i]->Term.insert(this->Rule[i]->Term.begin(), this->Rule[i]->Term[0]->Term[0]);
				this->DeleteTerm(this->Rule[i]->Term, 1, false);
			}
		}

		// Remove any (next (...)) statements
		if (this->Lexicon->Match(this->Rule[i]->Term[0]->PredicateIndex, "init")) {
			// Check for the correct number of arguments
			if (this->Rule[i]->Term[0]->Term.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Warning: improper use of '(init )' in hsfcSCL::Normalise\n");
			}
			if (this->Rule[i]->Term[0]->Term.size() == 1) {
				// Convert child to (true:predicate ...)
				Predicate = new char[strlen(this->Lexicon->Text(this->Rule[i]->Term[0]->Term[0]->PredicateIndex)) + 6];
				sprintf(Predicate, "init:%s", this->Lexicon->Text(this->Rule[i]->Term[0]->Term[0]->PredicateIndex));
				this->Rule[i]->Term[0]->Term[0]->PredicateIndex = this->Lexicon->Index(Predicate);
				delete[] Predicate;
				// Remove the term and promote the child
				this->Rule[i]->Term.insert(this->Rule[i]->Term.begin(), this->Rule[i]->Term[0]->Term[0]);
				this->DeleteTerm(this->Rule[i]->Term, 1, false);
			}
		}

		// Inspect the input terms for (true ...)
		for (unsigned int j = 1; j < this->Rule[i]->Term.size(); j++) {
			// Remove any (true (...)) statements
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->PredicateIndex, "true")) {
				// Check for the correct number of arguments
				if (this->Rule[i]->Term[j]->Term.size() != 1) {
					this->Lexicon->IO->WriteToLog(0, false, "Warning: improper use of '(true )' in hsfcSCL::Normalise\n");
				}
				if (this->Rule[i]->Term[j]->Term.size() == 1) {
					// Convert child to (true:predicate ...)
					Predicate = new char[strlen(this->Lexicon->Text(this->Rule[i]->Term[j]->Term[0]->PredicateIndex)) + 6];
					sprintf(Predicate, "true:%s", this->Lexicon->Text(this->Rule[i]->Term[j]->Term[0]->PredicateIndex));
					this->Rule[i]->Term[j]->Term[0]->PredicateIndex = this->Lexicon->Index(Predicate);
					delete[] Predicate;
					// Implement the not
					this->Rule[i]->Term[j]->Term[0]->Not = this->Rule[i]->Term[j]->Not;
					// Remove the term and promote the child
					this->Rule[i]->Term.push_back(this->Rule[i]->Term[j]->Term[0]);
					this->DeleteTerm(this->Rule[i]->Term, j, false);
					// Continue;
					j--;
				}
			}
		}

	}

	this->Lexicon->IO->WriteToLog(2, true, "  Finding (init ...)\n");

	// Look for (init ...)
	for (unsigned int i = 0; i < this->Statement.size(); i++) {
		// Remove any (true (...)) statements
		if (this->Lexicon->Match(this->Statement[i]->PredicateIndex, "init")) {
			// Check for the correct number of arguments
			if (this->Statement[i]->Term.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Warning: improper use of '(init )' in hsfcSCL::Normalise\n");
			}
			if (this->Statement[i]->Term.size() == 1) {
				// Convert child to (init:predicate ...)
				Predicate = new char[strlen(this->Lexicon->Text(this->Statement[i]->Term[0]->PredicateIndex)) + 6];
				sprintf(Predicate, "init:%s", this->Lexicon->Text(this->Statement[i]->Term[0]->PredicateIndex));
				this->Statement[i]->Term[0]->PredicateIndex = this->Lexicon->Index(Predicate);
				delete[] Predicate;
				// Remove the term and promote the child
				this->Statement.push_back(this->Statement[i]->Term[0]);
				this->DeleteTerm(this->Statement, i, false);
				// Continue;
				i--;
			}
		}
	}

	this->Lexicon->IO->WriteToLog(2, true, "  Normalising names\n");

	// Normalise the statements
	for (unsigned int i = 0; i < this->Statement.size(); i++) {
		this->Statement[i]->SetQualifiedName(0, 0);
	}

	// Normalise the rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		this->Rule[i]->NameID = this->Rule[i]->PredicateIndex;
		for (unsigned int j = 0; j < this->Rule[i]->Term.size(); j++) {
			this->Rule[i]->Term[j]->SetQualifiedName(0, 0);
		}
	}


	this->Lexicon->IO->WriteToLog(2, true, "  Integrity Check\n");

	// Check each rule for illegal inputs
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		for (unsigned int j = 1; j < this->Rule[i]->Term.size(); j++) {
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->NameID, "legal/2")) {
				this->Lexicon->IO->WriteToLog(0, true, "Error: Illegal GDL 'legal/2' as rule input in hsfcSCL::Normalise\n");
				return false;
			}
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->NameID, "goal/2")) {
				this->Lexicon->IO->WriteToLog(0, true, "Error: Illegal GDL 'goal/2' as rule input in hsfcSCL::Normalise\n");
				return false;
			}
			if (this->Lexicon->Match(this->Rule[i]->Term[j]->NameID, "sees/2")) {
				this->Lexicon->IO->WriteToLog(0, true, "Error: Illegal GDL 'sees/2' as rule input in hsfcSCL::Normalise\n");
				return false;
			}
			if (this->Lexicon->PartialMatch(this->Rule[i]->Term[j]->NameID, "next:")) {
				this->Lexicon->IO->WriteToLog(0, true, "Error: Illegal GDL 'next/1' as rule input in hsfcSCL::Normalise\n");
				return false;
			}
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// Stratify
//-----------------------------------------------------------------------------
bool hsfcSCL::Stratify() {

	hsfcSCLStratum* NewStratum;
	hsfcInputPath NewPath;
	vector<hsfcInputPath> Path;
	vector<unsigned int> CircularPath;
	unsigned int StratumSortIndex;
	hsfcSCLStratum* Temp;
	bool Finished;

	// Populate the stratum
	this->DeleteStrata();

	// Add all the rules to a stratum
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		// Find the right stratum
		for (unsigned int j = 0; j < this->Stratum.size(); j++) {
			if (this->Stratum[j]->HasOutput(this->Rule[i]->Term[0]->NameID)) {
				this->Stratum[j]->AddRule(this->Rule[i]);
				goto NextRule;
			}
		}
		// Create a new stratum
		NewStratum = new hsfcSCLStratum(this->Lexicon);
		NewStratum->Initialise();
		NewStratum->AddRule(this->Rule[i]);
		this->Stratum.push_back(NewStratum);
NextRule:;
	}

	// Check each stratum for a circular path
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {

RepeatStratum:
		// this stratum may have been combined with another
		if (this->Stratum[i]->Output.size() == 0) continue;

		// Start the path
		this->Stratum[i]->LinearPaths = false;
		Path.clear();

		NewPath.StratumIndex = UNDEFINED;
		NewPath.InputIndex = UNDEFINED;
		NewPath.InputNameID = this->Stratum[i]->Output[0];
		Path.push_back(NewPath);

		// Trace the path of each input
		while (true) {

			// Extend the path
			this->Stratum[i]->TracePath(Path, this->Stratum);
			if (Path.size() == 1) break;

			// Is there a circular reference
			if (this->Stratum[i]->PathIsCircular(Path, CircularPath)) {
				// Group the circular strata into one
				for (unsigned int j = 1; j < CircularPath.size(); j++) {
					this->Stratum[CircularPath[0]]->Combine(this->Stratum[CircularPath[j]]);
					this->Stratum[CircularPath[j]]->Initialise();
				}
				// Repeat this stratum
				goto RepeatStratum;

			}

		} 

		// Mark this stratum as linear
		this->Stratum[i]->LinearPaths = true;

	}

	// Sort the stratum into an order of dependence
	// Make integrity checks

	// First eliminate empty stratum
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		if (this->Stratum[i]->Output.size() == 0) {
			// Delete stratum
			delete this->Stratum[i];
			this->Stratum.erase(this->Stratum.begin() + i);
			i--;
		}
	}

	// Eliminate any junk strata
	Finished = false;
	while (!Finished) {
		Finished = true;
		// Check every rule 
		for (unsigned int i = 0; i < this->Stratum.size(); i++) {
			// Is this rule a keyword
			if (this->Stratum[i]->PartialMatch("next:")) continue;
			if (this->Stratum[i]->Match("sees/2")) continue;
			if (this->Stratum[i]->Match("legal/2")) continue;
			if (this->Stratum[i]->Match("goal/2")) continue;
			if (this->Stratum[i]->Match("terminal/0")) continue;
			if (this->Stratum[i]->PartialMatch("init:")) continue;
			// Is this rule required for another rule
			if (!this->Stratum[i]->RequiredFor(this->Stratum, 0, this->Stratum.size() - 1)) {
				delete this->Stratum[i];
				this->Stratum.erase(this->Stratum.begin() + i);
				i--;
				Finished = false;
			}
		}
	}

	// Sort strata
	StratumSortIndex = this->Stratum.size();
	if (StratumSortIndex < 3) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: not enough rules in hsfcSCL::Stratify\n");
		return false;
	}

	// Add all of the (next (...)) rules
	for (unsigned int i = 0; i < StratumSortIndex; i++) {
		if (this->Stratum[i]->PartialMatch("next:")) {
			// Check the stratum for purity
			if (this->Stratum[i]->Output.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: (next ) in cyclic path in hsfcSCL::Stratify\n");
				return false;
			}
			// Transfer the stratum to the new order
			StratumSortIndex--;
			Temp = this->Stratum[i];
			this->Stratum[i] = this->Stratum[StratumSortIndex];
			this->Stratum[StratumSortIndex] = Temp;
			i--;
		}
	}

	// Find all the strata needed to calculate (next (...))
	Finished = false;
	while (!Finished) {
		Finished = true;
		// Check all remaining strata
		for (unsigned int i = 0; i < StratumSortIndex; i++) {
			// Add the rule ALAP
			if ((this->Stratum[i]->RequiredFor(this->Stratum, StratumSortIndex, this->Stratum.size() - 1)) && (!this->Stratum[i]->RequiredFor(this->Stratum, 0, StratumSortIndex - 1))) {
				// Transfer the stratum to the new order
				StratumSortIndex--;
				Temp = this->Stratum[i];
				this->Stratum[i] = this->Stratum[StratumSortIndex];
				this->Stratum[StratumSortIndex] = Temp;
				i--;
				Finished = false;
			}
		}
	}

	// Add all of the (sees ...) rules
	for (unsigned int i = 0; i < StratumSortIndex; i++) {
		if (this->Stratum[i]->Match("sees/2")) {
			// Check the stratum for purity
			if (this->Stratum[i]->Output.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: (sees ) in cyclic path in hsfcSCL::Stratify\n");
				return false;
			}
			// Transfer the stratum to the new order
			StratumSortIndex--;
			Temp = this->Stratum[i];
			this->Stratum[i] = this->Stratum[StratumSortIndex];
			this->Stratum[StratumSortIndex] = Temp;
			i--;
		}
	}

	// Find all the strata needed to calculate (sees ...)
	Finished = false;
	while (!Finished) {
		Finished = true;
		// Check all remaining strata
		for (unsigned int i = 0; i < StratumSortIndex; i++) {
			// Add the rule ALAP
			if ((this->Stratum[i]->RequiredFor(this->Stratum, StratumSortIndex, this->Stratum.size() - 1)) && (!this->Stratum[i]->RequiredFor(this->Stratum, 0, StratumSortIndex - 1))) {
				// Transfer the stratum to the new order
				StratumSortIndex--;
				Temp = this->Stratum[i];
				this->Stratum[i] = this->Stratum[StratumSortIndex];
				this->Stratum[StratumSortIndex] = Temp;
				Finished = false;
				i--;
			}
		}
	}

	// Add all of the (legal ...) rules
	for (unsigned int i = 0; i < StratumSortIndex; i++) {
		if (this->Stratum[i]->Match("legal/2")) {
			// Check the stratum for purity
			if (this->Stratum[i]->Output.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: (legal ) in cyclic path in hsfcSCL::Stratify\n");
				return false;
			}
			// Transfer the stratum to the new order
			StratumSortIndex--;
			Temp = this->Stratum[i];
			this->Stratum[i] = this->Stratum[StratumSortIndex];
			this->Stratum[StratumSortIndex] = Temp;
			i--;
		}
	}

	// Find all the strata needed to calculate (legal ...)
	Finished = false;
	while (!Finished) {
		Finished = true;
		// Check all remaining strata
		for (unsigned int i = 0; i < StratumSortIndex; i++) {
			// Add the rule ALAP
			if ((this->Stratum[i]->RequiredFor(this->Stratum, StratumSortIndex, this->Stratum.size() - 1)) && (!this->Stratum[i]->RequiredFor(this->Stratum, 0, StratumSortIndex - 1))) {
				// Transfer the stratum to the new order
				StratumSortIndex--;
				Temp = this->Stratum[i];
				this->Stratum[i] = this->Stratum[StratumSortIndex];
				this->Stratum[StratumSortIndex] = Temp;
				Finished = false;
				i--;
			}
		}
	}

	// Add all of the (goal ...) rules
	for (unsigned int i = 0; i < StratumSortIndex; i++) {
		if (this->Stratum[i]->Match("goal/2")) {
			// Check the stratum for purity
			if (this->Stratum[i]->Output.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: (goal ) in cyclic path in hsfcSCL::Stratify\n");
				return false;
			}
			// Transfer the stratum to the new order
			StratumSortIndex--;
			Temp = this->Stratum[i];
			this->Stratum[i] = this->Stratum[StratumSortIndex];
			this->Stratum[StratumSortIndex] = Temp;
			i--;
		}
	}

	// Find all the strata needed to calculate (goal ...)
	Finished = false;
	while (!Finished) {
		Finished = true;
		// Check all remaining strata
		for (unsigned int i = 0; i < StratumSortIndex; i++) {
			// Add the rule ALAP
			if ((this->Stratum[i]->RequiredFor(this->Stratum, StratumSortIndex, this->Stratum.size() - 1)) && (!this->Stratum[i]->RequiredFor(this->Stratum, 0, StratumSortIndex - 1))) {
				// Transfer the stratum to the new order
				StratumSortIndex--;
				Temp = this->Stratum[i];
				this->Stratum[i] = this->Stratum[StratumSortIndex];
				this->Stratum[StratumSortIndex] = Temp;
				Finished = false;
				i--;
			}
		}
	}

	// Add all of the (terminal) rules
	for (unsigned int i = 0; i < StratumSortIndex; i++) {
		if (this->Stratum[i]->Match("terminal/0")) {
			// Check the stratum for purity
			if (this->Stratum[i]->Output.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: (terminal) in cyclic path in hsfcSCL::Stratify\n");
				return false;
			}
			// Transfer the stratum to the new order
			StratumSortIndex--;
			Temp = this->Stratum[i];
			this->Stratum[i] = this->Stratum[StratumSortIndex];
			this->Stratum[StratumSortIndex] = Temp;
			i--;
		}
	}

	// Find all the strata needed to calculate (terminal)
	Finished = false;
	while (!Finished) {
		Finished = true;
		// Check all remaining strata
		for (unsigned int i = 0; i < StratumSortIndex; i++) {
			// Add the rule ALAP
			if ((this->Stratum[i]->RequiredFor(this->Stratum, StratumSortIndex, this->Stratum.size() - 1)) && (!this->Stratum[i]->RequiredFor(this->Stratum, 0, StratumSortIndex - 1))) {
				// Transfer the stratum to the new order
				StratumSortIndex--;
				Temp = this->Stratum[i];
				this->Stratum[i] = this->Stratum[StratumSortIndex];
				this->Stratum[StratumSortIndex] = Temp;
				Finished = false;
				i--;
			}
		}
	}

	// Add all of the (next (...)) rules
	for (unsigned int i = 0; i < StratumSortIndex; i++) {
		if (this->Stratum[i]->PartialMatch("init:")) {
			// Check the stratum for purity
			if (this->Stratum[i]->Output.size() != 1) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: (init ) in cyclic path in hsfcSCL::Stratify\n");
				return false;
			}
			// Transfer the stratum to the new order
			StratumSortIndex--;
			Temp = this->Stratum[i];
			this->Stratum[i] = this->Stratum[StratumSortIndex];
			this->Stratum[StratumSortIndex] = Temp;
			i--;
		}
	}

	// Find all the strata needed to calculate (init:)
	Finished = false;
	while (!Finished) {
		Finished = true;
		// Check all remaining strata
		for (unsigned int i = 0; i < StratumSortIndex; i++) {
			// Add the rule ALAP
			if ((this->Stratum[i]->RequiredFor(this->Stratum, StratumSortIndex, this->Stratum.size() - 1)) && (!this->Stratum[i]->RequiredFor(this->Stratum, 0, StratumSortIndex - 1))) {
				// Transfer the stratum to the new order
				StratumSortIndex--;
				Temp = this->Stratum[i];
				this->Stratum[i] = this->Stratum[StratumSortIndex];
				this->Stratum[StratumSortIndex] = Temp;
				Finished = false;
				i--;
			}
		}
	}

	// Delete all of the remaining stratum as junk
	while (StratumSortIndex > 0) {
		delete this->Stratum[0];
		this->Stratum.erase(this->Stratum.begin());
		StratumSortIndex--;
	}

	// Check strata for self references
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		// Reset
		this->Stratum[i]->SelfReferenceCount = 0;
		// Check all input against all outputs
		for (unsigned int j = 0; j < this->Stratum[i]->Output.size(); j++) {
			for (unsigned int k = 0; k < this->Stratum[i]->Input.size(); k++) {
				// Is there a match
				if (this->Stratum[i]->Input[k] == this->Stratum[i]->Output[j]) {
					this->Stratum[i]->SelfReferenceCount++;
				}
			}
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// IdentifyComplexRigids
//-----------------------------------------------------------------------------
bool hsfcSCL::IdentifyRigids() {

	bool StratumIsRigid;

	// Inspect the stratum to find the rigids
	// They are already ordered according to dependency

	// If its not in the output of a non rigid rule, then its not rigid
	this->RigidNameID.clear();

	// Add in all of the statement name ids
	for (unsigned int i = 0; i < this->Statement.size(); i++) {
		// Is it already listed, or not
		if (!this->NameIDisRigid(this->Statement[i]->NameID)) {
			this->RigidNameID.push_back(this->Statement[i]->NameID);
		}
	}
	// Add in all of the rule output name ids
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		for (unsigned int j = 0; j < this->Stratum[i]->Rule.size(); j++) {
			if (!this->NameIDisRigid(this->Stratum[i]->Rule[j]->Term[0]->NameID)) {
				this->RigidNameID.push_back(this->Stratum[i]->Rule[j]->Term[0]->NameID);
			}
		}
	}

	// Identify the rigidity of each relation name id
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {

		// Are all of its inputs rigids
		StratumIsRigid = true;
		for (unsigned int j = 0; j < this->Stratum[i]->Input.size(); j++) {
			// Ignore Distinct
			if (this->Lexicon->PartialMatch(this->Stratum[i]->Input[j], "distinct")) continue;
			// Test if it might be rigid; if the input is already classified non rigid, then it fails
			if (!this->NameIDisRigid(this->Stratum[i]->Input[j])) {
				StratumIsRigid = false;
				break;
			}
		}

		// Is this stratum NOT rigid
		if (!StratumIsRigid) {
			// Classify the outputs from the strata
			for (unsigned int j = 0; j < this->Stratum[i]->Output.size(); j++) {
				this->NameIDisNotRigid(this->Stratum[i]->Output[j]);
			}
		}

	}

	// Find the complex rigids
	//if (!this->IdentifyComplexRigids()) return false;

	return true;

}

//-----------------------------------------------------------------------------
// IdentifyComplexRigids
//-----------------------------------------------------------------------------
bool hsfcSCL::IdentifyComplexRigids() {

	vector<unsigned int> RigidInputs;
	vector<int> VariableNameID;
	vector<int> VariableCount;
	hsfcSCLAtom* Atom0;
	hsfcSCLAtom* Atomk;

	// Scan each stratum in order
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
	    // Scan each rule for two rigids with a common variable
		for (unsigned int j = 0; j < this->Stratum[i]->Rule.size(); j++) {

			// Check and record each input
			RigidInputs.clear();
			for (unsigned int k = 1; k < this->Stratum[i]->Rule[j]->Term.size(); k++) {
				// Is this relation rigid
				if (this->NameIDisRigid(this->Stratum[i]->Rule[j]->Term[k]->NameID)) {
					// Tag it
					RigidInputs.push_back(k);
				}
			}

			// Were there more than one rigid input
			while (RigidInputs.size() > 1) {
				// Look for a common variable

				// Compare the first rigid input with all of the others
				// Inspect each of the other rigid inputs
				for (unsigned int k = 1; k < RigidInputs.size(); k++) {

					// Look at each term in the 0th rigid for a variable
					this->Stratum[i]->Rule[j]->Term[RigidInputs[0]]->ResetEnumeration();
					while(Atom0 = this->Stratum[i]->Rule[j]->Term[RigidInputs[0]]->EnumerateTerms()) {
						// Is it a variable
						if (this->Lexicon->IsVariable(Atom0->NameID)) {
							// Look at each term in the kth rigid for a variable
							this->Stratum[i]->Rule[j]->Term[RigidInputs[k]]->ResetEnumeration();
							while(Atomk = this->Stratum[i]->Rule[j]->Term[RigidInputs[k]]->EnumerateTerms()) {
								// Is it the same variable
								if (Atom0->NameID == Atomk->NameID) {
									//printf("Stratum %d, Rule %d, %s == %s\n", i, j, this->Lexicon->Text(Atom0->NameID), this->Lexicon->Text(Atomk->NameID));
									this->BuildComplexRigid(i, j, RigidInputs[0], RigidInputs[k]); 
									// Add the new rigid to the list
									this->RigidNameID.push_back(this->Stratum[i]->Rule[0]->Term[0]->NameID);
									// Reprocess the same stratum, now the i+1 th 
									goto Repeat;
								}
							}
						}
					}
				}

				// Clear the first rigid input from the list
				RigidInputs.erase(RigidInputs.begin());

			}
		}
Repeat:;

	}

	return true;

}

//-----------------------------------------------------------------------------
// BuildComplexRigid
//-----------------------------------------------------------------------------
void hsfcSCL::BuildComplexRigid(unsigned int StratumIndex, unsigned int RuleIndex, unsigned int Rigid1Index, unsigned int Rigid2Index) {

	hsfcSCLStratum* NewStratum;
	hsfcSCLAtom* NewRule;
	hsfcSCLAtom* NewRelation;
	hsfcSCLAtom* NewVariable;
	hsfcSCLAtom* Atom;

	// Create the new rule schema
	NewRule = new hsfcSCLAtom(this->Lexicon);
	NewRule->Initialise();
	NewRule->NameID = this->Lexicon->Index("<=");
	NewRule->PredicateIndex = NewRule->NameID;

	// Create the head of the rule
	NewRelation = new hsfcSCLAtom(this->Lexicon);
	NewRelation->Initialise();
	NewRule->Term.push_back(NewRelation);

	// Create the body of the rule
	// One relation at a time
	NewRelation = new hsfcSCLAtom(this->Lexicon);
	NewRelation->FromSCLAtom(this->Stratum[StratumIndex]->Rule[RuleIndex]->Term[Rigid1Index]);
	NewRule->Term.push_back(NewRelation);

	NewRelation = new hsfcSCLAtom(this->Lexicon);
	NewRelation->FromSCLAtom(this->Stratum[StratumIndex]->Rule[RuleIndex]->Term[Rigid2Index]);
	NewRule->Term.push_back(NewRelation);

	// Populate all of the variables in the head of the rule
	// Enumerate the terms in the body
	NewRule->Term[1]->ResetEnumeration();
	while(Atom = NewRule->Term[1]->EnumerateTerms()) {
		// Is it a variable term
		if (this->Lexicon->IsVariable(Atom->NameID)) {
			// Create the new variable
			NewVariable = new hsfcSCLAtom(this->Lexicon);
			NewVariable->FromSCLAtom(Atom);
			NewRule->Term[0]->Term.push_back(NewVariable);
		}
	}

	NewRule->Term[2]->ResetEnumeration();
	while(Atom = NewRule->Term[2]->EnumerateTerms()) {
		// Is it a variable term
		if (this->Lexicon->IsVariable(Atom->NameID)) {
			// Create the new variable, there will be duplicates
			for (unsigned int i = 0; i < NewRule->Term[0]->Term.size(); i++) {
				if (NewRule->Term[0]->Term[i]->NameID == Atom->NameID) {
					goto SkipDuplicate;
				}
			}
			NewVariable = new hsfcSCLAtom(this->Lexicon);
			NewVariable->FromSCLAtom(Atom);
			NewRule->Term[0]->Term.push_back(NewVariable);
		}
SkipDuplicate:;
	}

	// Complete the new rule head relation
	NewRule->Term[0]->NameID = this->Lexicon->NewRigidNameID(NewRule->Term[0]->Term.size());
	NewRule->Term[0]->PredicateIndex = NewRule->Term[0]->NameID;

	// Remove the old rigids from the old rule
	// First copy the new rigid into the first input
	this->Stratum[StratumIndex]->Rule[RuleIndex]->Term[Rigid1Index]->FromSCLAtom(NewRule->Term[0]);
	// Now delete the second input
	this->Stratum[StratumIndex]->Rule[RuleIndex]->Term.erase(this->Stratum[StratumIndex]->Rule[RuleIndex]->Term.begin() + Rigid2Index);

	// Create the new stratum
	NewStratum = new hsfcSCLStratum(this->Lexicon);
	NewStratum->Initialise();
	NewStratum->AddRule(NewRule);
	this->Stratum.insert(this->Stratum.begin() + StratumIndex, NewStratum);

}

