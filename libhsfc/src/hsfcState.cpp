//=============================================================================
// Project: High Speed Forward Chaining
// Module: State
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcState.h"

using namespace std;

//=============================================================================
// CLASS: hsfcStateManager
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcStateManager::hsfcStateManager(hsfcLexicon* Lexicon, hsfcDomainManager* DomainManager){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->DomainManager = DomainManager;
	this->NextRelationIndex = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcStateManager::~hsfcStateManager(void){

	// Destroy the domains
	//this->FreeDomains();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcStateManager::Initialise(){

	// Destroy the domains
	//this->FreeDomains();

	// Set the properties
	this->Schema = NULL;

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
	this->MaxRelationSize = 0;
	//this->FullPermanent.clear(); 
	//this->PartPermanent.clear(); 
	//this->Initial.clear();
	//this->Next.clear();

}

//-----------------------------------------------------------------------------
// SetSchema
//-----------------------------------------------------------------------------
bool hsfcStateManager::SetSchema(hsfcSchema* Schema){

	int NoNextRelation;
	int Index;
	unsigned int RoleSize;
	unsigned int Size;
	hsfcTuple* RoleEntry;
	hsfcTuple* GoalEntry;
	hsfcTuple* LegalEntry;
	hsfcTuple* DoesEntry;
	hsfcTuple* SeesEntry;
	int TrueIndex;
	hsfcReference NewReference;

	this->Lexicon->IO->WriteToLog(2, true, "Translating Schema ...\n");

	// Set the properties
	this->Initialise();
	this->MaxRelationSize = this->Lexicon->IO->Parameters->MaxRelationSize;
	this->Schema = Schema;

	this->Lexicon->IO->WriteToLog(2, true, "  Classify Relations\n");

	// Set the relation indexes
	NoNextRelation = 0;
	for (unsigned int i = 1; i < this->Schema->RelationSchema.size(); i++) {
		if (this->Lexicon->Match(this->Schema->RelationSchema[i]->NameID, "role/1")) {
			this->RoleRelationIndex = i;
			this->Lexicon->IO->FormatToLog(3, true, "    RoleRelationIndex = %u\n", i);
		}
		if (this->Lexicon->Match(this->Schema->RelationSchema[i]->NameID, "terminal/0")) {
			this->TerminalRelationIndex = i;
			this->Lexicon->IO->FormatToLog(3, true, "    TerminalRelationIndex = %u\n", i);
		}
		if (this->Lexicon->Match(this->Schema->RelationSchema[i]->NameID, "goal/2")) {
			this->GoalRelationIndex = i;
			this->Lexicon->IO->FormatToLog(3, true, "    GoalRelationIndex = %u\n", i);
		}
		if (this->Lexicon->Match(this->Schema->RelationSchema[i]->NameID, "legal/2")) {
			this->LegalRelationIndex = i;
			this->Lexicon->IO->FormatToLog(3, true, "    LegalRelationIndex = %u\n", i);
		}
		if (this->Lexicon->Match(this->Schema->RelationSchema[i]->NameID, "does/2")) {
			this->DoesRelationIndex = i;
			this->Lexicon->IO->FormatToLog(3, true, "    DoesRelationIndex = %u\n", i);
		}
		if (this->Lexicon->Match(this->Schema->RelationSchema[i]->NameID, "sees/2")) {
			this->SeesRelationIndex = i;
			this->Lexicon->IO->FormatToLog(3, true, "    SeesRelationIndex = %u\n", i);
		}
		if (this->Lexicon->PartialMatch(this->Schema->RelationSchema[i]->NameID, "next:")) {
			this->NoNextRelation++;
		}
	}

	this->Lexicon->IO->WriteToLog(2, true, "  Index (next ...)\n");

	// Create the next relation array
	this->NextRelationIndex = new unsigned int[this->NoNextRelation];

	// Add the reference to the next relations
	Index = 0;
	for (unsigned int i = 1; i < this->Schema->RelationSchema.size(); i++) {
		if (this->Lexicon->PartialMatch(this->Schema->RelationSchema[i]->NameID, "next:")) {
			NextRelationIndex[Index] = i;
			this->Lexicon->IO->FormatToLog(3, true, "    NextRelationIndex = %d\n", i);
			Index++;
		}
	}

	this->Lexicon->IO->WriteToLog(2, true, "  Sizing State\n");

	// Check the maximum relation size
	if (this->MaxRelationSize < MAX_DOMAIN_ENTRIES) {
		this->Lexicon->IO->WriteToLog(0, false, "Warning: resetting MaxRelationSize = MAX_DOMAIN_ENTRIES hsfcStateManager::SetSchema\n");
		//this->MaxRelationSize = MAX_DOMAIN_ENTRIES;
	}

	// Calculate the sizes of the state
	this->StateSize = 0;
	for (unsigned int i = 1; i < this->Schema->RelationSchema.size(); i++) {
		if (this->Schema->RelationSchema[i]->IsInState) {
			// Calculate the size of the state
			if (this->DomainManager->Domain[i].IDCount > this->MaxRelationSize) {
				this->StateSize += this->MaxRelationSize * sizeof(unsigned int);
				this->StateSize += this->MaxRelationSize * sizeof(unsigned int);
				this->Lexicon->IO->FormatToLog(4, true, "    Relation %d  Size = %d\n", i, 2 * this->MaxRelationSize * sizeof(unsigned int));
			} else {
				this->StateSize += this->DomainManager->Domain[i].IDCount * sizeof(unsigned int);
				this->StateSize += this->DomainManager->Domain[i].IDCount * sizeof(bool);
				this->Lexicon->IO->FormatToLog(4, true, "    Relation %d  Size = %d\n", i, this->DomainManager->Domain[i].IDCount * sizeof(unsigned int) + this->DomainManager->Domain[i].IDCount * sizeof(bool));
			}
		}
		// Is the state too big
		this->Lexicon->IO->Parameters->StateSize = this->StateSize;
		if (this->StateSize > this->Lexicon->IO->Parameters->MaxStateSize) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: state too big in hsfcStateManager::SetSchema\n");
			return false;
		}
	}

	// Cross link goal, legal, does, sees to role
	if (this->RoleRelationIndex == 0) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: no (role ...) relation found in hsfcStateManager::SetSchema\n");
		return false;
	} else {
		RoleSize = this->DomainManager->Domain[this->RoleRelationIndex].Size[0];
	}

	// Goal to role
	if (this->GoalRelationIndex == 0) {

		this->Lexicon->IO->WriteToLog(0, false, "Error: no (goal ...) relation found in hsfcStateManager::SetSchema\n");
		return false;

	} else {

		// Create the cross link
		Size = this->DomainManager->Domain[this->GoalRelationIndex].Size[0];
		this->GoalToRole = new unsigned int[Size];

		// Populate the cross reference for each goal entry
		for (unsigned int i = 0; i < Size; i++) {
			this->GoalToRole[i] = UNDEFINED;
			GoalEntry = &this->DomainManager->Domain[this->GoalRelationIndex].Record[0][i].Relation;
			// Find a matching role entry
			for (unsigned int j = 0; j < RoleSize; j++) {
				RoleEntry = &this->DomainManager->Domain[this->RoleRelationIndex].Record[0][j].Relation;
				if (RoleEntry->Index != GoalEntry->Index) continue;
				if (RoleEntry->ID != GoalEntry->ID) continue;
				this->GoalToRole[i] = j;
			}
		}
	}

	// Legal to role
	if (this->LegalRelationIndex == 0) {

		this->Lexicon->IO->WriteToLog(0, false, "Error: no (Legal ...) relation found in hsfcStateManager::SetSchema\n");
		return false;

	} else {

		// Create the cross link
		Size = this->DomainManager->Domain[this->LegalRelationIndex].Size[0];
		this->LegalToRole = new unsigned int[Size];

		// Populate the cross reference for each Legal entry
		for (unsigned int i = 0; i < Size; i++) {
			this->LegalToRole[i] = UNDEFINED;
			LegalEntry = &this->DomainManager->Domain[this->LegalRelationIndex].Record[0][i].Relation;
			// Find a matching role entry
			for (unsigned int j = 0; j < RoleSize; j++) {
				RoleEntry = &this->DomainManager->Domain[this->RoleRelationIndex].Record[0][j].Relation;
				if (RoleEntry->Index != LegalEntry->Index) continue;
				if (RoleEntry->ID != LegalEntry->ID) continue;
				this->LegalToRole[i] = j;
			}
		}
	}

	// Does to role
	if (this->DoesRelationIndex == 0) {

		this->Lexicon->IO->WriteToLog(0, false, "Error: no (Does ...) relation found in hsfcStateManager::SetSchema\n");
		return false;

	} else {

		// Create the cross link
		Size = this->DomainManager->Domain[this->DoesRelationIndex].Size[0];
		this->DoesToRole = new unsigned int[Size];

		// Populate the cross reference for each Does entry
		for (unsigned int i = 0; i < Size; i++) {
			this->DoesToRole[i] = UNDEFINED;
			DoesEntry = &this->DomainManager->Domain[this->DoesRelationIndex].Record[0][i].Relation;
			// Find a matching role entry
			for (unsigned int j = 0; j < RoleSize; j++) {
				RoleEntry = &this->DomainManager->Domain[this->RoleRelationIndex].Record[0][j].Relation;
				if (RoleEntry->Index != DoesEntry->Index) continue;
				if (RoleEntry->ID != DoesEntry->ID) continue;
				this->DoesToRole[i] = j;
			}
		}
	}

	// Sees to role
	if (this->SeesRelationIndex == 0) {

		//this->Lexicon->IO->WriteToLog(0, false, "Error: no (Sees ...) relation found in hsfcStateManager::SetSchema\n");
		//return false;

	} else {

		// Create the cross link
		Size = this->DomainManager->Domain[this->SeesRelationIndex].Size[0];
		this->SeesToRole = new unsigned int[Size];

		// Populate the cross reference for each Sees entry
		for (unsigned int i = 0; i < Size; i++) {
			this->SeesToRole[i] = UNDEFINED;
			SeesEntry = &this->DomainManager->Domain[this->SeesRelationIndex].Record[0][i].Relation;
			// Find a matching role entry
			for (unsigned int j = 0; j < RoleSize; j++) {
				RoleEntry = &this->DomainManager->Domain[this->RoleRelationIndex].Record[0][j].Relation;
				if (RoleEntry->Index != SeesEntry->Index) continue;
				if (RoleEntry->ID != SeesEntry->ID) continue;
				this->SeesToRole[i] = j;
			}
		}
	}

	// Create Next references
	this->Lexicon->IO->WriteToLog(2, true, "  Setting (next ...) references\n");

	// Reset the list
	this->Next.clear();

	// Link all Next --> True
	for (unsigned int i = 1; i < this->Schema->RelationSchema.size(); i++) {
		// Is this a (next ...)
		if (this->Schema->RelationSchema[i]->Fact == hsfcFactNext) {
			// Create the reference
			TrueIndex = this->Lexicon->TrueFrom(this->Schema->RelationSchema[i]->Index);
			NewReference.SourceIndex = this->Schema->RelationSchema[i]->Index;
			NewReference.DestinationIndex = TrueIndex;
			this->Next.push_back(NewReference);
		}
	}

	this->Lexicon->IO->FormatToLog(3, true, "  State Size = %u\n", this->StateSize);
	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");

	return true;

}

