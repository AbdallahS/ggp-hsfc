//=============================================================================
// Project: High Speed Forward Chaining
// Module: State
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcState.h"
#include "hsfc_config.h"

using namespace std;

//=============================================================================
// CLASS: hsfcStateManager
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcStateManager::hsfcStateManager(hsfcLexicon* Lexicon){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->NextRelationIndex = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcStateManager::~hsfcStateManager(void){



}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcStateManager::Initialise(){

	// Set the properties
	this->Schema = Schema;

	// Reset the relation indexes
	this->RoleRelationIndex = 0;
	this->TerminalRelationIndex = 0;
	this->GoalRelationIndex = 0;
	this->LegalRelationIndex = 0;
	this->DoesRelationIndex = 0;
	this->SeesRelationIndex = 0;
	if (this->NextRelationIndex != NULL) delete[](this->NextRelationIndex);
	this->NoNextRelation = 0;
	this->StateSize = 0;
	this->MaxStateSize = 0;
	this->MaxRelationSize = 1000000;

}

//-----------------------------------------------------------------------------
// SetSchema
//-----------------------------------------------------------------------------
void hsfcStateManager::SetSchema(hsfcSchema* Schema, int MaxRelationSize){

	int NoNextRelation;
	int Index;

	// Set the properties
	this->Initialise();
	this->MaxRelationSize = MaxRelationSize;
	this->Schema = Schema;

	// Set the relation indexes
	NoNextRelation = 0;
	for (unsigned int i = 0; i < this->Schema->Relation.size(); i++) {
		if (this->Lexicon->Match(this->Schema->Relation[i]->PredicateIndex, "role/1")) {
			this->RoleRelationIndex = i;
		}
		if (this->Lexicon->Match(this->Schema->Relation[i]->PredicateIndex, "terminal/0")) {
			this->TerminalRelationIndex = i;
		}
		if (this->Lexicon->Match(this->Schema->Relation[i]->PredicateIndex, "goal/2")) {
			this->GoalRelationIndex = i;
		}
		if (this->Lexicon->Match(this->Schema->Relation[i]->PredicateIndex, "legal/2")) {
			this->LegalRelationIndex = i;
		}
		if (this->Lexicon->Match(this->Schema->Relation[i]->PredicateIndex, "does/2")) {
			this->DoesRelationIndex = i;
		}
		if (this->Lexicon->Match(this->Schema->Relation[i]->PredicateIndex, "sees/2")) {
			this->SeesRelationIndex = i;
		}
		if (this->Lexicon->PartialMatch(this->Schema->Relation[i]->PredicateIndex, "next~")) {
			this->NoNextRelation++;
		}
	}

	// Create the next relation array
	this->NextRelationIndex = new int[this->NoNextRelation];

	// Add the reference to the next relations
	Index = 0;
	for (unsigned int i = 0; i < this->Schema->Relation.size(); i++) {
		if (this->Lexicon->PartialMatch(this->Schema->Relation[i]->PredicateIndex, "next~")) {
			NextRelationIndex[Index] = i;
			Index++;
		}
	}

}

//-----------------------------------------------------------------------------
// CreateState
//-----------------------------------------------------------------------------
hsfcState* hsfcStateManager::CreateState(void){

	hsfcState* State;

	// Create the State
	State = new hsfcState();

	// Set the properties
	State->CurrentStep = 0;
	State->Round = 0;
	// Set Arrays

	State->MaxNumRelations = NULL;
	State->NumRelations = NULL;
	State->RelationID = NULL;
	State->RelationExists = NULL;
	State->RelationIDSorted = NULL;

	return State;

}

//-----------------------------------------------------------------------------
// FreeState
//-----------------------------------------------------------------------------
void hsfcStateManager::FreeState(hsfcState* State){

	// Is there actually a state to free
	if (State->RelationID == NULL) return;
	
	// Free the memory for the state
	for (int i = 0; i < this->NumRelationLists; i++) {
		if ((State->RelationID[i] != NULL) && (State->RelationID[i] != NULL)) {
			delete[](State->RelationID[i]);
		}
		if ((State->RelationExists != NULL) && (State->RelationExists[i] != NULL)) {
			delete[](State->RelationExists[i]);
		}
		if ((State->RelationIDSorted != NULL) && (State->RelationIDSorted[i] != NULL)) {
			delete[](State->RelationIDSorted[i]);
		}
	}
	if (State->MaxNumRelations != NULL) {
		delete[](State->MaxNumRelations);
		State->MaxNumRelations = NULL;
	}
	if (State->NumRelations != NULL) {
		delete[](State->NumRelations);
		State->NumRelations = NULL;
	}
	if (State->RelationID != NULL) {
		delete[](State->RelationID);
		State->RelationID = NULL;
	}
	if (State->RelationExists != NULL) {
		delete[](State->RelationExists);
		State->RelationExists = NULL;
	}
	if (State->RelationIDSorted != NULL) {
		delete[](State->RelationIDSorted);
		State->RelationID = NULL;
	}

}

//-----------------------------------------------------------------------------
// InitialiseState
//-----------------------------------------------------------------------------
void hsfcStateManager::InitialiseState(hsfcState* State){

	// Free the memory for the state
	this->FreeState(State);

	// Initialise the State from the Schema
	this->NumRelationLists = this->Schema->Relation.size();

	// Create the arrays for each relation list
	State->NumRelations = new int[this->NumRelationLists];
	State->MaxNumRelations = new int[this->NumRelationLists];
	State->RelationID = new int*[this->NumRelationLists];
	State->RelationExists = new bool*[this->NumRelationLists];
	State->RelationIDSorted = new int*[this->NumRelationLists];

	// Create the arrays for each relation
	for (int i = 0; i < this->NumRelationLists; i++) {
		State->NumRelations[i] = 0;
		State->MaxNumRelations[i] = 0;
		State->RelationID[i] = NULL;
		State->RelationIDSorted[i] = NULL;
		State->RelationExists[i] = NULL;
		if (this->Schema->Relation[i]->IsInState) {
			if (this->Schema->Relation[i]->IDCountDbl < (double)this->MaxRelationSize) {
				State->MaxNumRelations[i] = this->Schema->Relation[i]->IDCount;
				State->RelationID[i] = new int[State->MaxNumRelations[i]];
				this->StateSize += State->MaxNumRelations[i] * sizeof(int);
				State->RelationExists[i] = new bool[State->MaxNumRelations[i]];
				this->StateSize += State->MaxNumRelations[i] * sizeof(bool);
				// Clear the exists array
				for (int j = 0; j < State->MaxNumRelations[i]; j++) {
					State->RelationExists[i][j] = false;
				}
			} else {
				State->MaxNumRelations[i] = this->MaxRelationSize;
				State->RelationID[i] = new int[this->MaxRelationSize];
				this->StateSize += this->MaxRelationSize * sizeof(int);
				State->RelationIDSorted[i] = new int[this->MaxRelationSize];
				this->StateSize += this->MaxRelationSize * sizeof(int);
			}
		}
	}

	// Fully populate any permanent fact relations
	// Permanent facts cannot use IDSorted
	for (unsigned int i = 0; i < this->Schema->Relation.size(); i++) {
		if (this->Schema->Relation[i]->Fact == hsfcFactPermanent) {
			for (int j = 0; j < State->MaxNumRelations[i]; j++) {
				State->RelationID[i][j] = j;
				State->RelationExists[i][j] = true;
			}
			State->NumRelations[i] = State->MaxNumRelations[i];
		}
	}

	// Add the Fact relations from the reference table
	for (unsigned int i = 0; i < this->Schema->Fact.size(); i++) {
		// Add the RelationID to the state
		this->AddRelation(State, &(this->Schema->Fact[i]));
	}

	// Reset the step counters
	State->CurrentStep = 0;
	State->Round = 0;

}

//-----------------------------------------------------------------------------
// FromState
//-----------------------------------------------------------------------------
void hsfcStateManager::FromState(hsfcState* State, hsfcState* Source){

	// Initialise the state
	this->ResetState(State);

	// Copy the relations
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (this->Schema->Relation[i]->Fact != hsfcFactPermanent) {
			for (int j = 0; j < Source->NumRelations[i]; j++) {
				if (State->RelationID[i] != NULL) {
					State->RelationID[i][j] = Source->RelationID[i][j];
				}
				if (State->RelationExists[i] != NULL) {
					State->RelationExists[i][Source->RelationID[i][j]] = true;
				}
				if (State->RelationIDSorted[i] != NULL) {
					State->RelationIDSorted[i][j] = Source->RelationIDSorted[i][j];
				}
			}
			State->NumRelations[i] = Source->NumRelations[i];
		}
	}

	// Copy the details
	State->CurrentStep = Source->CurrentStep;
	State->Round = Source->Round;

}

