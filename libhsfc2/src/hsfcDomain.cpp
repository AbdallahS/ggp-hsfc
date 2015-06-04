//=============================================================================
// Project: High Speed Forward Chaining
// Module: Domain
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcDomain.h"

using namespace std;

//=============================================================================
// CLASS: hsfcDomainManager
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcDomainManager::hsfcDomainManager(hsfcLexicon* Lexicon){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->Domain = NULL;
	this->DomainSize = 0;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcDomainManager::~hsfcDomainManager(void){

	// Free the domain memory
	this->FreeDomains();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcDomainManager::Initialise(){

	// Free the domain memory
	this->FreeDomains();

}

//-----------------------------------------------------------------------------
// CreateDomains
//-----------------------------------------------------------------------------
void hsfcDomainManager::CreateDomains(hsfcSchema* Schema) {

	this->Lexicon->IO->WriteToLog(2, true, "Creating Domain Structures ...\n");

	// One domain for each relation schema
	this->Domain = new hsfcDomain[Schema->RelationSchema.size()]; 
	this->DomainSize = Schema->RelationSchema.size();

	// Build the relation domains from the schema
	// Remember the first relation is the lexicon
	for (unsigned int i = 1; i < this->DomainSize; i++) {

		// Set the header properties
		this->Domain[i].NameID = Schema->RelationSchema[i]->NameID;
		this->Domain[i].Rigid = false;
		this->Domain[i].Arity = Schema->RelationSchema[i]->DomainSchema.size();
		this->Domain[i].Size = new unsigned int[Schema->RelationSchema[i]->DomainSchema.size()];
		this->Domain[i].Complete = false;
		this->Domain[i].IDCount = 0;
		this->Domain[i].RecordSize = new unsigned int[Schema->RelationSchema[i]->DomainSchema.size()];
		this->Domain[i].Record = new hsfcDomainRecord*[Schema->RelationSchema[i]->DomainSchema.size()];

		// Create the individual domains for each argument
		for (unsigned int j = 0; j < Schema->RelationSchema[i]->DomainSchema.size(); j++) {
			this->Domain[i].RecordSize[j] = Schema->RelationSchema[i]->DomainSchema[j]->Term.size();
			this->Domain[i].Record[j] = new hsfcDomainRecord[Schema->RelationSchema[i]->DomainSchema[j]->Term.size()];
		}

	}

	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");

}

//-----------------------------------------------------------------------------
// FreeDomains
//-----------------------------------------------------------------------------
void hsfcDomainManager::FreeDomains() {

	// Are the domains already freed
	if (this->Domain == NULL) return;

	// Release the domain memory
	for (unsigned int i = 1; i < this->DomainSize; i++) {
		// Release the memory for each argument
		for (unsigned int j = 0; j < this->Domain[i].Arity; j++) {
			// Release the memory for each domain
			delete[] this->Domain[i].Record[j];
		}
		// Release the memory for each domain
		delete[] this->Domain[i].Size;
		delete[] this->Domain[i].RecordSize;
		delete[] this->Domain[i].Record;
	}
	delete[] this->Domain;
	this->Domain = NULL;
	this->DomainSize = 0;

}

//-----------------------------------------------------------------------------
// BuildDomains
//-----------------------------------------------------------------------------
bool hsfcDomainManager::BuildDomains(hsfcSchema* Schema) {

	bool CanBeBuilt;
	bool Finished;
	int BuildCount;
	unsigned int RelationIndex;
	unsigned int IndexBase;
	float SizeCheck;

	this->Lexicon->IO->WriteToLog(2, true, "Building Domain Data ...\n");

	Finished = false;
	while (!Finished) {

		Finished = true;
		BuildCount = 0;

		// Build the relation domains from the schema
		for (unsigned int i = 1; i < Schema->RelationSchema.size(); i++) {

			// Is this domain elready built
			if (this->Domain[i].Complete) continue;
			Finished = false;

			// Does the domain have entries from other relations
			CanBeBuilt = true;
			// Check every domain for every argument
			for (unsigned int j = 0; j < Schema->RelationSchema[i]->DomainSchema.size(); j++) {
				// Check every term in the domain
				for (unsigned int k = 0; k < Schema->RelationSchema[i]->DomainSchema[j]->Term.size(); k++) {
					// Is the term and embedded relation
					if (Schema->RelationSchema[i]->DomainSchema[j]->Term[k].Index > 0) {
						RelationIndex = Schema->RelationSchema[i]->DomainSchema[j]->Term[k].Index;
						// Is the domain for the embedded domain complete
						if (!this->Domain[RelationIndex].Complete) {
							CanBeBuilt = false;
							break;
						}
					}
				}
				// Can we build the domain
				if (!CanBeBuilt) break;
			}
			// Can we build the domain
			if (!CanBeBuilt) continue;

			// Build the domains
			BuildCount++;
			this->Domain[i].IDCount = 1;
			for (unsigned int j = 0; j < Schema->RelationSchema[i]->DomainSchema.size(); j++) {
				// Check every term in the domain
				IndexBase = 0;
				for (unsigned int k = 0; k < Schema->RelationSchema[i]->DomainSchema[j]->Term.size(); k++) {
					this->Domain[i].Record[j][k].Relation.Index = Schema->RelationSchema[i]->DomainSchema[j]->Term[k].Index;
					this->Domain[i].Record[j][k].Relation.ID = Schema->RelationSchema[i]->DomainSchema[j]->Term[k].ID;
					this->Domain[i].Record[j][k].IndexBase = IndexBase;
					// Calculate the size of the domain
					if (Schema->RelationSchema[i]->DomainSchema[j]->Term[k].Index == 0) {
						IndexBase++;
					} else {
						RelationIndex = Schema->RelationSchema[i]->DomainSchema[j]->Term[k].Index;
						IndexBase += this->Domain[RelationIndex].IDCount;
					}
					// Check the size of the domain
					if (IndexBase > MAX_DOMAIN_SIZE) {
						this->Lexicon->IO->WriteToLog(0, false, "Error: exceeded maximum domain size in hsfcDomainManager::BuildDomains\n");
						return false; 
					}
				}
				// Was the domain empty
				if (IndexBase == 0) {
					this->Lexicon->IO->FormatToLog(0, false, "Error: empty domain in '%s' in hsfcDomainManager::BuildDomains\n", this->Lexicon->Text(Schema->RelationSchema[i]->NameID));
					return false; 
				}
				// Calculate the size of the relation
				this->Domain[i].Size[j] = IndexBase;
				SizeCheck = (float)this->Domain[i].IDCount;
				SizeCheck = SizeCheck * (float)IndexBase;
				if (SizeCheck == MAX_RELATION_SIZE) {
					this->Lexicon->IO->WriteToLog(0, false, "Error: exceeded maximum relation size in hsfcDomainManager::BuildDomains\n");
					return false; 
				}
				this->Domain[i].IDCount = this->Domain[i].IDCount * IndexBase;

			}
			this->Domain[i].Complete = true;

		}

		// Look for a cyclic reference in embedded domains
		if ((!Finished) && (BuildCount == 0)) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: cyclic embedded reference in hsfcDomainManager::BuildDomains\n");
			return false; 
		}

	}

	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");
	if (this->Lexicon->IO->Parameters->LogDetail > 3) this->Print();

	return true;
	
}

//-----------------------------------------------------------------------------
// TermsToID
//-----------------------------------------------------------------------------
bool hsfcDomainManager::TermsToID(int RelationIndex, hsfcTuple Term[], unsigned int& ID) {

	unsigned int Result;
	unsigned int Factor;
	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;
	hsfcDomainRecord* Record;

	// For speed; there is no error checking
	// Term.size == Relation.Arity
	// The first term contains the predicate

	// Initialise the result
	Result = 0;
	Factor = 1;

	// Find the each term in the appropriate domain
	for (unsigned int i = 0; i < this->Domain[RelationIndex].Arity; i++) {

		// Binary search
		LowerBound = 0;
		UpperBound = this->Domain[RelationIndex].RecordSize[i] - 1;
		Compare = -1;
		Record = this->Domain[RelationIndex].Record[i];

		// Look for the term according to its value
		while (LowerBound <= UpperBound) {

			// Compare terms
			Target = (LowerBound + UpperBound) / 2;
			Compare = Term[i+1].Index - Record[Target].Relation.Index;

			// If the term is 0.##, we find it exactly
			// If the term in ##.0, we find its index base
			if (Compare == 0) {
				if (Term[i+1].Index == 0) {
					Compare = Term[i+1].ID - Record[Target].Relation.ID;
				}
			}

			// Have we found the matching term
			if (Compare == 0) {
				if (Term[i+1].Index == 0) {
					Result += Factor * Record[Target].IndexBase;
				} else {
					Result += Factor * (Record[Target].IndexBase + Term[i+1].ID);
				}
				Factor *= this->Domain[RelationIndex].Size[i];
				break;
			}

			// Narrow the search an go again
			if (Compare < 0) {
				UpperBound = Target - 1;
			} else {
				LowerBound = Target + 1;
			}

		}

		// Was there no match
		if (Compare != 0) return false;

	}

	// Finished 
	ID = Result;
	return true;

}

//-----------------------------------------------------------------------------
// IDToTerms
//-----------------------------------------------------------------------------
bool hsfcDomainManager::IDToTerms(int RelationIndex, hsfcTuple Term[], unsigned int ID) {

	unsigned int Factor;
	unsigned int Index;
	int Target;
	int LowerBound;
	int UpperBound;
	hsfcDomainRecord* Record;

	// For speed; there is no error checking
	// Term.size == Relation.Arity
	// The first term contains the predicate

	Term[0].ID = this->Domain[RelationIndex].NameID;
	Term[0].Index = 0;

	// Initialise the result
	Factor = ID;

	// Find the each term in the appropriate domain
	for (unsigned int i = 0; i < this->Domain[RelationIndex].Arity; i++) {

		// Calculate the domain index
		Index = Factor % this->Domain[RelationIndex].Size[i];
		Factor = Factor / this->Domain[RelationIndex].Size[i];

		// Binary search
		LowerBound = 0;
		UpperBound = this->Domain[RelationIndex].RecordSize[i] - 1;
		Target = UpperBound;
		Record = this->Domain[RelationIndex].Record[i];

		// Check if its less than the last entry
		// If not we will catch the correct record at the end of the if statement
		if (Index < Record[UpperBound].IndexBase) {

			// Reduce the upper bound
			// If the upper bound was zero; then the target is zero
			UpperBound--;

			// Look for the term according to its value
			while (LowerBound <= UpperBound) {

				// Compare terms
				Target = (LowerBound + UpperBound) / 2;

				// Have we found the matching term
				if (Index < Record[Target].IndexBase) {
					UpperBound = Target - 1;
					continue;
				}

				if (Index >= Record[Target+1].IndexBase) {
					LowerBound = Target + 1;
					continue;
				}

				// We have a match
				break;

			}

		}

		// Add the term
		if (Record[Target].Relation.Index == 0) {
			Term[i+1].Index = 0;
			Term[i+1].ID = Record[Target].Relation.ID;
		} else {
			Term[i+1].Index = Record[Target].Relation.Index;
			Term[i+1].ID = Index - Record[Target].IndexBase;
		}

	}

	// Finished 
	return true;

}

//-----------------------------------------------------------------------------
// LoadTerms
//-----------------------------------------------------------------------------
bool hsfcDomainManager::LoadTerms(hsfcSCLAtom* SCLAtom, hsfcTuple Term[]) {

	hsfcTuple NestedTerm[MAX_RELATION_ARITY + 1];
	unsigned int ID;
	unsigned int RelationIndex;

	// Recursively decode and load terms for the parent relation

	// The predicate comes first
	Term[0].Index = 0;
	Term[0].ID = SCLAtom->NameID;

	// Now for the arguments
	for (unsigned int i = 0; i < SCLAtom->Term.size(); i++) {
		// Look at each argument for an embedded relation
		if (SCLAtom->Term[i]->Term.size() == 0) {
			Term[i+1].Index = 0;
			Term[i+1].ID = SCLAtom->Term[i]->NameID;
		} else {
			if (!this->LoadTerms(SCLAtom->Term[i], NestedTerm)) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: conversion failure in hsfcDomainManager::LoadTerms\n");
				return false;
			}
			RelationIndex = this->Lexicon->RelationIndex(NestedTerm[0].ID);
			if (RelationIndex == UNDEFINED) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: bad relation name in hsfcDomainManager::LoadTerms\n");
				return false;
			}
			this->TermsToID(RelationIndex, NestedTerm, ID);
			Term[i+1].Index = RelationIndex;
			Term[i+1].ID = ID;
		}

	}

	return true;

}

