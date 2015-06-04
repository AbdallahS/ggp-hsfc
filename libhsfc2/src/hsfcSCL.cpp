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
		sprintf(QualifiedName, "%s/%d", this->Lexicon->Text(this->PredicateIndex), this->Term.size());
		this->NameID = this->Lexicon->Index(QualifiedName);
		delete[] QualifiedName;
	} else {
		QualifiedName = new char[strlen(this->Lexicon->Text(ParentNameID)) + strlen(this->Lexicon->Text(this->PredicateIndex)) + 12];
		//sprintf(QualifiedName, "%s:%d:%s/%d", this->Lexicon->Text(ParentNameID), ArgumentIndex, this->Lexicon->Text(this->PredicateIndex), this->Term.size());
		// This is easier in Schema to create joins in rule inputs
		sprintf(QualifiedName, "%s/%d", this->Lexicon->Text(this->PredicateIndex), this->Term.size());
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
// Normalise
//-----------------------------------------------------------------------------
bool hsfcSCL::Normalise() {

	hsfcSCLAtom* NewRule;
	hsfcSCLAtom* NewStatement;
	char* Predicate;

	// Rules are guaranteed to have more than one statement from hsfcGDL::Read
	// Remove any logical functions

	this->Lexicon->IO->WriteToLog(2, true, "  Flattening\n");

	// Check each statement in each rule; not the output 
	for (unsigned int i = 0; i < this->Rule.size(); i++) {

		this->Lexicon->IO->FormatToLog(3, true, "    Rule %d\n", i, NULL);

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

		// Next Rule
		continue;

		// Repeat this rule
		RepeatRule:
		i--;

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
	if (StratumSortIndex < 4) {
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