//-----------------------------------------------------------------------------
// ResetState
//-----------------------------------------------------------------------------
void hsfcStateManager::ResetState(hsfcState* State) {

	// Go through all of the relations and clear all the lists; except the facts
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (this->Schema->Relation[i]->Fact != hsfcFactPermanent) {
			if (State->RelationExists[i] != NULL) {
				for (int j = 0; j < State->NumRelations[i]; j++) {
					State->RelationExists[i][State->RelationID[i][j]] = false;
				}
			}
			State->NumRelations[i] = 0;
		}
	}

	// Add the Fact relations from the reference table
	for (unsigned int i = 0; i < this->Schema->Fact.size(); i++) {
		this->AddRelation(State, &(this->Schema->Fact[i]));
	}

}

//-----------------------------------------------------------------------------
// SetInitialState
//-----------------------------------------------------------------------------
void hsfcStateManager::SetInitialState(hsfcState* State) {

	// Reset the state
	this->ResetState(State);

	// Add the (init (...)) relations from the state
	for (unsigned int i = 0; i < this->Schema->Initial.size(); i++) {
		this->AddRelation(State, &this->Schema->Initial[i]);
	}

	// Reset the cycle counter
	State->Round = 0;
	State->CurrentStep = 0;

}

//-----------------------------------------------------------------------------
// NextState
//-----------------------------------------------------------------------------
void hsfcStateManager::NextState(hsfcState* State) {

	int SourceIndex;
	int DestinationIndex;
	hsfcTuple Reference;

	// (next (cell ... ...)) ==> (cell ... ...)
	// (next_cell ... ...) ==> (cell ... ...)
	// Domains for (next_cell ) and (cell ) are identical
	// So the lists can just be copied

	// Clear all of the lists except for the facts and the next relations
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (this->Schema->Relation[i]->Fact == hsfcFactNone) {
			if (State->RelationExists[i] != NULL) {
				for (int j = 0; j < State->NumRelations[i]; j++) {
					State->RelationExists[i][State->RelationID[i][j]] = false;
				}
			}
			State->NumRelations[i] = 0;
		}
	}

	// Transfer the lists from next to predicate
	for (unsigned int i = 0; i < this->Schema->Next.size(); i++) {

		SourceIndex = this->Schema->Next[i].SourceListIndex;
		DestinationIndex = this->Schema->Next[i].DestinationListIndex;

		// Transfer of relations
		// There is an opportunity for a bulk transfer flag
		for (int j = 0; j < State->NumRelations[SourceIndex]; j++) {
			Reference.RelationIndex = DestinationIndex;
			Reference.ID = State->RelationID[SourceIndex][j];
			this->AddRelation(State, &Reference);
		}

	}

	// Clear all of the next> lists
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (this->Schema->Relation[i]->Fact == hsfcFactNext) {
			if (State->RelationExists[i] != NULL) {
				for (int j = 0; j < State->NumRelations[i]; j++) {
					State->RelationExists[i][State->RelationID[i][j]] = false;
				}
			}
			State->NumRelations[i] = 0;
		}
	}

	// Add in any permanent relations that are in nonpermanent lists eg. (legal role noop)
	for (unsigned int i = 0; i < this->Schema->Fact.size(); i++) {
		this->AddRelation(State, &(this->Schema->Fact[i]));
	}

	// Advance the Cycle counter
	State->Round = State->Round + 1;
	State->CurrentStep = 0;

}