//=============================================================================
// State Methods
//=============================================================================
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
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
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
	this->NumRelationLists = this->Schema->RelationSchema.size();

	// Create the arrays for each relation list
	State->NumRelations = new unsigned int[this->NumRelationLists];
	State->MaxNumRelations = new unsigned int[this->NumRelationLists];
	State->RelationID = new unsigned int*[this->NumRelationLists];
	State->RelationExists = new bool*[this->NumRelationLists];
	State->RelationIDSorted = new unsigned int*[this->NumRelationLists];

	// Create the arrays for each relation
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		State->NumRelations[i] = 0;
		State->MaxNumRelations[i] = 0;
		State->RelationID[i] = NULL;
		State->RelationIDSorted[i] = NULL;
		State->RelationExists[i] = NULL;
		if (this->Schema->RelationSchema[i]->IsInState) {
			if (this->DomainManager->Domain[i].IDCount < this->MaxRelationSize) {
				State->MaxNumRelations[i] = this->DomainManager->Domain[i].IDCount;
				State->RelationID[i] = new unsigned int[State->MaxNumRelations[i]];
				this->StateSize += State->MaxNumRelations[i] * sizeof(int);
				State->RelationExists[i] = new bool[State->MaxNumRelations[i]];
				this->StateSize += State->MaxNumRelations[i] * sizeof(bool);
				// Clear the exists array
				for (unsigned int j = 0; j < State->MaxNumRelations[i]; j++) {
					State->RelationExists[i][j] = false;
				}
			} else {
				State->MaxNumRelations[i] = this->MaxRelationSize;
				State->RelationID[i] = new unsigned int[this->MaxRelationSize];
				this->StateSize += this->MaxRelationSize * sizeof(int);
				State->RelationIDSorted[i] = new unsigned int[this->MaxRelationSize];
				this->StateSize += this->MaxRelationSize * sizeof(int);
			}
		}
	}

	// Add the permanent relations from the reference table
	for (unsigned int i = 0; i < this->FullPermanent.size(); i++) {
		this->AddRelation(State, this->FullPermanent[i]);
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
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		if (this->Schema->RelationSchema[i]->Rigidity != hsfcRigidityFull) {
			for (unsigned int j = 0; j < Source->NumRelations[i]; j++) {
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

	// Go through all of the relations and clear all the lists; except the permanent facts
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		if (this->Schema->RelationSchema[i]->Rigidity != hsfcRigidityFull) {
			if (State->RelationExists[i] != NULL) {
				for (unsigned int j = 0; j < State->NumRelations[i]; j++) {
					State->RelationExists[i][State->RelationID[i][j]] = false;
				}
			}
			State->NumRelations[i] = 0;
		}
	}

}

//-----------------------------------------------------------------------------
// SetInitialState
//-----------------------------------------------------------------------------
void hsfcStateManager::SetInitialState(hsfcState* State) {

	// Reset the state
	this->ResetState(State);

	// Add the (init (...)) relations from the state
	for (unsigned int i = 0; i < this->Initial.size(); i++) {
		this->AddRelation(State, this->Initial[i]);
	}

	// Add the permanent relations from the reference table
	for (unsigned int i = 0; i < this->PartPermanent.size(); i++) {
		this->AddRelation(State, this->PartPermanent[i]);
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
	hsfcTuple NewTuple;

	// (next (cell ... ...)) ==> (cell ... ...)
	// (next_cell ... ...) ==> (cell ... ...)
	// Domains for (next_cell ) and (cell ) are identical
	// So the lists can just be copied

	// Clear all of the lists except for the rigid and the next relations
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		if ((this->Schema->RelationSchema[i]->Rigidity != hsfcRigidityFull) && (this->Schema->RelationSchema[i]->Fact != hsfcFactNext)) {
			if (State->RelationExists[i] != NULL) {
				for (unsigned int j = 0; j < State->NumRelations[i]; j++) {
					State->RelationExists[i][State->RelationID[i][j]] = false;
				}
			}
			State->NumRelations[i] = 0;
		}
	}

	// Transfer the lists from next to predicate
	for (unsigned int i = 0; i < this->Next.size(); i++) {

		SourceIndex = this->Next[i].SourceIndex;
		DestinationIndex = this->Next[i].DestinationIndex;

		// Transfer of relations
		// There is an opportunity for a bulk transfer flag
		for (unsigned int j = 0; j < State->NumRelations[SourceIndex]; j++) {
			NewTuple.Index = DestinationIndex;
			NewTuple.ID = State->RelationID[SourceIndex][j];
			this->AddRelation(State, NewTuple);
		}

	}

	// Clear all of the next> lists
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		if (this->Schema->RelationSchema[i]->Fact == hsfcFactNext) {
			if (State->RelationExists[i] != NULL) {
				for (unsigned int j = 0; j < State->NumRelations[i]; j++) {
					State->RelationExists[i][State->RelationID[i][j]] = false;
				}
			}
			State->NumRelations[i] = 0;
		}
	}

	// Add in any permanent relations that are in nonpermanent lists eg. (legal role noop)
	for (unsigned int i = 0; i < this->PartPermanent.size(); i++) {
		this->AddRelation(State, this->PartPermanent[i]);
	}

	// Advance the Cycle counter
	State->Round = State->Round + 1;
	State->CurrentStep = 0;

}

