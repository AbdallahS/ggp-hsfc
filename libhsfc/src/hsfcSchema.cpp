//=============================================================================
// Project: High Speed Forward Chaining
// Module: Schema
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcSchema.h"

#include "hsfc_config.h"

using namespace std;

//=============================================================================
// Project: High Speed Forward Chaining
// Module: Relation
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include "hsfcSchema.h"

using namespace std;

//=============================================================================
// CLASS: hsfcRelationSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRelationSchema::hsfcRelationSchema(hsfcLexicon* Lexicon){

	// Set up the Lexicon
	this->Lexicon = Lexicon;

	// Initialise the pointers
	this->Domain = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcRelationSchema::~hsfcRelationSchema(void){

	// Free the domains
	for (unsigned int i = 0; i < this->vDomain.size(); i++) {
		this->vDomain[i].clear();
	}
	this->vDomain.clear();
	if (this->Domain != NULL) {
		for (int i = 0; i < this->Arity; i++) {
			delete[] this->Domain[i]->Entry;
			delete this->Domain[i];
		}
		delete[] this->Domain;
	}

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRelationSchema::Initialise(int PredicateIndex, int Arity){

	// Free the domains
	for (unsigned int i = 0; i < this->vDomain.size(); i++) {
		this->vDomain[i].clear();
	}
	this->vDomain.clear();
	if (this->Domain != NULL) {
		for (int i = 0; i < this->Arity; i++) {
			delete[] this->Domain[i]->Entry;
			delete this->Domain[i];
		}
		delete[] this->Domain;
	}
	this->Domain = NULL;

	// Allocate the memory
	this->vDomain.resize(Arity);

	// Set the properties
	this->PredicateIndex = PredicateIndex;
	this->Arity = Arity;
	this->Fact = hsfcFactNone;
	this->AveListLength = 1;
	this->Samples = 0;
	this->Index = 0;
	this->DomainIsComplete = false;
	this->IsInState = false;
	this->HasComplexEntries = true;
	this->IDCount = 0;
	this->IDCountDbl = 0;

}

//-----------------------------------------------------------------------------
// FromRelation
//-----------------------------------------------------------------------------
void hsfcRelationSchema::FromRelationSchema(hsfcRelationSchema* Source){

	this->Initialise(Source->PredicateIndex, Source->Arity);

	// Copy the domains
	for (unsigned int i = 0; i < Source->vDomain.size(); i++) {
		for (unsigned int j = 0; j < Source->vDomain[i].size(); j++) {
			this->vDomain[i].push_back(Source->vDomain[i][j]);
		}
	}

	// Create the domain arrays
	if (Source->Domain != NULL) {
		this->Domain = new hsfcDomain*[Source->Arity];
		for (int i = 0; i < Source->Arity; i++) {
			this->Domain[i] = new hsfcDomain();
			this->Domain[i]->Entry = new hsfcDomainEntry[Source->Domain[i]->Size];
			this->Domain[i]->Size = Source->Domain[i]->Size;
			this->Domain[i]->Count = Source->Domain[i]->Count;
			for (int j = 0; j < Source->Domain[i]->Size; j++) {
				this->Domain[i]->Entry[j].Tuple.ID = Source->Domain[i]->Entry[j].Tuple.ID;
				this->Domain[i]->Entry[j].Tuple.RelationIndex = Source->Domain[i]->Entry[j].Tuple.RelationIndex;
				this->Domain[i]->Entry[j].Count = Source->Domain[i]->Entry[j].Count;
				this->Domain[i]->Entry[j].Index = Source->Domain[i]->Entry[j].Index;
			}
		}
	}

	// Copy the properties
	this->PredicateIndex = Source->PredicateIndex;
	this->Arity = Source->Arity;
	this->Fact = Source->Fact;
	this->AveListLength = Source->AveListLength;
	this->Samples = Source->Samples;
	this->Index = Source->Index;
	this->DomainIsComplete = Source->DomainIsComplete;
	this->IsInState = Source->IsInState;

}

//-----------------------------------------------------------------------------
// AddToDomain
//-----------------------------------------------------------------------------
void hsfcRelationSchema::AddToDomain(int DomainIndex, hsfcDomainEntry* Entry) {

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;

    // Binary search of a specific domain; adding any unfound values in sort order
	Target = 0;
	LowerBound = 0;
	UpperBound = 0;
	Compare = -1;
	
	// Is the domain empty
	if (this->vDomain[DomainIndex].size() > 0) {

		UpperBound = this->vDomain[DomainIndex].size() - 1;

		// Look for the term according to its value
		while (LowerBound <= UpperBound) {
			// Find the target value
			Target = (LowerBound + UpperBound) / 2;
			Compare = Entry->Tuple.RelationIndex - this->vDomain[DomainIndex][Target].Tuple.RelationIndex;
			if (Compare == 0) {
				if (this->vDomain[DomainIndex][Target].Tuple.ID == -1) {
					Compare = 0;
				} else {
					Compare = Entry->Tuple.ID - this->vDomain[DomainIndex][Target].Tuple.ID;
				}
			}
			// Compare the values
			if (Compare == 0) return;
			if (Compare < 0) UpperBound = Target - 1;
			if (Compare > 0) LowerBound = Target + 1;
		}
	}

	// Not found
	// Last compare will have LowerBound == UpperBound == Target
	// If Compare > 0 then new value is after Target
	// If Compare < 0 then new value is before Target
	if (Compare > 0) Target++;

	// Add the new value to the domain before the Target
	if (Target == this->vDomain[DomainIndex].size()) {
		this->vDomain[DomainIndex].push_back(*Entry);
	} else {
		this->vDomain[DomainIndex].insert(this->vDomain[DomainIndex].begin() + Target, *Entry);
	}

}

//-----------------------------------------------------------------------------
// AddToFactDomain
//-----------------------------------------------------------------------------
void hsfcRelationSchema::AddToFactDomain(vector<hsfcDomainEntry>& Entry) {

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;

	// TermIndex(0) = Predicate Index
	// Is there anything to add
	if (Entry.size() != this->vDomain.size() + 1) return; 

    // Binary search of all domains; adding any unfound values in sort order
	Target = 0;
	LowerBound = 0;
	UpperBound = 0;
	Compare = -1;
	
	// Is the domain empty
	if (this->vDomain[0].size() > 0) {

		UpperBound = this->vDomain[0].size() - 1;

		// Look for the term according to its value
		while (LowerBound <= UpperBound) {
			// Find the target value
			Target = (LowerBound + UpperBound) / 2;

			// Check all of the domains for a match
			for (unsigned int j = 0; j < this->vDomain.size(); j++) {
				Compare = Entry[j+1].Tuple.RelationIndex - this->vDomain[j][Target].Tuple.RelationIndex;
				if (Compare == 0) Compare = Entry[j+1].Tuple.ID - this->vDomain[j][Target].Tuple.ID;
				if (Compare != 0) break;
			}

			// Compare the values
			if (Compare == 0) return;
			if (Compare < 0) UpperBound = Target - 1;
			if (Compare > 0) LowerBound = Target + 1;
		}
	}

	// Not found
	// Last compare will have LowerBound == UpperBound == Target
	// If Compare > 0 then new value is after Target
	// If Compare < 0 then new value is before Target
	if (Compare > 0) Target++;

	// Add the new value to the domain before the Target
	if (Target == this->vDomain[0].size()) {
		for (unsigned int j = 0; j < this->vDomain.size(); j++) {
			this->vDomain[j].push_back(Entry[j+1]);
		}
	} else {
		for (unsigned int j = 0; j < this->vDomain.size(); j++) {
			this->vDomain[j].insert(this->vDomain[j].begin() + Target, Entry[j+1]);
		}
	}

}

//-----------------------------------------------------------------------------
// IndexDomains
//-----------------------------------------------------------------------------
void hsfcRelationSchema::IndexDomains() {

	int Index;
	double Count;

	// Convert the vectors to arrays for faster access
	this->Domain = new hsfcDomain*[this->Arity];
	for (int i = 0; i < this->Arity; i++) {
		this->Domain[i] = new hsfcDomain();
		this->Domain[i]->Size = this->vDomain[i].size();
		this->Domain[i]->Entry = new hsfcDomainEntry[this->Domain[i]->Size];
		for (unsigned int j = 0; j < this->vDomain[i].size(); j++) {
			this->Domain[i]->Entry[j].Tuple.ID = this->vDomain[i][j].Tuple.ID;
			this->Domain[i]->Entry[j].Tuple.RelationIndex = this->vDomain[i][j].Tuple.RelationIndex;
			this->Domain[i]->Entry[j].Count = this->vDomain[i][j].Count;
			this->Domain[i]->Entry[j].Index = this->vDomain[i][j].Index;
		}
		this->vDomain[i].clear();
	}
	this->vDomain.clear();

	// Index each domain
	this->HasComplexEntries = false;
	for (int i = 0; i < this->Arity; i++) {
		Index = 0;
		for (int j = 0; j < this->Domain[i]->Size; j++) {
			this->Domain[i]->Entry[j].Index = Index;
			Index += this->Domain[i]->Entry[j].Count;
			if (this->Domain[i]->Entry[j].Tuple.ID == -1) this->HasComplexEntries = true;
		}
		this->Domain[i]->Count = Index;
	}

	// Caclulate the size of the relation
	Count = 1;
	if (this->Fact == hsfcFactPermanent) {
		// Calculate the number of TupleIDs
		if (this->Arity > 0) {
			Count = (double)this->Domain[0]->Count;
		}
	} else {
		// Calculate the number of TupleIDs
		for (int i = 0; i < this->Arity; i++) {
			Count = Count * (double)this->Domain[i]->Count;
		}
	}

	// Set the size of the relation
	this->IDCountDbl = Count;
	if (Count > (double)INT_MAX) {
		this->IDCount = INT_MAX;
	} else {
		this->IDCount = (int)this->IDCountDbl;
	}

}

//-----------------------------------------------------------------------------
// IntersectBufferDomain
//-----------------------------------------------------------------------------
void hsfcRelationSchema::IntersectBufferDomain(vector<hsfcTuple>& BufferDomain, unsigned int DomainIndex) {

	// Return the intersection of the domains
	// The Buffer domain is sorted, the Schema relation domain may not be
	// This is a real problem as we need Schema relation domain to be sorted.

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;
	vector<bool> Found;

	// Is the buffer domain empty
	if (BufferDomain.size() == 0) return;

	// Set the found to false for all buffer domain elements
	for (unsigned int i = 0; i < BufferDomain.size(); i++) {
		Found.push_back(false);
	}

    // Go through each term in the domain (may be unsorted if its a permanent fact)
	for (int i = 0; i < this->Domain[DomainIndex]->Size; i++) {
	
		// Binary search buffer domain; adding any unfound values in sort order
		Target = 0;
		LowerBound = 0;
		UpperBound = BufferDomain.size() - 1;
		Compare = -1;
		
		// Look for the term according to its value
		while (LowerBound <= UpperBound) {
			// Find the target value
			Target = (LowerBound + UpperBound) / 2;
			Compare = this->Domain[DomainIndex]->Entry[i].Tuple.RelationIndex - BufferDomain[Target].RelationIndex;
			if (Compare == 0) {
				if (this->Domain[DomainIndex]->Entry[i].Tuple.ID == -1) {
					Compare = 0;
				} else {
					Compare = this->Domain[DomainIndex]->Entry[i].Tuple.ID - BufferDomain[Target].ID;
				}
			}
			// Compare the values
			if (Compare == 0) break;
			if (Compare < 0) UpperBound = Target - 1;
			if (Compare > 0) LowerBound = Target + 1;
		}

		// Was it in both
		if (Compare == 0) Found[Target] = true;

	}

	// Cull any buffer domain entries not found
	for (int i = Found.size() - 1; i >= 0; i--) {
		if (!Found[i]) BufferDomain.erase(BufferDomain.begin() + i);
	}

}

//-----------------------------------------------------------------------------
// AddToBufferDomain
//-----------------------------------------------------------------------------
void hsfcRelationSchema::AddToBufferDomain(vector<hsfcTuple>& BufferDomain, unsigned int DomainIndex) {

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;
	hsfcDomainEntry DomainEntry;
	int BaseID;

	// Load the domain into the buffer domain, sorted

    // Go through each term in the domain (may be unsorted if its a permanent fact)
	for (int i = 0; i < this->Domain[DomainIndex]->Size; i++) {

		// Get the domain entry
		DomainEntry.Tuple.RelationIndex = this->Domain[DomainIndex]->Entry[i].Tuple.RelationIndex;
		BaseID = this->Domain[DomainIndex]->Entry[i].Tuple.ID;
		if (BaseID == -1) BaseID = 0;
		DomainEntry.Count = this->Domain[DomainIndex]->Entry[i].Count;

		// This may be a list of entries
		for (int j = 0; j < DomainEntry.Count; j++) {

			DomainEntry.Tuple.ID = BaseID + j;
	
			// Binary search buffer domain; adding any unfound values in sort order
			Target = 0;
			LowerBound = 0;
			UpperBound = 0;
			Compare = -1;
			
			// Is the domain empty
			if (BufferDomain.size() > 0) {

				UpperBound = BufferDomain.size() - 1;

				// Look for the term according to its value
				while (LowerBound <= UpperBound) {
					// Find the target value
					Target = (LowerBound + UpperBound) / 2;
					Compare = DomainEntry.Tuple.RelationIndex - BufferDomain[Target].RelationIndex;
					if (Compare == 0) Compare = DomainEntry.Tuple.ID - BufferDomain[Target].ID;
					// Compare the values
					if (Compare == 0) break;
					if (Compare < 0) UpperBound = Target - 1;
					if (Compare > 0) LowerBound = Target + 1;
				}
			}

			// Was it added already
			if (Compare == 0) continue;

			// Not found
			// Last compare will have LowerBound == UpperBound == Target
			// If Compare > 0 then new value is after Target
			// If Compare < 0 then new value is before Target
			if (Compare > 0) Target++;

			// Add the new value to the domain before the Target
			if (Target == BufferDomain.size()) {
				BufferDomain.push_back(DomainEntry.Tuple);
			} else {
				BufferDomain.insert(BufferDomain.begin() + Target, DomainEntry.Tuple);
			}

		}

	}

}

//-----------------------------------------------------------------------------
// ID
//-----------------------------------------------------------------------------
int hsfcRelationSchema::ID(vector<hsfcTuple>& Term) {

	int Index;
	int Result;
	int Factor;
	bool AllMatch;

	// Is this prior to the iindexing of the domains
	if (this->Domain == NULL) return this->vID(Term);

	// TermIndex(0) = Predicate Index
	// Initial value
	Result = -1;

	// Are there some terms
	if (Term.size() < this->Arity + 1) return Result;
	
	// Is it a permanent fact
	if (this->Fact == hsfcFactPermanent) {

		// Check the domains
		for (Index = 0; Index < (int)this->Domain[0]->Size; Index++) {
			// Check that all terms match
			AllMatch = true;
			for (int i = 0; i < this->Arity; i++) {
				// Is it a match
				if ((Term[i+1].ID != this->Domain[i]->Entry[Index].Tuple.ID) || (Term[i+1].RelationIndex != this->Domain[i]->Entry[Index].Tuple.RelationIndex)) {
					AllMatch = false;
					break;
				}
			}
			// Found it so exit
			if (AllMatch) {
				Result = Index;
				break;
			}
		}

		return Result;

	} else {

		// Calculate the Relation ID
		Result = 0;
		Factor = 1;
		for (int i = 0; i < this->Arity; i++) {
			Index = -1;
			// Look for the tuple in the domain entries
			for (int j = 0; j < this->Domain[i]->Size; j++) {
				if (this->Domain[i]->Entry[j].Tuple.ID == -1) {
					if (this->Domain[i]->Entry[j].Tuple.RelationIndex == Term[i+1].RelationIndex) {
						Index = this->Domain[i]->Entry[j].Index + Term[i+1].ID;
						break;
					}
				} else {
					if ((this->Domain[i]->Entry[j].Tuple.ID == Term[i+1].ID) && (this->Domain[i]->Entry[j].Tuple.RelationIndex == Term[i+1].RelationIndex)){
						Index = this->Domain[i]->Entry[j].Index;
						break;
					}
				}
			}
			// Is it an error: this value is used as fail in condition references
			if (Index == -1) return -1;

			// Calculate the ID
			Result = Result + Factor * Index;
			Factor = Factor * this->Domain[i]->Size;
		}

		return Result;

	}

}

//--- overload ----------------------------------------------------------------
int hsfcRelationSchema::ID(hsfcRuleCompactTerm Term[], int Offset, int NumTerms) {

	int Index;
	int Result;
	int Factor;
	bool AllMatch;

	// TermIndex(0) = Predicate Index
	// Initial value
	Result = -1;

	// Are there some terms
	if (NumTerms + Offset < (int)this->Arity + 1) return Result;
	
	// Is it a permanent fact
	if (this->Fact == hsfcFactPermanent) {

		// Check the domains
		for (Index = 0; Index < (int)this->Domain[0]->Size; Index++) {
			// Check that all terms match
			AllMatch = true;
			for (int i = 0; i < this->Arity; i++) {
				// Is it a match
				if ((Term[Offset+i+1].Tuple.ID != this->Domain[i]->Entry[Index].Tuple.ID) || (Term[Offset+i+1].Tuple.RelationIndex != this->Domain[i]->Entry[Index].Tuple.RelationIndex)) {
					AllMatch = false;
					break;
				}
			}
			// Found it so exit
			if (AllMatch) {
				Result = Index;
				break;
			}
		}

		return Result;

	} else {

		// Calculate the Relation ID
		Result = 0;
		Factor = 1;
		for (int i = 0; i < this->Arity; i++) {
			Index = -1;
			for (int j = 0; j < this->Domain[i]->Size; j++) {
				if (this->Domain[i]->Entry[j].Tuple.ID == -1) {
					if (this->Domain[i]->Entry[j].Tuple.RelationIndex == Term[Offset+i+1].Tuple.RelationIndex) {
						Index = this->Domain[i]->Entry[j].Index + Term[Offset+i+1].Tuple.ID;
						break;
					}
				} else {
					if ((this->Domain[i]->Entry[j].Tuple.ID == Term[Offset+i+1].Tuple.ID) && (this->Domain[i]->Entry[j].Tuple.RelationIndex == Term[Offset+i+1].Tuple.RelationIndex)){
						Index = this->Domain[i]->Entry[j].Index;
						break;
					}
				}
			}
			// Is it an error
			if (Index == -1) return -1;
			// Calculate the ID
			Result = Result + Factor * Index;
			Factor = Factor * this->Domain[i]->Size;
		}

		return Result;

	}

}

//-----------------------------------------------------------------------------
// vID
//-----------------------------------------------------------------------------
int hsfcRelationSchema::vID(vector<hsfcTuple>& Term) {

	int Index;
	int Result;
	int Factor;
	bool AllMatch;

	// TermIndex(0) = Predicate Index
	// Initial value
	Result = -1;

	// Are there some terms
	if (Term.size() < this->Arity + 1) return Result;
	
	// Is it a permanent fact
	if (this->Fact == hsfcFactPermanent) {

		// Check the domains
		for (Index = 0; Index < (int)this->vDomain[0].size(); Index++) {
			// Check that all terms match
			AllMatch = true;
			for (int i = 0; i < this->Arity; i++) {
				// Is it a match
				if ((Term[i+1].ID != this->vDomain[i][Index].Tuple.ID) || (Term[i+1].RelationIndex != this->vDomain[i][Index].Tuple.RelationIndex)) {
					AllMatch = false;
					break;
				}
			}
			// Found it so exit
			if (AllMatch) {
				Result = Index;
				break;
			}
		}

		return Result;

	} else {

		// Calculate the Relation ID
		Result = 0;
		Factor = 1;
		for (int i = 0; i < this->Arity; i++) {
			Index = -1;
			// Look for the tuple in the domain entries
			for (unsigned int j = 0; j < this->vDomain[i].size(); j++) {
				if (this->vDomain[i][j].Tuple.ID == -1) {
					if (this->vDomain[i][j].Tuple.RelationIndex == Term[i+1].RelationIndex) {
						Index = this->vDomain[i][j].Index + Term[i+1].ID;
						break;
					}
				} else {
					if ((this->vDomain[i][j].Tuple.ID == Term[i+1].ID) && (this->vDomain[i][j].Tuple.RelationIndex == Term[i+1].RelationIndex)){
						Index = this->vDomain[i][j].Index;
						break;
					}
				}
			}
			// Is it an error: this value is used as fail in condition references
			if (Index == -1) return -1;

			// Calculate the ID
			Result = Result + Factor * Index;
			Factor = Factor * this->vDomain[i].size();
		}

		return Result;

	}

}

//-----------------------------------------------------------------------------
// RelationSize
//-----------------------------------------------------------------------------
double hsfcRelationSchema::RelationSize() {

	int Index;
	double Result;

	Result = 1;

	if (this->Fact == hsfcFactPermanent) {

		// Calculate the number of TupleIDs
		if (this->Arity > 0) {
			if (this->Domain == NULL) {
				Result = (double)this->vDomain[0].size();
			} else {
				Result = (double)this->Domain[0]->Size;
			}
		}

		return Result;

	} else {

		// Calculate the number of TupleIDs
		for (int i = 0; i < this->Arity; i++) {

			Index = 0;
			if (this->Domain == NULL) {
				for (unsigned int j = 0; j < this->vDomain[i].size(); j++) {
					Index += this->vDomain[i][j].Count;
				}
			} else {
				for (int j = 0; j < this->Domain[i]->Size; j++) {
					Index += this->Domain[i]->Entry[j].Count;
				}
			}
			Result = Result * (double)Index;
		}

		return Result;

	}
	
}

//-----------------------------------------------------------------------------
// Terms
//-----------------------------------------------------------------------------
void hsfcRelationSchema::Terms(int ID, vector<hsfcTuple>& Term) {

	int DomainIndex;
	hsfcTuple NewTerm;

	// Clear the terms
	Term.clear();

	// Set the terms in the vector
	NewTerm.RelationIndex = -1;
	NewTerm.ID = this->PredicateIndex;
	Term.push_back(NewTerm);

	// Is it a fact
	if (this->Fact == hsfcFactPermanent) {

		for (int i = 0; i < this->Arity; i++) {
			Term.push_back(this->Domain[i]->Entry[ID].Tuple);
		}

	} else {

		// Are there complex entries in the domain
		if (this->HasComplexEntries) {

			// Calculate the Argument Index
			NewTerm.ID = -1;
			for (int i = 0; i < this->Arity; i++) {
				DomainIndex = ID % this->Domain[i]->Count;
				for (int j = 0; j < this->Domain[i]->Size; j++) {
					if (DomainIndex < this->Domain[i]->Entry[j].Index + this->Domain[i]->Entry[j].Count) {
						NewTerm.RelationIndex = this->Domain[i]->Entry[j].Tuple.RelationIndex;
						if (this->Domain[i]->Entry[j].Tuple.ID == -1) {
							NewTerm.ID = DomainIndex - this->Domain[i]->Entry[j].Index;
						} else {
							NewTerm.ID = this->Domain[i]->Entry[j].Tuple.ID;
						}
						break;
					}
				}
				Term.push_back(NewTerm);
				ID = ID / this->Domain[i]->Count;
			}

		} else {

			// Calculate the Argument Index
			for (int i = 0; i < this->Arity; i++) {
				DomainIndex = ID % this->Domain[i]->Count;
				NewTerm.ID = this->Domain[i]->Entry[DomainIndex].Tuple.ID;
				Term.push_back(NewTerm);
				ID = ID / this->Domain[i]->Count;
			}

		}

	}

}

//--- Overload ----------------------------------------------------------------
void hsfcRelationSchema::Terms(int ID, vector<hsfcRuleTerm>& Term) {

	int DomainIndex;
	hsfcRuleTerm NewTerm;

	// Clear the terms
	Term.clear();

	// Set the terms in the vector
	NewTerm.RelationIndex = this->Index;
	NewTerm.Tuple.RelationIndex = -1;
	NewTerm.Tuple.ID = this->PredicateIndex;
	Term.push_back(NewTerm);

	// Is it a fact
	if (this->Fact == hsfcFactPermanent) {

		for (int i = 0; i < this->Arity; i++) {
			NewTerm.Tuple.RelationIndex = this->Domain[i]->Entry[ID].Tuple.RelationIndex;
			NewTerm.Tuple.ID = this->Domain[i]->Entry[ID].Tuple.ID;
			Term.push_back(NewTerm);
		}

	} else {

		// Are there complex entries in the domain
		if (this->HasComplexEntries) {

			// Calculate the Argument Index
			NewTerm.Tuple.ID = -1;
			for (int i = 0; i < this->Arity; i++) {
				DomainIndex = ID % this->Domain[i]->Count;
				for (int j = 0; j < this->Domain[i]->Size; j++) {
					if (DomainIndex < this->Domain[i]->Entry[j].Index + this->Domain[i]->Entry[j].Count) {
						NewTerm.Tuple.RelationIndex = this->Domain[i]->Entry[j].Tuple.RelationIndex;
						if (this->Domain[i]->Entry[j].Tuple.ID == -1) {
							NewTerm.Tuple.ID = DomainIndex - this->Domain[i]->Entry[j].Index;
						} else {
							NewTerm.Tuple.ID = this->Domain[i]->Entry[j].Tuple.ID;
						}
						break;
					}
				}
				Term.push_back(NewTerm);
				ID = ID / this->Domain[i]->Count;
			}

		} else {

			// Calculate the Argument Index
			for (int i = 0; i < this->Arity; i++) {
				DomainIndex = ID % this->Domain[i]->Count;
				NewTerm.Tuple.ID = this->Domain[i]->Entry[DomainIndex].Tuple.ID;
				Term.push_back(NewTerm);
				ID = ID / this->Domain[i]->Count;
			}

		}
	}

}

//-----------------------------------------------------------------------------
// GetDomainCount
//-----------------------------------------------------------------------------
int hsfcRelationSchema::GetDomainCount(int Index) {

	return this->Domain[Index]->Count;

}

//-----------------------------------------------------------------------------
// ListDomain
//-----------------------------------------------------------------------------
void hsfcRelationSchema::ListDomain(vector<string>& List) {

	// Clear the vector
	List.clear();

	// Add in the entries
	if (this->Domain == NULL) {

		for (int i = 0; i < this->vDomain[0].size(); i++) {
			if (this->vDomain[0][i].Tuple.ID == -1) {
				List.push_back(this->Lexicon->Text(this->vDomain[0][i].Tuple.ID));
			}
		}

	} else {

		for (int i = 0; i < this->Domain[0]->Size; i++) {
			if (this->Domain[0]->Entry->Tuple.RelationIndex == -1) {
				List.push_back(this->Lexicon->Text(this->Domain[0]->Entry[i].Tuple.ID));
			}
		}

	}

}

//-----------------------------------------------------------------------------
// PrintDomains
//-----------------------------------------------------------------------------
void hsfcRelationSchema::PrintDomains() {

	bool AllDone;

	if (this->Domain == NULL) {

		// Print the domains up to a limit
		for (unsigned int n = 0; n < 32; n++) {
			AllDone = true;
			for (int i = 0; i < this->Arity; i++) {
				if (n < this->vDomain[i].size()) {
					printf("%2d.%d %d %d\t", this->vDomain[i][n].Tuple.RelationIndex, this->vDomain[i][n].Tuple.ID, this->vDomain[i][n].Count, this->vDomain[i][n].Index);
					AllDone = false;
				} else {
					printf("\t");
				}
			}
			printf("\n");
			if (AllDone) break;
		}

	} else {

		// Print the domains up to a limit
		for (int n = 0; n < 32; n++) {
			AllDone = true;
			for (int i = 0; i < this->Arity; i++) {
				if (n < this->Domain[i]->Size) {
					printf("%2d.%d %d %d\t", this->Domain[i]->Entry[n].Tuple.RelationIndex, this->Domain[i]->Entry[n].Tuple.ID, this->Domain[i]->Entry[n].Count, this->Domain[i]->Entry[n].Index);
					AllDone = false;
				} else {
					printf("\t");
				}
			}
			printf("\n");
			if (AllDone) break;
		}

	}

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRelationSchema::Print() {

	// Print the Relation
	printf("%s / %d\n", this->Lexicon->Text(this->PredicateIndex), this->Arity);
	printf("     IDCount = %d\n", this->IDCount);
	printf("        Fact = %d\n", this->Fact);
	printf("   AveLength = %.2f\n", this->AveListLength);
	printf(".....................................\n");
	this->PrintDomains();
	printf("-------------------------------------\n");

}

//=============================================================================
// CLASS: hsfcRuleRelationSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRuleRelationSchema::hsfcRuleRelationSchema(hsfcLexicon* Lexicon){

	// Allocate the memory
	this->Template.resize(16);

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcRuleRelationSchema::~hsfcRuleRelationSchema(void){


}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRuleRelationSchema::Initialise(){

	// Initialise
	this->PredicateIndex = 0;
	this->Arity = 0;
	this->Fact = hsfcFactNone;
	this->Function = hsfcFunctionNone;
	this->Type = hsfcRuleNone;
	this->SortOrder = 0;
	this->ReferenceSize = 0;

	// Reset the Terms
	this->Template.clear();

}

//-----------------------------------------------------------------------------
// FromRuleRelationSchema
//-----------------------------------------------------------------------------
void hsfcRuleRelationSchema::FromRuleRelationSchema(hsfcRuleRelationSchema* Source) {

	// Initialise the rule relation
	this->Initialise();

	// Copy the properties
	this->PredicateIndex = Source->PredicateIndex;
	this->Arity = Source->Arity;
	this->Fact = Source->Fact;
	this->Function = Source->Function;

	// Copy the Terms
	for (unsigned int i = 0; i < Source->Template.size(); i++) {
		this->Template.push_back(Source->Template[i]);
	}

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
void hsfcRuleRelationSchema::Create(vector<hsfcGDLTerm>& GDLTerm, vector<hsfcBufferTerm>& Buffer) {

	bool Found;
	hsfcRuleTemplate NewTemplate;
	hsfcBufferTerm NewBufferTerm;
	int Index;

	// Validate arguments
	if (GDLTerm.size() < 1) {
		printf("Error: No terms\n");
		abort();
	}

	// Initialise the rule relation
	this->Initialise();

	// Is it a DISTINCT function
	if (this->Lexicon->Match(GDLTerm[0].Tuple.ID, "distinct")) {
		this->Function = (this->Function | hsfcFunctionDistinct);
	}
	
	// Set the properties
	this->PredicateIndex = GDLTerm[0].Tuple.ID;
	this->Arity = GDLTerm.size() - 1;

	// Now get the terms and fill the Term array
	for (unsigned int i = 0; i < GDLTerm.size(); i++) {

		// Set the new term properties
		NewTemplate.PredicateIndex = GDLTerm[i].PredicateIndex;
		NewTemplate.RelationSchema = NULL;
		NewTemplate.Tuple.RelationIndex = GDLTerm[i].Tuple.RelationIndex;
		NewTemplate.Tuple.ID = GDLTerm[i].Tuple.ID;
		NewTemplate.BufferIndex = -1;
		NewTemplate.ArgumentIndex = -1;
		NewTemplate.DomainSize = 1;
		NewTemplate.Fixed = !this->Lexicon->IsVariable(GDLTerm[i].Tuple.ID);

		// Add the new term
		this->Template.push_back(NewTemplate);

	}

	// Go through the Terms and identify their Argument Number
	// (legal ?player (mark ?x ?y))
	// (0      1      (0     1  2))
	for (unsigned int i = 0; i < this->Template.size(); i++) {

		// Is this part of a new relation
		if ((i == 0) || (this->Template[i].PredicateIndex != this->Template[i-1].PredicateIndex)) {
			// Count back and look for this to be an argument of a parent relation
			Index = 0;
			if (i > 0) {
				for (int j = i - 1; j >= 0; j--) {
					if (this->Template[i].PredicateIndex == this->Template[j].PredicateIndex) {
						Index = this->Template[j].ArgumentIndex + 1;
						break;
					}
				}
			}
		}

		// Set the argument index
		this->Template[i].ArgumentIndex = Index;
		Index++;
	}

	// Correct for rolled up relations
	// (legal ?player (mark ?x ?y))
	// (0      1      (0     1  2))
	// (legal ?player (?mark))
	// (0      1        2)


	// Go through the GDLTerms (Buffer) and align the variables
	for (unsigned int i = 0; i < this->Template.size(); i++) {

		// Is it a variable
		if (!this->Template[i].Fixed) {

			// Find it in the rule Terms list
			Found = false;
			for (unsigned int j = 0; j < Buffer.size(); j++) {
				// Do the terms match
				if ((this->Template[i].Tuple.RelationIndex == Buffer[j].RelationIndex) && (this->Template[i].Tuple.ID == Buffer[j].ID)) {
					this->Template[i].BufferIndex = j;
					Found = true;
					break;
				}
			}

			// Was it a new Term
			if (!Found) {
				// Set the new term properties
				NewBufferTerm.RelationIndex = GDLTerm[i].Tuple.RelationIndex;
				NewBufferTerm.ID = GDLTerm[i].Tuple.ID;

				// Add the new term to the buffer
				this->Template[i].BufferIndex = Buffer.size();
				Buffer.push_back(NewBufferTerm);
			}
		}
	}


}

//-----------------------------------------------------------------------------
// Terms
//-----------------------------------------------------------------------------
bool hsfcRuleRelationSchema::Terms(int TupleID, vector<hsfcRuleTerm>& Term) {

	vector<hsfcRuleTerm> NestedTerm;
	hsfcRelationSchema* Relation;

	// Clear the Term; this is done by the call below

	// Use the underlying relation functionality to get the terms
	this->Template[0].RelationSchema->Terms(TupleID, Term);

	// The Term array will be in its most compressed form
	// eg. (cell ?0 ?1)
	// But the rule may require an expanded form
	// eg. (cell (cell~0~at ?0 ?1) ?2)

	Relation = this->Template[0].RelationSchema;
	for (unsigned int i = 1; i < this->Template.size(); i++) {

		// Do we need to expand this term
		if ((this->Template[i].RelationSchema != Relation) && (this->Template[i].RelationSchema != this->Template[0].RelationSchema)) {

			// Is the term from the correct relation
			if (Term[i].Tuple.RelationIndex != this->Template[i].RelationSchema->Index) {
				return false;
			}

			// Get the relation for this term
			Relation = this->Template[i].RelationSchema;
			// Get the nested relation
			Relation->Terms(Term[i].Tuple.ID, NestedTerm);
			// Delete the term and insert the nested terms
			Term.erase(Term.begin() + i);
			for (unsigned int j = 0; j < NestedTerm.size(); j++) {
				Term.insert(Term.begin() + i + j, NestedTerm[j]);
			}

		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// ID
//-----------------------------------------------------------------------------
int hsfcRuleRelationSchema::ID(vector<hsfcTuple>& Term, bool Validate) {

	//hsfcRelationSchema* Relation;
	hsfcRelationSchema* ParentRelationSchema;
	vector<hsfcTuple> NestedTerm;
	hsfcTuple NewTerm;
	vector<hsfcRuleCompactTerm> CompactTerm;
	hsfcRuleCompactTerm NewCompactTerm;
	hsfcTuple Result;
	int NextArgumentIndex;
	hsfcRuleCompactTerm CompactTerm$[16];
	int NumCompactTerms;
	bool Found;
	int NestedSize;

	// The terms may be in the expanded form
	// eg. (cell (at ?0 ?1) ?2)
	// But we need them in the compact form
	// eg. (cell ?0 ?1)

	// Are the vectors the same size
	if (Term.size() != this->Template.size()) {
		printf("Error: Wrong size term vector\n");
		abort();
	}

	//--- HighSpeed version -------------------------------------------------------------------

	if (Term.size() < 16) {

		// Copy from the original terms to the compact array
		NumCompactTerms = 0;
		for (unsigned int i = 0; i < this->Template.size(); i++) {
			CompactTerm$[NumCompactTerms].RelationSchema = this->Template[i].RelationSchema;
			CompactTerm$[NumCompactTerms].ArgumentIndex = this->Template[i].ArgumentIndex;
			CompactTerm$[NumCompactTerms].Tuple.RelationIndex = Term[i].RelationIndex;
			CompactTerm$[NumCompactTerms].Tuple.ID = Term[i].ID;
			NumCompactTerms++;
		}

		// Look for any contiguous terms with the same Schema relation and arguments in sequence 0 1 2 etc..
		while (true) {

			// Special case where the compact terms are reduced to a single term
			if (NumCompactTerms == 1) {

				// Get the tuple for the nested terms
				Result.RelationIndex = CompactTerm$[0].RelationSchema->Index;
				Result.ID = CompactTerm$[0].RelationSchema->ID(CompactTerm$, 0, 1);
				
				if (!Validate && (Result.ID == -1)) {
					printf ("Error: Bad ID\n");
					printf("Terms\n");
					for (unsigned int i = 0; i < Term.size(); i++) {
						printf("   %3d.%d", Term[i].RelationIndex, Term[i].ID);
						if (Term[i].RelationIndex == -1) printf("\t%s", this->Lexicon->Text(Term[i].ID));
						printf("\n");
					}
					printf("NestedTerms\n");
					for (int i = 0; i < NumCompactTerms; i++) {
						printf("   %3d.%d", CompactTerm$[i].Tuple.RelationIndex, CompactTerm$[i].Tuple.ID);
						if (CompactTerm$[i].Tuple.RelationIndex == -1) printf("\t%s", this->Lexicon->Text(CompactTerm$[i].Tuple.ID));
						printf("\n");
					}
					printf("Relation\n");
					CompactTerm$[0].RelationSchema->Print();
					abort();
				}

				return Result.ID;

			}

			// Set up the parent Schema relation
			ParentRelationSchema = CompactTerm$[0].RelationSchema;
			NextArgumentIndex = 0;

			for (int Offset = 0; Offset < NumCompactTerms; Offset++) {

				// Is this the 0th argument of a nested term
				if (CompactTerm$[Offset].ArgumentIndex == 0) {

					// Build a set of nested terms
					NestedSize = CompactTerm$[Offset].RelationSchema->Arity + 1;
					Found = true;
					for (int i = 1; i < NestedSize; i++) {
						// Construct the nested terms, or fail
						if (CompactTerm$[Offset + i].RelationSchema != CompactTerm$[Offset].RelationSchema) {
							Found = false;
							break;
						}
					}

					// Was there a nested term
					if (Found) {
						
						// Get the tuple for the nested terms
						Result.RelationIndex = CompactTerm$[Offset].RelationSchema->Index;
						Result.ID = CompactTerm$[Offset].RelationSchema->ID(CompactTerm$, Offset, NestedSize);
						
						if (!Validate && (Result.ID == -1)) {
							printf ("Error: Bad ID\n");
							printf("Terms\n");
							for (unsigned int i = 0; i < Term.size(); i++) {
								printf("   %3d.%d", Term[i].RelationIndex, Term[i].ID);
								if (Term[i].RelationIndex == -1) printf("\t%s", this->Lexicon->Text(Term[i].ID));
								printf("\n");
							}
							printf("NestedTerms\n");
							for (int i = 0; i < NestedSize; i++) {
							printf("   %3d.%d", CompactTerm$[Offset+i].Tuple.RelationIndex, CompactTerm$[Offset+i].Tuple.ID);
							if (CompactTerm$[Offset+i].Tuple.RelationIndex == -1) printf("\t%s", this->Lexicon->Text(CompactTerm$[Offset+i].Tuple.ID));
								printf("\n");
							}
							printf("Relation\n");
							CompactTerm$[Offset].RelationSchema->Print();
							abort();
						}

						// Was this the last iteration
						if (Offset == 0) {
							return Result.ID;
						}
						
						// Replace the nested terms with the single entry
						for (int i = 1; i < NestedSize; i++) {
							CompactTerm$[Offset+i].RelationSchema = CompactTerm$[Offset+i+NestedSize-1].RelationSchema;
							CompactTerm$[Offset+i].ArgumentIndex = CompactTerm$[Offset+i+NestedSize-1].ArgumentIndex;
							CompactTerm$[Offset+i].Tuple.RelationIndex = CompactTerm$[Offset+i+NestedSize-1].Tuple.RelationIndex;
							CompactTerm$[Offset+i].Tuple.ID = CompactTerm$[Offset+i+NestedSize-1].Tuple.ID;
							NumCompactTerms--;
						}
						CompactTerm$[Offset].RelationSchema = ParentRelationSchema;
						CompactTerm$[Offset].ArgumentIndex = NextArgumentIndex;
						CompactTerm$[Offset].Tuple.RelationIndex = Result.RelationIndex;
						CompactTerm$[Offset].Tuple.ID = Result.ID;

						// Start all over again
						break;

					} else {

						ParentRelationSchema = CompactTerm$[Offset].RelationSchema;
						NextArgumentIndex = CompactTerm$[Offset].ArgumentIndex + 1;

					}
				}
			}
		}
	}

	//--- LowSpeed version -------------------------------------------------------------------

	// Copy from the original terms to the compact array
	CompactTerm.clear();
	for (unsigned int i = 0; i < this->Template.size(); i++) {
		NewCompactTerm.RelationSchema = this->Template[i].RelationSchema;
		NewCompactTerm.ArgumentIndex = this->Template[i].ArgumentIndex;
		NewCompactTerm.Tuple.RelationIndex = Term[i].RelationIndex;
		NewCompactTerm.Tuple.ID = Term[i].ID;
		CompactTerm.push_back(NewCompactTerm);
	}

	// Look for any contiguous terms with the same Schema relation and arguments in sequence 0 1 2 etc..
	while (true) {

		// Special case where the compact terms are reduced to a single term
		if (CompactTerm.size() == 1) {

			// Get the tuple for the nested terms
			NestedTerm.clear();
			NewTerm.RelationIndex = CompactTerm[0].Tuple.RelationIndex;
			NewTerm.ID = CompactTerm[0].Tuple.ID;
			NestedTerm.push_back(NewTerm);
			Result.RelationIndex = CompactTerm[0].RelationSchema->Index;
			Result.ID = CompactTerm[0].RelationSchema->ID(NestedTerm);
			
			if (!Validate && (Result.ID == -1)) {
				printf ("Error: Bad ID\n");
				printf("Terms\n");
				for (unsigned int i = 0; i < Term.size(); i++) {
					printf("   %3d.%d", Term[i].RelationIndex, Term[i].ID);
					if (Term[i].RelationIndex == -1) printf("\t%s", this->Lexicon->Text(Term[i].ID));
					printf("\n");
				}
				printf("NestedTerms\n");
				for (unsigned int i = 0; i < NestedTerm.size(); i++) {
					printf("   %3d.%d", NestedTerm[i].RelationIndex, NestedTerm[i].ID);
					if (NestedTerm[i].RelationIndex == -1) printf("\t%s", this->Lexicon->Text(NestedTerm[i].ID));
					printf("\n");
				}
				printf("Relation\n");
				CompactTerm[0].RelationSchema->Print();
				abort();
			}

			return Result.ID;

		}


		// Set up the parent Schema relation
		ParentRelationSchema = CompactTerm[0].RelationSchema;
		NextArgumentIndex = 0;

		for (unsigned int Offset = 0; Offset < CompactTerm.size(); Offset++) {

			// Is this the 0th argument of a nested term
			if (CompactTerm[Offset].ArgumentIndex == 0) {

				// Build a set of nested terms
				NestedTerm.clear();
				for (int i = 0; i <= CompactTerm[Offset].RelationSchema->Arity; i++) {
					// Construct the nested terms, or fail
					if (CompactTerm[Offset + i].RelationSchema == CompactTerm[Offset].RelationSchema) {
						NewTerm.RelationIndex = CompactTerm[Offset + i].Tuple.RelationIndex;
						NewTerm.ID = CompactTerm[Offset + i].Tuple.ID;
						NestedTerm.push_back(NewTerm);
					} else {
						NestedTerm.clear();
						break;
					}
				}

				// Was there a nested term
				if (NestedTerm.size() > 0) {
					
					// Get the tuple for the nested terms
					Result.RelationIndex = CompactTerm[Offset].RelationSchema->Index;
					Result.ID = CompactTerm[Offset].RelationSchema->ID(NestedTerm);
					
					if (!Validate && (Result.ID == -1)) {
						printf ("Error: Bad ID\n");
						printf("Terms\n");
						for (unsigned int i = 0; i < Term.size(); i++) {
							printf("   %3d.%d", Term[i].RelationIndex, Term[i].ID);
							if (Term[i].RelationIndex == -1) printf("\t%s", this->Lexicon->Text(Term[i].ID));
							printf("\n");
						}
						printf("NestedTerms\n");
						for (unsigned int i = 0; i < NestedTerm.size(); i++) {
							printf("   %3d.%d", NestedTerm[i].RelationIndex, NestedTerm[i].ID);
							if (NestedTerm[i].RelationIndex == -1) printf("\t%s", this->Lexicon->Text(NestedTerm[i].ID));
							printf("\n");
						}
						printf("Relation\n");
						CompactTerm[Offset].RelationSchema->Print();
						abort();
					}

					// Was this the last iteration
					if (Offset == 0) {
						return Result.ID;
					}
					
					// Replace the nested terms with the single entry
					for (int i = 0; i < CompactTerm[Offset].RelationSchema->Arity; i++) {
						CompactTerm.erase(CompactTerm.begin() + Offset + 1);
					}
					CompactTerm[Offset].RelationSchema = ParentRelationSchema;
					CompactTerm[Offset].ArgumentIndex = NextArgumentIndex;
					CompactTerm[Offset].Tuple.RelationIndex = Result.RelationIndex;
					CompactTerm[Offset].Tuple.ID = Result.ID;

					// Start all over again
					break;

				} else {

					ParentRelationSchema = CompactTerm[Offset].RelationSchema;
					NextArgumentIndex = CompactTerm[Offset].ArgumentIndex + 1;

				}
			}
		}
	}

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRuleRelationSchema::Print(){

	if (this->Function & hsfcFunctionNot) printf("not ");
	//if (this->Function & hsfcFunctionTrue) printf("(true ");
	printf("(");

	// Print each of the Terms
	for (unsigned int i = 0; i < this->Template.size(); i++) {

		if (i > 0) printf(" ");
		// Is it a variable or fixed 
		if (this->Template[i].Fixed) {
			printf("%s", this->Lexicon->Text(this->Template[i].Tuple.ID));
		// Is it a vriable
		} else {
			printf("?%d", this->Template[i].BufferIndex);
		}

	}
	printf(")");
	//if (this->Function & hsfcFunctionNot) printf(")");
	//if (this->Function & hsfcFunctionTrue) printf(")");

	printf("  /%d", this->Arity);
	printf("  > %d", this->Type);
	printf("\n");

}




//=============================================================================
// CLASS: hsfcRuleSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRuleSchema::hsfcRuleSchema(hsfcLexicon* Lexicon){

	// Allocate the memory
	this->Buffer.resize(16);
	this->Relation.resize(16);

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcRuleSchema::~hsfcRuleSchema(void){

	// Destroy all of the relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		delete this->Relation[i];
	}
	// Free the buffer
	for (unsigned int i = 0; i < this->Buffer.size(); i++) {
		this->Buffer[i].Domain.clear();
	}
	this->Buffer.clear();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRuleSchema::Initialise(){

	// Free the buffer
	for (unsigned int i = 0; i < this->Buffer.size(); i++) {
		this->Buffer[i].Domain.clear();
	}
	this->Buffer.clear();

	// Clear the relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		delete this->Relation[i];
	}
	this->Relation.clear();

	//Reset the properties
	this->SelfReferencing = false;
	this->Stratum = 0;
	this->EstReferenceSize = 0;

}

//-----------------------------------------------------------------------------
// CreateRule
//-----------------------------------------------------------------------------
void hsfcRuleSchema::Create(hsfcGDLRule* GDLRule) {

	hsfcRuleRelationSchema* NewRelation;
	vector<hsfcGDLTerm> Term;

	// Initialise
	this->Initialise();

	// Create the rule relations from the source GDL rule
	for (unsigned int i = 0; i < GDLRule->Relation.size(); i++) {

		// Create an empty relation
		NewRelation = new hsfcRuleRelationSchema(this->Lexicon);
		NewRelation->Initialise();

		// Get a list of the rule terms forming the relation
		Term.clear();
		GDLRule->Relation[i]->Terms(Term);

		// Create the rule relation
		NewRelation->Create(Term, this->Buffer);
		if (GDLRule->Relation[i]->Not) NewRelation->Function = (NewRelation->Function | hsfcFunctionNot);
		if (this->Relation.size() == 0) NewRelation->Type = hsfcRuleResult;

		// Add it to the rule
		// Is this a self referencing rule
		if ((i > 0) && (NewRelation->PredicateIndex == this->Relation[0]->PredicateIndex)) {
			this->SelfReferencing = true;
			this->Relation.insert(this->Relation.begin() + 1, NewRelation);
		} else {
			this->Relation.push_back(NewRelation);
		}

	}

}

//-----------------------------------------------------------------------------
// Optimise
//-----------------------------------------------------------------------------
int hsfcRuleSchema::Optimise(bool OrderInputs) {

	int Index;
	vector<int> Permutation;
	vector<int> OrigInputNo;
	vector<int> InputIndex;
	double Cost;
	double BestCost;
	vector<int> BestInputIndex;
	int Count;
	int NextIndex;

	// Keep track of the permutations of the input indexes by addressing them indirectly
	// Not all of the relations are inputs, so the initial vectors will look like
	// OrigInputNo	0, 2, 3, 5, 6
	// Permutation	0, 1, 2, 3, 4
	// InputIndex	0, 2, 3, 5, 6


	// Calssify the type of input relation
	for (unsigned int i = 1; i < this->Relation.size(); i++) {
		this->Relation[i]->Type = hsfcRulePreCondition;
		for (unsigned int j = 0; j < this->Relation[i]->Template.size(); j++) {
			// Does it have variables
			if (!this->Relation[i]->Template[j].Fixed) {
				// Is it a function (not (...)) or (distinct ... ...)
				if (((this->Relation[i]->Function & hsfcFunctionNot) == hsfcFunctionNot) || ((this->Relation[i]->Function & hsfcFunctionDistinct) == hsfcFunctionDistinct)) {
					this->Relation[i]->Type = hsfcRuleCondition;
				} else {
					this->Relation[i]->Type = hsfcRuleInput;
				}
				break;
			}
		}
	}

	// Find the best Cost and order the inputs accordingly
	Index = 0;
	BestCost = 1e99;
	InputIndex.clear();
	OrigInputNo.clear();
	BestInputIndex.clear();
	Permutation.clear();
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		if (this->Relation[i]->Type == hsfcRuleInput) {
			BestInputIndex.push_back(i);
			OrigInputNo.push_back(i);
			Permutation.push_back(Permutation.size());
			InputIndex.push_back(0);
		}
	}

	// Co8unt haow many permutations are tested
	Count = 0;
	if (OrigInputNo.size() == 0) return Count;

	// Consider every permutation of the input relations
	while (true) {

		//for (unsigned int i = 0; i < Permutation.size(); i++) {
		//	printf(" %d", Permutation[i]);
		//}
		//printf("\n");

		// Load this permutation
		for (unsigned int i = 0; i < OrigInputNo.size(); i++) {
			InputIndex[i] = OrigInputNo[Permutation[i]];
		}

		// There is a special type of rule that self references
		if (this->SelfReferencing) {
			if (this->Relation[InputIndex[0]]->PredicateIndex != this->Relation[0]->PredicateIndex) {
				Cost = 1e99;
			} else {
				Cost = this->Cost(InputIndex);
			}
		} else {
			Cost = this->Cost(InputIndex);
		}
		Count++;

		// Is this the best Cost so far
		if (Cost < BestCost) {
			BestCost = Cost;
			for (unsigned int i = 0; i < InputIndex.size(); i++) {
				BestInputIndex[i] = InputIndex[i];
			}
		}

		// Return inputs to inputs (may have been changed to conditions)
		for (unsigned int i = 0; i < InputIndex.size(); i++) {
			this->Relation[InputIndex[i]]->Type = hsfcRuleInput;
		}

		// Advance the input permutation 
		if (!OrderInputs) break;
		if (Permutation.size() == 1)  break;

		// A bit obvoius
		if (Permutation.size() == 2)  {
			if (Count == 2) break;
			Permutation[0] = 1;
			Permutation[1] = 0;
			continue;
		}

		// Advance to the next permutation of 3 in N
		Permutation[2]++;
		while ((Permutation[2] == Permutation[0]) || (Permutation[2] == Permutation[1])) Permutation[2]++;
		if (Permutation[2] == Permutation.size()) {
			Permutation[2] = 0;
			Permutation[1]++;
			while (Permutation[1] == Permutation[0]) Permutation[1]++;
			if (Permutation[1] == Permutation.size()) {
				Permutation[1] = 0;
				Permutation[0]++;
				if (Permutation[0] == Permutation.size()) break;
				while (Permutation[1] == Permutation[0]) Permutation[1]++;
			}
			while ((Permutation[2] == Permutation[0]) || (Permutation[2] == Permutation[1])) Permutation[2]++;
		}
		NextIndex = 0;
		for (unsigned int i = 3; i < Permutation.size(); i++) {
			while ((NextIndex == Permutation[0]) || (NextIndex == Permutation[1]) || (NextIndex == Permutation[2])) NextIndex++;
			Permutation[i] = NextIndex;
			NextIndex++;
		}

	}

	// OK; so now we know the best sequence, now sort the inputs accordingly
	//printf("Best Sort Order\n");
	//printf("\n");
	if (DEBUG) printf("    %.0f\n", BestCost);
	this->Cost(BestInputIndex);
	this->Sort();
	//printf("\n");

	return Count;


}

//-----------------------------------------------------------------------------
// Stratify
//-----------------------------------------------------------------------------
void hsfcRuleSchema::Stratify() {

	hsfcRelationSchema* RelationSchema;
	bool NewVariable;

	// Reorder the relations in the body
	// Result			SortOrder = 0
	// Preconditions	SortOrder = 0 -> size-1
	// Input			SortOrder = size -> 2*size-1
	// Condition		SortOrder = 2*size -> 3*size-1

	// Result always comes first
	this->Relation[0]->SortOrder = 0;

	// Use the Buffer to identify redundant inputs
	for (unsigned int i = 0; i < this->Buffer.size(); i++) {
		this->Buffer[i].ID = 0;
	}

	// Calssify the type of input relation
	for (unsigned int i = 1; i < this->Relation.size(); i++) {

		// Is it an input or a condition or a precondition
		this->Relation[i]->Type = hsfcRulePreCondition;
		this->Relation[i]->SortOrder = i;

		// Check the template for variables
		NewVariable = false;
		for (unsigned int j = 0; j < this->Relation[i]->Template.size(); j++) {

			// Set up the size of the domain in the rule relation
			// Fixed terms are size = 1
			this->Relation[i]->Template[j].DomainSize = 1;
			
			// Only check the variables
			if (!this->Relation[i]->Template[j].Fixed) {

				// Get the size of the domain for this variable
				RelationSchema = this->Relation[i]->Template[j].RelationSchema;
				if (RelationSchema != NULL) {
					this->Relation[i]->Template[j].DomainSize = RelationSchema->GetDomainCount(this->Relation[i]->Template[j].ArgumentIndex - 1);
				}

				// Is it a function (not (...)) or (distinct ... ...) with variables
				if (((this->Relation[i]->Function & hsfcFunctionNot) == hsfcFunctionNot) || ((this->Relation[i]->Function & hsfcFunctionDistinct) == hsfcFunctionDistinct)) {
					this->Relation[i]->Type = hsfcRuleCondition;
					this->Relation[i]->SortOrder = 2 * this->Relation.size() + i;
					break;
				} 

				// Only inputs from here on
				// Check if this is a new variable
				if (this->Buffer[this->Relation[i]->Template[j].BufferIndex].ID == 0) {
					this->Buffer[this->Relation[i]->Template[j].BufferIndex].ID = 1;
					NewVariable = true;
					// Construct the Buffer domain for this variable 
					if (RelationSchema != NULL) {
						if (this->Relation[i]->Template[j].ArgumentIndex <= 0) {
							printf("Error: Relation Argument Index wrong\n");
							abort();
						}
						RelationSchema->AddToBufferDomain(this->Buffer[this->Relation[i]->Template[j].BufferIndex].Domain, this->Relation[i]->Template[j].ArgumentIndex - 1);  
					}
				} else {
					// Construct the Buffer domain for this variable from the intersection
					if (RelationSchema != NULL) {
						if (this->Relation[i]->Template[j].ArgumentIndex <= 0) {
							printf("Error: Relation Argument Index wrong\n");
							abort();
						}
						RelationSchema->IntersectBufferDomain(this->Buffer[this->Relation[i]->Template[j].BufferIndex].Domain, this->Relation[i]->Template[j].ArgumentIndex - 1);  
					}
				}

				// Did this relation introduce any new variables
				if (NewVariable) {
					this->Relation[i]->Type = hsfcRuleInput;
					this->Relation[i]->SortOrder = this->Relation.size() + i;
				} else {
					this->Relation[i]->Type = hsfcRuleCondition;
					this->Relation[i]->SortOrder = 2 * this->Relation.size() + i;
				}

			}
		}
	}

	// Sort the relations
	this->Sort();

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRuleSchema::Print(){

	bool AllDone;

	// Print all of the relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		if (i == 0) {
			printf("(<= ");
		} else {
			printf("    ");
		}
		this->Relation[i]->Print();
	}
	printf(")\n");

	// Print the buffer domains
	printf("Variables\n");

	// Print the domains up to a limit
	for (unsigned int n = 0; n < 16; n++) {
		AllDone = true;
		for (unsigned int i = 0; i < this->Buffer.size(); i++) {
			if (n < this->Buffer[i].Domain.size()) {
				printf("%2d.%d\t", this->Buffer[i].Domain[n].RelationIndex, this->Buffer[i].Domain[n].ID);
				AllDone = false;
			} else {
				printf("\t");
			}
		}
		printf("\n");
		if (AllDone) break;
	}


}

//-----------------------------------------------------------------------------
// Cost
//-----------------------------------------------------------------------------
double hsfcRuleSchema::Cost(vector<int>& InputIndex) {

	int BufferIndex;
	double AveListLength;
	int MaxListLength;
	int VariableCount;
	int NoUniqueIDs;
	int NoInputsRelations;
	double ReferenceSize;
	int NewDataSize;
	int InputDataSize;
	double NoIterations;
	double NoInputCycles;
	double Probability;
	double TotalCost;
	bool NewVariableFound;
	bool PartiallyGround;

	// Calculate the size of each of the reference tables
	// Calculate the number of iterations
	// Calculate the total cost of executing this rule
	/* Costs
		Advance database cursor: Cost = 1
		Read input relations: Cost = 1
		Test condition relation exists: Cost = 1
		Test condition relation in sorted list: Cost = log(n)
		Subsumed rigid as a condition: Cost = 0
		Subsumed distinct: Cost = 0
		Preconditions: Cost = 0
	*/

	NoUniqueIDs = 1;
	NoInputCycles = 1;
	ReferenceSize = 0;
	NoIterations = 0;

	// --- Preconditions ------------------------------------------------------
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		// Is it a precondition
		if (this->Relation[i]->Type == hsfcRulePreCondition) {

			// Calculate the database transactions
			NoIterations += NoInputCycles;

			if ((this->Relation[i]->Function & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
				Probability = 0.5;
			} else {
				// Get the list information
				AveListLength = this->Relation[i]->Template[0].RelationSchema->AveListLength;
				MaxListLength = this->Relation[i]->Template[0].RelationSchema->IDCount;
				
				// Calculate the probability of this precondiction being met
				if ((this->Relation[i]->Function & hsfcFunctionNot) == hsfcFunctionNot) {
					Probability = 1.0 - AveListLength / MaxListLength;
				} else {
					Probability = AveListLength / MaxListLength;
				}
			}

			// Use the probability for sorting and cycle calculations
			if ((Probability < 0) || (Probability > 1)) {
				printf("Error: Bad precondition probability %f\n", Probability);
				abort();
			}
			this->Relation[i]->SortOrder = Probability;

			// Calculate the reference size
			this->Relation[i]->ReferenceSize = 1;
			ReferenceSize += (double)this->Relation[i]->ReferenceSize;
		}
	}

	// Ignore the cost and probaility of preconditions as it is fixed
	NoUniqueIDs = 1;
	NoInputCycles = 1;
	ReferenceSize = 0;
	NoIterations = 0;
	NoInputsRelations = 0;
	AveListLength = 1;
	MaxListLength = 1;
	TotalCost = 0;

	// Reset the variable buffer
	VariableCount = 0;
	for (unsigned int i = 0; i < this->Buffer.size(); i++) {
		this->Buffer[i].ID = 0;
	}

	// --- Inputs ------------------------------------------------------------
	for (unsigned int i = 0; i < InputIndex.size(); i++) {
		
		// Calculate the size of the new data introduced by this input
		NewDataSize = 1;
		InputDataSize = 1;
		NewVariableFound = false;
		PartiallyGround = false;

		// Inspect term in the input relation
		for (unsigned int j = 0; j < this->Relation[InputIndex[i]]->Template.size(); j++) {
			// Was this term a variable
			if (!this->Relation[InputIndex[i]]->Template[j].Fixed) {
				InputDataSize *= this->Relation[InputIndex[i]]->Template[j].DomainSize;
				// Is it already in the buffer
				BufferIndex = this->Relation[InputIndex[i]]->Template[j].BufferIndex; 
				if (this->Buffer[BufferIndex].ID == 0) {
					// Add it to the Buffer
					this->Buffer[BufferIndex].ID = 1;
					NewVariableFound = true;
					// Find the size of the domain of the new variable
					NewDataSize *= this->Buffer[BufferIndex].Domain.size();
				} else {
					PartiallyGround = true;
				}
			}
		}

		// Were all of the variables already present in the buffer
		if (!NewVariableFound) {

			// Flag it as a condition for later calculations
			this->Relation[InputIndex[i]]->Type = hsfcRuleCondition;

		} else {

			// Remember the input index in case this is the best
			this->Relation[InputIndex[i]]->SortOrder = 2.0 + i;

			// Get the list information
			NoInputsRelations++;
			AveListLength = this->Relation[InputIndex[i]]->Template[0].RelationSchema->AveListLength;
			MaxListLength = this->Relation[InputIndex[i]]->Template[0].RelationSchema->IDCount;

			// Calculate the size of the reference table from the previous number of unique IDs
			this->Relation[InputIndex[i]]->ReferenceSize = NoUniqueIDs * MaxListLength;
			ReferenceSize += (double)this->Relation[InputIndex[i]]->ReferenceSize;

			// Calculate the number of unique combinations of variables
			NoUniqueIDs *= NewDataSize;

			// Calculate the probability of a pass
			Probability = (double)NewDataSize / (double)InputDataSize;
			if (Probability > 1.0) Probability = 1.0;

			// Keep a running total of the cost from reading the input
			// Count the failed cycles
			TotalCost += NoInputCycles * AveListLength * (1.0 - Probability) * NoInputsRelations; 
			NoIterations += NoInputCycles * AveListLength;		

			// Remember the pass rate for the next round
			NoInputCycles *= Probability * AveListLength;

			// At this point look for permanent facts or distinct that can be subsumed by this input



		}
	}

	// Keep a running total of the cost from reading the input
	// Count the passed cycles
	TotalCost += NoInputCycles * NoInputsRelations; 
	// Tally the cost for managing the relation cursors
	TotalCost += NoIterations;

	// --- Conditions ---------------------------------------------------------
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		// Is it a condition
		if (this->Relation[i]->Type == hsfcRuleCondition) {

			// Calculate the cost of checking this condition
			TotalCost += NoInputCycles;
			
			if ((this->Relation[i]->Function & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
				Probability = 0.5;
			} else {
				// Get the list information
				AveListLength = this->Relation[i]->Template[0].RelationSchema->AveListLength;
				MaxListLength = this->Relation[i]->Template[0].RelationSchema->IDCount;
				
				// Calculate the probability of this condiction being met
				if ((this->Relation[i]->Function & hsfcFunctionNot) == hsfcFunctionNot) {
					Probability = 1.0 - AveListLength / MaxListLength;
				} else {
					Probability = AveListLength / MaxListLength;
				}
			}

			// Calculate the reference size
			this->Relation[i]->ReferenceSize = NoUniqueIDs;
			ReferenceSize += (double)this->Relation[i]->ReferenceSize;

			// Use the probability for sorting and cycle calculations
			if ((Probability < 0) || (Probability > 1)) {
				printf("Error: Bad probability %f\n", Probability);
				abort();
			}
			this->Relation[i]->SortOrder = 2.0 + (double)InputIndex.size() + Probability;
			NoInputCycles *= Probability;
		
		}

	}

	// --- Result ------------------------------------------------------------
	this->Relation[0]->ReferenceSize = NoUniqueIDs;
	ReferenceSize += (double)this->Relation[0]->ReferenceSize;
	TotalCost += NoInputCycles;

	// Cost the results of the calculations
	// 1 unit of Cost = 10 -> 100 clock cycles
	// If the reference table stays in L3 cache there is no cost
	if (ReferenceSize > 10000.0) {
		TotalCost += (ReferenceSize - 10000.0) / 1000.0; 
	}
	//printf("    %.0f\n", Cost);

	return TotalCost;

}

//-----------------------------------------------------------------------------
// Sort
//-----------------------------------------------------------------------------
void hsfcRuleSchema::Sort() {

	bool OrderChanged;
	hsfcRuleRelationSchema* Relation;

	// Reorder the relations in the body
	// Result
	// Preconditions - ordered by probability (lowest first)
	// Input - ordered by lowest cost index
	// Condition - ordered by probability (lowest first)

	// Process until the order no longer changes
	OrderChanged = true;
	while (OrderChanged) {

		// Sort everything except the result relation[0]
		OrderChanged = false;
		for (unsigned int i = 2; i < this->Relation.size(); i++) {

			// Is the order correct
			if (this->Relation[i-1]->SortOrder > this->Relation[i]->SortOrder) {
				OrderChanged = true;
				Relation = this->Relation[i-1];
				this->Relation[i-1] = this->Relation[i];
				this->Relation[i] = Relation;
			}

		}

	}

}

//=============================================================================
// CLASS: hsfcSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcSchema::hsfcSchema(hsfcLexicon* Lexicon) {

	// Allocate the memory
	this->Rule.resize(32);
	this->Relation.resize(32);

	// Set up the Lexicon
	this->Lexicon = Lexicon;

	// Set up the GDL
	this->GDL = new hsfcGDL(this->Lexicon);

	// Set up the SCL
	this->SCL = new hsfcGDL(this->Lexicon);

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcSchema::~hsfcSchema(void){

	// Destroy all of the relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		delete this->Relation[i];
	}

	// Destroy all of the rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete this->Rule[i];
	}

	// Destroy the Lexicon
	delete this->Lexicon;

	// Destroy the GDL
	delete this->GDL;

	// Destroy the SCL
	delete this->SCL;

	// Destroy the stratum
	this->Stratum.clear();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcSchema::Initialise(){

	// Initialise the Lexicon
	this->Lexicon->Initialise();

	// Initialise the GDL
	this->GDL->Initialise();

	// Initialise the SCL
	this->SCL->Initialise();

	// Initialise all of the relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		delete this->Relation[i];
	}
	this->Relation.clear();

	// Initialise all of the rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete this->Rule[i];
	}
	this->Rule.clear();

	// Initialise all of the facts and initials
	this->Fact.clear();
	this->Initial.clear();
	this->Next.clear();

}

//-----------------------------------------------------------------------------
// CreateGDL
//-----------------------------------------------------------------------------
bool hsfcSchema::Create(char* Script){

	// Read the GDL file; specially constructed with domain information
	if (!this->ReadGDL(Script)) return false;
	
	// Was there anything
	if (this->GDL->Rule.size() == 0) return false;

	// Set the permanent facts from the GDL relations
	this->SetPermanentFacts();

	// Index the domains
	if (DEBUG) this->PrintRelations();
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		printf("Indexing Domains\n");
		this->Relation[i]->IndexDomains();
	}
	if (DEBUG) this->PrintRelations();

	// Sort the rules to a just in time order
	this->Stratify();

	// Set all of the references for initilaising the state
	this->SetInitialRelations();
	this->SetNextRelations();

	return true;

}