//-----------------------------------------------------------------------------
// KIFLength
//-----------------------------------------------------------------------------
unsigned int hsfcDomainManager::RelationAsKIF(hsfcTuple& Relation, char** KIF) {

	hsfcTuple Term[MAX_RELATION_ARITY + 1];
	unsigned int Length;
	char* Text;

	// Initialise with brackets and terminal
	if (*KIF == NULL) {
		Length = this->KIFLength(Relation);
		*KIF = new char[Length];
		*KIF[0] = 0;
	}

	// Get the terms
	if (!this->IDToTerms(Relation.Index, Term, Relation.ID)) return 0;

	// Print the predicate
	Text = this->Lexicon->Copy(Term[0].ID, false);
	Length = sprintf(*KIF, "(%s", Text);
	delete[] Text;

	// Print every term
	for (unsigned int i = 0; i < this->Domain[Relation.Index].Arity; i++) {
		if (Term[i+1].Index == 0) {
			Length += sprintf(*KIF + Length, " %s", this->Lexicon->Text(Term[i+1].ID));
		} else {
			Text = NULL;
			RelationAsKIF(Term[i+1], &Text);
			Length += sprintf(*KIF + Length, " %s", Text);
			delete[] Text;
		}
	}

	Length += sprintf(*KIF + Length, ")");

	return Length;

}