//-----------------------------------------------------------------------------
// GetFluents
//-----------------------------------------------------------------------------
void hsfcStateManager::GetFluents(hsfcState* State, vector<hsfcTuple>& Fluent) {

	unsigned int Count;

	// Independent of current step

	// Go through all of the lists and count the number of fluents
	Count = 0;
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		if (this->Schema->RelationSchema[i]->Fact == hsfcFactTrue) {
			Count += State->NumRelations[i];
		}
	}

	// Resize the vector
	Fluent.resize(Count);

	// Go through all of the lists and add the fluents
	Count = 0;
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		if (this->Schema->RelationSchema[i]->Fact == hsfcFactTrue) {
			for (unsigned int j = 0; j < State->NumRelations[i]; j++) {
				Fluent[Count].Index = i;
				Fluent[Count].ID = State->RelationID[i][j];
				Count++;
			}
		}
	}


}

//-----------------------------------------------------------------------------
// AddRelation
//-----------------------------------------------------------------------------
bool hsfcStateManager::AddRelation(hsfcState* State, hsfcTuple& Tuple){

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;

	// Does the list have Exists or Sorted
	if (State->RelationIDSorted[Tuple.Index] != NULL) {

		// Uses Sorted
		// Binary search of the list; adding any unfound values in sort order
		Target = 0;
		LowerBound = 0;
		UpperBound = 0;
		Compare = 0;
		
		// Is the list empty
		if (State->NumRelations[Tuple.Index] > 0) {

			// Look for the Tuple according to its value
			UpperBound = State->NumRelations[Tuple.Index] - 1;
			while (LowerBound <= UpperBound) {

				// Find the target value and compare
				Target = (LowerBound + UpperBound) / 2;
				if (State->RelationIDSorted[Tuple.Index][Target] > Tuple.ID) {
					Compare = -1;
				} else {
					if (State->RelationIDSorted[Tuple.Index][Target] == Tuple.ID) {
						Compare = 0;
					} else {
						Compare = 1;
					}
				}

				// Compare the values
				if (Compare == 0) return false;
				if (Compare < 0) UpperBound = Target - 1;
				if (Compare > 0) LowerBound = Target + 1;
			}
		}

		// Not found
		// Last compare will have LowerBound == UpperBound == Target
		// If Compare > 0 then new value is after Target
		// If Compare < 0 then new value is before Target
		if (Compare > 0) Target++;

		if (State->NumRelations[Tuple.Index] >= State->MaxNumRelations[Tuple.Index]) {
			this->Lexicon->IO->FormatToLog(0, false, "Warning: MaxRelationSize exceeded in %s in hsfcStateManager::AddRelation\n\n", this->Lexicon->Relation(Tuple.Index));
			return false;
		}

		// Shuffle everything down
		for (int i = State->NumRelations[Tuple.Index]; i > Target; i--) {
			State->RelationIDSorted[Tuple.Index][i] = State->RelationIDSorted[Tuple.Index][i - 1];
		}
		// Add it to the list
		State->RelationID[Tuple.Index][State->NumRelations[Tuple.Index]] = Tuple.ID;
		State->RelationIDSorted[Tuple.Index][Target] = Tuple.ID;

		// Increment the number of relations in the list
		State->NumRelations[Tuple.Index]++;

		return true;

	} else {

		// Uses Exists
		if (State->RelationExists[Tuple.Index][Tuple.ID]) {
			return false;
		} else {
			// Add it to the list
			State->RelationID[Tuple.Index][State->NumRelations[Tuple.Index]] = Tuple.ID;
			State->RelationExists[Tuple.Index][Tuple.ID] = true;
			// Increment the number of relations in the list
			State->NumRelations[Tuple.Index]++;
			return true;
		}

	}

}

