//=============================================================================
// Project: High Speed Forward Chaining
// Module: Grinder
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include "hsfcGrinder.h"

#include "hsfc_config.h"

using namespace std;

//=============================================================================
// CLASS: hsfcRule
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRule::hsfcRule(hsfcLexicon* Lexicon, hsfcStateManager* StgateManager){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->StateManager = StateManager;
	this->RuleSchema = NULL;

	this->Buffer = NULL;
	this->Input = NULL;
	this->Condition = NULL;
	this->PreCondition = NULL;
	this->ConditionFunction = NULL;
	this->PreConditionFunction = NULL;
	this->ResultLookup = NULL;
	this->InputLookup = NULL;
	this->ConditionLookup = NULL;
	this->PreConditionLookup = NULL;
	this->InputRelation = NULL;
	this->ConditionRelation = NULL;
	this->PreConditionRelation = NULL;

	// Reset the counters
	NumInputs = 0;
	NumConditions = 0;
	NumPreConditions = 0;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcRule::~hsfcRule(void){

	// Free the resources
	if (this->Buffer != NULL) {
		delete[](this->Buffer);
		this->Buffer = NULL;
	}
	if (this->ResultLookup != NULL) {
		delete[](this->ResultLookup);
		this->ResultLookup = NULL;
	}

	if (this->InputLookup != NULL) {
		for (int i = 0; i < this->NumInputs; i++) {
			if (this->InputLookup[i] != NULL) {
				delete[](this->InputLookup[i]);
			}
		}
		delete[](this->Input);
		delete[](this->InputLookup);
		delete[](this->InputRelation);
	}

	if (this->ConditionLookup != NULL) {
		for (int i = 0; i < this->NumConditions; i++) {
			if (this->ConditionLookup[i] != NULL) {
				delete[](this->ConditionLookup[i]);
			}
		}
		delete[](this->Condition);
		delete[](this->ConditionFunction);
		delete[](this->ConditionLookup);
		delete[](this->ConditionRelation);
	}

	if (this->PreConditionLookup != NULL) {
		for (int i = 0; i < this->NumPreConditions; i++) {
			if (this->PreConditionLookup[i] != NULL) {
				delete[](this->PreConditionLookup[i]);
			}
		}
		delete[](this->PreCondition);
		delete[](this->PreConditionFunction);
		delete[](this->PreConditionLookup);
		delete[](this->PreConditionRelation);
	}

	
}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRule::Initialise(){

	// Reset the counters
	this->LowSpeed = false;
	this->NumInputs = 0;
	this->NumConditions = 0;
	this->NumPreConditions = 0;
	this->BufferSize = 0;
	this->ReferenceSize = 0;

	// Free the references
	if (this->Buffer != NULL) {
		delete[](this->Buffer);
		this->Buffer = NULL;
	}
	this->Result = 0;
	if (this->ResultLookup != NULL) {
		delete[](this->ResultLookup);
		this->ResultLookup = NULL;
	}
	if (this->Input != NULL) {
		for (int i = 0; i < this->NumInputs; i++) {
			if (this->InputLookup[i] != NULL) {
				delete[](this->InputLookup[i]);
				this->InputLookup[i] = NULL;
			}
		}
		delete[](this->Input);
		this->Input = NULL;
		delete[](this->InputLookup);
		this->InputLookup = NULL;
		delete[](this->InputRelation);
		this->InputRelation = NULL;
	}
	if (this->Condition != NULL) {
		for (int i = 0; i < this->NumConditions; i++) {
			if (this->ConditionLookup[i] != NULL) {
				delete[](this->ConditionLookup[i]);
				this->ConditionLookup[i] = NULL;
			}
		}
		delete[](this->Condition);
		this->Condition = NULL;
		delete[](this->ConditionFunction);
		this->ConditionFunction = NULL;
		delete[](this->ConditionLookup);
		this->ConditionLookup = NULL;
		delete[](this->ConditionRelation);
		this->ConditionRelation = NULL;
	}
	if (this->PreCondition != NULL) {
		for (int i = 0; i < this->NumPreConditions; i++) {
			if (this->PreConditionLookup[i] != NULL) {
				delete[](this->PreConditionLookup[i]);
				this->PreConditionLookup[i] = NULL;
			}
		}
		delete[](this->PreCondition);
		this->PreCondition = NULL;
		delete[](this->PreConditionFunction);
		this->PreConditionFunction = NULL;
		delete[](this->PreConditionLookup);
		this->PreConditionLookup = NULL;
		delete[](this->PreConditionRelation);
		this->PreConditionRelation = NULL;
	}

}

//-----------------------------------------------------------------------------
// FromSchema
//-----------------------------------------------------------------------------
void hsfcRule::FromSchema(hsfcRuleSchema* RuleSchema, bool LowSpeed) {

	int Index;

	// Initialise the Rule
	this->Initialise();
	this->LowSpeed = LowSpeed;
	this->RuleSchema = RuleSchema;

	// Count the inputs, conditions, preconditions
	this->NumInputs = 0;
	this->NumConditions = 0;
	this->NumPreConditions = 0;
	for (unsigned int i = 0; i < RuleSchema->Relation.size(); i++) {
		if (RuleSchema->Relation[i]->Type == hsfcRuleInput) this->NumInputs++;
		if (RuleSchema->Relation[i]->Type == hsfcRuleCondition) this->NumConditions++;
		if (RuleSchema->Relation[i]->Type == hsfcRulePreCondition) this->NumPreConditions++;
	}

	// Set up the result linkages
	this->Result = RuleSchema->Relation[0]->Template[0].RelationSchema->Index;
	this->ResultRelation = RuleSchema->Relation[0];

	// Set up the Input linkages
	if (this->NumInputs > 0) {
		// Create and populate the the arrays
		this->Input = new int[this->NumInputs];
		this->Cursor = new int[this->NumInputs];
		this->InputRelation = new hsfcRuleRelationSchema*[this->NumInputs];
		Index = 0;
		// Link to the lists in the state
		for (unsigned int i = 0; i < RuleSchema->Relation.size(); i++) {
			if (RuleSchema->Relation[i]->Type == hsfcRuleInput) {
				this->Input[Index] = RuleSchema->Relation[i]->Template[0].RelationSchema->Index;
				this->InputRelation[Index] = RuleSchema->Relation[i];
				Index++;
			}
		}
	}

	// Set up the Condition linkages
	if (this->NumConditions > 0) {
		// Create and populate the the arrays
		this->Condition = new int[this->NumConditions];
		this->ConditionFunction = new int[this->NumConditions];
		this->ConditionRelation = new hsfcRuleRelationSchema*[this->NumConditions];
		Index = 0;
		// Link to the lists in the state
		for (unsigned int i = 0; i < RuleSchema->Relation.size(); i++) {
			if (RuleSchema->Relation[i]->Type == hsfcRuleCondition) {
				if ((RuleSchema->Relation[i]->Function & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
					this->Condition[Index] = -1;
					this->ConditionRelation[Index] = RuleSchema->Relation[i];
				} else {
					this->Condition[Index] = RuleSchema->Relation[i]->Template[0].RelationSchema->Index;
					this->ConditionRelation[Index] = RuleSchema->Relation[i];
				}
				this->ConditionFunction[Index] = RuleSchema->Relation[i]->Function;
				Index++;
			}
		}
	}

	// Set up the PreCondition linkages
	if (this->NumPreConditions > 0) {
		// Create and populate the the arrays
		this->PreCondition = new int[this->NumPreConditions];
		this->PreConditionFunction = new int[this->NumPreConditions];
		this->PreConditionRelation = new hsfcRuleRelationSchema*[this->NumPreConditions];
		Index = 0;
		// Link to the lists in the state
		for (unsigned int i = 0; i < RuleSchema->Relation.size(); i++) {
			if (RuleSchema->Relation[i]->Type == hsfcRulePreCondition) {
				if ((RuleSchema->Relation[i]->Function & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
					this->PreCondition[Index] = -1;
					this->PreConditionRelation[Index] = RuleSchema->Relation[i];
				} else {
					this->PreCondition[Index] = RuleSchema->Relation[i]->Template[0].RelationSchema->Index;
					this->PreConditionRelation[Index] = RuleSchema->Relation[i];
				}
				this->PreConditionFunction[Index] = RuleSchema->Relation[i]->Function;
				Index++;
			}
		}
	}

	// Set up the buffer for assembling the terms 
	this->Buffer = new hsfcBufferEntry[RuleSchema->Buffer.size()];
	for (unsigned int i = 0; i < RuleSchema->Buffer.size(); i++) {
		this->Buffer[i].ID = RuleSchema->Buffer[i].ID;
		this->Buffer[i].RelationIndex = RuleSchema->Buffer[i].RelationIndex;
	}
	this->BufferSize = RuleSchema->Buffer.size();

}

//-----------------------------------------------------------------------------
// Execute
//-----------------------------------------------------------------------------
void hsfcRule::Execute(hsfcState* State, bool Audit) {

	vector<hsfcTuple> Term;
	hsfcTuple Tuple;
	int InputIndex;
	bool LoadFailed;
	int LowInputIndex;

	// Check the preconditions
	if (!this->CheckPreConditions(State, Audit)) {
		return;
	}

	// This may be a fully ground rule with no inputs; so we do it at least once
	// Initialise all of the input cursors; no inputs return true; empty input list return false
	if (!this->InitialiseInput(State)) {
		if (Audit) printf(" x0"); 
		return;
	}

	// Load all the permutations
	LowInputIndex = 0;
	do {

		// Load the buffer with a valid tuple and test if it is valid
		this->ClearBuffer(LowInputIndex);
		// Load each input in turn
		LoadFailed = false;
		for (InputIndex = LowInputIndex; InputIndex < this->NumInputs; InputIndex++) {

			// Count the transactions performed
			this->Transactions++;

			// Load the buffer
			if (!this->LoadBuffer(InputIndex, State, Audit)) {
				if (Audit) printf(" xi%d", InputIndex); 
				LoadFailed = true;
				break;
			}

		}

		// InputIndex points to the Input that needs to be inrecemtned 
		// for the next permutation
		// If a load fails early, InputIndex shows where
		if (LoadFailed) continue;
		InputIndex = this->NumInputs - 1;

		// Check the conditions
		if (this->CheckConditions(State, Audit)) {

			// Count the transactions performed
			this->Transactions++;

			// Clear the Terms
			Term.clear();

			// Read the terms for the result
			for (unsigned int i = 0; i < this->ResultRelation->Template.size(); i++) {
				if (this->ResultRelation->Template[i].Fixed) {
					Term.push_back(this->ResultRelation->Template[i].Tuple);
				} else {
					Tuple.RelationIndex = this->Buffer[this->ResultRelation->Template[i].BufferIndex].RelationIndex;
					Tuple.ID = this->Buffer[this->ResultRelation->Template[i].BufferIndex].ID;
					Term.push_back(Tuple);
				}
			}

			// Convert the terms to a tuple
			Tuple.RelationIndex = this->Result;
			Tuple.ID = this->ResultRelation->ID(Term, false);
			if (Tuple.ID == -1) {
				if (Audit) printf(" xr"); 
			} else {
				// Record the result 
				if (Audit) printf(" %d", Tuple.ID); 
				this->StateManager->AddRelation(State, &Tuple);
			}

			// Is the relation full
			if (State->NumRelations[this->Result] == State->MaxNumRelations[this->Result]) break;
		}

	// Advance the inputs
	} while (this->AdvanceInput(&LowInputIndex, InputIndex, State));

}

//-----------------------------------------------------------------------------
// SetSpeed
//-----------------------------------------------------------------------------
void hsfcRule::SetSpeed(double MaxRefernceSize, double MaxRelationSize) {

	hsfcRelationSchema* RelationSchema;

	// Check the lookup size
	if (this->RuleSchema->EstReferenceSize > MaxRefernceSize) {
		this->LowSpeed = true;
	}

	// Check every relation
	for (unsigned int i = 0; i < this->RuleSchema->Relation.size(); i++) {
		RelationSchema = this->RuleSchema->Relation[i]->Template[0].RelationSchema;
		if (RelationSchema != NULL) {
			if (RelationSchema->IDCountDbl > MaxRelationSize) {
				this->LowSpeed = true;
				break;
			}
		}
	}

}

//-----------------------------------------------------------------------------
// Grind
//-----------------------------------------------------------------------------
void hsfcRule::Grind() {

	vector<hsfcTuple> Term;
	vector<hsfcRuleTerm> RuleTerm;
	int InputIndex;
	int iii;
	int ConditionIndex;
	int PreConditionIndex;
	int ID;
	int LookupValue[MAX_NO_OF_INPUTS];
	int LookupIndex[MAX_NO_OF_INPUTS];
	int* NextLookupValue;
	bool PreviousFailed;
	int Count;
	int Lowiii;

	// So far the calculations have only been approximations
	// We must do the inputs in sequence to get the exact table sizes
	// InputReferenceSize = PrevNoUniqueIDs * InputRelationSize

	// Schema Relations are now sorted to their final ordering
	// Dimension the lookup array for inputs and set values to -1 = Fail
	InputIndex = 0;
	ConditionIndex = 0;
	PreConditionIndex = 0;
	this->ReferenceSize = 0;

	// Walk through the Schema rule relations
	for (unsigned int i = 0; i < this->RuleSchema->Relation.size(); i++) {

		// Is it a precondition
		if (this->RuleSchema->Relation[i]->Type == hsfcRulePreCondition) {

			// Record the reference for this input
			this->PreConditionFunction[PreConditionIndex] = this->RuleSchema->Relation[i]->Function;
			this->PreConditionRelation[PreConditionIndex] = this->RuleSchema->Relation[i];
			if (this->RuleSchema->Relation[i]->Template[0].RelationSchema == NULL) {
				this->PreCondition[PreConditionIndex] = -1;
			} else {
				this->PreCondition[PreConditionIndex] = this->RuleSchema->Relation[i]->Template[0].RelationSchema->Index;
			}
			PreConditionIndex++;
		}

		// Is it an input
		if (this->RuleSchema->Relation[i]->Type == hsfcRuleInput) {

			// Record the reference for this input
			this->InputRelation[InputIndex] = this->RuleSchema->Relation[i];
			this->Input[InputIndex] = this->RuleSchema->Relation[i]->Template[0].RelationSchema->Index;
			InputIndex++;
		}

		// Is it a condition
		if (this->RuleSchema->Relation[i]->Type == hsfcRuleCondition) {

			// Record the reference for this input
			this->ConditionFunction[ConditionIndex] = this->RuleSchema->Relation[i]->Function;
			this->ConditionRelation[ConditionIndex] = this->RuleSchema->Relation[i];
			if (this->RuleSchema->Relation[i]->Template[0].RelationSchema == NULL) {
				this->Condition[ConditionIndex] = -1;
			} else {
				this->Condition[ConditionIndex] = this->RuleSchema->Relation[i]->Template[0].RelationSchema->Index;
			}
			ConditionIndex++;
		}

	}

	// Dimension the maximum lookup values
	this->MaxInputLookup = new int[this->NumInputs];
	for (int i = 0; i < this->NumInputs; i++) this->MaxInputLookup[i] = 0;

	this->MaxConditionLookup = new int[this->NumConditions];		
	for (int i = 0; i < this->NumConditions; i++) this->MaxConditionLookup[i] = 0;

	this->MaxPreConditionLookup = new int[this->NumPreConditions];
	for (int i = 0; i < this->NumPreConditions; i++) this->MaxPreConditionLookup[i] = 0;


	// Dimension the lookup arrays
	this->InputLookup = new int*[this->NumInputs];
	this->ConditionLookup = new int*[this->NumConditions];		
	this->PreConditionLookup = new int*[this->NumPreConditions];

	//--- PreConditions ---------------------------------------------------------------------------

	PreConditionIndex = 0;
	for (unsigned int i = 0; i < this->RuleSchema->Relation.size(); i++) {

		// Is it a precondition
		if (this->RuleSchema->Relation[i]->Type == hsfcRulePreCondition) {

			// Set up the array
			this->PreConditionRelation[PreConditionIndex]->ReferenceSize = 1;
			this->ReferenceSize += 1;

			this->PreConditionLookup[PreConditionIndex] = new int[1];

			// Collect all of the terms in the relation
			Term.clear();
			for (unsigned int j = 0; j < this->RuleSchema->Relation[i]->Template.size(); j++) {
				Term.push_back(this->RuleSchema->Relation[i]->Template[j].Tuple);
			}

			// Get the relation ID
			ID = this->RuleSchema->Relation[i]->ID(Term, true);
			//if (ID == -1) {
			//	printf("Error: Bad ID\n");
			//	abort();
			//}

			// Record the references for this precondition and lookup values
			this->PreConditionLookup[PreConditionIndex][0] = ID;
			//printf("    %5d: %5d\n", 0, ID);

			if (DEBUG) printf("    PreCondition %d   Size = 1   Unique = 1\n", PreConditionIndex);
			PreConditionIndex++;

		}
	}

	// Some rules have no inputs, just preconditions
	// For these rules the result is already ground
	if (this->NumInputs == 0) {

		// Create the reference table
		this->ResultRelation->ReferenceSize = 1;
		this->ReferenceSize += 1;

		this->ResultLookup = new int[this->ResultRelation->ReferenceSize];

		// Get the result
		this->ResultLookup[0] = this->CheckResult();
		//printf("    %5d: %5d\n", 0, this->ResultLookup[0]);
		if (DEBUG) printf("    Result   Size = %d   Count = %d\n", this->ResultRelation->ReferenceSize, 1);

		return;

	}

	//--- Inputs ---------------------------------------------------------------------------

	// Construct the Reference Tables for the inputs itteratively
	// Enumerate Input0 and construct table
	// Enumerate Input1 and load all permutations below and construct table
	// etc..

	// Zero the lookup counters
	NextLookupValue = new int[this->NumInputs];
	this->InputCount = new int[this->NumInputs];
	for (int i = 0; i < this->NumInputs; i++) {
		NextLookupValue[i] = 0;
		this->InputCount[i] = this->InputRelation[i]->Template[0].RelationSchema->IDCount;
	}

	// Process the inputs
	for (InputIndex = 0; InputIndex < this->NumInputs; InputIndex++) {

		// Calculate the reference size
		if (InputIndex == 0) {
			this->InputRelation[InputIndex]->ReferenceSize = this->InputRelation[InputIndex]->Template[0].RelationSchema->IDCount;
		} else {
			this->InputRelation[InputIndex]->ReferenceSize = (this->MaxInputLookup[InputIndex-1] + 1) * this->InputRelation[InputIndex]->Template[0].RelationSchema->IDCount;
		}
		this->ReferenceSize += this->InputRelation[InputIndex]->ReferenceSize;

		this->InputLookup[InputIndex] = new int[this->InputRelation[InputIndex]->ReferenceSize];
		// Initialise to -1 for fail
		for (int j = 0; j < this->InputRelation[InputIndex]->ReferenceSize; j++) {
			this->InputLookup[InputIndex][j] = -1;
		}

		// Reset the cursors
		for (int i = 0; i < this->NumInputs; i++) {
			this->Cursor[i] = 0;
		}

		// Load all the permutations
		Lowiii = 0;
		do {

			// Clear each of the terms in the buffer
			this->ClearBuffer(Lowiii);

			// Process each if the inputs into the buffer
			for (iii = Lowiii; iii <= InputIndex; iii++) {

				// Construct the lookup index
				if (iii == 0) {
					LookupIndex[iii] = this->Cursor[iii];
					LookupValue[iii] = this->InputLookup[iii][LookupIndex[iii]];
				} else {
					LookupIndex[iii] = LookupValue[iii - 1] + (this->MaxInputLookup[iii-1] + 1) * this->Cursor[iii];
					LookupValue[iii] = this->InputLookup[iii][LookupIndex[iii]];
				}

				// Is the lookup value valid; remember LookupValue(InputIndex) == -1 by default
				if ((iii < InputIndex) && (LookupValue[iii] == -1)) break;

				// Load the buffer 
				if (this->LoadBuffer(iii)) {
					// Record the results in the lookup for the highest input only
					if (iii == InputIndex) {
						LookupValue[iii] = NextLookupValue[InputIndex];
						this->InputLookup[InputIndex][LookupIndex[iii]] = LookupValue[iii];
						this->MaxInputLookup[InputIndex] = LookupValue[iii]; 
						NextLookupValue[InputIndex]++;
					}
				} else {
					// If the load fails then the lookup is -1
					LookupValue[iii] = -1;
					break;
				}

			}

			// Advance the inputs
			if (iii > InputIndex) iii = InputIndex;
		} while (this->AdvanceInput(&Lowiii, iii));

		// Print the reference table
		//for (int i = 0; i < this->InputRelation[InputIndex]->ReferenceSize; i++) 
			//printf("    %5d: %5d\n", i, this->InputLookup[InputIndex][i]);
		if (DEBUG) printf("    Input %d   Size = %d   Unique = %d\n", InputIndex, this->InputRelation[InputIndex]->ReferenceSize, this->MaxInputLookup[InputIndex] + 1);

	}

	//--- Conditions & Result ----------------------------------------------------------------

	// At this point we know the size of the condition and result references

	// Improvement:
	// It is possible to back-propogate from condition(0) >> input(last) if condition is fact
	// It is also possible to back propogate from input(i) >>  input(i-1)

	// Create the condition references
	for (int i = 0; i < this->NumConditions; i++) {

		// Set the reference size
		this->ConditionRelation[i]->ReferenceSize = this->MaxInputLookup[this->NumInputs-1] + 1;
		this->ReferenceSize += this->ConditionRelation[i]->ReferenceSize;

		this->ConditionLookup[i] = new int[this->ConditionRelation[i]->ReferenceSize];
		// Initialise to -1 for fail
		for (int j = 0; j < this->ConditionRelation[i]->ReferenceSize; j++) {
			this->ConditionLookup[i][j] = -1;
		}

	}

	// Create the result references

	// Set the reference size
	this->ResultRelation->ReferenceSize = this->MaxInputLookup[this->NumInputs-1] + 1;
		this->ReferenceSize += this->ResultRelation->ReferenceSize;

		this->ResultLookup = new int[this->ResultRelation->ReferenceSize];
	// Initialise to -1 for fail
	for (int j = 0; j < this->ResultRelation->ReferenceSize; j++) {
		this->ResultLookup[j] = -1;
	}

	// Walk through all of the inputs (again) setting the conditiona and the result

	// Reset the cursors
	for (int i = 0; i < this->NumInputs; i++) {
		this->Cursor[i] = 0;
	}

	// Load all the permutations
	Lowiii = 0;
	do {

		// Clear each of the terms in the buffer
		this->ClearBuffer(Lowiii);

		// Process each if the inputs into the buffer
		for (iii = Lowiii; iii < this->NumInputs; iii++) {

			// Construct the lookup index
			if (iii == 0) {
				LookupIndex[iii] = this->Cursor[iii];
				LookupValue[iii] = this->InputLookup[iii][LookupIndex[iii]];
			} else {
				LookupIndex[iii] = LookupValue[iii - 1] + (this->MaxInputLookup[iii-1] + 1) * this->Cursor[iii];
				LookupValue[iii] = this->InputLookup[iii][LookupIndex[iii]];
			}

			// Is the lookup value valid
			if (LookupValue[iii] == -1) break;

			// If the load fails then the lookup is -1
			if (!this->LoadBuffer(iii)) {
				LookupValue[iii] = -1;
				break;
			}
		}

		// Did the last full permutation pass or was it a failure
		if ((iii == this->NumInputs) && (LookupValue[iii - 1] != -1)) {

			// The lookup index is the value from the last input
			LookupIndex[iii] = LookupValue[iii - 1];

			// Rules for condition lookups
			// Distinct						-1 = fail	0 = pass
			// Not Distinct = !Distinct		-1 = pass	0 = fail
			// True							-1 = fail	0+ = lookup
			// Not True	= !True				-1 = pass	0+ = !lookup

			// Construct the condition lookups
			PreviousFailed = false;
			for (int i = 0; i < this->NumConditions; i++) {
				// Did the previous test fail
				if (PreviousFailed) {
					this->ConditionLookup[i][LookupIndex[iii]] = -1;
				} else {
					// Test the condition to see if its in the negative
					if ((this->ConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
						this->ConditionLookup[i][LookupIndex[iii]] = this->CheckCondition(i);
						// A value of -1 is an automatic pass
					} else {
						this->ConditionLookup[i][LookupIndex[iii]] = this->CheckCondition(i);
						if (this->ConditionLookup[i][LookupIndex[iii]] == -1) PreviousFailed = true;
					}
				}
			}

			// Construct the result lookup
			this->ResultLookup[LookupIndex[iii]] = this->CheckResult();

		}

		// Advance the inputs
		if (iii == this->NumInputs) iii = this->NumInputs - 1;
	} while (this->AdvanceInput(&Lowiii, iii));

	// Print the remaining references
	for (int i = 0; i < this->NumConditions; i++) {
		Count = 0;
		for (int j = 0; j < this->ConditionRelation[i]->ReferenceSize; j++) {
			if (this->ConditionLookup[i][j] >= 0) Count++;
			//printf("    %5d: %5d\n", j, this->ConditionLookup[i][j]);
		}
		if (DEBUG) printf("    Condition %d   Size = %d   Count = %d\n", i, this->ConditionRelation[i]->ReferenceSize, Count);
	}
	Count = 0;
	for (int j = 0; j < this->ResultRelation->ReferenceSize; j++) {
		if (this->ResultLookup[j] >= 0) Count++;
		//printf("    %5d: %5d\n", j, this->ResultLookup[j]);
	}
	if (DEBUG) printf("    Result   Size = %d   Count = %d\n", this->ResultRelation->ReferenceSize, Count);

	// Cleanup
	delete[] NextLookupValue;

}

//-----------------------------------------------------------------------------
// HighSpeedExecute
//-----------------------------------------------------------------------------
void hsfcRule::HighSpeedExecute(hsfcState* State){

	int Index;
	int Lookup[MAX_NO_OF_INPUTS];
	int ID;
	int InputNo;
	int LowInputIndex;
	//int MaxCursor[MAX_NO_OF_INPUTS];

	// Check each PreCondition
	for (int i = 0; i < this->NumPreConditions; i++) {
		// Get the PreCondition RelationID
		ID = this->PreConditionLookup[i][0];
		// Look up the PreCondition based on the function
		if ((this->PreConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
			// Apply the condition in the negative
			if ((this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
				if (ID == 0) return;
			} else {
				if ((ID != -1) && (State->RelationExists[this->PreCondition[i]][ID])) return;
			}
		} else {
			// Apply the condition
			if ((this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
				if (ID == -1) return;
			} else {
				if ((ID == -1) || (!State->RelationExists[this->PreCondition[i]][ID])) return;
			}
		}
		//if (this->PreConditionFunction[i] == 2) {
		//	if ((ID != -1) && (State->RelationExists[this->PreCondition[i]][ID])) return;
		//}
		//if (this->PreConditionFunction[i] == 3) {
		//	if ((ID == -1) || (!State->RelationExists[this->PreCondition[i]][ID])) return;
		//}
	}

	// Set up the cursors on the Input lists
	for (int i = 0; i < this->NumInputs; i++) {
		// Initialise each list
		this->Cursor[i] = 0;
		//MaxCursor[i] = State->NumRelations[this->Input[i]] - 1; 
		// If any lists are empty then exit
		if (State->NumRelations[this->Input[i]] - 1 < 0) return;
	}
	LowInputIndex = 0;

	// Do until all inputs have been processed
	while (true) {

		// Keep track of the lowest input to fail
		InputNo = this->NumInputs - 1;

		// Look up the reference tables
		Index = 0;
		for (int i = LowInputIndex; i < this->NumInputs; i++) {
			// Offset the index based on the current RelationID
			if (i == 0) {
				Index = State->RelationID[this->Input[i]][this->Cursor[i]];
			} else {
				Index = Lookup[i-1] + (this->MaxInputLookup[i-1] + 1) * State->RelationID[this->Input[i]][this->Cursor[i]];
			}
			Lookup[i] = this->InputLookup[i][Index];
			if (Lookup[i] == -1) {
				InputNo = i;
				goto NextCombination;
			}
			// The reference becomes the index for the next input
			Index = Lookup[i];
		}

		// Now we have the final index
		// Check each condition
		for (int i = 0; i < this->NumConditions; i++) {
			// Get the condition RelationID
			ID = this->ConditionLookup[i][Index];
			// Look up the condition based on the function
			if ((this->ConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
				// Apply the condition in the negative
				if ((this->ConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
					if (ID == 0) goto NextCombination;
				} else {
					if ((ID != -1) && (State->RelationExists[this->Condition[i]][ID])) goto NextCombination;
				}
			} else {
				// Apply the condition
				if ((this->ConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
					if (ID == -1) goto NextCombination;
				} else {
					if ((ID == -1) || (!State->RelationExists[this->Condition[i]][ID])) goto NextCombination;
				}
			}
		}

		// We have satisfied all the conditions
		// Get the result ID and add it to the list
		ID = this->ResultLookup[Index];
		if (ID != -1) {
			//Does the relation already exist on the list
			if (!State->RelationExists[this->Result][ID]) {
				// Add the relation to the list
				State->RelationID[this->Result][State->NumRelations[this->Result]] = ID;
				State->RelationExists[this->Result][ID] = true;
				State->NumRelations[this->Result]++;
				if (State->NumRelations[this->Result] == State->MaxNumRelations[this->Result]) return;
			}
		}

NextCombination:
		// Increment the cursors
		for (LowInputIndex = InputNo; LowInputIndex >= 0; LowInputIndex--) {

			// Is the cursor at the end of the list
			//if (this->Cursor[LowInputIndex] == MaxCursor[LowInputIndex]) {
			if (this->Cursor[LowInputIndex] == State->NumRelations[this->Input[LowInputIndex]] - 1) {
				// Reset the cursor to the beginning and advance the next cursor
				this->Cursor[LowInputIndex] = 0;
			} else {
				// Advance this cursor only
				(this->Cursor[LowInputIndex])++;
				break;
			}
		}

		// Are we finished
		if (LowInputIndex < 0) return;

	}

}

//-----------------------------------------------------------------------------
// HighSpeedAudit
//-----------------------------------------------------------------------------
void hsfcRule::HighSpeedAudit(hsfcState* State){

	int Index;
	int Lookup[MAX_NO_OF_INPUTS];
	int ID;
	int InputNo;
	int LowInputIndex;
	//int MaxCursor[MAX_NO_OF_INPUTS];

	// Check each PreCondition
	for (int i = 0; i < this->NumPreConditions; i++) {
		// Get the PreCondition RelationID
		ID = this->PreConditionLookup[i][0];
		// Look up the PreCondition based on the function
		if ((this->PreConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
			// Apply the condition in the negative
			if ((this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
				if (ID == 0) {
					printf(" xp%d", i); 
					return;
				}
			} else {
				if ((ID != -1) && (State->RelationExists[this->PreCondition[i]][ID])) {
					printf(" xp%d", i); 
					return;
				}
			}
		} else {
			// Apply the condition
			if ((this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
				if (ID == -1) {
					printf(" xp%d", i); 
					return;
				}
			} else {
				if ((ID == -1) || (!State->RelationExists[this->PreCondition[i]][ID])) {
					printf(" xp%d", i); 
					return;
				}
			}
		}
	}

	// Set up the cursors on the Input lists
	for (int i = 0; i < this->NumInputs; i++) {
		// Initialise each list
		this->Cursor[i] = 0;
		//MaxCursor[i] = State->NumRelations[this->Input[i]] - 1; 
		// If any lists are empty then exit
		if (State->NumRelations[this->Input[i]] - 1 < 0) {
			printf(" x0"); 
			return;
		}
	}
	LowInputIndex = 0;

	// Do until all inputs have been processed
	while (true) {

		// Keep track of the lowest input to fail
		InputNo = this->NumInputs - 1;

		// Look up the reference tables
		Index = 0;
		for (int i = LowInputIndex; i < this->NumInputs; i++) {
			// Offset the index based on the current RelationID
			if (i == 0) {
				Index = State->RelationID[this->Input[i]][this->Cursor[i]];
			} else {
				Index = Lookup[i-1] + (this->MaxInputLookup[i-1] + 1) * State->RelationID[this->Input[i]][this->Cursor[i]];
			}
			Lookup[i] = this->InputLookup[i][Index];
			if (Lookup[i] == -1) {
				InputNo = i;
				printf(" xi%d", i); 
				goto NextCombination;
			}
			// The reference becomes the index for the next input
			Index = Lookup[i];
		}

		// Now we have the final index
		// Check each condition
		for (int i = 0; i < this->NumConditions; i++) {
			// Get the condition RelationID
			ID = this->ConditionLookup[i][Index];
			// Look up the condition based on the function
			if ((this->ConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
				// Apply the condition in the negative
				if ((this->ConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
					if (ID == 0) {
						printf(" xc%d", i); 
						goto NextCombination;
					}
				} else {
					if ((ID != -1) && (State->RelationExists[this->Condition[i]][ID])) {
						printf(" xc%d", i); 
						goto NextCombination;
					}
				}
			} else {
				// Apply the condition
				if ((this->ConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
					if (ID == -1) {
						printf(" xc%d", i); 
						goto NextCombination;
					}
				} else {
					if ((ID == -1) || (!State->RelationExists[this->Condition[i]][ID])) {
						printf(" xc%d", i); 
						goto NextCombination;
					}
				}
			}
		}

		// We have satisfied all the conditions
		// Get the result ID and add it to the list
		ID = this->ResultLookup[Index];
		if (ID != -1) {
			printf(" %d", ID);
			//Does the relation already exist on the list
			if (!State->RelationExists[this->Result][ID]) {
				// Add the relation to the list
				State->RelationID[this->Result][State->NumRelations[this->Result]] = ID;
				State->RelationExists[this->Result][ID] = true;
				State->NumRelations[this->Result]++;
				if (State->NumRelations[this->Result] == State->MaxNumRelations[this->Result]) return;
			}
		}

NextCombination:
		// Increment the cursors
		for (LowInputIndex = InputNo; LowInputIndex >= 0; LowInputIndex--) {

			// Is the cursor at the end of the list
			//if (this->Cursor[LowInputIndex] == MaxCursor[LowInputIndex]) {
			if (this->Cursor[LowInputIndex] == State->NumRelations[this->Input[LowInputIndex]] - 1) {
				// Reset the cursor to the beginning and advance the next cursor
				this->Cursor[LowInputIndex] = 0;
			} else {
				// Advance this cursor only
				(this->Cursor[LowInputIndex])++;
				break;
			}
		}

		// Are we finished
		if (LowInputIndex < 0) return;

	}

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRule::Print(){

	int Index;


	Index = 0;
	printf("\n%5d <= %s", this->Result, this->Lexicon->Text(this->ResultRelation->PredicateIndex));
	for (unsigned int j = 0; j < this->RuleSchema->Relation[Index]->Template.size(); j++) {
		printf(" %d:", this->RuleSchema->Relation[Index]->Template[j].PredicateIndex);
		if (this->RuleSchema->Relation[Index]->Template[j].Fixed) {
			printf("%d.%d", this->RuleSchema->Relation[Index]->Template[j].Tuple.RelationIndex, this->RuleSchema->Relation[Index]->Template[j].Tuple.ID);
		} else {
			printf("?%d", this->RuleSchema->Relation[Index]->Template[j].BufferIndex);
		}
	}
	printf("\n");
	Index++;

	for (int i = 0; i < this->NumPreConditions; i++) {
		printf("%5d ??", this->PreCondition[i]);
		if ((this->PreConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) printf(" not");
		if ((this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) printf(" distinct");
		printf(" %s", this->Lexicon->Text(this->RuleSchema->Relation[Index]->PredicateIndex));
		for (unsigned int j = 0; j < this->RuleSchema->Relation[Index]->Template.size(); j++) {
			printf(" %d:", this->RuleSchema->Relation[Index]->Template[j].PredicateIndex);
			if (this->RuleSchema->Relation[Index]->Template[j].Fixed) {
				printf("%d.%d", this->RuleSchema->Relation[Index]->Template[j].Tuple.RelationIndex, this->RuleSchema->Relation[Index]->Template[j].Tuple.ID);
			} else {
				printf("?%d", this->RuleSchema->Relation[Index]->Template[j].BufferIndex);
			}
		}
		printf("\n");
		Index++;
	}

	for (int i = 0; i < this->NumInputs; i++) {
		printf("%5d =>", this->Input[i]);
		printf(" %s", this->Lexicon->Text(this->RuleSchema->Relation[Index]->PredicateIndex));
		for (unsigned int j = 0; j < this->RuleSchema->Relation[Index]->Template.size(); j++) {
			printf(" %d:", this->RuleSchema->Relation[Index]->Template[j].PredicateIndex);
			if (this->RuleSchema->Relation[Index]->Template[j].Fixed) {
				printf("%d.%d", this->RuleSchema->Relation[Index]->Template[j].Tuple.RelationIndex, this->RuleSchema->Relation[Index]->Template[j].Tuple.ID);
			} else {
				printf("?%d", this->RuleSchema->Relation[Index]->Template[j].BufferIndex);
			}
		}
		printf("\n");
		Index++;
	}

	for (int i = 0; i < this->NumConditions; i++) {
		printf("%5d ??", this->Condition[i]);
		if ((this->ConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) printf(" not");
		printf(" %s", this->Lexicon->Text(this->RuleSchema->Relation[Index]->PredicateIndex));
		for (unsigned int j = 0; j < this->RuleSchema->Relation[Index]->Template.size(); j++) {
			printf(" %d:", this->RuleSchema->Relation[Index]->Template[j].PredicateIndex);
			if (this->RuleSchema->Relation[Index]->Template[j].Fixed) {
				printf("%d.%d", this->RuleSchema->Relation[Index]->Template[j].Tuple.RelationIndex, this->RuleSchema->Relation[Index]->Template[j].Tuple.ID);
			} else {
				printf("?%d", this->RuleSchema->Relation[Index]->Template[j].BufferIndex);
			}
		}
		printf("\n");
		Index++;
	}
	if (this->LowSpeed) printf("      LowSpeedExecution = NoReferenceTables\n");

}

//-----------------------------------------------------------------------------
// PrintBuffer
//-----------------------------------------------------------------------------
void hsfcRule::PrintBuffer(){

	for (int i = 0; i < this->BufferSize; i++) 
		printf("%4d:%d.%d", i, this->Buffer[i].RelationIndex, this->Buffer[i].ID);
	printf("\n");
		
}

//-----------------------------------------------------------------------------
// ClearBuffer
//-----------------------------------------------------------------------------
void hsfcRule::ClearBuffer(){

	// Clear each of the Terms in the buffer
	for (int i = 0; i < this->BufferSize; i++) {
		this->Buffer[i].RelationIndex = -1;
		this->Buffer[i].ID = -1;
	}

}

//--- Overload ----------------------------------------------------------------
void hsfcRule::ClearBuffer(int LowInputIndex){

	// Make sure everything is clear
	if (LowInputIndex == 0) {
		for (int i = 0; i < this->BufferSize; i++) {
			this->Buffer[i].RelationIndex = -1;
			this->Buffer[i].ID = -1;
			this->Buffer[i].InputIndex = -1;
		}
		return;
	}
	
	// Clear each of the Terms in the buffer
	for (int i = 0; i < this->BufferSize; i++) {
		if (this->Buffer[i].InputIndex >= LowInputIndex) {
			this->Buffer[i].RelationIndex = -1;
			this->Buffer[i].ID = -1;
		}
	}

}

//-----------------------------------------------------------------------------
// LoadBuffer
//-----------------------------------------------------------------------------
bool hsfcRule::LoadBuffer(int InputIndex, hsfcState* State, bool Audit){

	int RelationID;
	int RelationIndex;
	int BufferIndex;
	vector<hsfcRuleTerm> RuleTerm;

	// Get the Relation ID from the State for the input cursor
	RelationIndex = this->Input[InputIndex];
	RelationID = State->RelationID[RelationIndex][this->Cursor[InputIndex]];

	// Parse the RelationID into terms, and unify against the template
	if (!this->InputRelation[InputIndex]->Terms(RelationID, RuleTerm)) {
		return false;
	}

	// Load each of the input terms into the buffer checking as we go
	for (unsigned int j = 0; j < this->InputRelation[InputIndex]->Template.size(); j++) {

		// Check the fixed terms agianst the template
		if (this->InputRelation[InputIndex]->Template[j].Fixed) {
			if (RuleTerm[j].Tuple.ID != this->InputRelation[InputIndex]->Template[j].Tuple.ID) {
				return false;
			}
			continue;
		} 

		// Is the buffer empty, or already loaded and in agreement
		BufferIndex = this->InputRelation[InputIndex]->Template[j].BufferIndex;

		// Load / check the buffer
		if (this->Buffer[BufferIndex].ID == -1) {
			//// Is the term in the buffer domain; it may not be due to later restriction
			//if (this->RuleSchema->Buffer[BufferIndex].Domain[TermIndex[j]]) {
			//	// Load the buffer
			//	this->RuleSchema->Buffer[BufferIndex].TupleIndex = TermIndex[j];
			//} else {
			//	if (Debug) printf(" fail domain\n");
			//	return false;
			//}
			this->Buffer[BufferIndex].RelationIndex = RuleTerm[j].Tuple.RelationIndex;
			this->Buffer[BufferIndex].ID = RuleTerm[j].Tuple.ID;
			this->Buffer[BufferIndex].InputIndex = InputIndex;
		} else {
			// Does the joining Term match
			if ((this->Buffer[BufferIndex].RelationIndex != RuleTerm[j].Tuple.RelationIndex) || (this->Buffer[BufferIndex].ID != RuleTerm[j].Tuple.ID)) {
				return false;
			}
		}

	//	// Check the distinct conditions
	//	for (int k = 0; k < this->RuleSchema->NoDistincts; k++) {
	//		// Do we have enough info to perform the test
	//		for (int l = 0; l <= 1; l++) {
	//			if (this->RuleSchema->Distinct[k]->Term[l].Fixed) {
	//				TermIndex[l] = this->RuleSchema->Distinct[k]->Term[l].TupleIndex;
	//			} else {
	//				TermIndex[l] = this->RuleSchema->Buffer[this->RuleSchema->Distinct[k]->Term[l].BufferIndex].TupleIndex;
	//			}
	//		}
	//		if ((TermIndex[0] != 0) && (TermIndex[1] != 0) && (TermIndex[0] == TermIndex[1])) {
	//			if (Debug) printf(" fail distinct\n");
	//			return false;
	//		}
	//	}

	}

	return true;

}

//--- Overload ----------------------------------------------------------------
bool hsfcRule::LoadBuffer(int InputIndex){

	int RelationID;
	int RelationIndex;
	int BufferIndex;
	vector<hsfcRuleTerm> RuleTerm;

	// Get the Relation ID from the State for the input cursor
	RelationIndex = this->Input[InputIndex];
	RelationID = this->Cursor[InputIndex];

	// Parse the RelationID into terms, and unify against the template
	if (!this->InputRelation[InputIndex]->Terms(RelationID, RuleTerm)) {
		return false;
	}

	// Load each of the input terms into the buffer checking as we go
	for (unsigned int i = 0; i < this->InputRelation[InputIndex]->Template.size(); i++) {

		// Check the fixed terms agianst the template
		if (this->InputRelation[InputIndex]->Template[i].Fixed) {
			if (RuleTerm[i].Tuple.ID != this->InputRelation[InputIndex]->Template[i].Tuple.ID) {
				return false;
			}
			continue;
		} 

		// Is the buffer empty, or already loaded and in agreement
		BufferIndex = this->InputRelation[InputIndex]->Template[i].BufferIndex;

		// Load / check the buffer
		if (this->Buffer[BufferIndex].ID == -1) {
			// Check the term against the buffer domain
			//Found = false;
			//for (unsigned int j = 0; j < this->RuleSchema->Buffer[BufferIndex].Domain.size(); j++) {
			//	if ((RuleTerm[i].Tuple.RelationIndex == this->RuleSchema->Buffer[BufferIndex].Domain[j].RelationIndex) && (RuleTerm[i].Tuple.ID == this->RuleSchema->Buffer[BufferIndex].Domain[j].ID)) {
			//		Found = true;
			//		break;
			//	}
			//}
			//// Was it found in the domain for this variable
			//if (Found) {
				this->Buffer[BufferIndex].RelationIndex = RuleTerm[i].Tuple.RelationIndex;
				this->Buffer[BufferIndex].ID = RuleTerm[i].Tuple.ID;
				this->Buffer[BufferIndex].InputIndex = InputIndex;
			//} else {
			//	return false;
			//}
		} else {
			// Does the joining Term match
			if ((this->Buffer[BufferIndex].RelationIndex != RuleTerm[i].Tuple.RelationIndex) || (this->Buffer[BufferIndex].ID != RuleTerm[i].Tuple.ID)) {
				return false;
			}
		}

	}

	return true;

}

//-----------------------------------------------------------------------------
// CheckPreConditions
//-----------------------------------------------------------------------------
bool hsfcRule::CheckPreConditions(hsfcState* State, bool Audit){

	vector<hsfcTuple> Term;
	hsfcTuple Tuple;

	// PreCondictions contain only ground terms
	// so we don't nned to consult the buffer

	// Check each of the PreCondition
	for (int i = 0; i < this->NumPreConditions; i++) {

		// Count the transactions performed
		this->Transactions++;

		// Construct the relation terms
		Term.clear();
		for (unsigned int j = 0; j < this->PreConditionRelation[i]->Template.size(); j++) {
			Term.push_back(this->PreConditionRelation[i]->Template[j].Tuple);
		}

		// Is it a (distinct ?1 ?2); this makes no sense, but is possible
		if ((this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
			if ((Term[1].RelationIndex != Term[2].RelationIndex) || (Term[1].ID != Term[2].ID)) {
				return true;
			} else {
				if (Audit) printf(" xp%d", i); 
				return false;
			}
		}

		// Check to see if the relation exists
		Tuple.RelationIndex = this->PreCondition[i];
		Tuple.ID = this->PreConditionRelation[i]->ID(Term, true);
		if (Tuple.ID == -1) {
			if (Audit) printf(" xp%d", i); 
			return false;
		}

		// Not true
		if ((this->PreConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
			if (this->StateManager->RelationExists(State, &Tuple)) {
				if (Audit) printf(" xp%d", i); 
				return false;
			}
		} else {
			// True
			if (!this->StateManager->RelationExists(State, &Tuple)) {
				if (Audit) printf(" xp%d", i); 
				return false;
			}
		}

	}

	return true;

}

//-----------------------------------------------------------------------------
// CheckConditions
//-----------------------------------------------------------------------------
bool hsfcRule::CheckConditions(hsfcState* State, bool Audit){

	vector<hsfcTuple> Term;
	hsfcTuple NewTerm;
	hsfcTuple Tuple;

	// Check each of the Condition
	for (int i = 0; i < this->NumConditions; i++) {

		// Count the transactions performed
		this->Transactions++;

		// Construct the relation terms
		Term.clear();
		for (unsigned int j = 0; j < this->ConditionRelation[i]->Template.size(); j++) {
			// Does it come from the buffer or the template
			if (this->ConditionRelation[i]->Template[j].BufferIndex >= 0) {
				NewTerm.RelationIndex = this->Buffer[this->ConditionRelation[i]->Template[j].BufferIndex].RelationIndex;
				NewTerm.ID = this->Buffer[this->ConditionRelation[i]->Template[j].BufferIndex].ID;
				Term.push_back(NewTerm);
			} else {
				Term.push_back(this->ConditionRelation[i]->Template[j].Tuple);
			}
		}

		// Is it a (distinct ?1 ?2)
		if ((this->ConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
			if ((Term[1].RelationIndex != Term[2].RelationIndex) || (Term[1].ID != Term[2].ID)) {
				continue;
			} else {
				if (Audit) printf(" xc%d", i); 
				return false;
			}
		}

		// Check to see if the relation exists
		Tuple.RelationIndex = this->Condition[i];
		Tuple.ID = this->ConditionRelation[i]->ID(Term, true);
		if (Tuple.ID == -1) {
			if (Audit) printf(" xc%d", i); 
			return false;
		}

		// Does the condition have a not (condition) in front of it
		if ((this->ConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
			if (this->StateManager->RelationExists(State, &Tuple)) {
				if (Audit) printf(" xc%d", i); 
				return false;
			}
		} else {
			if (!this->StateManager->RelationExists(State, &Tuple)) {
				if (Audit) printf(" xc%d", i); 
				return false;
			}
		}

	}

	return true;

}

//--- Overload ----------------------------------------------------------------
int hsfcRule::CheckCondition(int Index){

	vector<hsfcTuple> Term;
	hsfcTuple NewTerm;

	// Rules for condition lookups
	// Distinct						-1 = fail	0 = pass
	// Not Distinct = !Distinct		-1 = pass	0 = fail
	// True							-1 = fail	0+ = lookup
	// Not True	= !True				-1 = pass	0+ = !lookup

	// Construct the relation terms
	Term.clear();
	for (unsigned int i = 0; i < this->ConditionRelation[Index]->Template.size(); i++) {
		// Does it come from the buffer or the template
		if (this->ConditionRelation[Index]->Template[i].BufferIndex >= 0) {
			NewTerm.RelationIndex = this->Buffer[this->ConditionRelation[Index]->Template[i].BufferIndex].RelationIndex;
			NewTerm.ID = this->Buffer[this->ConditionRelation[Index]->Template[i].BufferIndex].ID;
			Term.push_back(NewTerm);
		} else {
			Term.push_back(this->ConditionRelation[Index]->Template[i].Tuple);
		}
	}

	// Is it a (distinct ?1 ?2)
	if ((this->ConditionFunction[Index] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
		if ((Term[1].RelationIndex != Term[2].RelationIndex) || (Term[1].ID != Term[2].ID)) {
			return 0;
		} else {
			return -1;
		}
	}

	// Get the id
	return this->ConditionRelation[Index]->ID(Term, true);

}

//-----------------------------------------------------------------------------
// CheckResult
//-----------------------------------------------------------------------------
int hsfcRule::CheckResult(){

	vector<hsfcTuple> Term;
	hsfcTuple NewTerm;

	// Construct the relation terms
	Term.clear();
	for (unsigned int i = 0; i < this->ResultRelation->Template.size(); i++) {
		// Does it come from the buffer or the template
		if (this->ResultRelation->Template[i].BufferIndex >= 0) {
			NewTerm.RelationIndex = this->Buffer[this->ResultRelation->Template[i].BufferIndex].RelationIndex;
			NewTerm.ID = this->Buffer[this->ResultRelation->Template[i].BufferIndex].ID;
			Term.push_back(NewTerm);
		} else {
			Term.push_back(this->ResultRelation->Template[i].Tuple);
		}
	}

	// Get the id
	return this->ResultRelation->ID(Term, true);

}

//-----------------------------------------------------------------------------
// InitialiseInput
//-----------------------------------------------------------------------------
bool hsfcRule::InitialiseInput(hsfcState* State){

	// Set up the cursors on the source lists
	for (int i = 0; i < this->NumInputs; i++) {

		// Initialise each list
		this->Cursor[i] = 0;
		// If any lists are empty then exit
		if (State->NumRelations[this->Input[i]] == 0) {
			return false;
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// AdvanceInput
//-----------------------------------------------------------------------------
bool hsfcRule::AdvanceInput(int InputIndex, hsfcState* State){

	int i;

	// Inputs are incremented from the highest index to the lowest index
	// InputIndex points to the Input that needs to be incremented
	// For example; of 4 inputs all passed, InputIndex = 3
	//				of 4 inputs the second failed, InputIndex = 1

	// Are any of the cursors at the end
	for (i = InputIndex; i >= 0; i--) {

		// Is the cursor at the end of the list
		if (this->Cursor[i] == State->NumRelations[this->Input[i]] - 1) {
			// Reset the cursor to the beginning and advance the next cursor
			this->Cursor[i] = 0;
		} else {
			// Advance this cursor only
			(this->Cursor[i])++;
			break;
		}
	}

	// Are we finished
	if (i < 0) {
		return false;
	}

	return true;

}

//--- Overload ----------------------------------------------------------------
bool hsfcRule::AdvanceInput(int* LowInputIndex, int InputIndex, hsfcState* State){

	int i;

	// Inputs are incremented from the highest index to the lowest index
	// InputIndex points to the Input that needs to be incremented
	// For example; of 4 inputs all passed, InputIndex = 3
	//				of 4 inputs the second failed, InputIndex = 1

	// Are any of the cursors at the end
	for (i = InputIndex; i >= 0; i--) {

		*LowInputIndex = i;
		// Is the cursor at the end of the list
		if (this->Cursor[i] == State->NumRelations[this->Input[i]] - 1) {
			// Reset the cursor to the beginning and advance the next cursor
			this->Cursor[i] = 0;
		} else {
			// Advance this cursor only
			(this->Cursor[i])++;
			break;
		}
	}

	// Are we finished
	if (i < 0) {
		return false;
	}

	return true;

}

//--- Overload ----------------------------------------------------------------
bool hsfcRule::AdvanceInput(int InputIndex){

	int i;

	// Inputs are incremented from the highest index to the lowest index
	// InputIndex points to the Input that needs to be incremented
	// For example; of 4 inputs all passed, InputIndex = 3
	//				of 4 inputs the second failed, InputIndex = 1

	// Are any of the cursors at the end
	for (i = InputIndex; i >= 0; i--) {

		// Is the cursor at the end of the list
		if (this->Cursor[i] == (this->InputCount[i] - 1)) {
			// Reset the cursor to the beginning and advance the next cursor
			this->Cursor[i] = 0;
		} else {
			// Advance this cursor only
			(this->Cursor[i])++;
			break;
		}
	}

	// Are we finished
	if (i < 0) {
		return false;
	}

	return true;

}

//--- Overload ----------------------------------------------------------------
bool hsfcRule::AdvanceInput(int* LowInputIndex, int InputIndex){

	int i;

	// Inputs are incremented from the highest index to the lowest index
	// InputIndex points to the Input that needs to be incremented
	// For example; of 4 inputs all passed, InputIndex = 3
	//				of 4 inputs the second failed, InputIndex = 1

	// Are any of the cursors at the end
	for (i = InputIndex; i >= 0; i--) {

		*LowInputIndex = i;
		// Is the cursor at the end of the list
		if (this->Cursor[i] == (this->InputCount[i] - 1)) {
			// Reset the cursor to the beginning and advance the next cursor
			this->Cursor[i] = 0;
		} else {
			// Advance this cursor only
			(this->Cursor[i])++;
			break;
		}
	}

	// Are we finished
	if (i < 0) {
		return false;
	}

	return true;

}

//=============================================================================
// CLASS: hsfcGrinderEngine
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGrinderEngine::hsfcGrinderEngine(hsfcLexicon* Lexicon, hsfcStateManager* StateManager){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->StateManager = StateManager;
	this->Schema = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGrinderEngine::~hsfcGrinderEngine(void){

	// Free the resources
	if (this->Schema != NULL) {
		delete(this->Schema);
	}
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete(this->Rule[i]);
	}
	this->Rule.clear();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::Initialise(){

	// Create the Schema
	if (this->Schema == NULL) {
		this->Schema = new hsfcSchema(this->Lexicon);
	}
	this->Schema->Initialise();

	// Clear the rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete(this->Rule[i]);
	}
	this->Rule.clear();

	// Set properties
	this->EstReferenceSize = 0;
	this->RulePermutations = 0;
	this->MaxRefernceSize = 1.0e6;

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcGrinderEngine::Create(char* Script, int MaxRelationSize, double MaxReferenceSize) {

	hsfcRule* NewRule;
	double Size;

	// Initialise everything
	this->Initialise();
	this->MaxRefernceSize = MaxReferenceSize;

	// Create the Schema
	if (!this->Schema->Create(Script)) return false;

	// Create the StateManager from the Schema
	this->StateManager->SetSchema(this->Schema, MaxRelationSize);

	// Reset the step indexes
	for (int i = 0; i < 6; i++) {
		this->FirstRuleIndex[i] = 0;
		this->LastRuleIndex[i] = 0;
	}

	// Create the rules
	for (unsigned int i = 0; i < this->Schema->Rule.size(); i++) {

		// Create the new rule
		NewRule = new hsfcRule(this->Lexicon, this->StateManager);
		// Set the rule from the Schema and add it to the collection
		NewRule->FromSchema(this->Schema->Rule[i], true);
		this->Rule.push_back(NewRule);
		
		// Check to see if it is the last of a group of rules
		if (this->Lexicon->Match(NewRule->ResultRelation->PredicateIndex, "terminal|0")) {
			this->LastRuleIndex[1] = i;
			this->LastRuleIndex[2] = i;
			this->LastRuleIndex[3] = i;
			this->LastRuleIndex[4] = i;
			this->LastRuleIndex[5] = i;
		}
		if (this->Lexicon->Match(NewRule->ResultRelation->PredicateIndex, "legal|2")) {
			this->LastRuleIndex[2] = i;
			this->LastRuleIndex[3] = i;
			this->LastRuleIndex[4] = i;
		}
		if (this->Lexicon->Match(NewRule->ResultRelation->PredicateIndex, "sees|2")) {
			this->LastRuleIndex[3] = i;
			this->LastRuleIndex[4] = i;
		}
		if (this->Lexicon->PartialMatch(NewRule->ResultRelation->PredicateIndex, "next>")) {
			this->LastRuleIndex[4] = i;
		}
		if (this->Lexicon->Match(NewRule->ResultRelation->PredicateIndex, "goal|2")) {
			this->LastRuleIndex[5] = i;
		}

	}

	// Set up the starting point for the rule execution
	this->FirstRuleIndex[1] = 0;
	this->FirstRuleIndex[2] = this->LastRuleIndex[5] + 1;
	this->FirstRuleIndex[3] = this->LastRuleIndex[2] + 1;
	this->FirstRuleIndex[4] = this->LastRuleIndex[3] + 1;
	this->FirstRuleIndex[5] = this->LastRuleIndex[1] + 1;  // goal rules are execute on demand
	this->LastRuleIndex[0] = this->LastRuleIndex[4];

	// Set the upperbounds on the reference size and the rule permutations
	this->EstReferenceSize = 0;
	this->RulePermutations = 0;
	for (unsigned int i = 0; i < this->Schema->Rule.size(); i++) {
		this->RulePermutations += Factorial(this->Schema->Rule[i]->Relation.size() - 1);
		Size = 1;
		for (unsigned int j = 0; j < this->Schema->Rule[i]->Buffer.size(); j++) {
			Size *= (double)this->Schema->Rule[i]->Buffer[j].Domain.size();
		}
		this->EstReferenceSize += Size;
	}

	return true;

}

//-----------------------------------------------------------------------------
// OptimiseRules
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::OptimiseRules(bool OrderRules) {

	// This must be done in the Schema as there is a direct correlation 
	// between the Schema and the Engine in terms of rule relation ordering

	if (DEBUG) printf("\n--- Ordering Inputs ---------------------------------------\n");

	// Optimise each rule in the Schema
	this->RulePermutations = 0;
	for (unsigned int i = 0; i < this->Schema->Rule.size(); i++) {
		if (DEBUG) printf("Rule %d\n", i);
		this->RulePermutations += this->Schema->Rule[i]->Optimise(OrderRules);
	}

	// Calculate the estimated reference size
	this->EstReferenceSize = 0;
	for (unsigned int i = 0; i < this->Schema->Rule.size(); i++) {
		this->Schema->Rule[i]->EstReferenceSize = 0;
		for (unsigned int j = 0; j < this->Schema->Rule[i]->Relation.size(); j++) {
			this->Schema->Rule[i]->EstReferenceSize += (double)this->Schema->Rule[i]->Relation[j]->ReferenceSize;
		}
		if (this->Schema->Rule[i]->EstReferenceSize > this->MaxRefernceSize) {
			this->EstReferenceSize += this->Schema->Rule[i]->EstReferenceSize;
		}
	}

	if (DEBUG) printf("\n--- Recreating Engine -------------------------------------\n");

	if (DEBUG) this->Schema->Print();

	if (DEBUG) printf("\n--- Rules -------------------------------------\n");
	// Copy the optimised rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		
		// Reset the rule from the Schema and add it to the collection
		this->Rule[i]->FromSchema(this->Schema->Rule[i], false);
		this->Rule[i]->SetSpeed(this->MaxRefernceSize, this->StateManager->MaxRelationSize);
		if (DEBUG) printf("Rule %d", i);
		if (DEBUG) this->Rule[i]->Print();

	}

}

//-----------------------------------------------------------------------------
// GrindRules
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::GrindRules() {

	if (DEBUG) printf("\n--- Grinding ----------------------------------------------\n");

	// Grind each rule
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		if (DEBUG) printf("Rule %d\n", i);
		if (this->Rule[i]->LowSpeed) {
			if (DEBUG) printf("    LowSpeedExecution = NoReferenceTables\n");
		} else {
			this->Rule[i]->Grind();
		}
	}

}

//-----------------------------------------------------------------------------
// SetInitialState
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::SetInitialState(hsfcState* State) {

	// Set the initial state
	this->StateManager->SetInitialState(State);

}

//-----------------------------------------------------------------------------
// AdvanceState
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::AdvanceState(hsfcState* State, int Step) {

	int NextStep;
	int NoSteps;

	// Is it the goal relations
	if (Step == 5) {
		this->ProcessRules(State, 5);
		return;
	}

	// Process the next step until we match the requested step
	NoSteps = ((Step + 5) - State->CurrentStep) % 5;
	if (NoSteps == 0) NoSteps = 5;

	// Process each step
	for (int i = 0; i < NoSteps; i++) {
		NextStep = (State->CurrentStep + 1) % 5;

		// Process the rules
		if (NextStep == 0) {
			this->StateManager->NextState(State);
		} else {
			this->ProcessRules(State, NextStep);
		}

		// Update the current step
		State->CurrentStep = NextStep;

	}

}

//-----------------------------------------------------------------------------
// ProcessRules
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::ProcessRules(hsfcState* State, int Step) {

	// Step
	// 1 = Terminal Rules 
	// 2 = Legal Rules
	// 3 = Sees Rules
	// 4 = Next Rules
	// 5 = Goal Rules (processed on requirement)
	
	// Go through all of the rules
	for (int i = this->FirstRuleIndex[Step]; i <= this->LastRuleIndex[Step]; i++) {
		//if (this->Rule[i]->Manual) {
		//	this->Rule[i]->ExecuteManually(State, false);
		//} else {
			this->Rule[i]->Execute(State, false);
		//}
	}
	
}

//-----------------------------------------------------------------------------
// IsTerminal
//-----------------------------------------------------------------------------
bool hsfcGrinderEngine::IsTerminal(hsfcState* State) {

	// Have we exceeded the max no rounds
	if (State->Round >= 300) return true;
	
	// Look for a terminal relation
	if (State->NumRelations[this->StateManager->TerminalRelationIndex] > 0) {
		return true;
	}
	return false;

}

//-----------------------------------------------------------------------------
// GetLegalMoves
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::GetLegalMoves(hsfcState* State, vector<hsfcTuple>& Move) {

	hsfcTuple NewMove;

	// Copy all the relations in the list
	for (int i = 0; i < State->NumRelations[this->StateManager->LegalRelationIndex]; i++) {
		NewMove.ID = State->RelationID[this->StateManager->LegalRelationIndex][i];
		NewMove.RelationIndex = this->StateManager->LegalRelationIndex;
		Move.push_back(NewMove);
	}

}

//-----------------------------------------------------------------------------
// ChooseRandomMoves
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::ChooseRandomMoves(hsfcState* State) {

	hsfcTuple NewMove[MAX_NUM_ROLES];
	int RandomNo;
	int RandomIndex;
	int RoleIndex;
	int NumMoves[MAX_NUM_ROLES];

	// Choose randomly
	RandomNo = rand();

	// Go through all of the legal maoves
	for (int i = 0; i < State->NumRelations[this->StateManager->RoleRelationIndex]; i++) {
		NumMoves[i] = 0;
		NewMove[i].ID = 0;
		NewMove[i].RelationIndex = this->StateManager->DoesRelationIndex;
	}

	// Go through all of the legal maoves
	for (int i = 0; i < State->NumRelations[this->StateManager->LegalRelationIndex]; i++) {

		// Who is the move for
		RoleIndex = State->RelationID[this->StateManager->LegalRelationIndex][i] % State->NumRelations[this->StateManager->RoleRelationIndex];
		NumMoves[RoleIndex]++;
		// Calculate the chance of this move being selected
		RandomIndex = (RandomNo / (RoleIndex + 1)) % NumMoves[RoleIndex];
		if (RandomIndex == 0) {
			NewMove[RoleIndex].ID = State->RelationID[this->StateManager->LegalRelationIndex][i];
			NewMove[RoleIndex].RelationIndex = this->StateManager->DoesRelationIndex;
		}
	}
	for (int i = 0; i < State->NumRelations[this->StateManager->RoleRelationIndex]; i++) {
		this->StateManager->AddRelation(State, &NewMove[i]);
	}

}

////-----------------------------------------------------------------------------
//// GoalValue
////-----------------------------------------------------------------------------
//int hsfcGrinderEngine::GoalValue(hsfcState* State, int RoleIndex){
//
//	int Result;
//	int Value;
//	int RelationID;
//	int RelationRoleIndex;
//	vector<hsfcTuple> Term;
//
//	Result = 0;
//
//	// Assumes the states is at Step 1 or greater
//	// Execute the goal rules
//	this->ProcessRules(State, 5);
//
//	// Go through the Goal relations
//	for (int i = 0; i < State->NumRelations[this->StateManager->GoalRelationIndex]; i++) {
//		RelationID = State->RelationID[this->StateManager->GoalRelationIndex][i];
//		RelationRoleIndex = RelationID % State->NumRelations[this->StateManager->RoleRelationIndex];
//		// Is this the right role
//		if (RelationRoleIndex == RoleIndex) {
//			this->Schema->Relation[this->StateManager->GoalRelationIndex]->Terms(RelationID, Term);
//			Value = atoi(this->Lexicon->Text(Term[1].ID));
//			if (Value > Result) Result = Value;
//		}
//	}
//
//	return Result;
//
//}

//-----------------------------------------------------------------------------
// ResetStatistics
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::ResetStatistics(){

	// Reset the frequency counters on the Schema relations
	for (unsigned int i = 0; i < this->Schema->Relation.size(); i++) {
		this->Schema->Relation[i]->AveListLength = 0;
		this->Schema->Relation[i]->Samples = 0;
	}
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		this->Rule[i]->Transactions = 0;
	}

}

//-----------------------------------------------------------------------------
// CollectStatistics
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::CollectStatistics(hsfcState* State){

	// Reset the frequency counters on the Schema relations
	for (unsigned int i = 0; i < this->Schema->Relation.size(); i++) {
		this->Schema->Relation[i]->AveListLength = ((this->Schema->Relation[i]->AveListLength * this->Schema->Relation[i]->Samples) + (double)State->NumRelations[i]) / (this->Schema->Relation[i]->Samples + 1);
		this->Schema->Relation[i]->Samples++;
	}

}

//-----------------------------------------------------------------------------
// PrintStatistics
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::PrintStatistics(){

	printf("\n--- Statistics --------------------------------------------\n");
	printf("Playouts = %d\n", this->Playouts);
	printf("  States = %d   %.2f\n", this->States, (double)this->States / (double)this->Playouts);
	printf("\nAverage List Length\n");
	for (unsigned int i = 0; i < this->Schema->Relation.size(); i++) {
		printf("%2d. %.2f\t\t%s\n", i, this->Schema->Relation[i]->AveListLength, this->Lexicon->Text(this->Schema->Relation[i]->PredicateIndex));
	}
	printf("\nAverage Rule Transactions\n");
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		printf("%2d. %.2f\t\t%s\n", i, (double)this->Rule[i]->Transactions / (double)this->States, this->Lexicon->Text(this->Schema->Rule[i]->Relation[0]->PredicateIndex));
	}

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcGrinderEngine::Print(){

	//this->Schema->Print();

	printf("\n--- Rule Engine --------------------------------------------\n");
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		printf("%d.", i);
		this->Rule[i]->Print();
	}

}

//=============================================================================
// CLASS: hsfcGrinder
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGrinder::hsfcGrinder(hsfcLexicon* Lexicon, hsfcStateManager* StateManager){

	// Allocate the memory
	this->StateManager = StateManager;
	this->Lexicon = Lexicon;
	this->Engine = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGrinder::~hsfcGrinder(void){

	// Free the resources
	if (this->Engine != NULL) {
		delete(this->Engine);
	}

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcGrinder::Initialise(){

	// Randomise everything
	srand(time(NULL));

	// Create the Engine
	if (this->Engine == NULL) {
		this->Engine = new hsfcGrinderEngine(this->Lexicon, this->StateManager);
	}

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcGrinder::Create(char* Script, int MaxRelationSize, double MaxReferenceSize) {

	// Initialise
	this->Initialise();

	// Create the Engine
	if (!this->Engine->Create(Script, MaxRelationSize, MaxReferenceSize)) return false;

	return true;

}

//-----------------------------------------------------------------------------
// Optimise
//-----------------------------------------------------------------------------
void hsfcGrinder::Optimise() {

	hsfcState* State;
	clock_t Start;

	if (DEBUG) printf("\n--- Optimising --------------------------------------------\n");

	// Create a State to use for optimisation
	State = this->StateManager->CreateState();
	this->StateManager->InitialiseState(State);

	// Use random playouts to collect statistics
	this->Engine->ResetStatistics();
	this->Engine->Playouts = 0;
	this->Engine->States = 0;

	// Run for some number of random games or some time
	Start = clock();
	for (int i = 0; i < 1000; i++) {

		if (DEBUG) printf(".");

		// Set the initial state
		this->StateManager->SetInitialState(State);
//this->StateManager->PrintRelations(State, true);

		// Advance to get legal movea and check terminal
		this->Engine->AdvanceState(State, 2);
//this->StateManager->PrintRelations(State, true);

		// Play the game
		while (true) {

			// Choose moves randomly
			this->Engine->ChooseRandomMoves(State);
//this->StateManager->PrintRelations(State, true);

			// Advance to the end of the rules processing everything
			this->Engine->AdvanceState(State, 4);
//this->StateManager->PrintRelations(State, true);
			this->Engine->ProcessRules(State, 5);
//this->StateManager->PrintRelations(State, true);

			// Collect the statistics
			this->Engine->CollectStatistics(State);
			this->Engine->States++;

			// Advance to the next state
			this->Engine->AdvanceState(State, 2);

			// If its terminal process all of the rules, one last time
			if (this->Engine->IsTerminal(State)) {
				this->Engine->ChooseRandomMoves(State);
				this->Engine->AdvanceState(State, 4);
				this->Engine->ProcessRules(State, 5);
				this->Engine->States++;
				this->Engine->CollectStatistics(State);
				break;
			}

			if (clock() > Start + 100 * CLOCKS_PER_SEC) break;

		}

		// Count the playouts and check the time
		this->Engine->Playouts++;
		if (clock() > Start + 100 * CLOCKS_PER_SEC) break;
		if ((this->Engine->Playouts >= 10) && (clock() > Start + 30 * CLOCKS_PER_SEC)) break;
		if ((this->Engine->Playouts >= 100) && (clock() > Start + 10 * CLOCKS_PER_SEC)) break;

	}

	if (DEBUG) printf("\n");
	if (DEBUG) this->Engine->PrintStatistics();

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcGrinder::Print(){

	this->Engine->Print();

}

//-----------------------------------------------------------------------------
// ResetStatistics
//-----------------------------------------------------------------------------
void hsfcGrinder::ResetStatistics(){

	this->Engine->ResetStatistics();

}