//-----------------------------------------------------------------------------
// GetRoles
//-----------------------------------------------------------------------------
void hsfcSchema::GetRoles(vector<string>& Role) {

	hsfcRelationSchema* RelationSchema;

	// Get the role relation
	RelationSchema = this->RelationSchema(this->Lexicon->Index("role|1"), 1);
	RelationSchema->ListDomain(Role);

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcSchema::PrintRelation(hsfcTuple* Tuple, bool CRLF) {

	vector<hsfcTuple> Term;
	vector<hsfcTuple> NestedTerm;

	// Use the underlying relation functionality to get the terms
	this->Relation[Tuple->RelationIndex]->Terms(Tuple->ID, Term);

	// The Term array will be in its most compressed form
	// eg. (cell ?0 ?1)
	// But the rule may require an expanded form
	// eg. (cell (cell~0~at ?0 ?1) ?2)

	for (unsigned int i = 1; i < Term.size(); i++) {

		// Do we need to expand this term
		if (Term[i].RelationIndex != -1) {

			// Get the relation for this term
			this->Relation[Term[i].RelationIndex]->Terms(Term[i].ID, NestedTerm);
			// Delete the term and insert the nested terms
			Term.erase(Term.begin() + i);
			for (unsigned int j = 0; j < NestedTerm.size(); j++) {
				Term.insert(Term.begin() + i + j, NestedTerm[j]);
			}

		}
	}

	// Print the Relation
	for (unsigned int i = 0; i < Term.size(); i++) {
		printf("%s ", this->Lexicon->Text(Term[i].ID));
	}
	if (CRLF) printf("\n");

}

//-----------------------------------------------------------------------------
// RelationAsText
//-----------------------------------------------------------------------------
void hsfcSchema::RelationAsText(hsfcTuple* Tuple, char* Text) {

	vector<hsfcTuple> Term;
	vector<hsfcTuple> NestedTerm;
	int Index;
	const char* TermText;

	// Use the underlying relation functionality to get the terms
	this->Relation[Tuple->RelationIndex]->Terms(Tuple->ID, Term);

	// The Term array will be in its most compressed form
	// eg. (cell ?0 ?1)
	// But the rule may require an expanded form
	// eg. (cell (cell~0~at ?0 ?1) ?2)

	for (unsigned int i = 1; i < Term.size(); i++) {

		// Do we need to expand this term
		if (Term[i].RelationIndex != -1) {

			// Get the relation for this term
			this->Relation[Term[i].RelationIndex]->Terms(Term[i].ID, NestedTerm);
			// Delete the term and insert the nested terms
			Term.erase(Term.begin() + i);
			for (unsigned int j = 0; j < NestedTerm.size(); j++) {
				Term.insert(Term.begin() + i + j, NestedTerm[j]);
			}

		}
	}

	// Print the Relation
	Index = 0;
	for (unsigned int i = 0; i < Term.size(); i++) {
		TermText = this->Lexicon->Text(Term[i].ID);
		if (strlen(TermText) + Index > 250) break;
		Index += sprintf(&Text[Index], "%s ", TermText);
	}
	Text[Index] = 0;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcSchema::Print(){

	this->SCL->Print("SCL");

	printf("\n--- Schema --------------------------------------------------\n");

	this->PrintRelations();
	this->PrintRules();
	this->PrintReferences();

}

//-----------------------------------------------------------------------------
// ReadGDL
//-----------------------------------------------------------------------------
bool hsfcSchema::ReadGDL(char* Buffer){

	//char* Buffer;
	char* RuleScript;
	//char* FactScript;
	char* StratScript;
	char* DomainScript;
	char* PathScript;
	vector<char*> Word;
	vector<char*> DomainEntryValue;
	vector<char*> DomainEntryList;

	//-------------------------------------------------------------------------
	// Read the GDL
	//-------------------------------------------------------------------------

	// This could be a very large file MegaBytes
	/*	
		;;;; RULES
		(<= (.....)(....)(....))\n
		;;;; PERMANENT FACTS
		(init (....))\n
		(.....)\n
		;;;; STRATS
		(strat predicate ##)
		;;;; PATHS
		(arg predicate index value) 
		(arg predicate index predicate index value) 
		;;;; DOMAINS
		(domain predicate (set .. .. ..)(set .. .. ..)) 
	*/

	// Parse the buffer into subscripts for each of the sections
	// Find the Rules
	RuleScript = strstr(Buffer, ";;;; RULES");
	if (RuleScript == NULL) {
		printf("Error: GDL file does not conatin ';;;; RULES'\n");
		abort();
	}
	//// Find the Facts
	//FactScript = strstr(Buffer, ";;;; PERMANENT FACTS");
	//if (FactScript == NULL) {
	//	printf("Error: GDL file does not conatin ';;;; PERMANENT FACTS'\n%s\n", FileName);
	//	abort();
	//}
	// Find the Strats
	StratScript = strstr(Buffer, ";;;; STRATS");
	if (StratScript == NULL) {
		printf("Error: GDL file does not conatin ';;;; STRATS'\n");
		abort();
	}
	// Find the Paths
	PathScript = strstr(Buffer, ";;;; PATHS");
	if (PathScript == NULL) {
		printf("Error: GDL file does not conatin ';;;; PATHS'\n");
		abort();
	}
	// Find the Domains
	DomainScript = strstr(Buffer, ";;;; DOMAINS");
	if (DomainScript == NULL) {
		printf("Error: GDL file does not conatin ';;;; DOMAINS'\n");
		abort();
	}

	// Blank out the headings to create end-of-string for each section
	// Assumes Rules + Facts = GDL
	for (int i = 0; i < 10; i++) RuleScript[i] = 0;
	RuleScript += 10;
	//for (int i = 0; i < 20; i++) FactScript[i] = ' ';
	//FactScript += 20;
	for (int i = 0; i < 11; i++) StratScript[i] = 0;
	StratScript += 11;
	for (int i = 0; i < 10; i++) PathScript[i] = 0;
	PathScript += 10;
	for (int i = 0; i < 12; i++) DomainScript[i] = 0;
	DomainScript += 12;

	// Create the GDL & SCL
	if (!this->ReadRules(RuleScript)) return false;

	//-------------------------------------------------------------------------
	// Process the rule ordering
	//-------------------------------------------------------------------------

	//// Clear the stratum
	//this->Stratum.clear();
	//Predicate = new char[256];
	//
	//// Look for "(strat relation order)"
	//for (int i = 0; StratScript[i] != 0; i++) {
	//	// is the is the beginning of a strat clause
	//	if (strncmp(&StratScript[i], "(strat ", 7) == 0) {
	//		// Read the stratum
	//		sscanf(&StratScript[i], "(strat %s %d)", Predicate, &Order);
	//		Reference.RelationIndex = this->Lexicon->Index(Predicate);
	//		Reference.ID = Order;
	//		this->Stratum.push_back(Reference);
	//	}
	//}

	//delete[](Predicate);

	// Read the domains
	if (!this->ReadDomains(PathScript)) return false;

	return true;

}

//-----------------------------------------------------------------------------
// ReadRules
//-----------------------------------------------------------------------------
bool hsfcSchema::ReadRules(char* RuleScript){

	vector<char*> Word;
	vector<char*> DomainEntryValue;
	vector<char*> DomainEntryList;
	hsfcRelationSchema* RelationSchema;
	//char EmbeddedPredicate[256];
	//hsfcGDLRelation* NewRelation;

	//-------------------------------------------------------------------------
	// Create the GDL & SCL
	//-------------------------------------------------------------------------

	// Process the GDL and the SCL
	this->GDL->Read(RuleScript);

	// Was there anything
	if (this->GDL->Rule.size() == 0) {
		printf("Error: No rules\n");
		return false;
	}

	// Create the structured chaining language
	this->CreateSCL();

	// Print
	if (DEBUG) this->Lexicon->Print();
	//this->GDL->Print("GDL");
	if (DEBUG) this->SCL->Print("SCL");

	//-------------------------------------------------------------------------
	// Create the Schema
	//-------------------------------------------------------------------------

	// Read each of the relations
	for (unsigned int i = 0; i < this->SCL->Relation.size(); i++) {
		this->AddRelationSchema(this->SCL->Relation[i]);
	}

	// Read each of the rules
	for (unsigned int i = 0; i < this->SCL->Rule.size(); i++) {
		this->AddRuleSchema(this->SCL->Rule[i]);
		// Add the relations inside the rule
		for (unsigned int j = 0; j < this->SCL->Rule[i]->Relation.size(); j++) {
			this->AddRelationSchema(this->SCL->Rule[i]->Relation[j]);
		}
		// Link all of the rule relations to Schema relations
		for (unsigned int j = 0; j < this->Rule[i]->Relation.size(); j++) {
			for (unsigned int k = 0; k < this->Rule[i]->Relation[j]->Template.size(); k++) {
				this->Rule[i]->Relation[j]->Template[k].RelationSchema = this->RelationSchema(this->Rule[i]->Relation[j]->Template[k].PredicateIndex);
			}
		}

	}

	// At this point every primary relation is considered permanent facts
	for (unsigned int i = 0; i < this->SCL->Relation.size(); i++) {
		RelationSchema = this->RelationSchema(this->SCL->Relation[i]->PredicateIndex());
		if (RelationSchema != NULL) {
			RelationSchema->Fact = hsfcFactPermanent;
			// Correct any key words
			if (this->Lexicon->PartialMatch(RelationSchema->PredicateIndex, "init|")) RelationSchema->Fact = hsfcFactInitial ;
			if (this->Lexicon->PartialMatch(RelationSchema->PredicateIndex, "terminal|")) RelationSchema->Fact = hsfcFactNone ;
			if (this->Lexicon->PartialMatch(RelationSchema->PredicateIndex, "goal|")) RelationSchema->Fact = hsfcFactNone ;
			if (this->Lexicon->PartialMatch(RelationSchema->PredicateIndex, "legal|")) RelationSchema->Fact = hsfcFactNone ;
			if (this->Lexicon->PartialMatch(RelationSchema->PredicateIndex, "does|")) RelationSchema->Fact = hsfcFactNone ;
			if (this->Lexicon->PartialMatch(RelationSchema->PredicateIndex, "sees|")) RelationSchema->Fact = hsfcFactNone ;
			if (this->Lexicon->PartialMatch(RelationSchema->PredicateIndex, "next>")) RelationSchema->Fact = hsfcFactNone ;
			// Check it against the output relation of each rule
			for (unsigned int j = 0; j < this->SCL->Rule.size(); j++) {
				if (RelationSchema == this->Rule[j]->Relation[0]->Template[0].RelationSchema) {
					RelationSchema->Fact = hsfcFactNone;
					break;
				}
			}
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// ReadDomains
//-----------------------------------------------------------------------------
bool hsfcSchema::ReadDomains(char* PathScript){

	int Length;
	int DELength;
	char* DomainEntry;
	char Entry[512];
	char* ListEntry;
	char* NewEntry;
	char* Predicate;
	char NewPredicate[256];
	char* Index;
	char* Value;
	vector<char*> Word;
	vector<char*> DomainEntryValue;
	vector<char*> DomainEntryList;
	hsfcDomainEntry Reference;
	int DomainIndex;
	char* Colon;
	hsfcRelationSchema* RelationSchema;
	hsfcRelationSchema* NestedRelation;
	hsfcRelationDetail NewRelationDetail;
	bool Found;

	//-------------------------------------------------------------------------
	// Read the domains
	//-------------------------------------------------------------------------

	// Get the domains for all of the relations
	// (arg predicate index predicate index value)) ==> (predicate_index_predicate index value)
	// (arg true 0 predicate index value)) ==> (predicate index value)
	// (arg next 0 predicate index value)) ==> (next>predicate predicate index value)

	if (DEBUG) printf("\n--- Domains -------------------------------------------------\n");

	// We do this in two passes
	// 1st pass - Identify all of the relations by predicate / arity
	// 2nd pass - Construct the domains

	// Parse everything into a two vectors so we can read in multiple times
	// DomainEntryValue = single values
	// DomainEntryList = lists of values
	Length = strlen(PathScript);
	DomainEntry = PathScript;
	DomainEntryValue.clear();
	DomainEntryList.clear();
	while (DomainEntry < PathScript + Length) {

		// Advance to the first character of the entry, just right of the '('
		while (DomainEntry < PathScript + Length) {
			if (DomainEntry[0] == '(') {
				DomainEntry++;
				break;
			} else {
				DomainEntry++;
			}
		}

		// Zero out the ending bracket ')'
		for (int i = 0; i < (PathScript - DomainEntry) + Length; i++) {
			if (DomainEntry[i] == ')') {
				if (i > 255) {
					printf("Error: Path entry too long\n");
					abort();
				}
				DomainEntry[i] = 0;
				break;
			}
		}
		DELength = strlen(DomainEntry);

		//printf("%s\n", DomainEntry);

		// Now parse the entry one word at a time skipping the first word "arg"
		Word.clear();
		for (int i = 0; DomainEntry[i] != 0; i++) {
			if ((DomainEntry[i] == ' ') && (DomainEntry[i+1] != 0)) {
				Word.push_back(DomainEntry + i + 1);
				DomainEntry[i] = 0;
				i++;
			}
		}

		// Check the number of words
		if (Word.size() < 3 ) {
			goto NextEntry;
		}

		// Get the Predicate, Index, and Value
		// (arg plan 0 move 2 piece 1 king) ==> (piece 1 king) 
		//    AND (move 2 list:piece) 
		//    AND (plan 0 list:move)
		// (arg next 0 cell 0 1) ==> (next>cell 0 cell 0 1)

		//// Was it of the form (arg does 0 predicate index value)
		//if (strcmp(Word[0], "does") == 0) {
		//	goto NextEntry;
		//}

		// Was it of the form (arg true 0 predicate index value)
		if (strcmp(Word[0], "true") == 0) {
			Word.erase(Word.begin());
			Word.erase(Word.begin());
		}

		// Check the number of words
		if (Word.size() < 3 ) {
			goto NextEntry;
		}

		// Create a domain entries for later processing
		for (unsigned int n = 0; n < Word.size() - 1; n += 2) {

			// Remember "arg" has been removed
			// (arg plan 0 move 2 piece 1 king)
			// n = 0 (plan 0 list:move)
			// n = 2 (move 2 list:piece) 
			// n = 4 (piece 1 king) 

			// (arg legal 1 move 2 piece 1 king)
			// n = 0 (legal 1 list:move)
			// n = 2 (move 2 list:piece) 
			// n = 4 (piece 1 king) 

			// (arg next 0 move 2 piece 1 king)
			// n = 0 (next>move 0 list:move)
			// n = 2 (move 2 list:piece) 
			// n = 4 (piece 1 king) 

			Entry[0] = 0;

			strcat(Entry, Word[n]);
			if ((n == 0) && (strcmp(Word[0], "next") == 0)) {
				strcat(Entry, ">");
				strcat(Entry, Word[2]);
			}

			//// Create the predicate
			//for (unsigned int i = 0; i < Word.size() - n - 2; i++) {

			//	// Is it a keyword
			//	if ((n < Word.size() - 3) && (i <= 1) && (strcmp(Word[0], "init") == 0)) continue;
			//	if ((n < Word.size() - 3) && (i <= 1) && (strcmp(Word[0], "legal") == 0)) continue;
			//	if ((n < Word.size() - 3) && (i <= 1) && (strcmp(Word[0], "does") == 0)) continue;
			//	if ((n < Word.size() - 3) && (i <= 1) && (strcmp(Word[0], "sees") == 0)) continue;
			//	if ((n < Word.size() - 3) && (i <= 1) && (strcmp(Word[0], "next") == 0)) continue;

			//	strcat(Entry, Word[i]);
			//	if ((i == 0) && (strcmp(Word[0], "next") == 0)) {
			//		strcat(Entry, ">");
			//		strcat(Entry, Word[2]);
			//	}
			//	if (i < Word.size() - n - 3) {
			//		strcat(Entry, "~");
			//	}
			//}
			
			// Create the index
			strcat(Entry, " ");
			strcat(Entry, Word[n+1]);

			// Create the value or list:
			strcat(Entry, " ");
			if (n == Word.size() - 3) {
				strcat(Entry, Word[Word.size()-1]);
			} else {
				strcat(Entry, "list:");
				strcat(Entry, Word[n+2]);
				//for (unsigned int i = 0; i < Word.size() - n; i++) {
				//	// Is it a keyword
				//	if ((i <= 1) && (strcmp(Word[0], "init") == 0)) continue;
				//	if ((i <= 1) && (strcmp(Word[0], "legal") == 0)) continue;
				//	if ((i <= 1) && (strcmp(Word[0], "does") == 0)) continue;
				//	if ((i <= 1) && (strcmp(Word[0], "sees") == 0)) continue;
				//	if ((i <= 1) && (strcmp(Word[0], "next") == 0)) continue;
				//	// Concatonate the predicates
				//	strcat(Entry, Word[i]);
				//	if (i < Word.size() - n - 1) {
				//		strcat(Entry, "~");
				//	}
				//}
			}

			// Save the entry for later processing
			Colon = strstr(Entry, "list:");
			if (Colon == NULL) {

				// Store it for later processing
				NewEntry = new char[strlen(Entry) + 1];
				strcpy(NewEntry, Entry);
				DomainEntryValue.push_back(NewEntry);

			} else {

				// Store it for later processing
				// Does it exist already
				Found = false;
				for (unsigned int i = 0; i < DomainEntryList.size(); i++) {
					if (strcmp(Entry, DomainEntryList[i]) == 0) {
						Found = true;
					}
				}
				
				// Store it for later processing
				if (!Found) {
					//printf("      Stored\n");
					NewEntry = new char[strlen(Entry) + 1];
					strcpy(NewEntry, Entry);
					DomainEntryList.push_back(NewEntry);
				}

			}

		}

NextEntry:
		// Advance to the next domain entry
		DomainEntry += DELength;

	}

	// Look for relations that are hidden by rule variables
	this->RelationDetail.clear();
	for (unsigned int i = 0; i < DomainEntryValue.size(); i++) {

		// Parse the entry
		strcpy(Entry, DomainEntryValue[i]); 
		Predicate = Entry;
		Index = strstr(Entry, " ");
		if (Index != NULL) {
			Index[0] = 0;
			Index++;
			Value = strstr(Index, " ");
			if (Value != NULL) {
				Value[0] = 0;
				Value++;
			}
		}
		DomainIndex = atoi(Index);

		// Do we have this relation in our list
		Found = false;
		for (unsigned int j = 0; j < RelationDetail.size(); j++) {
			if (this->Lexicon->Match(RelationDetail[j].PredicateIndex, Predicate)) {
				Found = true;
				if (RelationDetail[j].Arity <= DomainIndex) {
					RelationDetail[j].Arity = DomainIndex + 1;
				}
				break;
			}
		}

		// Did we find an entry
		if (!Found) {
			NewRelationDetail.PredicateIndex = this->Lexicon->Index(Predicate);
			NewRelationDetail.Arity = DomainIndex + 1;
			RelationDetail.push_back(NewRelationDetail);
		}
	}

	for (unsigned int i = 0; i < DomainEntryList.size(); i++) {

		// Parse the entry
		strcpy(Entry, DomainEntryList[i]); 
		Predicate = Entry;
		Index = strstr(Entry, " ");
		if (Index != NULL) {
			Index[0] = 0;
			Index++;
			Value = strstr(Index, " ");
			if (Value != NULL) {
				Value[0] = 0;
				Value++;
			}
		}
		DomainIndex = atoi(Index);

		// Do we have this relation in our list
		Found = false;
		for (unsigned int j = 0; j < RelationDetail.size(); j++) {
			if (this->Lexicon->Match(RelationDetail[j].PredicateIndex, Predicate)) {
				Found = true;
				if (RelationDetail[j].Arity <= DomainIndex) {
					RelationDetail[j].Arity = DomainIndex + 1;
				}
				break;
			}
		}

		// Did we find an entry
		if (!Found) {
			NewRelationDetail.PredicateIndex = this->Lexicon->Index(Predicate);
			NewRelationDetail.Arity = DomainIndex + 1;
			RelationDetail.push_back(NewRelationDetail);
		}
	}

	// Add the arity to the end of the predicate
	for (unsigned int i = 0; i < RelationDetail.size(); i++) {
		sprintf(NewPredicate, "%s|%d", this->Lexicon->Text(RelationDetail[i].PredicateIndex), RelationDetail[i].Arity);
		RelationDetail[i].PredicateIndex = this->Lexicon->Index(NewPredicate);
	}

	// Make sure we have all of the relations in the Schema
	for (unsigned int i = 0; i < RelationDetail.size(); i++) {
		if (DEBUG) printf("Found %s / %d\n", this->Lexicon->Text(RelationDetail[i].PredicateIndex), RelationDetail[i].Arity);
		this->RelationSchema(RelationDetail[i].PredicateIndex, RelationDetail[i].Arity);
	}

	// Process the entries like (Predicate DomainIndex Value)
	for (unsigned int i = 0; i < DomainEntryValue.size(); i++) {

		// Parse the entry
		strcpy(Entry, DomainEntryValue[i]); 
		Predicate = Entry;
		Index = strstr(Entry, " ");
		if (Index != NULL) {
			Index[0] = 0;
			Index++;
			Value = strstr(Index, " ");
			if (Value != NULL) {
				Value[0] = 0;
				Value++;
			}
		}

		// Check to see if the "value" is actualy a zero arity predicate value|0
		sprintf(NewPredicate, "%s|0", Value);
		if (this->RelationSchemaExists(NewPredicate)) {
			ListEntry = new char[256];
			sprintf(ListEntry, "%s %s list:%s", Entry, Index, Value);
			if (DEBUG) printf("%s\n", ListEntry);
			DomainEntryList.push_back(ListEntry);
		} else {

			RelationSchema = this->RelationSchemaByName(this->Lexicon->Index(Predicate));
			if (RelationSchema != NULL) {
				if (RelationSchema->Fact == hsfcFactPermanent) {
					//printf("   Permanent Fact\n");
				} else {
					//printf("\n");
					DomainIndex = atoi(Index);
					// The value might be a zero arity relation or a lexicon entry
					Reference.Tuple.RelationIndex = -1;
					Reference.Tuple.ID = this->Lexicon->Index(Value);
					Reference.Count = 1;
					Reference.Index = 0;
					// Look for zero arity predicate
					for (unsigned int i = 0; i < this->Relation.size(); i++) {
						if (this->Lexicon->Match(this->Relation[i]->PredicateIndex, Value)) {
							Reference.Tuple.RelationIndex = i;
							Reference.Tuple.ID = 0;
							break;
						}
					}
					RelationSchema->AddToDomain(DomainIndex, &Reference);
				}
			}
		}

	}

	// Process the entries like (Predicate DomainIndex List:Name)
	while (DomainEntryList.size() > 0) {

		// First see if the domains are complete
		for (unsigned int i = 0; i < this->Relation.size(); i++) {
			this->Relation[i]->DomainIsComplete = true;
			for (unsigned int j = 0; j < DomainEntryList.size(); j++) {
				strcpy(Entry, DomainEntryList[j]);
				Index = strstr(Entry, " ");
				Index[0] = 0;
				if (this->Lexicon->PartialMatch(this->Relation[i]->PredicateIndex, Entry)) {
					this->Relation[i]->DomainIsComplete = false;
					break;
				}
			}
		}

		// Process the entry
		for (unsigned int i = 0; i < DomainEntryList.size(); i++) {

			// Parse the entry
			strcpy(Entry, DomainEntryList[i]);
			Predicate = Entry;
			Index = strstr(Entry, " ");
			if (Index != NULL) {
				Index[0] = 0;
				Index++;
				Value = strstr(Index, " ");
				if (Value != NULL) {
					Value[0] = 0;
					Value++;
				}
			}

			// Get the target relation from the predicate and the target domain index
			RelationSchema = this->RelationSchemaByName(this->Lexicon->Index(Predicate));
			DomainIndex = atoi(Index);

			// Only process non facts
			if (RelationSchema != NULL) {
				if (RelationSchema->Fact == hsfcFactPermanent) {
					
					if (DEBUG) printf("Processing %s    Permanent Fact\n", DomainEntryList[i]);
					// Clear the entry
					delete [] DomainEntryList[i];
					DomainEntryList.erase(DomainEntryList.begin() + i);
					i--;

				} else {

					// Only process if the list predicate is complete
					Colon = strstr(Value, ":");
					NestedRelation = this->RelationSchemaByName(this->Lexicon->Index(&Colon[1]));
					if (NestedRelation == NULL) {
						// Clear the entry
						if (DEBUG) printf("Processing %s *** not found\n", DomainEntryList[i]);
						delete [] DomainEntryList[i];
						DomainEntryList.erase(DomainEntryList.begin() + i);
						i--;
					} else {
						if (NestedRelation->DomainIsComplete) {
							// Add the entry
							if (DEBUG) printf("Processing %s = %d items\n", DomainEntryList[i], (int)NestedRelation->RelationSize());
							Reference.Tuple.RelationIndex = NestedRelation->Index;
							Reference.Tuple.ID = -1;
							Reference.Count = (int)NestedRelation->RelationSize();
							Reference.Index = 0;
							RelationSchema->AddToDomain(DomainIndex, &Reference);
							// Clear the entry
							delete [] DomainEntryList[i];
							DomainEntryList.erase(DomainEntryList.begin() + i);
							i--;		
						}
					}
				}
			}
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// CreateSCL
//-----------------------------------------------------------------------------
bool hsfcSchema::CreateSCL(){

	hsfcGDLRelation* NewRelation;
	hsfcGDLRule* NewRule;
	bool Negated;

	// Copy the GDL relations
	for (unsigned int i = 0; i < this->GDL->Relation.size(); i++) {
		// Create the relation and copy
		NewRelation = new hsfcGDLRelation(this->Lexicon);
		NewRelation->FromGDLRelation(this->GDL->Relation[i]);
		// Add to the SCL
		this->SCL->Relation.push_back(NewRelation);
	}

	// Copy the GDL rules
	for (unsigned int i = 0; i < this->GDL->Rule.size(); i++) {
		// Create the rule and copy
		NewRule = new hsfcGDLRule(this->Lexicon);
		NewRule->FromGDLRule(this->GDL->Rule[i]);
		// Add to the SCL
		this->SCL->Rule.push_back(NewRule);
	}

	// Find zero arity
	for (unsigned int i = 0; i < this->SCL->Relation.size(); i++) {
		this->SCL->Relation[i]->FindZeroArity();
	}

	// Find zero arity
	for (unsigned int i = 0; i < this->SCL->Rule.size(); i++) {
		for (unsigned int j = 0; j < this->SCL->Rule[i]->Relation.size(); j++) {
			this->SCL->Rule[i]->Relation[j]->FindZeroArity();
		}
	}

	// Remove any (not ()) and (true ()) and flag the relation accordingly
	for (unsigned int i = 0; i < this->SCL->Rule.size(); i++) {
		for (unsigned int j = 1; j < this->SCL->Rule[i]->Relation.size(); j++) {
			// Is this a (not ()) or a (not atom) or (not (true ())) or (not (true atom))
			if (this->Lexicon->Match(this->SCL->Rule[i]->Relation[j]->PredicateIndex(), "not")) {
				// Point the relation to the second atom
				if (this->SCL->Rule[i]->Relation[j]->Atom[1]->Relation == NULL) {
					this->SCL->Rule[i]->Relation[j]->Atom.erase(this->SCL->Rule[i]->Relation[j]->Atom.begin());
				} else {
					this->SCL->Rule[i]->Relation[j] = this->SCL->Rule[i]->Relation[j]->Atom[1]->Relation;
				}
				this->SCL->Rule[i]->Relation[j]->Not = true;
			}
			// Is this a (true ()) or a (true atom)
			if (this->Lexicon->Match(this->SCL->Rule[i]->Relation[j]->PredicateIndex(), "true")) {
				// Point the relation to the second atom
				Negated = this->SCL->Rule[i]->Relation[j]->Not;
				if (this->SCL->Rule[i]->Relation[j]->Atom[1]->Relation == NULL) {
					this->SCL->Rule[i]->Relation[j]->Atom.erase(this->SCL->Rule[i]->Relation[j]->Atom.begin());
				} else {
					this->SCL->Rule[i]->Relation[j] = this->SCL->Rule[i]->Relation[j]->Atom[1]->Relation;
				}
				this->SCL->Rule[i]->Relation[j]->Not = Negated;
			}
		}
	}

	// Normalise the relation
	for (unsigned int i = 0; i < this->SCL->Relation.size(); i++) {
		this->SCL->Relation[i]->NormaliseTerms();
	}

	// Normalise the rule
	for (unsigned int i = 0; i < this->SCL->Rule.size(); i++) {
		for (unsigned int j = 0; j < this->SCL->Rule[i]->Relation.size(); j++) {
			this->SCL->Rule[i]->Relation[j]->NormaliseTerms();
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// Stratify
//-----------------------------------------------------------------------------
void hsfcSchema::Stratify(){

	unsigned int FirstUnsortedIndex;
	unsigned int LastUnsortedIndex;
	hsfcRuleSchema* RuleSchema;
	int Count;
	hsfcTuple Predicate;
	bool Found;


	// Sort the rules so they are executed as late as possible
	// 0 - Terminal
	// 1 - Goal
	// 2 - Legal
	// 3 - Sees
	// 4 - Next

	// Make everything a 5 to start
	this->Stratum.clear();
	for (unsigned int i = 0; i < this->Lexicon->Size(); i++) {
		Predicate.ID = 5;
		Predicate.RelationIndex = i;
		this->Stratum.push_back(Predicate);
	}

	// Now find all of the key words
	for (unsigned int i = 0; i < this->Lexicon->Size(); i++) {
		if (this->Lexicon->PartialMatch(i, "terminal|")) this->Stratum[i].ID = 0;
		if (this->Lexicon->PartialMatch(i, "goal|")) this->Stratum[i].ID = 1;
		if (this->Lexicon->PartialMatch(i, "legal|")) this->Stratum[i].ID = 2;
		if (this->Lexicon->PartialMatch(i, "sees|")) this->Stratum[i].ID = 3;
		if (this->Lexicon->PartialMatch(i, "next>")) this->Stratum[i].ID = 4;
	}

	// Now find everything necessary to calculate all of the keywords
	for (int k = 0; k < 5; k++) {
		Found = true;
		while (Found) {
			Found = false;
			for (unsigned int i = 0; i < this->Rule.size(); i++) {
				// Check the outputs
				if (this->Stratum[this->Rule[i]->Relation[0]->PredicateIndex].ID == k) {
					for (unsigned int j = 1; j < this->Rule[i]->Relation.size(); j++) {
						if (this->Stratum[this->Rule[i]->Relation[j]->PredicateIndex].ID > k) {
							this->Stratum[this->Rule[i]->Relation[j]->PredicateIndex].ID = k;
							Found = true;
						}
					}
				}
			}
		}
	}

	//for (unsigned int i = 0; i < this->Lexicon->Size(); i++) {
	//	printf("%s %d\n", this->Lexicon->Text(i), this->Stratum[i].ID);
	//}

	//getchar();

	// Sort the predicates into groups according to their stratum
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		// Count how many rules change place
		Count = 0;
		for (unsigned int j = 1; j < this->Stratum.size(); j++) {
			// Are the rules out of order
			if (this->Stratum[j-1].ID > this->Stratum[j].ID) {
				// Swap places
				Predicate.ID = this->Stratum[j].ID;
				Predicate.RelationIndex = this->Stratum[j].RelationIndex;
				this->Stratum[j].ID = this->Stratum[j-1].ID;
				this->Stratum[j].RelationIndex = this->Stratum[j-1].RelationIndex;
				this->Stratum[j-1].ID = Predicate.ID;
				this->Stratum[j-1].RelationIndex = Predicate.RelationIndex;
				Count++;
			}
		}
		if (Count == 0) break;
	}

	// Now sort rule predicates within the strata
	FirstUnsortedIndex = 0;
	for (unsigned int i = 0; i < 5; i++) {

		// This is where it gets tricky
		// A rule predicate is sorted into the stratification if it does not required 
		// an unsorted rule as its predecessor
		// Any cyclic references are left to the end of the stratum

		// Is the group empty
		if (FirstUnsortedIndex >= this->Stratum.size()) continue;
		if (this->Stratum[FirstUnsortedIndex].ID != i) continue;

		// Keep track of the sorted and unsorted predicates
		LastUnsortedIndex = FirstUnsortedIndex;
		for (unsigned int j = FirstUnsortedIndex; j < this->Stratum.size(); j++) {
			if (this->Stratum[j].ID == i) {
				LastUnsortedIndex = j;
			} else {
				break;
			}
		}

		// Process the predicates until all non cyclic rules are sorted
		Count = 1;
		while (Count > 0) {
			Count = 0;
			// Process the unsorted predicates in the group
			for (unsigned int j = FirstUnsortedIndex; j <= LastUnsortedIndex; j++) {

				// Does this predicates require any unsorted rules
				Found = false;
				// Check every rule
				for (unsigned int k = FirstUnsortedIndex; k <= LastUnsortedIndex; k++) {
					// Don't check self referencing rules
					if (this->Stratum[j].RelationIndex == this->Stratum[k].RelationIndex) continue;
					// Can we find a link
					if (Required(this->Stratum[j].RelationIndex, this->Stratum[k].RelationIndex)) {
						Found = true;
						break;
					}
				}
				
				// Can this now be sorted
				if (!Found) {
					Predicate.ID = this->Stratum[j].ID;
					Predicate.RelationIndex = this->Stratum[j].RelationIndex;
					this->Stratum[j].ID = this->Stratum[FirstUnsortedIndex].ID;
					this->Stratum[j].RelationIndex = this->Stratum[FirstUnsortedIndex].RelationIndex;
					this->Stratum[FirstUnsortedIndex].ID = Predicate.ID;
					this->Stratum[FirstUnsortedIndex].RelationIndex = Predicate.RelationIndex;
					FirstUnsortedIndex++;
					Count++;
				}
			}

		}

		// Anything left can stay in their unsorted order as there is a cyclic reference
		FirstUnsortedIndex = LastUnsortedIndex + 1;

	}

	// Now sort the rules according to their output

	// Stratify the body of the rule and set its stratum number
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		// Stratify the body of the rule
		this->Rule[i]->Stratify();
		// Find which group the rule belongs to
		for (unsigned int j = 0; j < this->Stratum.size(); j++) {
			if (this->Rule[i]->Relation[0]->PredicateIndex == this->Stratum[j].RelationIndex) {
				this->Rule[i]->Stratum = j;
			}
		}
	}

	// Sort the rules 
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		// Count how many rules change place
		Count = 0;
		for (unsigned int j = 1; j < this->Rule.size(); j++) {
			// Are the rules out of order
			if (this->Rule[j-1]->Stratum > this->Rule[j]->Stratum) {
				// Swap places
				RuleSchema = this->Rule[j];
				this->Rule[j] = this->Rule[j-1];
				this->Rule[j-1] = RuleSchema;
				Count++;
			}
		}
		if (Count == 0) break;
	}

}

//-----------------------------------------------------------------------------
// SetPermanentFacts
//-----------------------------------------------------------------------------
void hsfcSchema::SetPermanentFacts() {

	hsfcRelationSchema* RelationSchema;
	unsigned int PredicateIndex;
	vector<hsfcGDLTerm> GDLTerm;
	hsfcDomainEntry NewReference;
	vector<hsfcDomainEntry> Reference;
	unsigned int i;

	// Every relation in the SCL is either a fact or a (init (...))

	// Go through all of the SCL relations and construct the fact domains
	// The domains for facts are not independent
	// Do it in reverse order as embedded facts are added at the end of the array
	for (unsigned int ni = 0; ni < this->SCL->Relation.size(); ni++) {

		i = this->SCL->Relation.size() - 1 - ni;

		// Get the Schema relation
		PredicateIndex = this->SCL->Relation[i]->PredicateIndex();
		RelationSchema = this->RelationSchema(PredicateIndex);

		// Is this relation a fact that is not permanent
		if (RelationSchema->Fact == hsfcFactPermanent) {

			// Find the terms that make up the fact
			GDLTerm.clear();
			this->SCL->Relation[i]->Terms(GDLTerm);

			// Nest the terms from the flat SCL
			this->NestTerms(GDLTerm, 0, PredicateIndex);

			// Copy the terms to tuples
			Reference.clear();
			NewReference.Count = 1;
			NewReference.Index = 0;
			for (unsigned int j = 0; j < GDLTerm.size(); j++) {
				NewReference.Tuple.RelationIndex = GDLTerm[j].Tuple.RelationIndex;
				NewReference.Tuple.ID = GDLTerm[j].Tuple.ID;
				Reference.push_back(NewReference);
			}

			// Load the fact into the domain
			RelationSchema->AddToFactDomain(Reference);

		}

	}

}

//-----------------------------------------------------------------------------
// SetInitialRelations
//-----------------------------------------------------------------------------
void hsfcSchema::SetInitialRelations() {

	hsfcRelationSchema* RelationSchema;
	unsigned int PredicateIndex;
	vector<hsfcGDLTerm> GDLTerm;
	hsfcTuple NewReference;
	vector<hsfcTuple> Reference;

	// At this point all of the lists have been identfied
	// Now create the reference lists for initialising the state
	// Read each of the relations in the SCL

	if (DEBUG) printf("\n--- Initial Relations ---------------------------------------------\n");

	this->Fact.clear();
	this->Initial.clear();

	for (unsigned int i = 0; i < this->SCL->Relation.size(); i++) {

		// Get the Schema relation
		PredicateIndex = this->SCL->Relation[i]->PredicateIndex();
		RelationSchema = this->RelationSchema(PredicateIndex);

		// Find the terms that make up the fact
		GDLTerm.clear();
		this->SCL->Relation[i]->Terms(GDLTerm);

		// Nest the terms from the flat scl
		this->NestTerms(GDLTerm, 0, PredicateIndex);

		// Copy the terms to tuples
		Reference.clear();
		for (unsigned int j = 0; j < GDLTerm.size(); j++) {
			NewReference.RelationIndex = GDLTerm[j].Tuple.RelationIndex;
			NewReference.ID = GDLTerm[j].Tuple.ID;
			Reference.push_back(NewReference);
		}

		if (RelationSchema->Fact == hsfcFactInitial) {

			// Add it to the list of initials
			NewReference.RelationIndex = Reference[1].RelationIndex;
			NewReference.ID = Reference[1].ID;
			this->Initial.push_back(NewReference);
			if (DEBUG) printf("%6d.%d   Initial   ", NewReference.RelationIndex, NewReference.ID);
			if (DEBUG) this->PrintRelation(&NewReference, true);

		} else {

			// Is the relation a fact that is not a permanent fact
			if (RelationSchema->Fact != hsfcFactPermanent) {
				// Add it to the list of facts
				NewReference.RelationIndex = RelationSchema->Index;
				NewReference.ID = RelationSchema->ID(Reference);
				if (NewReference.ID == -1) {
					printf("Error: Bad ID\n");
					abort();
				}
				this->Fact.push_back(NewReference);
				if (DEBUG) printf("%6d.%d   Fact      ", NewReference.RelationIndex, NewReference.ID);
				if (DEBUG) this->PrintRelation(&NewReference, true);
			}
		}
	}

	// Identify the State relations
	for (unsigned int i = 0; i < this->SCL->Relation.size(); i++) {
		// Get the Schema relation
		PredicateIndex = this->SCL->Relation[i]->PredicateIndex();
		RelationSchema = this->RelationSchema(PredicateIndex);
		RelationSchema->IsInState = true;
		// Is it an (init (...))
		if (this->Lexicon->PartialMatch(PredicateIndex, "init|")) {
			PredicateIndex = this->SCL->Relation[i]->Atom[1]->Relation->PredicateIndex();
			RelationSchema = this->RelationSchema(PredicateIndex);
			RelationSchema->IsInState = true;
		}
	}

	// Identify the State relations
	for (unsigned int i = 0; i < this->SCL->Rule.size(); i++) {
		// Get the Schema relation
		PredicateIndex = this->SCL->Rule[i]->Relation[0]->PredicateIndex();
		RelationSchema = this->RelationSchema(PredicateIndex);
		RelationSchema->IsInState = true;
		if (this->Lexicon->PartialMatch(PredicateIndex, "next>")) {
			PredicateIndex = this->SCL->Rule[i]->Relation[0]->Atom[1]->Relation->PredicateIndex();
			RelationSchema = this->RelationSchema(PredicateIndex);
			RelationSchema->IsInState = true;
		}
	}

	// Add in (does ... ...)
	PredicateIndex = this->Lexicon->Index("does|2");
	RelationSchema = this->RelationSchema(PredicateIndex);
	RelationSchema->IsInState = true;

}

//-----------------------------------------------------------------------------
// SetNextRelations
//-----------------------------------------------------------------------------
void hsfcSchema::SetNextRelations() {

	hsfcRelationSchema* RelationSchema;
	unsigned int PredicateIndex;
	char Predicate[256];
	vector<hsfcTuple> Reference;
	hsfcRelationLink Link;

	// Now create the reference lists for linking next to predicate
	for (unsigned int i = 0; i < this->Relation.size(); i++) {

		// Is it a (next (...))
		if (this->Lexicon->PartialMatch(this->Relation[i]->PredicateIndex, "next>")) {

			// Flag this list
			this->Relation[i]->Fact = hsfcFactNext;

			// Get the index of the state relation; eg (next>cell ... ...) we want (cell ... ...)
			strcpy(Predicate, this->Lexicon->Text(this->Relation[i]->PredicateIndex));
			for (unsigned int j = 0; j < strlen(Predicate); j++) {
				if (Predicate[j] == '|') Predicate[j] = 0;
			}
			PredicateIndex = this->Lexicon->Index(&Predicate[5]);
			RelationSchema = this->RelationSchemaByName(PredicateIndex);

			// Add it to the list of facts
			Link.SourceListIndex = i;
			Link.DestinationListIndex = RelationSchema->Index;
			this->Next.push_back(Link);

		}
	}

}

//-----------------------------------------------------------------------------
// AddRelationSchema
//-----------------------------------------------------------------------------
void hsfcSchema::AddRelationSchema(hsfcGDLRelation* GDLRelation) {

	//unsigned int TermIndex;

	// Ignore (distinct )
	if (GDLRelation->Atom[0]->TermIndex == this->Lexicon->Index("distinct")) {
		return;
	}
	
	// Ignore (true (....))
	if (GDLRelation->Atom[0]->TermIndex == this->Lexicon->Index("true")) {
		if (GDLRelation->Atom[1] == NULL) {
			printf("Error: Badly formed (true (...))\n");
			abort();
		}
		this->AddRelationSchema(GDLRelation->Atom[1]->Relation);
		return;
	}

	// Ignore (not (....))
	if (GDLRelation->Atom[0]->TermIndex == this->Lexicon->Index("not")) {
		if (GDLRelation->Atom[1] == NULL) {
			printf("Error: Badly formed (not (...))\n");
			abort();
		}
		this->AddRelationSchema(GDLRelation->Atom[1]->Relation);
		return;
	}

	// Find a matching Schema relation
	this->RelationSchema(GDLRelation->PredicateIndex(), GDLRelation->Arity());

	//// Was this an (next>predicate ... ...); if so then we also need to add (predicate ... ...)
	//if (this->Lexicon->PartialMatch(GDLRelation->PredicateIndex(), "next>")) {
	//	//// Add the new predicate to the lexicon
	//	//TermIndex = this->Lexicon->Index(&this->Lexicon->Text(GDLRelation->PredicateIndex())[5]);
	//	//// Add the new relation
	//	//this->RelationSchema(TermIndex, GDLRelation->Arity());
	//	// Add the new predicate to the lexicon
	//	TermIndex = GDLRelation->Atom[1]->Relation->PredicateIndex();
	//	// Add the new relation
	//	this->RelationSchema(TermIndex, GDLRelation->Atom[1]->Relation->Arity());
	//}

	//// Was this an (init~predicate ... ...); if so then we also need to add (predicate ... ...)
	//if (this->Lexicon->Match(GDLRelation->PredicateIndex(), "init")) {
	//	//// Add the new predicate to the lexicon
	//	//TermIndex = this->Lexicon->Index(&this->Lexicon->Text(GDLRelation->PredicateIndex())[5]);
	//	//// Add the new relation
	//	//this->RelationSchema(TermIndex, GDLRelation->Arity());
	//	// Add the new predicate to the lexicon
	//	TermIndex = GDLRelation->Atom[1]->Relation->PredicateIndex();
	//	// Add the new relation
	//	this->RelationSchema(TermIndex, GDLRelation->Atom[1]->Relation->Arity());
	//}

	// Go through recursively to find all of the relations
	for (unsigned int i = 0; i < GDLRelation->Atom.size(); i++) {
		// Is this atom a relation
		if (GDLRelation->Atom[i]->Relation != NULL) {
			this->AddRelationSchema(GDLRelation->Atom[i]->Relation);
		}
	}

}

//-----------------------------------------------------------------------------
// AddRuleSchema
//-----------------------------------------------------------------------------
void hsfcSchema::AddRuleSchema(hsfcGDLRule* GDLRule) {

	hsfcRuleSchema* NewRule;

	// Add the new rule
	NewRule = new hsfcRuleSchema(this->Lexicon);
	NewRule->Initialise();
	this->Rule.push_back(NewRule);

	// Create the rule
	NewRule->Create(GDLRule);

}

//-----------------------------------------------------------------------------
// RelationSchema
//-----------------------------------------------------------------------------
hsfcRelationSchema* hsfcSchema::RelationSchema(int PredicateIndex, int Arity) {

	hsfcRelationSchema* Target;

	// Does the relation already exist
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		if ((this->Relation[i]->PredicateIndex == PredicateIndex) && (this->Relation[i]->Arity == Arity)) {
			return this->Relation[i];
		}
	}

	// Not found so add it
	Target = new hsfcRelationSchema(this->Lexicon);
	Target->Initialise(PredicateIndex, Arity);
	Target->Index = (int)this->Relation.size();
	this->Relation.push_back(Target);

	return Target;

}

//--- Overload ----------------------------------------------------------------
hsfcRelationSchema* hsfcSchema::RelationSchema(int PredicateIndex) {

	// Does the relation already exist
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		if (this->Relation[i]->PredicateIndex == PredicateIndex) {
			return this->Relation[i];
		}
	}

	if (!this->Lexicon->Match(PredicateIndex, "distinct")) {
		printf("Error: Relation %d does not exist\n", PredicateIndex);
	}

	return NULL;

}


//-----------------------------------------------------------------------------
// RelationSchemaByName
//-----------------------------------------------------------------------------
hsfcRelationSchema* hsfcSchema::RelationSchemaByName(int PredicateIndex) {

	char Predicate[256];

	// Create the predicate
	sprintf(Predicate, "%s|", this->Lexicon->Text(PredicateIndex));

	// Does the relation already exist
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		if (this->Lexicon->PartialMatch(this->Relation[i]->PredicateIndex, Predicate)) {
			return this->Relation[i];
		}
	}

	if (!this->Lexicon->Match(PredicateIndex, "distinct")) {
		printf("Error: Relation %d does not exist\n", PredicateIndex);
	}

	return NULL;

}


//-----------------------------------------------------------------------------
// RelationSchemaByName
//-----------------------------------------------------------------------------
bool hsfcSchema::RelationSchemaExists(char* Predicate) {

	// Does the relation already exist
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		if (this->Lexicon->Match(this->Relation[i]->PredicateIndex, Predicate)) {
			return true;
		}
	}

	return false;

}


//-----------------------------------------------------------------------------
// RuleIsRequired
//-----------------------------------------------------------------------------
bool hsfcSchema::RuleIsRequired(hsfcRuleSchema* Rule, unsigned int FirstIndex, unsigned int LastIndex) {

	// Check the output for the specific rule
	for (unsigned int i = FirstIndex; i <= LastIndex; i++) {
		// Don't check a rule against itself
		if (this->Rule[i] != Rule) {
			// Check all of the input relations from all of the rules in the range
			for (unsigned int j = 1; j < this->Rule[i]->Relation.size(); j++) {
				// Is the output of one rule an input for another rule
				if (Rule->Relation[0]->PredicateIndex == this->Rule[i]->Relation[j]->PredicateIndex) {
					return true;
				}
			}
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// RuleRequires
//-----------------------------------------------------------------------------
bool hsfcSchema::Required(int OutputPredicateIndex, int InputPredicateIndex) {

	// Check for a rule that links the two predicates
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		// Does this rule produce the required predicate
		if (this->Rule[i]->Relation[0]->PredicateIndex == OutputPredicateIndex) {
			// Check all of the input relations
			for (unsigned int j = 1; j < this->Rule[i]->Relation.size(); j++) {
				// Is the output of one rule an input for this rule
				if (this->Rule[i]->Relation[j]->PredicateIndex == InputPredicateIndex) {
					return true;
				}
			}
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// NestTerms
//-----------------------------------------------------------------------------
void hsfcSchema::NestTerms(vector<hsfcGDLTerm>& Term, int RelationOffset, int PredicateIndex) {

	hsfcRelationSchema* Relation;
	hsfcRelationSchema* NestedRelation;
	hsfcTuple Tuple;
	vector<hsfcTuple> NestedTerm;
	int NestedID;

	// Go through the terms an replace them with a single Term from a nested relation
	// Terms: 17:-1.5 (5:-1.7 5:-1.9) 17:-1.3

	// Get the relation for this subset of terms
	Relation = this->RelationSchema(Term[RelationOffset].PredicateIndex);

	// Are there enough terms
	if ((RelationOffset + Relation->Arity) > (int)Term.size()) {
		printf("Error: Too few terms\n");
		return;
	}

	// Look for yet another level of nesting
	for (int i = RelationOffset; i <= RelationOffset + Relation->Arity; i++) {
		// Is the relation the same
		if (Term[i].PredicateIndex != Term[RelationOffset].PredicateIndex) {
			// Get the nested relation
			NestedRelation = this->RelationSchema(Term[i].PredicateIndex);
			this->NestTerms(Term, i, Term[RelationOffset].PredicateIndex);
			// The size of the array has changed; are there enough terms
			if ((RelationOffset + Relation->Arity) > (int)Term.size()) {
				printf("Error: Too few terms\n");
				return;
			}
		}
	}

	// Is this the primary relation
	if (RelationOffset == 0) return;

	// Now we have all the terms from a single relation
	// Replace all of those terms with a single term
	for (int i = RelationOffset; i <= RelationOffset + Relation->Arity; i++) {
		Tuple.RelationIndex = Term[i].Tuple.RelationIndex;
		Tuple.ID = Term[i].Tuple.ID;
		NestedTerm.push_back(Tuple);
	}

	// Replace the first nested term with a single term
	NestedID = Relation->ID(NestedTerm);
	if (NestedID == -1) {
		printf("Error: Bad ID\n");
		printf("Terms\n");
		for (unsigned int i = 0; i < NestedTerm.size(); i++) {
			printf("   %3d.%d", NestedTerm[i].RelationIndex, NestedTerm[i].ID);
			if (NestedTerm[i].RelationIndex == -1) printf("\t%s", this->Lexicon->Text(NestedTerm[i].ID));
			printf("\n");
		}
		printf("Relation\n");
		Relation->Print();
		abort();
	}

	Term[RelationOffset].PredicateIndex = PredicateIndex; 
	Term[RelationOffset].Tuple.RelationIndex = Relation->Index; 
	Term[RelationOffset].Tuple.ID = NestedID; 

	// Delete the remaining terms
	for (int i = RelationOffset + 1; i <= RelationOffset + Relation->Arity; i++) {
		Term.erase(Term.begin() + RelationOffset + 1);
	}

}

//-----------------------------------------------------------------------------
// PrintRelations
//-----------------------------------------------------------------------------
void hsfcSchema::PrintRelations() {

    printf("Relations\n");

	// Print out the relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		printf("%d. ", i);
		this->Relation[i]->Print();
	}

}
//-----------------------------------------------------------------------------
// PrintRules
//-----------------------------------------------------------------------------
void hsfcSchema::PrintRules() {

    printf("Rules\n");

	// Print out the Schema Rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
	    printf("%d.\n", i);
		this->Rule[i]->Print();
	}

}

//-----------------------------------------------------------------------------
// PrintReferences
//-----------------------------------------------------------------------------
void hsfcSchema::PrintReferences() {

	// Print out the Permanent Facts
    printf("Permanent Facts\n");
	for (unsigned int i = 0; i < this->Fact.size(); i++) {
		printf("   %d.%d - ", this->Fact[i].RelationIndex, this->Fact[i].ID);
		this->PrintRelation(&this->Fact[i], true);
	}

	// Print out the Initial Facts
    printf("Initial Facts\n");
	for (unsigned int i = 0; i < this->Initial.size(); i++) {
		printf("   %d.%d - ", this->Initial[i].RelationIndex, this->Initial[i].ID);
		this->PrintRelation(&this->Initial[i], true);
	}

	// Print out the Next Linkages
    printf("Next Links\n");
	for (unsigned int i = 0; i < this->Next.size(); i++) {
		printf("   %d => %d\n", this->Next[i].SourceListIndex, this->Next[i].DestinationListIndex);
	}

}

//-----------------------------------------------------------------------------
// Factorial
//-----------------------------------------------------------------------------
int Factorial(int Number) {

	int Result;
	
	Result = 1;
	
	if (Number > 10) {
		Result = 39916800; // 11!
		return Result;
	}

	for (int i = 2; i <= Number; i++) {
		Result *= i;
	}

	return Result;

}