//-----------------------------------------------------------------------------
// RelationExists
//-----------------------------------------------------------------------------
bool hsfcStateManager::RelationExists(hsfcState* State, hsfcTuple& Tuple){

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;

	// Does the list have Exists or Sorted
	if (State->RelationIDSorted[Tuple.Index] != NULL) {

		// Uses Sorted
		// Binary search of the list; adding any unfound values in sort order
		Target = 0;
		LowerBound = 0;
		UpperBound = 0;
		Compare = 0;
		
		// Is the list empty
		if (State->NumRelations[Tuple.Index] > 0) {

			// Look for the Tuple according to its value
			UpperBound = State->NumRelations[Tuple.Index] - 1;
			while (LowerBound <= UpperBound) {

				// Find the target value and compare
				Target = (LowerBound + UpperBound) / 2;
				if (State->RelationIDSorted[Tuple.Index][Target] > Tuple.ID) {
					Compare = -1;
				} else {
					if (State->RelationIDSorted[Tuple.Index][Target] == Tuple.ID) {
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
		return State->RelationExists[Tuple.Index][Tuple.ID];

	}

}

//-----------------------------------------------------------------------------
// PrintRelations
//-----------------------------------------------------------------------------
void hsfcStateManager::PrintRelations(hsfcState* State, bool ShowRigids) {

	hsfcTuple Relation;
	char* KIF;
	hsfcRelationSchema* RelationSchema;

	this->Lexicon->IO->WriteToLog(0, false, "\n--- State ---\n");

	// Print the relations
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		if (this->Schema->RelationSchema[i]->IsInState) {

			this->Lexicon->IO->FormatToLog(0, false, "Relation %-3d - %-24s", i, this->Lexicon->Relation(i)); 
			this->Lexicon->IO->FormatToLog(0, false, "  Count %-8d Max %-8d ", State->NumRelations[i], State->MaxNumRelations[i]); 

			RelationSchema = this->Schema->RelationSchema[i];
			if (RelationSchema != NULL) {
				switch (RelationSchema->Fact) {
					case 0:
						this->Lexicon->IO->WriteToLog(0, false, "None ");
						break;
					case 1:
						this->Lexicon->IO->WriteToLog(0, false, "Emb  ");
						break;
					case 2:
						this->Lexicon->IO->WriteToLog(0, false, "Aux  ");
						break;
					case 3:
						this->Lexicon->IO->WriteToLog(0, false, "Init ");
						break;
					case 4:
						this->Lexicon->IO->WriteToLog(0, false, "True ");
						break;
					case 5:
						this->Lexicon->IO->WriteToLog(0, false, "Next ");
						break;
					default:
						this->Lexicon->IO->WriteToLog(0, false, "Err  ");
						break;
				}
				switch (RelationSchema->Rigidity) {
					case 0:
						this->Lexicon->IO->WriteToLog(0, false, "None\n");
						break;
					case 1:
						this->Lexicon->IO->WriteToLog(0, false, "Some\n");
						break;
					case 2:
						this->Lexicon->IO->WriteToLog(0, false, "Full\n");
						break;
					default:
						this->Lexicon->IO->WriteToLog(0, false, "Err \n");
						break;
				}
				if (ShowRigids || (RelationSchema->Rigidity != hsfcRigidityFull)) {
					for (unsigned int j = 0; (j < State->NumRelations[i]) && (j < 128); j++) {
						this->Lexicon->IO->FormatToLog(0, false, "%4d.%-9d", i, State->RelationID[i][j]);
						Relation.Index = i;
						Relation.ID = State->RelationID[i][j];
						KIF = NULL;
						this->DomainManager->RelationAsKIF(Relation, &KIF);
						this->Lexicon->IO->FormatToLog(0, false, "  %s\n", KIF);
						delete[] KIF;
					}
					if ((RelationSchema->Rigidity != hsfcRigidityFull) && (State->NumRelations[i] > 128)) {
						this->Lexicon->IO->FormatToLog(0, false, "Count = %u\n", State->NumRelations[i]);
					}
				}
			}

		} else {
			this->Lexicon->IO->FormatToLog(0, false, "Relation %-3d - %-24s Not stored in State\n", i, this->Lexicon->Relation(i)); 
		}
	}

	this->Lexicon->IO->WriteToLog(0, false, "--- End of State ---\n\n");

}

//-----------------------------------------------------------------------------
// CreateRigids
//-----------------------------------------------------------------------------
void hsfcStateManager::CreateRigids(hsfcState* State) {

	hsfcRelationSchema* RelationSchema;
	hsfcTuple* Term;
	hsfcFactType FactType;
	unsigned int ID;

	// Change the schema to compress the rigid IDs
	// Leave Init as is because the domains for (init: ...) (true: ...) (next: ...) are identical

	// Load relations from the state to the schema as permanents or initial
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		// Find the relation schema
		RelationSchema = this->Schema->RelationSchema[i];
		if ((RelationSchema->Rigidity == hsfcRigidityFull) && (RelationSchema->IsInState)) {
			// Is this a permanent or an initial
			//if (!this->Lexicon->PartialMatch(RelationSchema->NameID, "init:")) {
				// Create the terms array
				Term = new hsfcTuple[RelationSchema->Arity + 1];
				// Reset the schema
				FactType = RelationSchema->Fact;
				RelationSchema->Initialise(RelationSchema->NameID, RelationSchema->Arity, RelationSchema->Index);
				RelationSchema->Rigidity = hsfcRigidityFull;
				RelationSchema->IsInState = true;
				RelationSchema->Fact = FactType;
				for (int j = 0; j < RelationSchema->Arity; j++) {
					RelationSchema->DomainSchema[j]->Rigid = true;
				}
				
				// Rebuild the schema from the rigids in the state
				// Load each relation
				for (unsigned int j = 0; j < State->NumRelations[i]; j++) {
					// Get the terms from the old domain structure
					this->DomainManager->IDToTerms(i, Term, State->RelationID[i][j]); 
					// Load the terms into the new schema
					RelationSchema->AddRigidTerms(Term);
				}

				// Clean up memory
				delete[] Term;
			//}
		}
	}

	// Reduild the domain as a rigid domain
	// Load relations from the state to the schema as permanents or initial
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		// Find the relation schema
		RelationSchema = this->Schema->RelationSchema[i];
		if ((RelationSchema->Rigidity == hsfcRigidityFull) && (RelationSchema->IsInState)) {
			this->DomainManager->Domain[i].Rigid = true;
			this->DomainManager->RebuildRigidDomain(RelationSchema, i);
		}
	}

	// Print the new domains
	if (this->Lexicon->IO->Parameters->LogDetail > 2) this->DomainManager->Print();

	// Validate the new domains
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		// Find the relation schema
		RelationSchema = this->Schema->RelationSchema[i];
		if ((RelationSchema->Rigidity == hsfcRigidityFull) && (RelationSchema->IsInState)) {
			Term = new hsfcTuple[RelationSchema->Arity + 1];
			for (unsigned int j = 0; j < this->DomainManager->Domain[i].IDCount; j++) {
				this->DomainManager->IDToTerms(i, Term, j);
				this->DomainManager->TermsToID(i, Term, ID);
				if (ID != j) {
					this->Lexicon->IO->WriteToLog(0, false, "Warning: Bad ID to Terms to ID conversion::CreateRigids\n\n");
				}
			}
			delete[] Term;
		}
	}


}