//-----------------------------------------------------------------------------
// AddRelation
//-----------------------------------------------------------------------------
void hsfcStateManager::AddRelation(hsfcState* State, hsfcTuple* Tuple){

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;

	// Does the list have Exists or Sorted
	if (State->RelationIDSorted[Tuple->RelationIndex] != NULL) {

		// Uses Sorted
		// Binary search of the list; adding any unfound values in sort order
		Target = 0;
		LowerBound = 0;
		UpperBound = 0;
		Compare = 0;
		
		// Is the list empty
		if (State->NumRelations[Tuple->RelationIndex] > 0) {

			// Look for the Tuple according to its value
			UpperBound = State->NumRelations[Tuple->RelationIndex] - 1;
			while (LowerBound <= UpperBound) {

				// Find the target value and compare
				Target = (LowerBound + UpperBound) / 2;
				if (State->RelationIDSorted[Tuple->RelationIndex][Target] > Tuple->ID) {
					Compare = -1;
				} else {
					if (State->RelationIDSorted[Tuple->RelationIndex][Target] == Tuple->ID) {
						Compare = 0;
					} else {
						Compare = 1;
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

		// Shuffle everything down
		for (int i = State->NumRelations[Tuple->RelationIndex]; i > Target; i--) {
			State->RelationIDSorted[Tuple->RelationIndex][i] = State->RelationIDSorted[Tuple->RelationIndex][i - 1];
		}
		// Add it to the list
		State->RelationID[Tuple->RelationIndex][State->NumRelations[Tuple->RelationIndex]] = Tuple->ID;
		State->RelationIDSorted[Tuple->RelationIndex][Target] = Tuple->ID;

		// Increment the number of relations in the list
		State->NumRelations[Tuple->RelationIndex]++;

		if (State->NumRelations[Tuple->RelationIndex] == State->MaxNumRelations[Tuple->RelationIndex]) {
			printf("Error: MaxRelationSize exceeded\n");
			abort();
		}

	} else {

		// Uses Exists
		if (!State->RelationExists[Tuple->RelationIndex][Tuple->ID]) {
			// Add it to the list
			State->RelationID[Tuple->RelationIndex][State->NumRelations[Tuple->RelationIndex]] = Tuple->ID;
			State->RelationExists[Tuple->RelationIndex][Tuple->ID] = true;
			// Increment the number of relations in the list
			State->NumRelations[Tuple->RelationIndex]++;
		}

	}

}

//-----------------------------------------------------------------------------
// RelationExists
//-----------------------------------------------------------------------------
bool hsfcStateManager::RelationExists(hsfcState* State, hsfcTuple* Tuple){

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;

	// Does the list have Exists or Sorted
	if (State->RelationIDSorted[Tuple->RelationIndex] != NULL) {

		// Uses Sorted
		// Binary search of the list; adding any unfound values in sort order
		Target = 0;
		LowerBound = 0;
		UpperBound = 0;
		Compare = 0;
		
		// Is the list empty
		if (State->NumRelations[Tuple->RelationIndex] > 0) {

			// Look for the Tuple according to its value
			UpperBound = State->NumRelations[Tuple->RelationIndex] - 1;
			while (LowerBound <= UpperBound) {

				// Find the target value and compare
				Target = (LowerBound + UpperBound) / 2;
				if (State->RelationIDSorted[Tuple->RelationIndex][Target] > Tuple->ID) {
					Compare = -1;
				} else {
					if (State->RelationIDSorted[Tuple->RelationIndex][Target] == Tuple->ID) {
						Compare = 0;
					} else {
						Compare = 1;
					}
				}

				// Compare the values
				if (Compare == 0) return true;
				if (Compare < 0) UpperBound = Target - 1;
				if (Compare > 0) LowerBound = Target + 1;
			}
		}

		return false;

	} else {

		// Uses Exists
		return State->RelationExists[Tuple->RelationIndex][Tuple->ID];

	}

}

//-----------------------------------------------------------------------------
// CalculateStateSize
//-----------------------------------------------------------------------------
bool hsfcStateManager::CalculateStateSize() {

	bool Result;

	// Initialise the State from the Schema
	Result = true;
	this->NumRelationLists = this->Schema->Relation.size();
	this->MaxStateSize = 0;
	this->StateSize = 0;

	// Calculate the maximum size of the database
	this->MaxStateSize += (double)(2 * this->NumRelationLists * sizeof(int));
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (this->Schema->Relation[i]->IsInState) {
			this->MaxStateSize += this->Schema->Relation[i]->IDCountDbl * (double)sizeof(int);
			this->MaxStateSize += this->Schema->Relation[i]->IDCountDbl * (double)sizeof(bool);
		}
	}

	// Calculate the size of the database
	this->StateSize = 0;
	this->StateSize += 2 * this->NumRelationLists * sizeof(int);
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (this->Schema->Relation[i]->IsInState) {
			if (this->Schema->Relation[i]->IDCountDbl > (double)this->MaxRelationSize) {
				this->StateSize += this->MaxRelationSize * sizeof(int);
				this->StateSize += this->MaxRelationSize * sizeof(bool);
				if (this->Schema->Relation[i]->Fact == hsfcFactPermanent) {
					printf("Error: Permanent Fact too big\n");
					Result = false;
				}
			} else {
				this->StateSize += this->Schema->Relation[i]->IDCount * sizeof(int);
				this->StateSize += this->Schema->Relation[i]->IDCount * sizeof(bool);
			}
		}
	}

	return Result;

}

//-----------------------------------------------------------------------------
// CompareStates
//-----------------------------------------------------------------------------
void hsfcStateManager::CompareStates(hsfcState* State1, hsfcState* State2) {

	hsfcTuple Tuple;
	int Index;

	printf("\n--- States -------------------------------------------------\n");

	// Print the relations
	for (int i = 0; i < this->NumRelationLists; i++) {
		Index = 0;
		Tuple.RelationIndex = i;
		while ((Index < State1->NumRelations[i]) || (Index < State2->NumRelations[i])) {
			if (this->Schema->Relation[i]->Fact == hsfcFactPermanent) {
				if (State1->RelationID[i][Index] != State2->RelationID[i][Index]) {
					Tuple.ID = State1->RelationID[i][Index];
					this->Schema->PrintRelation(&Tuple, false);
					printf(" != \t");
					Tuple.ID = State2->RelationID[i][Index];
					this->Schema->PrintRelation(&Tuple, true);
				}
			} else {
				if (Index < State1->NumRelations[i]) {
					printf("%6d.%d\t", i, State1->RelationID[i][Index]);
					Tuple.ID = State1->RelationID[i][Index];
					this->Schema->PrintRelation(&Tuple, false);
				}
				if (Index < State2->NumRelations[i]) {
					printf("%6d.%d\t", i, State2->RelationID[i][Index]);
					Tuple.ID = State2->RelationID[i][Index];
					this->Schema->PrintRelation(&Tuple, false);
				}
				printf("\n");
			}
			Index++;
		}
	}

	printf("------------------------------------------------------------\n");

}

//-----------------------------------------------------------------------------
// StateAsText
//-----------------------------------------------------------------------------
char* hsfcStateManager::StateAsText(hsfcState* State) {

	char* Text;
	int Length;
	char TupleText[24];

	// Start with the round and the step
	Length = sprintf(TupleText, "%d %d ", State->Round, State->CurrentStep);

	// Go through all of the relations and convert to tuples; except the facts
	// Calculate the size of the sting first
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (this->Schema->Relation[i]->Fact != hsfcFactPermanent) {
			for (int j = 0; j < State->NumRelations[i]; j++) {
				Length += sprintf(TupleText, "%d.%d ", i, State->RelationID[i][j]);
			}
		}
	}

	// Allocate the memory
	Text = new char[Length + 1];

	// Do it for real
	Length = sprintf(Text, "%d %d ", State->Round, State->CurrentStep);

	// Convert to text
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (this->Schema->Relation[i]->Fact != hsfcFactPermanent) {
			for (int j = 0; j < State->NumRelations[i]; j++) {
				Length += sprintf(&Text[Length], "%d.%d ", i, State->RelationID[i][j]);
			}
		}
	}
	Text[Length - 1] = 0;

	return Text;

}