//-----------------------------------------------------------------------------
// KIFLength
//-----------------------------------------------------------------------------
unsigned int hsfcDomainManager::KIFLength(hsfcTuple& Relation) {

	hsfcTuple Term[MAX_RELATION_ARITY + 1];
	unsigned int Result;
	char* Text;

	// Initialise with brackets and terminal
	Result = 3;

	// Get the terms
	if (!this->IDToTerms(Relation.Index, Term, Relation.ID)) return Result;
	Text = this->Lexicon->Copy(Term[0].ID, false);
	Result += strlen(Text);
	delete[] Text;

	// Add in every term removinf '/n' arity
	for (unsigned int i = 0; i < this->Domain[Relation.Index].Arity; i++) {
		if (Term[i+1].Index == 0) {
			Text = this->Lexicon->Copy(Term[i+1].ID, false);
			Result += strlen(Text) + 1;
			delete[] Text;
		} else {
			Result += this->KIFLength(Term[i+1]) + 1;
		}
	}

	return Result;

}

//-----------------------------------------------------------------------------
// TestDomains
//-----------------------------------------------------------------------------
void hsfcDomainManager::TestDomains() {

	hsfcTuple Term[32];
	unsigned int ID1;
	unsigned int ID2;

	for (unsigned int i = 1; i < this->DomainSize; i++) {
		for (unsigned int j = 0; j < this->Domain[i].IDCount; j++) {
			ID1 = j;
			this->IDToTerms(i, Term, ID1);
			this->TermsToID(i, Term, ID2);
			if (ID1 != ID2) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: conversion failure in hsfcDomainManager::TestDomains\n");
				return; 
			}
		}
	}

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcDomainManager::Print() {

	this->Lexicon->IO->WriteToLog(2, true, "------------------------------------\n");

	// Print the domain information
	for (unsigned int i = 1; i < this->DomainSize; i++) {
		this->Lexicon->IO->FormatToLog(2, true, "\n%s\n", this->Lexicon->Text(this->Domain[i].NameID));
		for (unsigned int j = 0; j < this->Domain[i].Arity; j++) {
			this->Lexicon->IO->FormatToLog(2, true, "Argument %d\n", j, NULL);
			for (unsigned int k = 0; k < this->Domain[i].RecordSize[j]; k++) {
				this->Lexicon->IO->FormatToLog(2, true, "%4d:%4d.", k, this->Domain[i].Record[j][k].Relation.Index); 
				this->Lexicon->IO->FormatToLog(2, true, "%4d%6d\n", this->Domain[i].Record[j][k].Relation.ID, this->Domain[i].Record[j][k].IndexBase); 
			}
		}
		this->Lexicon->IO->WriteToLog(2, true, "------------------------------------\n");
	}

}