//-----------------------------------------------------------------------------
// CreatePermanents
//-----------------------------------------------------------------------------
void hsfcStateManager::CreatePermanents(hsfcState* State) {

	hsfcTuple Relation;
	hsfcRelationSchema* RelationSchema;

	// This is calculated from an initiailsed state after running all/only of the rigid rules
	this->PartPermanent.clear();
	this->FullPermanent.clear();
	this->Initial.clear();

	// All the rigid relations have now got different IDs
	// But they are all fully instantiated
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		Relation.Index = i;
		// Find the relation schema
		RelationSchema = this->Schema->RelationSchema[i];
		if ((RelationSchema->Rigidity == hsfcRigidityFull) && (RelationSchema->IsInState)) {
			// Is this a permanent or an initial
			if (!this->Lexicon->PartialMatch(RelationSchema->NameID, "init:")) {
				// Add every one to the state
				for (unsigned int j = 0; j < this->DomainManager->Domain[i].IDCount; j++) {
					Relation.ID = j;
					// Add the relation instance to the schema
					this->FullPermanent.push_back(Relation);
				}
			}
		}
	}

	// Load relations from the state to the schema as permanents or initial
	for (unsigned int i = 1; i < this->NumRelationLists; i++) {
		Relation.Index = i;
		for (unsigned int j = 0; j < State->NumRelations[i]; j++) {
			// Find the relation schema
			RelationSchema = this->Schema->RelationSchema[i];
			Relation.ID = State->RelationID[i][j];
			// Is this a permanent or an initial
			if (this->Lexicon->PartialMatch(RelationSchema->NameID, "init:")) {
				// The domains for (init: ...) (true: ...) (next: ...) are identical
				Relation.Index = this->Lexicon->TrueFrom(Relation.Index);
				// Add the relation instance to the schema
				this->Initial.push_back(Relation);
			} else {
				// Is this a partial permanent, fully permanents ar ealready done
				if (RelationSchema->Rigidity != hsfcRigidityFull) {
					this->PartPermanent.push_back(Relation);
				}
			}
		}
	}

}