//-----------------------------------------------------------------------------
// StateFromText
//-----------------------------------------------------------------------------
bool hsfcStateManager::StateFromText(hsfcState* State, char* Text) {

	hsfcTuple Tuple;
	char* TupleText;

	// Text format
	// "# # #.# #.# #.# #.#" == "round step list.id list.id ... "

	// Reset the state
	this->ResetState(State);

	// Start with the round 
	TupleText = Text;
	if (sscanf(TupleText, "%d", &State->Round) != 1) return false;
	TupleText = strchr(TupleText, ' ');
	if (TupleText == NULL) return false;

	// Next the current step
	TupleText++;
	if (sscanf(TupleText, "%d", &State->CurrentStep) != 1) return false;
	TupleText = strchr(TupleText, ' ');
	if (TupleText == NULL) return true;

	// Go through all of the relations and convert text to relations
	while(TupleText != NULL) {
		TupleText++;
		if (sscanf(TupleText, "%d.%d", &Tuple.RelationIndex, &Tuple.ID) != 2) return false;
		this->AddRelation(State, &Tuple);
		TupleText = strchr(TupleText, ' ');
	}

	return true;

}

//-----------------------------------------------------------------------------
// PrintRelations
//-----------------------------------------------------------------------------
void hsfcStateManager::PrintRelations(hsfcState* State, bool PermanentFacts) {

	hsfcTuple Tuple;

	printf("\n--- State --------------------------------------------------\n");

	// Print the relations
	for (int i = 0; i < this->NumRelationLists; i++) {
		if (PermanentFacts || (this->Schema->Relation[i]->Fact != hsfcFactPermanent)) {
			for (int j = 0; (j < State->NumRelations[i]) && (j <= 32); j++) {
				printf("%6d.%d\t", i, State->RelationID[i][j]);
				Tuple.RelationIndex = i;
				Tuple.ID = State->RelationID[i][j];
				this->Schema->PrintRelation(&Tuple, true);
			}
			if ((this->Schema->Relation[i]->Fact != hsfcFactPermanent) && (State->NumRelations[i] > 16)) {
				printf("Count = %d\n", State->NumRelations[i]);
			}
		}
	}

	printf("------------------------------------------------------------\n");

}



