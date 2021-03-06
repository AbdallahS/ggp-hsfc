//=============================================================================
// Project: High Speed Forward Chaining
// Module: Grinder
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include "hsfcRule.h"

using namespace std;

//=============================================================================
// CLASS: hsfcRule
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRule::hsfcRule(hsfcLexicon* Lexicon, hsfcStateManager* StateManager, hsfcDomainManager* DomainManager){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->StateManager = StateManager;
	this->DomainManager = DomainManager;

	this->Cursor = NULL;
	this->Variable = NULL;
	this->ResultCalculator.Term = NULL;
	this->Input = NULL;
	this->Condition = NULL;
	this->PreCondition = NULL;
	this->ConditionFunction = NULL;
	this->PreConditionFunction = NULL;
	this->ResultLookup = NULL;
	this->InputLookup = NULL;
	this->ConditionLookup = NULL;
	this->PreConditionLookup = NULL;
	this->InputCalculator = NULL;
	this->ConditionCalculator = NULL;
	this->PreConditionCalculator = NULL;

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
	this->ClearRule();
	
}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRule::Initialise(){

	// Free the references
	this->ClearRule();

	// Reset the counters
	this->RuleSchema = NULL;
	this->LowSpeed = false;
	this->NumInputs = 0;
	this->NumConditions = 0;
	this->NumPreConditions = 0;
	this->VariableSize = 0;
	this->LookupSize = 0;
	this->SelfReferenceCount = 0;

}

//-----------------------------------------------------------------------------
// FromSchema
//-----------------------------------------------------------------------------
void hsfcRule::FromSchema(hsfcRuleSchema* RuleSchema, bool LowSpeed) {

	int Index;

	// Initialise the Rule
	this->Initialise();
	this->RuleSchema = RuleSchema;
	this->LowSpeed = LowSpeed;
	this->SelfReferenceCount = 0;

	// Set up the buffer for assembling the variable terms 
	this->Variable = new hsfcBufferTerm[RuleSchema->VariableSchema.size()];
	this->VariableSize = RuleSchema->VariableSchema.size();
	for (int i = 0; i < this->VariableSize; i++) this->Variable[i].InputIndex = UNDEFINED;

	// Count the inputs, conditions, preconditions
	this->NumInputs = 0;
	this->NumConditions = 0;
	this->NumPreConditions = 0;
	for (unsigned int i = 0; i < RuleSchema->RelationSchema.size(); i++) {
		if (RuleSchema->RelationSchema[i]->Type == hsfcRuleInput) this->NumInputs++;
		if (RuleSchema->RelationSchema[i]->Type == hsfcRuleCondition) this->NumConditions++;
		if (RuleSchema->RelationSchema[i]->Type == hsfcRulePreCondition) this->NumPreConditions++;
	}

	// Check the rule size
	if (this->NumInputs > MAX_NO_OF_INPUTS) this->LowSpeed = true; 

	// Set up the result linkages
	this->Result = RuleSchema->RelationSchema[0]->TermSchema[0].RelationIndex;
	this->BuildCalculator(RuleSchema->RelationSchema[0], &this->ResultCalculator);

	// Set up the Input linkages
	if (this->NumInputs > 0) {
		// Create and populate the the arrays
		this->Input = new int[this->NumInputs];
		this->Cursor = new int[this->NumInputs];
		this->InputCalculator = new hsfcCalculator[this->NumInputs];
		Index = 0;
		// Link to the lists in the state
		for (unsigned int i = 0; i < RuleSchema->RelationSchema.size(); i++) {
			if (RuleSchema->RelationSchema[i]->Type == hsfcRuleInput) {
				this->Input[Index] = RuleSchema->RelationSchema[i]->TermSchema[0].RelationIndex;
				this->BuildCalculator(RuleSchema->RelationSchema[i], &this->InputCalculator[Index]);
				// Load the variable source
				for (unsigned int j = 0; j < RuleSchema->RelationSchema[i]->TermSchema.size(); j++) {
					if (RuleSchema->RelationSchema[i]->TermSchema[j].VariableIndex != UNDEFINED) {
						if (this->Variable[RuleSchema->RelationSchema[i]->TermSchema[j].VariableIndex].InputIndex == UNDEFINED) {
							this->Variable[RuleSchema->RelationSchema[i]->TermSchema[j].VariableIndex].InputIndex = Index;
						}
					}
				}
				Index++;
			}
		}
	}

	// Set up the Condition linkages
	if (this->NumConditions > 0) {
		// Create and populate the the arrays
		this->Condition = new int[this->NumConditions];
		this->ConditionFunction = new int[this->NumConditions];
		this->ConditionCalculator = new hsfcCalculator[this->NumConditions];
		Index = 0;
		// Link to the lists in the state
		for (unsigned int i = 0; i < RuleSchema->RelationSchema.size(); i++) {
			if (RuleSchema->RelationSchema[i]->Type == hsfcRuleCondition) {
				this->Condition[Index] = RuleSchema->RelationSchema[i]->TermSchema[0].RelationIndex;
				this->ConditionFunction[Index] = RuleSchema->RelationSchema[i]->Function;
				this->BuildCalculator(RuleSchema->RelationSchema[i], &this->ConditionCalculator[Index]);
				Index++;
			}
		}
	}

	// Set up the PreCondition linkages
	if (this->NumPreConditions > 0) {
		// Create and populate the the arrays
		this->PreCondition = new int[this->NumPreConditions];
		this->PreConditionFunction = new int[this->NumPreConditions];
		this->PreConditionCalculator = new hsfcCalculator[this->NumPreConditions];
		Index = 0;
		// Link to the lists in the state
		for (unsigned int i = 0; i < RuleSchema->RelationSchema.size(); i++) {
			if (RuleSchema->RelationSchema[i]->Type == hsfcRulePreCondition) {
				this->PreCondition[Index] = RuleSchema->RelationSchema[i]->TermSchema[0].RelationIndex;
				this->PreConditionFunction[Index] = RuleSchema->RelationSchema[i]->Function;
				this->BuildCalculator(RuleSchema->RelationSchema[i], &this->PreConditionCalculator[Index]);
				Index++;
			}
		}
	}

	// Count the self references
	for (unsigned int i = 1; i < RuleSchema->RelationSchema.size(); i++) {
		if (RuleSchema->RelationSchema[i]->TermSchema[0].NameID == RuleSchema->RelationSchema[0]->TermSchema[0].NameID) {
			this->SelfReferenceCount++;
		}
	}

	// Print the rule
	if (this->Lexicon->IO->Parameters->LogDetail > 3) this->Print(false);

}

//-----------------------------------------------------------------------------
// OptimiseInputs
//-----------------------------------------------------------------------------
void hsfcRule::OptimiseInputs(hsfcSchema* Schema) {

	unsigned int RelationIndex;
	unsigned int TermRelationIndex;
	unsigned int TermDomainIndex;
	vector<unsigned int> Indirect;		// Indirect adressing of inputs
	vector<unsigned int> InputIndex;
	vector<unsigned int> BestInputIndex;
	vector<unsigned int> InputRelationIndex;
	vector<unsigned int> Permutation; 
	vector<bool> VariableFixed;
	double ReadCount;
	double ExpectedFrequency;
	double PassProbability;
	double ExpectedInstances;
	double LookupSize;
	unsigned int VariableIndex;
	int Count;
	double BestReadCount;
	double BestLookupSize;
	bool BestPermutation;
	unsigned int NumPermutations;
	unsigned int Temp0;
	unsigned int Temp1;

	// Set the default speed
	this->LowSpeed = false;
	BestReadCount = 1e99;
	BestLookupSize = 1e99;

	// Are any of the relations 'sorted' rather than 'exists'
	for (unsigned int i = 0; i < this->RuleSchema->RelationSchema.size(); i++) {
		RelationIndex = this->RuleSchema->RelationSchema[i]->TermSchema[0].RelationIndex;
		if (this->DomainManager->Domain[RelationIndex].IDCount > this->StateManager->MaxRelationSize) {
			this->LowSpeed = true;
		}
	}

	// Even if it is low speed, we can still benefit from sorting the inputs

	// Do everything in the schema first, then recreate the rule from the new schema
	// Sort the rule relations so the possible inputs are first

	// Keep track of the permutations of the input indexes by addressing them indirectly
	// Not all of the relations are inputs, so the initial vectors will look like
	// OrigIndex	0, 1, 2, 3, 4, 5
	// Input/Cond   any order / any order	InputCount
	// InputIndex	shuffled  / ignored

	// Set all the input types
	for (unsigned int i = 1; i < this->RuleSchema->RelationSchema.size(); i++) {
		// By default its an input
		this->RuleSchema->RelationSchema[i]->Type = hsfcRuleInput;
		// Is it a precondition
		Count = 0;
		for (unsigned int j = 0; j < this->RuleSchema->RelationSchema[i]->TermSchema.size(); j++) {
			if (this->RuleSchema->RelationSchema[i]->TermSchema[j].Type == hsfcTypeVariable) Count++;
		}
		if (Count == 0) {
			this->RuleSchema->RelationSchema[i]->Type = hsfcRulePreCondition;
			continue;
		}
		// Is it a function
		if (this->RuleSchema->RelationSchema[i]->Function != hsfcFunctionNone) {
			this->RuleSchema->RelationSchema[i]->Type = hsfcRuleCondition;
			continue;
		}
		// Add it to the input list
		Indirect.push_back(0);
		InputIndex.push_back(i);
		BestInputIndex.push_back(i);
		InputRelationIndex.push_back(this->RuleSchema->RelationSchema[i]->TermSchema[0].RelationIndex);

	}
	// Keep track of each of the variables
	for (unsigned int i = 0; i < this->RuleSchema->VariableSchema.size(); i++) VariableFixed.push_back(false);

	// Is this a trivial exercise or de we need some permutation tables
	if (InputIndex.size() <= 1) return;

	// We need to create all the permutations for the first two inputs
	NumPermutations = 0;
	for (unsigned int i = 0; i < InputIndex.size(); i++) {
		for (unsigned int j = 0; j < InputIndex.size(); j++) {
			if (j != i) {
				Permutation.push_back(i);
				Permutation.push_back(j);
				NumPermutations++;
			}
		}
	}

	//// Calculate for each permutation of inputs
	//for (unsigned int p = 0; p < NumPermutations; p++) {

		// Set up the indirect addressing for this permutation
		for (unsigned int i = 0; i < InputIndex.size(); i++) {
			Indirect[i] = i;
		}
		// Indirect: 0, 1, 2, 3, etc
		// Now swap the first two inputs
		//printf("\nPermutation: ");
		//for (unsigned int i = 0; i < 2; i++) printf(" %d", Permutation[2*p+i]); 
		//Temp0 = Indirect[Permutation[2*p]];
		//Temp1 = Indirect[Permutation[2*p+1]];
		//Indirect[Permutation[2*p]] = 0;
		//Indirect[Permutation[2*p+1]] = 1;
		//Indirect[0] = Temp0;
		//Indirect[1] = Temp1;
		//printf("\nPermutation: ");
		//for (unsigned int i = 0; i < InputIndex.size(); i++) printf(" %d", InputIndex[Indirect[i]]); 
		//printf("\n");

		//continue;

		// Keep track of each of the variables
		for (unsigned int i = 0; i < this->RuleSchema->VariableSchema.size(); i++) VariableFixed[i] = false;

		// Calculate the ReadCost = number of reads of any input from the state
		ReadCount = 0;
		ExpectedFrequency = 1;
		ExpectedInstances = 1;
		LookupSize = 0;
		for (unsigned int i = 0; i < InputIndex.size(); i++) {

			// How many read operations wil there be
			ReadCount += ExpectedFrequency * Schema->RelationSchema[InputRelationIndex[Indirect[i]]]->Statistics.Average();
			ExpectedInstances = ExpectedInstances * this->DomainManager->Domain[InputRelationIndex[Indirect[i]]].IDCount;
			LookupSize += ExpectedInstances;

			// Calculate the pass probability based on the variable domain sizes
			PassProbability = 1;
			for (unsigned int j = 0; j < this->RuleSchema->RelationSchema[InputIndex[Indirect[i]]]->TermSchema.size(); j++) {

				// Check if the term is a variable
				if (this->RuleSchema->RelationSchema[InputIndex[Indirect[i]]]->TermSchema[j].Type == hsfcTypeVariable) {
					VariableIndex = this->RuleSchema->RelationSchema[InputIndex[Indirect[i]]]->TermSchema[j].VariableIndex;
					// Is it already present, and must match exactly
					if (VariableFixed[VariableIndex]) {
						PassProbability = PassProbability / this->RuleSchema->VariableSchema[VariableIndex]->Term.size();
					} else {
						VariableFixed[VariableIndex] = true;
					}
				}
				// Fixed arguments have only one chance of being correct
				if (this->RuleSchema->RelationSchema[InputIndex[Indirect[i]]]->TermSchema[j].Type == hsfcTypeFixed) {
					// We need to consult the domain
					TermRelationIndex = this->RuleSchema->RelationSchema[InputIndex[Indirect[i]]]->TermSchema[j].RelationIndex;
					TermDomainIndex = this->RuleSchema->RelationSchema[InputIndex[Indirect[i]]]->TermSchema[j].ArgumentIndex;
					PassProbability = PassProbability / this->DomainManager->Domain[TermRelationIndex].RecordSize[TermDomainIndex];
				}

			}

			// Calculate the numbe of instance at this point in the execution of the rule
			ExpectedFrequency = ExpectedFrequency * PassProbability * Schema->RelationSchema[InputRelationIndex[Indirect[i]]]->Statistics.Average();
			ExpectedInstances = ExpectedInstances * PassProbability;

			//// Debug
			//printf("Input #%d\n", i); 
			//this->RuleSchema->RelationSchema[InputIndex[Indirect[i]]]->Print();
			//printf("  ReadCount          %.0f\n", ReadCount); 
			//printf("  PassProbability    %.6f\n", PassProbability); 
			//printf("  ExpectedFrequency  %.3f\n", ExpectedFrequency); 
			//printf("  ExpectedInstances  %.3f\n", ExpectedInstances); 
			//printf("  LookupSize         %.0f\n", LookupSize); 

			// Check if all the variables are in the rule
			for (unsigned int j = 0; j < VariableFixed.size(); j++) {
				if (!VariableFixed[j]) goto NextInput;
			}
			// Add in the remaining conditions and the result
			for (unsigned int j = i; j < this->RuleSchema->RelationSchema.size(); j++) LookupSize += ExpectedInstances;
			//printf("  FinalLookupSize    %.0f\n", LookupSize); 

			break;

NextInput:;

		}

		// Is this the best so far
		// HighSpeed is always betterthan LowSpeed
		BestPermutation = false;
		if (BestLookupSize > this->Lexicon->IO->Parameters->MaxLookupSize) {
			if (LookupSize < this->Lexicon->IO->Parameters->MaxLookupSize) {
				BestPermutation = true;
			} else {
				if (ReadCount < BestReadCount) {
					BestPermutation = true;
				}
			}
		} else {
			if (ReadCount < BestReadCount) {
				if (LookupSize < this->Lexicon->IO->Parameters->MaxLookupSize) {
					BestPermutation = true;
				}
			}
		}

		// Record the best permutation
		if (BestPermutation) {
			BestLookupSize = LookupSize;
			BestReadCount = ReadCount;
			for (unsigned int i = 0; i < InputIndex.size(); i++) {
				BestInputIndex[i] = InputIndex[Indirect[i]];
			}
		}

	//}

	//printf("\nBestPermutation: ");
	//for (unsigned int i = 0; i < InputIndex.size(); i++) printf(" %d", BestInputIndex[i]); 
	//printf("\n");
	//printf("  BestReadCount      %.0f\n", BestReadCount); 
	//printf("  BestLookupSize     %.0f\n", BestLookupSize); 

	// Is the lookup table too big
	if (BestLookupSize > this->Lexicon->IO->Parameters->MaxLookupSize) {
		this->LowSpeed = true;
		//printf("  Set to LowSpeed\n"); 
	}


}


//-----------------------------------------------------------------------------
// CreateLookupTable
//-----------------------------------------------------------------------------
void hsfcRule::CreateLookupTable() {

	vector<hsfcTuple> Term;
	vector<hsfcRuleTerm> RuleTerm;
	int iii;
	int InputIndex;
	int PreConditionIndex;
	unsigned int* LookupIndex;
	unsigned int* LookupValue;
	unsigned int* NextLookupValue;
	bool PreviousFailed;
	int Count;
	int Lowiii;

	// So far the calculations have only been approximations
	// We must do the inputs in sequence to get the exact table sizes
	// InputReferenceSize = PrevNoUniqueIDs * InputRelationSize

	// Schema Relations are now sorted to their final ordering
	// Dimension the lookup array for inputs and set values to -1 = Fail

	LookupValue = NULL;
	LookupIndex = NULL;
	NextLookupValue = NULL;
	this->LookupSize = 0;

	// Dimension the lookup arrays
	// Dimension the maximum lookup values
	if (this->NumInputs > 0) {
		this->InputLookup = new unsigned int*[this->NumInputs];
		this->InputLookupSize = new unsigned int[this->NumInputs];
		this->MaxInputLookup = new unsigned int[this->NumInputs];
		for (int i = 0; i < this->NumInputs; i++) this->MaxInputLookup[i] = 0;
	}
	if (this->NumConditions > 0) {
		this->ConditionLookup = new unsigned int*[this->NumConditions];		
		this->ConditionLookupSize = new unsigned int[this->NumConditions];
		this->MaxConditionLookup = new unsigned int[this->NumConditions];		
		for (int i = 0; i < this->NumConditions; i++) this->MaxConditionLookup[i] = 0;
	}
	if (this->NumPreConditions > 0) {
		this->PreConditionLookup = new unsigned int*[this->NumPreConditions];
		this->PreConditionLookupSize = new unsigned int[this->NumPreConditions];
		this->MaxPreConditionLookup = new unsigned int[this->NumPreConditions];
		for (int i = 0; i < this->NumPreConditions; i++) this->MaxPreConditionLookup[i] = 0;
	}

	//--- PreConditions ---------------------------------------------------------------------------

	PreConditionIndex = 0;
	for (unsigned int i = 0; i < this->RuleSchema->RelationSchema.size(); i++) {

		// Is it a precondition
		if (this->RuleSchema->RelationSchema[i]->Type == hsfcRulePreCondition) {

			this->PreConditionLookupSize[PreConditionIndex] = 1;
			this->LookupSize += this->PreConditionLookupSize[PreConditionIndex];
			this->PreConditionLookup[PreConditionIndex] = new unsigned int[this->PreConditionLookupSize[PreConditionIndex]];
			this->Lexicon->IO->FormatToLog(3, true, "PreCondition %d   Size = 1\n", PreConditionIndex);

			// Calculate the value; out of range returns false 
			if (this->CalculateValue(this->PreConditionCalculator[PreConditionIndex])) {
				this->PreConditionLookup[PreConditionIndex][0] = this->PreConditionCalculator[PreConditionIndex].Value.ID;
			} else {
				this->PreConditionLookup[PreConditionIndex][0] = UNDEFINED;
			}

			if (this->Lexicon->IO->Parameters->LogDetail > 3) {
				Count = 0;
				for (unsigned int j = 0; j < this->PreConditionLookupSize[PreConditionIndex]; j++) {
					if (this->PreConditionLookup[PreConditionIndex][j] >= 0) Count++;
					this->Lexicon->IO->FormatToLog(0, true, "    %5d: %5d\n", j, this->PreConditionLookup[PreConditionIndex][j]);
				}
				this->Lexicon->IO->FormatToLog(0, true, "    Count = %d\n", Count);
			}

			PreConditionIndex++;

		}
	}

	// Some rules have no inputs, just preconditions
	// For these rules the result is already ground
	if (this->NumInputs == 0) {

		// Create the reference table
		this->ResultLookupSize = 1;
		this->LookupSize += this->ResultLookupSize;
		this->ResultLookup = new unsigned int[this->ResultLookupSize];
		this->Lexicon->IO->FormatToLog(3, true, "Result   Size = %u\n", this->ResultLookupSize);

		// Calculate the value; out of range returns false 
		if (this->CalculateValue(this->ResultCalculator)) {
			this->ResultLookup[0] = this->ResultCalculator.Value.ID;
		} else {
			this->ResultLookup[0] = UNDEFINED;
		}

		// Print the table
		if (this->Lexicon->IO->Parameters->LogDetail > 3) {
			Count = 0;
			for (unsigned int j = 0; j < this->ResultLookupSize; j++) {
				if (this->ResultLookup[j] >= 0) Count++;
				this->Lexicon->IO->FormatToLog(0, true, "    %5lu: %5lu\n", j, this->ResultLookup[j]);
			}
			this->Lexicon->IO->FormatToLog(0, true, "    Count = %d\n", Count);
		}

		this->Lexicon->IO->FormatToLog(3, true, "Lookup Size = %.0f\n", this->LookupSize * sizeof(int));

		return;

	}

	//--- Inputs ---------------------------------------------------------------------------

	// Construct the Reference Tables for the inputs iteratively
	// Enumerate Input0 and construct table
	// Enumerate Input1 and load all permutations below and construct table
	// etc..

	// Zero the lookup counters
	if (this->NumInputs > 0) {
		NextLookupValue = new unsigned int[this->NumInputs];
	    LookupValue = new unsigned int[this->NumInputs];
	    LookupIndex = new unsigned int[this->NumInputs + 1];
		this->InputCount = new int[this->NumInputs];
	}
	for (int i = 0; i < this->NumInputs; i++) {
		NextLookupValue[i] = 0;
		this->InputCount[i] = this->DomainManager->Domain[this->Input[i]].IDCount;
	}

	// Process the inputs
	for (InputIndex = 0; InputIndex < this->NumInputs; InputIndex++) {

		// Calculate the reference size
		if (InputIndex == 0) {
			this->InputLookupSize[InputIndex] = this->InputCount[InputIndex];
		} else {
			this->InputLookupSize[InputIndex] =  (this->MaxInputLookup[InputIndex-1] + 1) * this->InputCount[InputIndex];
		}
		this->InputLookup[InputIndex] = new unsigned int[this->InputLookupSize[InputIndex]];
		this->LookupSize += this->InputLookupSize[InputIndex];
		this->Lexicon->IO->FormatToLog(3, true, "Input %d   Size = %u\n", InputIndex, this->InputLookupSize[InputIndex]);

		// Initialise to -1 for fail
		for (unsigned int j = 0; j <  this->InputLookupSize[InputIndex]; j++) {
			this->InputLookup[InputIndex][j] = UNDEFINED;
		}

		// Reset the cursors
		for (int i = 0; i < this->NumInputs; i++) {
			this->Cursor[i] = 0;
		}

		// Load all the permutations
		Lowiii = 0;
		do {

			// Clear each of the terms in the buffer
			this->ClearVariables(Lowiii);

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

				// Is the lookup value valid; remember LookupValue(InputIndex) == UNDEFINED by default
				if ((iii < InputIndex) && (LookupValue[iii] == UNDEFINED)) break;

				// Load the buffer 
				if (this->LoadInput(iii)) {
					// Record the results in the lookup for the highest input only
					if (iii == InputIndex) {
						LookupValue[iii] = NextLookupValue[InputIndex];
						this->InputLookup[InputIndex][LookupIndex[iii]] = LookupValue[iii];
						this->MaxInputLookup[InputIndex] = LookupValue[iii]; 
						NextLookupValue[InputIndex]++;
					}
				} else {
					// If the load fails then the lookup is UNDEFINED
					LookupValue[iii] = UNDEFINED;
					break;
				}

			}

			// Advance the inputs
			if (iii > InputIndex) iii = InputIndex;
		} while (this->AdvanceInput(&Lowiii, iii));

		// Print the reference table
		if (this->Lexicon->IO->Parameters->LogDetail > 3) {
			for (unsigned int i = 0; i < this->InputLookupSize[InputIndex]; i++) 
				this->Lexicon->IO->FormatToLog(0, true, "    %5lu: %5lu\n", i, this->InputLookup[InputIndex][i]);
			this->Lexicon->IO->FormatToLog(0, true, "    Unique = %u\n", this->MaxInputLookup[InputIndex] + 1);
		}

	}

	//--- Conditions & Result ----------------------------------------------------------------

	// At this point we know the size of the condition and result references

	// Improvement:
	// It is possible to back-propogate from condition(0) > > input(last) if condition is fact
	// It is also possible to back propogate from input(i) > >  input(i-1)

	// Create the condition references
	for (int i = 0; i < this->NumConditions; i++) {

		// Set the reference size
		this->ConditionLookupSize[i] = this->MaxInputLookup[this->NumInputs-1] + 1;
		this->LookupSize += this->ConditionLookupSize[i];

		if (this->ConditionLookupSize[i] > 0) {
			this->ConditionLookup[i] = new unsigned int[this->ConditionLookupSize[i]];
		} else {
			this->Lexicon->IO->WriteToLog(0, false, "Error: Condition ReferenceSize <= 0\n\n");
			abort();
		}

		// Initialise to -1 for fail
		for (unsigned int j = 0; j < this->ConditionLookupSize[i]; j++) {
			this->ConditionLookup[i][j] = UNDEFINED;
		}

	}

	// Create the result references

	// Set the reference size
	this->ResultLookupSize = this->MaxInputLookup[this->NumInputs-1] + 1;
	this->LookupSize += this->ResultLookupSize;

	if (this->ResultLookupSize > 0) {
		this->ResultLookup = new unsigned int[this->ResultLookupSize];
	} else {
		this->Lexicon->IO->WriteToLog(0, false, "Error: Condition ReferenceSize <= 0\n\n");
		abort();
	}
	// Initialise to -1 for fail
	for (unsigned int j = 0; j < this->ResultLookupSize; j++) {
		this->ResultLookup[j] = UNDEFINED;
	}

	// Walk through all of the inputs (again) setting the conditions and the result

	// Reset the cursors
	for (int i = 0; i < this->NumInputs; i++) {
		this->Cursor[i] = 0;
	}

	// Load all the permutations
	Lowiii = 0;
	do {

		// Clear each of the terms in the buffer
		this->ClearVariables(Lowiii);

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
			if (!this->LoadInput(iii)) {
				LookupValue[iii] = UNDEFINED;
				break;
			}
		}

		// Did the last full permutation pass or was it a failure
		if ((iii == this->NumInputs) && (LookupValue[iii - 1] != UNDEFINED)) {

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
					this->ConditionLookup[i][LookupIndex[iii]] = UNDEFINED;
				} else {
					// Test the condition to see if its in the negative
					if ((this->ConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
						this->ConditionLookup[i][LookupIndex[iii]] = this->ConditionID(i);
						// A value of -1 is an automatic pass
					} else {
						this->ConditionLookup[i][LookupIndex[iii]] = this->ConditionID(i);
						if (this->ConditionLookup[i][LookupIndex[iii]] == UNDEFINED) PreviousFailed = true;
					}
				}
			}

			// Construct the result lookup
			this->ResultLookup[LookupIndex[iii]] = this->ResultID();

		}

		// Advance the inputs
		if (iii == this->NumInputs) iii = this->NumInputs - 1;
	} while (this->AdvanceInput(&Lowiii, iii));

	// Print the condition tables
	for (int i = 0; i < this->NumConditions; i++) {
		this->Lexicon->IO->FormatToLog(3, true, "Condition %u   Size = %d\n", i, this->ConditionLookupSize[i]);
		if (this->Lexicon->IO->Parameters->LogDetail > 3) {
			Count = 0;
			for (unsigned int j = 0; j < this->ConditionLookupSize[i]; j++) {
				if (this->ConditionLookup[i][j] >= 0) Count++;
				this->Lexicon->IO->FormatToLog(0, true, "    %5lu: %5d\n", j, this->ConditionLookup[i][j]);
			}
			this->Lexicon->IO->FormatToLog(0, true, "    Count = %d\n", Count);
		}
	}

	// Print the result table
	this->Lexicon->IO->FormatToLog(3, true, "Result   Size = %u\n", this->ResultLookupSize);
	if (this->Lexicon->IO->Parameters->LogDetail > 3) {
		Count = 0;
		for (unsigned int j = 0; j < this->ResultLookupSize; j++) {
			if (this->ResultLookup[j] >= 0) Count++;
			this->Lexicon->IO->FormatToLog(0, true, "    %5lu: %5lu\n", j, this->ResultLookup[j]);
		}
		this->Lexicon->IO->FormatToLog(0, true, "    Count = %d\n", Count);
	}

	// Collect the stats
	this->Lexicon->IO->FormatToLog(3, true, "Lookup Size = %.0f bytes\n", this->LookupSize * sizeof(int));
	this->Lexicon->IO->Parameters->TotalLookupSize += this->LookupSize;

	// Cleanup
	delete[] LookupValue;
	delete[] LookupIndex;
	delete[] NextLookupValue;

}

//-----------------------------------------------------------------------------
// Execute
//-----------------------------------------------------------------------------
int hsfcRule::Execute(hsfcState* State){

	int InputIndex;
	bool LoadFailed;
	int LowInputIndex;
	int NewRelationCount;

	NewRelationCount = 0;

	// Check the preconditions
	if (!this->CheckPreConditions(State)) {
		return NewRelationCount;
	}

	// This may be a fully ground rule with no inputs; so we do it at least once
	// Initialise all of the input cursors; no inputs return true; empty input list return false
	if (!this->InitialiseInput(State)) {
		return NewRelationCount;
	}

	// Load all the permutations
	LowInputIndex = 0;
	do {

		// Load the buffer with a valid tuple and test if it is valid
		this->ClearVariables(LowInputIndex);
		// Load each input in turn
		LoadFailed = false;
		for (InputIndex = LowInputIndex; InputIndex < this->NumInputs; InputIndex++) {

			// Count the transactions performed
			this->Transactions++;

			// Load the buffer
			if (!this->LoadInput(InputIndex, State)) {
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
		if (this->CheckConditions(State)) {

			// Count the transactions performed
			this->Transactions++;

			// Post the result to the state
			if (this->PostResult(State)) NewRelationCount++;

			// Is the relation full
			if (State->NumRelations[this->Result] == State->MaxNumRelations[this->Result]) break;
		}

	// Advance the inputs
	} while (this->AdvanceInput(&LowInputIndex, InputIndex, State));

	return NewRelationCount;

}

//-----------------------------------------------------------------------------
// HighSpeedExecute
//-----------------------------------------------------------------------------
int hsfcRule::HighSpeedExecute(hsfcState* State){

	unsigned int Index;
	unsigned int Lookup[MAX_NO_OF_INPUTS];
	unsigned int ID;
	int InputNo;
	int LowInputIndex;
	int NewRelationCount;
	hsfcTuple Term;

	NewRelationCount = 0;

	// Rules for condition lookups
	// Distinct						-1 = fail	0 = pass
	// Not Distinct = !Distinct		-1 = pass	0 = fail
	// True							-1 = fail	0+ = lookup
	// Not True	= !True				-1 = pass	0+ = !lookup

	// Check each PreCondition
	for (int i = 0; i < this->NumPreConditions; i++) {
		// Get the PreCondition RelationID
		ID = this->PreConditionLookup[i][0];
		// Look up the PreCondition based on the function
		if ((this->PreConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot) {
			// Apply the condition in the negative
			if ((this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
				if (ID == 0) return 0;
			} else {
				if ((ID != UNDEFINED) && (State->RelationExists[this->PreCondition[i]][ID])) return 0;
			}
		} else {
			// Apply the condition
			if ((this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
				if (ID == UNDEFINED) return 0;
			} else {
				if ((ID == UNDEFINED) || (!State->RelationExists[this->PreCondition[i]][ID])) return 0;
			}
		}
	}

	// Set up the cursors on the Input lists
	for (int i = 0; i < this->NumInputs; i++) {
		// Initialise each list
		this->Cursor[i] = 0;
		// If any lists are empty then exit
		if (State->NumRelations[this->Input[i]] == 0) return 0;
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
			if (Lookup[i] == UNDEFINED) {
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
					if ((ID != UNDEFINED) && (State->RelationExists[this->Condition[i]][ID])) goto NextCombination;
				}
			} else {
				// Apply the condition
				if ((this->ConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct) {
					if (ID == UNDEFINED) goto NextCombination;
				} else {
					if ((ID == UNDEFINED) || (!State->RelationExists[this->Condition[i]][ID])) goto NextCombination;
				}
			}
		}

		// We have satisfied all the conditions
		// Get the result ID and add it to the list
		Term.ID = this->ResultLookup[Index];
		if (Term.ID != UNDEFINED) {
			//Does the relation already exist on the list
			Term.Index = this->Result;
			if (this->StateManager->AddRelation(State, Term)) NewRelationCount++;
			if (State->NumRelations[this->Result] == State->MaxNumRelations[this->Result]) return NewRelationCount;
		}

NextCombination:
		// Increment the cursors
		for (LowInputIndex = InputNo; LowInputIndex >= 0; LowInputIndex--) {

			// Is the cursor at the end of the list
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
		if (LowInputIndex < 0) return NewRelationCount;

	}

}

//-----------------------------------------------------------------------------
// Test
//-----------------------------------------------------------------------------
int hsfcRule::Test(hsfcState* State){

	int InputIndex;
	bool LoadFailed;
	int LowInputIndex;
	int TestTransactions;
	int NewRelationCount;

	TestTransactions = 0;
	this->Transactions = 0;
	NewRelationCount = 0;

	printf("\nPreconditions -------------------------------------------------\n");

	// Check the preconditions
	if (!this->CheckPreConditions(State)) {
		for (int i = 0; i < this->Transactions; i++) {
			printf("%u.%u\n", this->PreConditionCalculator[i].Value.Index, this->PreConditionCalculator[i].Value.ID);
		}
		TestTransactions += this->Transactions;
		this->Transactions = 0;
		printf("Fail\n");
		printf("\nTransactions = %d\n", TestTransactions);
		printf("--------------------------------------------------------------\n");
		return NewRelationCount;
	}

	for (int i = 0; i < this->Transactions; i++) {
		printf("%u.%u\n", this->PreConditionCalculator[i].Value.Index, this->PreConditionCalculator[i].Value.ID);
	}
	TestTransactions += this->Transactions;
	this->Transactions = 0;
	printf("\nExecution ----------------------------------------------------\n");

	// This may be a fully ground rule with no inputs; so we do it at least once
	// Initialise all of the input cursors; no inputs return true; empty input list return false
	if (!this->InitialiseInput(State)) {
		printf("No Inputs\n");
		printf("\nTransactions = %d\n", TestTransactions);
		printf("--------------------------------------------------------------\n");
		return NewRelationCount;
	}

	// Load all the permutations
	LowInputIndex = 0;
	do {

		// Load the buffer with a valid tuple and test if it is valid
		this->ClearVariables(LowInputIndex);
		for (int i = 0; i < LowInputIndex; i++) {
			printf("%u.%u\t", this->InputCalculator[i].Value.Index, this->InputCalculator[i].Value.ID);
		}
		this->Transactions = 0;

		// Load each input in turn
		LoadFailed = false;
		for (InputIndex = LowInputIndex; InputIndex < this->NumInputs; InputIndex++) {

			// Count the transactions performed
			this->Transactions++;

			// Load the buffer
			if (!this->LoadInput(InputIndex, State)) {
				printf("%u.%u\t", this->InputCalculator[InputIndex].Value.Index, this->InputCalculator[InputIndex].Value.ID);
				printf("Fail\n");
				LoadFailed = true;
				break;
			}
			printf("%u.%u\t", this->InputCalculator[InputIndex].Value.Index, this->InputCalculator[InputIndex].Value.ID);

		}
		TestTransactions += this->Transactions;
		this->Transactions = 0;

		// InputIndex points to the Input that needs to be inrecemtned 
		// for the next permutation
		// If a load fails early, InputIndex shows where
		if (LoadFailed) continue;
		InputIndex = this->NumInputs - 1;

		printf("<>\t");
		// Check the conditions
		if (this->CheckConditions(State)) {

			for (int i = 0; i < this->Transactions; i++) {
				printf("%u.%u\t", this->ConditionCalculator[i].Value.Index, this->ConditionCalculator[i].Value.ID);
			}
			TestTransactions += this->Transactions;
			this->Transactions = 0;

			// Count the transactions performed
			this->Transactions++;

			// Post the result to the state
			if (this->PostResult(State)) NewRelationCount++;
			printf("==\t%u.%u\n", this->ResultCalculator.Value.Index, this->ResultCalculator.Value.ID);
			TestTransactions += this->Transactions;
			this->Transactions = 0;

			// Is the relation full
			if (State->NumRelations[this->Result] == State->MaxNumRelations[this->Result]) break;
		} else {
			for (int i = 0; i < this->Transactions; i++) {
				printf("%u.%u\t", this->ConditionCalculator[i].Value.Index, this->ConditionCalculator[i].Value.ID);
			}
			TestTransactions += this->Transactions;
			this->Transactions = 0;
			printf("Fail\n");
		}

	// Advance the inputs
	} while (this->AdvanceInput(&LowInputIndex, InputIndex, State));

	printf("\nTransactions = %d\n", TestTransactions);
	printf("--------------------------------------------------------------\n");

	return NewRelationCount;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRule::Print(bool ResetVariables) {

	// Print the rule
	this->RuleSchema->Print();

	// Print the calculators
	this->Lexicon->IO->WriteToLog(0, false, "-------------------------------------------------------------\n");
	this->Lexicon->IO->WriteToLog(0, false, "#\tTerm\t\t\tRelat.\tLink\tVar.\tFixed\n");
	this->PrintCalculator(this->ResultCalculator, ResetVariables);
	for (int i = 0; i < this->NumPreConditions; i++) this->PrintCalculator(this->PreConditionCalculator[i], ResetVariables);
	for (int i = 0; i < this->NumInputs; i++) this->PrintCalculator(this->InputCalculator[i], ResetVariables);
	for (int i = 0; i < this->NumConditions; i++) this->PrintCalculator(this->ConditionCalculator[i], ResetVariables);
	this->Lexicon->IO->WriteToLog(0, false, "-------------------------------------------------------------\n");

}

//-----------------------------------------------------------------------------
// ClearRule
//-----------------------------------------------------------------------------
void hsfcRule::ClearRule() {

	// Free the resources
	if (this->Cursor != NULL) {
		delete[](this->Cursor);
		this->Cursor = NULL;
	}
	if (this->Variable != NULL) {
		delete[](this->Variable);
		this->Variable = NULL;
	}

	if (this->ResultCalculator.Term != NULL) {
		delete[](this->ResultCalculator.Term);
		delete[](this->ResultCalculator.Link);
		delete[](this->ResultCalculator.Relation);
		delete[](this->ResultCalculator.Variable);
		delete[](this->ResultCalculator.Fixed);
	}
	this->ResultCalculator.Term = NULL;
	if (this->ResultLookup != NULL) {
		delete[](this->ResultLookup);
		this->ResultLookup = NULL;
	}

	if (this->Input != NULL) {
		delete[](this->Input);
		this->Input = NULL;
		for (int i = 0; i < this->NumInputs; i++) {
			delete[](this->InputCalculator[i].Term);
			delete[](this->InputCalculator[i].Link);
			delete[](this->InputCalculator[i].Relation);
			delete[](this->InputCalculator[i].Variable);
			delete[](this->InputCalculator[i].Fixed);
		}
		delete[](this->InputCalculator);
	}

	if (this->InputLookup != NULL) {
		for (int i = 0; i < this->NumInputs; i++) {
			if (this->InputLookup[i] != NULL) {
				delete[](this->InputLookup[i]);
			}
		}
		delete[](this->InputLookup);
		this->InputLookup = NULL;
	}

	if (this->Condition != NULL) {
		delete[](this->Condition);
		this->Condition = NULL;
		delete[](this->ConditionFunction);
		for (int i = 0; i < this->NumConditions; i++) {
			delete[](this->ConditionCalculator[i].Term);
			delete[](this->ConditionCalculator[i].Link);
			delete[](this->ConditionCalculator[i].Relation);
			delete[](this->ConditionCalculator[i].Variable);
			delete[](this->ConditionCalculator[i].Fixed);
		}
		delete[](this->ConditionCalculator);
	}

	if (this->ConditionLookup != NULL) {
		for (int i = 0; i < this->NumConditions; i++) {
			if (this->ConditionLookup[i] != NULL) {
				delete[](this->ConditionLookup[i]);
			}
		}
		delete[](this->ConditionLookup);
		this->ConditionLookup = NULL;
	}

	if (this->PreCondition != NULL) {
		delete[](this->PreCondition);
		this->PreCondition = NULL;
		delete[](this->PreConditionFunction);
		for (int i = 0; i < this->NumPreConditions; i++) {
			delete[](this->PreConditionCalculator[i].Term);
			delete[](this->PreConditionCalculator[i].Link);
			delete[](this->PreConditionCalculator[i].Relation);
			delete[](this->PreConditionCalculator[i].Variable);
			delete[](this->PreConditionCalculator[i].Fixed);
		}
		delete[](this->PreConditionCalculator);
	}

	if (this->PreConditionLookup != NULL) {
		for (int i = 0; i < this->NumPreConditions; i++) {
			if (this->PreConditionLookup[i] != NULL) {
				delete[](this->PreConditionLookup[i]);
			}
		}
		delete[](this->PreConditionLookup);
		this->PreConditionLookup = NULL;
	}


}
	
//-----------------------------------------------------------------------------
// BuildCalculator
//-----------------------------------------------------------------------------
void hsfcRule::BuildCalculator(hsfcRuleRelationSchema* RuleRelationSchema, hsfcCalculator* Calculator) {

	vector<unsigned int> TermIndex;
	unsigned int LinkIndex;
	unsigned int RelationIndex;
	unsigned int VariableIndex;
	unsigned int FixedIndex;
	unsigned int NewRelationIndex;
	vector<unsigned int> RelationArity;
	vector<unsigned int> ArgumentIndex;  // Not the same as TermSchema[i].ArgumentIndex
	vector<unsigned int> CurrentRelation;

	// Reset calculator
	Calculator->Value.Index = 0;
	Calculator->Value.ID = 0;
	Calculator->TermSize = -1;
	Calculator->LinkSize = 0;
	Calculator->RelationSize = 0;
	Calculator->VariableSize = 0;
	Calculator->FixedSize = 0;
	for (unsigned int i = 0; i < RuleRelationSchema->TermSchema.size(); i++) {
		Calculator->TermSize++;
		if (RuleRelationSchema->TermSchema[i].EmbeddedIndex != UNDEFINED) {
			Calculator->TermSize++;
			Calculator->LinkSize++;
			Calculator->RelationSize++;
		}
		if (RuleRelationSchema->TermSchema[i].VariableIndex != UNDEFINED) {
			Calculator->VariableSize++;
		}
		if (RuleRelationSchema->TermSchema[i].Type == hsfcTypeFixed) {
			Calculator->FixedSize++;
		}
	}
	Calculator->Term = new hsfcTuple[Calculator->TermSize];
	Calculator->Link = new hsfcReference[Calculator->LinkSize];
	Calculator->Relation = new hsfcReference[Calculator->RelationSize];
	Calculator->Variable = new hsfcReference[Calculator->VariableSize];
	Calculator->Fixed = new hsfcReference[Calculator->FixedSize];

	// Set up the calculator: relations first
	TermIndex.clear(); 
	LinkIndex = 0;
	RelationIndex = 0;
	VariableIndex = 0;
	FixedIndex = 0;
	NewRelationIndex = 0;
	RelationArity.clear();
	CurrentRelation.clear();
	ArgumentIndex.clear();

	// Initialise the calculator
	CurrentRelation.push_back(0);
	RelationArity.push_back(RuleRelationSchema->TermSchema[0].Arity);

	Calculator->Term[0].Index = RuleRelationSchema->TermSchema[0].Term.Index; 
	Calculator->Term[0].ID = RuleRelationSchema->TermSchema[0].Term.ID;
	TermIndex.push_back(0);
	ArgumentIndex.push_back(0);
	Calculator->Relation[0].SourceIndex = 0;												// Offset for the start of embedded[]
	Calculator->Relation[0].DestinationIndex = RuleRelationSchema->TermSchema[0].EmbeddedIndex;		// Relation Index
	RelationIndex++;
	Calculator->Link[0].SourceIndex = 0;			// Offset for the term in parent[]
	Calculator->Link[0].DestinationIndex = 0;		// Offset for the start of embedded[]
	LinkIndex++;

	TermIndex[CurrentRelation[0]]++;
	NewRelationIndex += RuleRelationSchema->TermSchema[0].Arity + 1;

	// Every term is either fixed, variable, or embedded
	for (unsigned int i = 1; i < RuleRelationSchema->TermSchema.size(); i++) {

		// Is this the start of a relation
		if (RuleRelationSchema->TermSchema[i].Type == hsfcTypeRelation) {

			// Start with the linkages from current terms index
			Calculator->Link[LinkIndex].SourceIndex = TermIndex[CurrentRelation[0]];			// Offset for the term in parent[]
			Calculator->Link[LinkIndex].DestinationIndex = NewRelationIndex;		// Offset for the start of embedded[]
			LinkIndex++;

			// Use the predicate as a placeholder
			Calculator->Term[TermIndex[CurrentRelation[0]]].Index = RuleRelationSchema->TermSchema[i].Term.Index; 
			Calculator->Term[TermIndex[CurrentRelation[0]]].ID = RuleRelationSchema->TermSchema[i].Term.ID;
			ArgumentIndex[CurrentRelation[0]]++;

			// Set the new term index
			TermIndex.push_back(NewRelationIndex);
			NewRelationIndex += RuleRelationSchema->TermSchema[i].Arity + 1;

			// Update the current relation
			RelationArity.push_back(RuleRelationSchema->TermSchema[i].Arity);
			CurrentRelation.insert(CurrentRelation.begin(), RelationIndex);

			// Set the relation
			Calculator->Relation[RelationIndex].SourceIndex = TermIndex[CurrentRelation[0]];				// Offset for the start of embedded[]
			Calculator->Relation[RelationIndex].DestinationIndex = RuleRelationSchema->TermSchema[i].EmbeddedIndex;		// Relation Index
			RelationIndex++;

			// Set the predicate in the new relation
			Calculator->Term[TermIndex[CurrentRelation[0]]].Index = RuleRelationSchema->TermSchema[i].Term.Index; 
			Calculator->Term[TermIndex[CurrentRelation[0]]].ID = RuleRelationSchema->TermSchema[i].Term.ID;
			ArgumentIndex.push_back(0);

		} else {

			// This is just a term: fixed or variable
			Calculator->Term[TermIndex[CurrentRelation[0]]].Index = RuleRelationSchema->TermSchema[i].Term.Index; 
			Calculator->Term[TermIndex[CurrentRelation[0]]].ID = RuleRelationSchema->TermSchema[i].Term.ID;
			ArgumentIndex[CurrentRelation[0]]++;
			
			// Is it a variable
			if (RuleRelationSchema->TermSchema[i].Type == hsfcTypeVariable) {
				Calculator->Variable[VariableIndex].SourceIndex = TermIndex[CurrentRelation[0]];							// Offset to the variable in term[]
				Calculator->Variable[VariableIndex].DestinationIndex = RuleRelationSchema->TermSchema[i].VariableIndex;		// Variable number
				VariableIndex++;
			}

			// Is it fixed
			if (RuleRelationSchema->TermSchema[i].Type == hsfcTypeFixed) {
				Calculator->Fixed[FixedIndex].SourceIndex = TermIndex[CurrentRelation[0]];							// Offset to the variable in term[]
				Calculator->Fixed[FixedIndex].DestinationIndex = RuleRelationSchema->TermSchema[i].NameID;		// Variable number
				FixedIndex++;
			}

		}
		
		// Was this the last argument in the relation
		while ((CurrentRelation.size() > 0) && (ArgumentIndex[CurrentRelation[0]] == RelationArity[CurrentRelation[0]])) {
			CurrentRelation.erase(CurrentRelation.begin());
		}

		// Increment the term index
		if (CurrentRelation.size() > 0) TermIndex[CurrentRelation[0]]++;

	}

	// Test calculator; this destroys term values
	//this->TestCalculator(*Calculator);

}

//-----------------------------------------------------------------------------
// CalculateValue
//-----------------------------------------------------------------------------
bool hsfcRule::CalculateValue(hsfcCalculator& Calculator) {

	unsigned int ID;

	// This converts the terms in the calculator to a value according to the plan

	// Initialise the value
	Calculator.Value.Index = Calculator.Relation[0].DestinationIndex;
	Calculator.Value.ID = UNDEFINED;

	// Do an integrity check
	for (unsigned int i = 0; i < Calculator.VariableSize; i++) {
		if (Calculator.Term[Calculator.Variable[i].SourceIndex].ID == UNDEFINED) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: undefined variable in hsfcRule::CalculatorValue\n");
			return false;
		}
	}
	for (unsigned int i = 0; i < Calculator.FixedSize; i++) {
		if (Calculator.Term[Calculator.Fixed[i].SourceIndex].ID != Calculator.Fixed[i].DestinationIndex) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: wrong fixed term in hsfcRule::CalculatorValue\n");
			return false;
		}
	}

	// We need to work in reverse order; remember RelationSize is unsigned
	for (unsigned int i = Calculator.RelationSize - 1; i < Calculator.RelationSize; i--) {
		// Get the ID from the domain manager
		// A return of false means the terms are out of the domain range for the relation
		if (!this->DomainManager->TermsToID(Calculator.Relation[i].DestinationIndex, &Calculator.Term[Calculator.Relation[i].SourceIndex], ID)) { 
			return false;
		}
		// Load the ID
		if (i == 0) {
			Calculator.Value.Index = Calculator.Relation[i].DestinationIndex;
			Calculator.Value.ID = ID;
		} else {
			Calculator.Term[Calculator.Link[i].SourceIndex].Index = Calculator.Relation[i].DestinationIndex;
			Calculator.Term[Calculator.Link[i].SourceIndex].ID = ID;
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// CalculatorTerms
//-----------------------------------------------------------------------------
bool hsfcRule::CalculateTerms(hsfcCalculator& Calculator) {

	unsigned int ID;

	// This converts the value in the calculator to terms according to the plan

	// Do an integrity check
	for (unsigned int i = 0; i < Calculator.VariableSize; i++) {
		if (Calculator.Term[Calculator.Variable[i].SourceIndex].ID == UNDEFINED) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: undefined variable in hsfcRule::CalculatorValue\n");
			return false;
		}
	}

	// Expand all of the relations
	for (unsigned int i = 0; i < Calculator.RelationSize; i++) {
		// Get the terms from the domain manager
		if (i == 0) {
			// Integrity check
			if (Calculator.Value.Index != Calculator.Relation[i].DestinationIndex) {
				return false;
			}
			// Load the ID
			ID = Calculator.Value.ID;
		} else {
			// Integrity check
			if (Calculator.Term[Calculator.Link[i].SourceIndex].Index != Calculator.Relation[i].DestinationIndex) {
				return false;
			}
			// Load the ID
			ID = Calculator.Term[Calculator.Link[i].SourceIndex].ID;
		}
		// A return of false means the ID is out of the domain range for the relation
		if (!this->DomainManager->IDToTerms(Calculator.Relation[i].DestinationIndex, &Calculator.Term[Calculator.Relation[i].SourceIndex], ID)) { 
			return false;
		}
	}

	// Check all the fixed terms
	for (unsigned int i = 0; i < Calculator.FixedSize; i++) {
		// Is the index the Lexicon
		if (Calculator.Term[Calculator.Fixed[i].SourceIndex].Index != 0) {
			return false;
		}
		// Is the ID
		if (Calculator.Term[Calculator.Fixed[i].SourceIndex].ID != Calculator.Fixed[i].DestinationIndex) {
			return false;
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// TestCalculator
//-----------------------------------------------------------------------------
void hsfcRule::TestCalculator(hsfcCalculator& Calculator){

	unsigned int RelationIndex;
	unsigned int ID1;
	unsigned int ID2;

	RelationIndex = Calculator.Relation[0].DestinationIndex;

	//this->PrintCalculator(Calculator);

	for (unsigned int i = 0; i < this->DomainManager->Domain[RelationIndex].IDCount; i++) {
		ID1 = i;
		Calculator.Value.Index = RelationIndex;
		Calculator.Value.ID = ID1;

		this->CalculateTerms(Calculator);
		//this->PrintCalculator(Calculator);
		
		this->CalculateValue(Calculator);
		//this->PrintCalculator(Calculator);
		
		ID2 = Calculator.Value.ID;
		if (ID1 != ID2) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: conversion failure in hsfcRule::TestCalculator\n");
			return; 
		}
	}

}

//-----------------------------------------------------------------------------
// PrintCalculator
//-----------------------------------------------------------------------------
void hsfcRule::PrintCalculator(hsfcCalculator& Calculator, bool ResetVariables){

	this->Lexicon->IO->WriteToLog(0, false, "-------------------------------------------------------------\n");
	this->Lexicon->IO->FormatToLog(0, false, "Value %u.%u\n", Calculator.Value.Index, Calculator.Value.ID);

	// Clear the variable terms
	if (ResetVariables) {
		for (unsigned int i = 0; i < Calculator.VariableSize; i++) {
			Calculator.Term[Calculator.Variable[i].SourceIndex].Index = 0;
			Calculator.Term[Calculator.Variable[i].SourceIndex].ID = 0;
		}
	}
	
	// Print the terms
	for (unsigned int i = 0; i < Calculator.TermSize; i++) {
		this->Lexicon->IO->FormatToLog(0, false, "%u", i);
		if (Calculator.Term[i].Index == 0) {
			if (Calculator.Term[i].ID == 0) {
				this->Lexicon->IO->FormatToLog(0, false, "\t%-18s", "?");
			} else {
				this->Lexicon->IO->FormatToLog(0, false, "\t%-18s", this->Lexicon->Text(Calculator.Term[i].ID));
			}
		} else {
			this->Lexicon->IO->FormatToLog(0, false, "\t%u.%u", Calculator.Term[i].Index, Calculator.Term[i].ID);
		}
		if (i < Calculator.RelationSize) {
			this->Lexicon->IO->FormatToLog(0, false, "\t%u.%u", Calculator.Relation[i].SourceIndex, Calculator.Relation[i].DestinationIndex);
		} else {
			this->Lexicon->IO->WriteToLog(0, false, "\t");
		}
		if (i < Calculator.LinkSize) {
			this->Lexicon->IO->FormatToLog(0, false, "\t%u.%u", Calculator.Link[i].SourceIndex, Calculator.Link[i].DestinationIndex);
		} else {
			this->Lexicon->IO->WriteToLog(0, false, "\t");
		}
		if (i < Calculator.VariableSize) {
			this->Lexicon->IO->FormatToLog(0, false, "\t%u.%u", Calculator.Variable[i].SourceIndex, Calculator.Variable[i].DestinationIndex);
		} else {
			this->Lexicon->IO->WriteToLog(0, false, "\t");
		}
		if (i < Calculator.FixedSize) {
			this->Lexicon->IO->FormatToLog(0, false, "\t%d.%s", Calculator.Fixed[i].SourceIndex, this->Lexicon->Text(Calculator.Fixed[i].DestinationIndex));
		} else {
			this->Lexicon->IO->WriteToLog(0, false, "\t");
		}
		this->Lexicon->IO->WriteToLog(0, false, "\n");
	}

}

//-----------------------------------------------------------------------------
// ClearVariables
//-----------------------------------------------------------------------------
void hsfcRule::ClearVariables(int LowInputIndex) {

	// Clear each of the Terms in the buffer
	for (int i = 0; i < this->VariableSize; i++) {
		if (this->Variable[i].InputIndex >= LowInputIndex) {
			this->Variable[i].Index = UNDEFINED;
			this->Variable[i].ID = UNDEFINED;
		}
	}

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

//-----------------------------------------------------------------------------
// LoadInput
//-----------------------------------------------------------------------------
bool hsfcRule::LoadInput(int InputIndex, hsfcState* State) { 

	unsigned int VariableIndex;
	unsigned int TermIndex;

	// Get the input value from the state
	this->InputCalculator[InputIndex].Value.Index = this->Input[InputIndex];
	this->InputCalculator[InputIndex].Value.ID = State->RelationID[this->Input[InputIndex]][this->Cursor[InputIndex]];

	// Calculate the terms in the input
	if (!this->CalculateTerms(this->InputCalculator[InputIndex])) {
		return false;
	}
	
	// Check all of the variables in the buffer
	for (unsigned int i = 0; i < this->InputCalculator[InputIndex].VariableSize; i++) {
		TermIndex = this->InputCalculator[InputIndex].Variable[i].SourceIndex;
		VariableIndex = this->InputCalculator[InputIndex].Variable[i].DestinationIndex;
		// Is the variable already loaded
		if (this->Variable[VariableIndex].ID == UNDEFINED) {
			this->Variable[VariableIndex].Index = this->InputCalculator[InputIndex].Term[TermIndex].Index;
			this->Variable[VariableIndex].ID = this->InputCalculator[InputIndex].Term[TermIndex].ID;
		} else {
			if (this->Variable[VariableIndex].Index != this->InputCalculator[InputIndex].Term[TermIndex].Index) return false;
			if (this->Variable[VariableIndex].ID != this->InputCalculator[InputIndex].Term[TermIndex].ID) return false;
		}
	}

	return true;

}

//--- Overload ----------------------------------------------------------------
bool hsfcRule::LoadInput(int InputIndex) { 

	unsigned int VariableIndex;
	unsigned int TermIndex;

	// Get the input value from the state
	this->InputCalculator[InputIndex].Value.Index = this->Input[InputIndex];
	this->InputCalculator[InputIndex].Value.ID = this->Cursor[InputIndex];

	// Calculate the terms in the input
	if (!this->CalculateTerms(this->InputCalculator[InputIndex])) {
		return false;
	}
	
	// Check all of the variables in the buffer
	for (unsigned int i = 0; i < this->InputCalculator[InputIndex].VariableSize; i++) {
		TermIndex = this->InputCalculator[InputIndex].Variable[i].SourceIndex;
		VariableIndex = this->InputCalculator[InputIndex].Variable[i].DestinationIndex;
		// Is the variable already loaded
		if (this->Variable[VariableIndex].ID == UNDEFINED) {
			this->Variable[VariableIndex].Index = this->InputCalculator[InputIndex].Term[TermIndex].Index;
			this->Variable[VariableIndex].ID = this->InputCalculator[InputIndex].Term[TermIndex].ID;
		} else {
			if (this->Variable[VariableIndex].Index != this->InputCalculator[InputIndex].Term[TermIndex].Index) return false;
			if (this->Variable[VariableIndex].ID != this->InputCalculator[InputIndex].Term[TermIndex].ID) return false;
		}
	}

	return true;


}

//-----------------------------------------------------------------------------
// PreConditionID
//-----------------------------------------------------------------------------
unsigned int hsfcRule::PreConditionID(int Index){

	bool Distinct;
	bool Not;
	bool Equal;

	// Rules for condition lookups
	// Distinct						-1 = fail	0 = pass
	// Not Distinct = !Distinct		-1 = pass	0 = fail
	// True							-1 = fail	0+ = lookup
	// Not True	= !True				-1 = pass	0+ = !lookup

	// Set the type of condition
	Not = (this->PreConditionFunction[Index] & hsfcFunctionNot) == hsfcFunctionNot;
	Distinct = (this->PreConditionFunction[Index] & hsfcFunctionDistinct) == hsfcFunctionDistinct;

	// Calculate the value; out of range returns false 
	if (!this->CalculateValue(this->PreConditionCalculator[Index])) {
		return UNDEFINED;
	}

	// Is it a (distinct x y)
	if (Distinct) {
		Equal = ((this->PreConditionCalculator[Index].Term[1].Index == this->PreConditionCalculator[Index].Term[2].Index) && (this->PreConditionCalculator[Index].Term[1].ID == this->PreConditionCalculator[Index].Term[2].ID));
		if ((!Not) && (Equal)) {
			return UNDEFINED;
		}
		if ((Not) && (!Equal)) {
			return UNDEFINED;
		}
		return 0;
	}

	// Do an integrity check
	if (this->PreConditionCalculator[Index].Value.ID > this->DomainManager->Domain[this->PreConditionCalculator[Index].Value.Index].IDCount) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: PreCondition out of range\n");
		abort();
	}

	return this->PreConditionCalculator[Index].Value.ID;

}

//-----------------------------------------------------------------------------
// CheckPreConditions
//-----------------------------------------------------------------------------
bool hsfcRule::CheckPreConditions(hsfcState* State) { 
	
	hsfcTuple Term;
	bool Distinct;
	bool Not;
	bool Exists;

	// Check each of the PreCondition
	for (int i = 0; i < this->NumPreConditions; i++) {

		// Count the transactions performed
		this->Transactions++;

		// Set the type of condition
		Not = (this->PreConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot;
		Distinct = (this->PreConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct;

		// Get the ID of the condition
		Term.Index = this->PreCondition[i];
		Term.ID = this->PreConditionID(i);
		if ((!Not) && (Term.ID == UNDEFINED)) return false;

		// Is it a (distinct x y); any negation is resolved in the call to PreConditionID
		if (Distinct) {
			if (Term.ID == UNDEFINED) return false;
			continue;
		}

		// Check to see if the relation exists
		Exists = ((Term.ID != UNDEFINED) && this->StateManager->RelationExists(State, Term));
		if ((Not) && (Exists)) {
			return false;
		}
		if ((!Not) && (!Exists)) {
			return false;
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// ConditionID
//-----------------------------------------------------------------------------
unsigned int hsfcRule::ConditionID(int Index){

	unsigned int VariableIndex;
	unsigned int TermIndex;
	bool Distinct;
	bool Not;
	bool Equal;

	// Rules for condition lookups
	// Distinct						-1 = fail	0 = pass
	// Not Distinct = !Distinct		-1 = pass	0 = fail
	// True							-1 = fail	0+ = lookup
	// Not True	= !True				-1 = pass	0+ = !lookup

	// Set the type of condition
	Not = (this->ConditionFunction[Index] & hsfcFunctionNot) == hsfcFunctionNot;
	Distinct = (this->ConditionFunction[Index] & hsfcFunctionDistinct) == hsfcFunctionDistinct;

	// Load the variables into the calculator
	for (unsigned int i = 0; i < this->ConditionCalculator[Index].VariableSize; i++) {
		TermIndex = this->ConditionCalculator[Index].Variable[i].SourceIndex;
		VariableIndex = this->ConditionCalculator[Index].Variable[i].DestinationIndex;
		// Load the variable
		this->ConditionCalculator[Index].Term[TermIndex].Index = this->Variable[VariableIndex].Index;
		this->ConditionCalculator[Index].Term[TermIndex].ID = this->Variable[VariableIndex].ID;
	}

	// Calculate the value; out of range returns false 
	if (!this->CalculateValue(this->ConditionCalculator[Index])) {
		return UNDEFINED;
	}

	// Is it a (distinct x y)
	if (Distinct) {
		Equal = ((this->ConditionCalculator[Index].Term[1].Index == this->ConditionCalculator[Index].Term[2].Index) && (this->ConditionCalculator[Index].Term[1].ID == this->ConditionCalculator[Index].Term[2].ID));
		if ((!Not) && (Equal)) {
			return UNDEFINED;
		}
		if ((Not) && (!Equal)) {
			return UNDEFINED;
		}
		return 0;
	}

	// Do an integrity check
	if (this->ConditionCalculator[Index].Value.ID > this->DomainManager->Domain[this->ConditionCalculator[Index].Value.Index].IDCount) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: Condition out of range\n");
		abort();
	}

	return this->ConditionCalculator[Index].Value.ID;

}

//-----------------------------------------------------------------------------
// CheckConditions
//-----------------------------------------------------------------------------
bool hsfcRule::CheckConditions(hsfcState* State) { 
	
	hsfcTuple Term;
	bool Distinct;
	bool Not;
	bool Exists;

	// Check each of the Condition
	for (int i = 0; i < this->NumConditions; i++) {

		// Count the transactions performed
		this->Transactions++;

		// Set the type of condition
		Not = (this->ConditionFunction[i] & hsfcFunctionNot) == hsfcFunctionNot;
		Distinct = (this->ConditionFunction[i] & hsfcFunctionDistinct) == hsfcFunctionDistinct;

		// Get the ID of the condition
		Term.Index = this->Condition[i];
		Term.ID = this->ConditionID(i);
		if ((!Not) && (Term.ID == UNDEFINED)) return false;

		// Is it a (distinct x y); any negation is resolved in the call to PreConditionID
		if (Distinct) {
			if (Term.ID == UNDEFINED) return false;
			continue;
		}

		// Check to see if the relation exists
		Exists = ((Term.ID != UNDEFINED) && this->StateManager->RelationExists(State, Term));
		if ((Not) && (Exists)) {
			return false;
		}
		if ((!Not) && (!Exists)) {
			return false;
		}

	}

	return true;

}

//-----------------------------------------------------------------------------
// ResultID
//-----------------------------------------------------------------------------
unsigned int hsfcRule::ResultID(){

	unsigned int VariableIndex;
	unsigned int TermIndex;

	// Load the variables into the calculator
	for (unsigned int i = 0; i < this->ResultCalculator.VariableSize; i++) {
		TermIndex = this->ResultCalculator.Variable[i].SourceIndex;
		VariableIndex = this->ResultCalculator.Variable[i].DestinationIndex;
		// Load the variable
		this->ResultCalculator.Term[TermIndex].Index = this->Variable[VariableIndex].Index;
		this->ResultCalculator.Term[TermIndex].ID = this->Variable[VariableIndex].ID;
	}

	// Calculate the value; out of range returns false 
	if (!this->CalculateValue(this->ResultCalculator)) {
		return UNDEFINED;
	}
	// Do an integrity check
	if (this->ResultCalculator.Value.ID > this->DomainManager->Domain[this->ResultCalculator.Value.Index].IDCount) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: Result out of range\n");
		abort();
	}

	return this->ResultCalculator.Value.ID;

}

//-----------------------------------------------------------------------------
// PostResult
//-----------------------------------------------------------------------------
bool hsfcRule::PostResult(hsfcState* State) { 
	
	hsfcTuple Term;

	// Get the ID of the condition
	Term.Index = this->Result;
	Term.ID = this->ResultID();
	
	// Check it is a valid result
	if (Term.ID == UNDEFINED) {
		this->Lexicon->IO->WriteToLog(0, false, "Warning: Undefined result\n");
		return false;
	}
	// Check it is a valid result
	if (Term.ID > this->DomainManager->Domain[Term.Index].IDCount) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: Out of range result\n");
		return false;
	}

	// Add it to the state
	return this->StateManager->AddRelation(State, Term);

}


//=============================================================================
// CLASS: hsfcStratum
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcStratum::hsfcStratum(hsfcLexicon* Lexicon, hsfcStateManager* StateManager, hsfcDomainManager* DomainManager){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->StateManager = StateManager;
	this->DomainManager = DomainManager;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcStratum::~hsfcStratum(void){

	// Free the resources
	this->DeleteRules();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcStratum::Initialise(){

	// Create the Schema
	this->DeleteRules();
	this->SelfReferenceCount = 0;
	this->MultiPass = false;
	this->IsRigid = false;
	this->LookupSize = 0;

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcStratum::Create(hsfcStratumSchema* StratumSchema, bool LowSpeedOnly) {

	hsfcRule* NewRule;
	int RuleSRC;

	// Initialise count
	RuleSRC = 0;

	// Create the strata
	for (unsigned int i = 0; i < StratumSchema->RuleSchema.size(); i++) {

		// Create the new Rule
		NewRule = new hsfcRule(this->Lexicon, this->StateManager, this->DomainManager);
		NewRule->Initialise();
		NewRule->FromSchema(StratumSchema->RuleSchema[i], LowSpeedOnly);
		this->Rule.push_back(NewRule);
		RuleSRC += NewRule->SelfReferenceCount;

	}

	// Is this a multipass rule
	this->SelfReferenceCount = StratumSchema->SelfReferenceCount;
	this->MultiPass = ((this->SelfReferenceCount > 1) || (RuleSRC > 1));

	// Is this stratum rigid
	this->IsRigid = (StratumSchema->Rigidity == hsfcRigidityFull);
	this->Type = StratumSchema->Type;

	return true;

}

//-----------------------------------------------------------------------------
// CreateLookupTables
//-----------------------------------------------------------------------------
void hsfcStratum::CreateLookupTables() {

	this->LookupSize = 0;

	// Create lookup tables for each rule
	for (unsigned int i = 0; i < this->Rule.size(); i++) {

		this->Lexicon->IO->FormatToLog(3, true, "      Rule %u\n", i);

		// Set the rule properties
		this->Rule[i]->LookupSize = 0;

		// Create lookups
		if (this->Rule[i]->LowSpeed) {
			this->Lexicon->IO->WriteToLog(3, true, "        LowSpeedExecution\n");
		} else {
			this->Lexicon->IO->LogIndent = 10;
			this->Rule[i]->CreateLookupTable();
			this->Lexicon->IO->LogIndent = 2;
		}

		// Calculate the lookup size
		this->LookupSize += this->Rule[i]->LookupSize;

	}

}

//-----------------------------------------------------------------------------
// ExecuteRules
//-----------------------------------------------------------------------------
void hsfcStratum::ExecuteRules(hsfcState* State, bool LowSpeed) {

	int NewRelationCount;
	int RuleRelationCount;

	// Execute all the rules at least once
	do {

		// Execute the rules
		NewRelationCount = 0;
		for (unsigned int i = 0; i < this->Rule.size(); i++) {
			// execute the rule at least once
			do {
				if ((this->Rule[i]->LowSpeed) || LowSpeed) {
					RuleRelationCount = this->Rule[i]->Execute(State);
				} else {
					RuleRelationCount = this->Rule[i]->HighSpeedExecute(State);
				}
				NewRelationCount += RuleRelationCount;
			} while ((this->Rule[i]->SelfReferenceCount > 1) && (RuleRelationCount > 0));
		}

	} while ((this->MultiPass) && (NewRelationCount > 0));

}

//-----------------------------------------------------------------------------
// TestRules
//-----------------------------------------------------------------------------
void hsfcStratum::TestRules(hsfcState* State) {

	int NewRelationCount;
	int RuleRelationCount;

	// Execute all the rules at least once
	do {

		// Execute the rules
		NewRelationCount = 0;
		for (unsigned int i = 0; i < this->Rule.size(); i++) {
			this->Rule[i]->Print(true);
			// execute the rule at least once
			do {
				RuleRelationCount = this->Rule[i]->Test(State);
				NewRelationCount += RuleRelationCount;
			} while ((this->Rule[i]->SelfReferenceCount > 1) && (RuleRelationCount > 0));
		}

	} while ((this->MultiPass) && (NewRelationCount > 0));

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcStratum::Print(){

	// Free the resources
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		this->Lexicon->IO->FormatToLog(0, true, "\nRule %u\n", i);
		this->Rule[i]->Print(true);
	}

}





//-----------------------------------------------------------------------------
// DeleteRules
//-----------------------------------------------------------------------------
void hsfcStratum::DeleteRules(){

	// Free the resources
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete this->Rule[i];
	}
	this->Rule.clear();

}


//=============================================================================
// CLASS: hsfcRulesEngine
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRulesEngine::hsfcRulesEngine(hsfcLexicon* Lexicon, hsfcStateManager* StateManager, hsfcDomainManager* DomainManager){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->StateManager = StateManager;
	this->DomainManager = DomainManager;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcRulesEngine::~hsfcRulesEngine(void){

	// Free the resources
	this->DeleteStrata();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRulesEngine::Initialise(){

	// Free the resources
	this->DeleteStrata();

	// Clear the steps
	for (unsigned int i = 0; i < this->Step.size(); i++) {
		this->Step[i].clear();
	}
	this->Step.clear();
	this->LookupSize = 0;

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcRulesEngine::Create(hsfcSchema* Schema, bool LowSpeedOnly) {

	hsfcStratum* NewStratum;

	this->Lexicon->IO->LogIndent = 2;
	this->Lexicon->IO->WriteToLog(2, true, "Creating Engine ...\n");

	// Record the schema
	this->Schema = Schema;

	// Create the strata
	for (unsigned int i = 0; i < Schema->StratumSchema.size(); i++) {

		// Create the new stratum
		NewStratum = new hsfcStratum(this->Lexicon, this->StateManager, this->DomainManager);
		NewStratum->Initialise();
		if (!NewStratum->Create(Schema->StratumSchema[i], LowSpeedOnly)) return false;
		this->Stratum.push_back(NewStratum);

	}

	// Set stratum execution properties
	this->Lexicon->IO->LogIndent = 2;
	this->Lexicon->IO->WriteToLog(2, true, "  Set Stratum Properties\n");
	this->SetStratumProperties();

	// Calculate all of the rigid facts
	this->Lexicon->IO->LogIndent = 2;
	this->Lexicon->IO->WriteToLog(2, true, "  Calculate Rigids\n");
	if (!this->CalculateRigids()) return false;

	// Optimise the rule inputs
	this->Lexicon->IO->LogIndent = 2;
	this->Lexicon->IO->WriteToLog(2, true, "  Optimise Rule Inputs\n");
	this->OptimiseRuleInputs();

	// Create the lookup tables
	this->Lexicon->IO->LogIndent = 2;
	this->Lexicon->IO->WriteToLog(2, true, "  Creating Lookup Tables\n");
	this->CreateLookupTables();

	// Free the state
	this->StateManager->FreeState(this->State);

	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");

	return true;

}

//-----------------------------------------------------------------------------
// SetInitialState
//-----------------------------------------------------------------------------
void hsfcRulesEngine::SetInitialState(hsfcState* State) {

	// Set the initial state
	this->StateManager->SetInitialState(State);

}

//-----------------------------------------------------------------------------
// AdvanceState
//-----------------------------------------------------------------------------
void hsfcRulesEngine::AdvanceState(hsfcState* State, int Step, bool LowSpeed) {

	int NextStep;
	int NoSteps;

	// Step
	// 1 = Terminal Rules 
	// 2 = Legal Rules
	// 3 = Sees Rules
	// 4 = Next Rules
	// 5 = Goal Rules (processed on requirement)

	// Is it the goal relations
	if (Step == 5) {
		this->ProcessRules(State, 5, LowSpeed, false);
		return;
	}

	// Correct for possible Goal calculations
	if (State->CurrentStep == 5)  State->CurrentStep = 1;

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
			this->ProcessRules(State, NextStep, LowSpeed, false);
		}

		// Update the current step
		State->CurrentStep = NextStep;

	}

}

//-----------------------------------------------------------------------------
// IsTerminal
//-----------------------------------------------------------------------------
bool hsfcRulesEngine::IsTerminal(hsfcState* State) {

	// Assumes the states is at Step 1 or greater

	// Look for a terminal relation
	if (State->NumRelations[this->StateManager->TerminalRelationIndex] > 0) {
		return true;
	}
	return false;

}

//-----------------------------------------------------------------------------
// GoalValue
//-----------------------------------------------------------------------------
int hsfcRulesEngine::GoalValue(hsfcState* State, int RoleIndex) {

	int Result;
	hsfcTuple Term[3];
	int Value;
	hsfcTuple Tuple;
	int RelationRoleIndex;
	unsigned int NumRoles;
	unsigned int NumRelations;

	Result = 0;

	// Assumes the states is at Step 1 or greater
	// Execute the goal rules
	if (State->CurrentStep != 5) this->ProcessRules(State, 5, false, false);

	// Initialise 
	NumRoles = this->DomainManager->Domain[this->StateManager->GoalRelationIndex].Size[0];
	NumRelations = State->NumRelations[this->StateManager->GoalRelationIndex];

	// Go through the Goal relations
	for (unsigned int i = 0; i < NumRelations; i++) {
		// Get the terms for the goal relation
		Tuple.Index = this->StateManager->GoalRelationIndex;
		Tuple.ID = State->RelationID[this->StateManager->GoalRelationIndex][i];
		// Is it the correct role
		RelationRoleIndex = this->StateManager->GoalToRole[Tuple.ID % NumRoles];
		// Is this the right role
		if (RelationRoleIndex == RoleIndex) {
			this->DomainManager->IDToTerms(Tuple.Index, Term, Tuple.ID);
			Value = atoi(this->Lexicon->Text(Term[2].ID));
			if (Value > Result) Result = Value;
		}
	}
		
	return Result;

}

//-----------------------------------------------------------------------------
// GetLegalMoves
//-----------------------------------------------------------------------------
void hsfcRulesEngine::GetLegalMoves(hsfcState* State, vector< vector<hsfcLegalMove> >& LegalMove) {

	hsfcLegalMove NewLegalMove;
	unsigned int RoleIndex;
	unsigned int NumRoles;
	unsigned int NumLegalRoles;
	unsigned int NumMoves;
	unsigned int RelationID;

	// Assumes the states is at Step 2 and not terminal

	// Get the number of arguments and roles
	NumRoles = this->DomainManager->Domain[this->StateManager->RoleRelationIndex].Size[0];
	NumLegalRoles = this->DomainManager->Domain[this->StateManager->LegalRelationIndex].Size[0];
	NumMoves = State->NumRelations[this->StateManager->LegalRelationIndex];

	// Resize the vector
	LegalMove.resize(NumRoles);

	// Clear the legal moves
	for (unsigned int i = 0; i < LegalMove.size(); i++) {
		LegalMove[i].clear();
	}

	// Go through all of the legal moves
	for (unsigned int i = 0; i < NumMoves; i++) {

		// Who is the move for
		RelationID = State->RelationID[this->StateManager->LegalRelationIndex][i];
		RoleIndex = this->StateManager->LegalToRole[RelationID % NumLegalRoles];
		// Sort the move according to role
		if (RoleIndex != UNDEFINED) {
			// Create the new legal move
			NewLegalMove.RoleIndex = RoleIndex;
			NewLegalMove.Tuple.Index = this->StateManager->DoesRelationIndex;
			NewLegalMove.Tuple.ID = RelationID;
			NewLegalMove.Text = NULL;
			LegalMove[RoleIndex].push_back(NewLegalMove);
		}

	}

}

//-----------------------------------------------------------------------------
// ChooseRandomMoves
//-----------------------------------------------------------------------------
void hsfcRulesEngine::ChooseRandomMoves(hsfcState* State) {

	vector<hsfcTuple> Move;
	hsfcTuple NewMove;
	int RandomNo;
	int RandomIndex;
	unsigned int RoleIndex;
	vector<int> NumMoves;
	unsigned int NumRoles;
	unsigned int NumLegalRoles;
	unsigned int NumRelations;
	unsigned int RelationID;

	// Choose randomly
	RandomNo = rand();

	// Get the number of arguments and roles
	NumRoles = this->DomainManager->Domain[this->StateManager->RoleRelationIndex].Size[0];
	NumLegalRoles = this->DomainManager->Domain[this->StateManager->LegalRelationIndex].Size[0];
	NumRelations = State->NumRelations[this->StateManager->LegalRelationIndex];

	// Go through all of the roles
	for (unsigned int i = 0; i < NumRoles; i++) {
		NumMoves.push_back(0);
		NewMove.ID = 0;
		NewMove.Index = this->StateManager->DoesRelationIndex;
		Move.push_back(NewMove);
	}

	// Go through all of the legal moves
	for (unsigned int i = 0; i < NumRelations; i++) {

		// Who is the move for
		RelationID = State->RelationID[this->StateManager->LegalRelationIndex][i];
		RoleIndex = this->StateManager->LegalToRole[RelationID % NumLegalRoles];
		if (RoleIndex != UNDEFINED) {
			NumMoves[RoleIndex]++;
			// Calculate the chance of this move being selected
			RandomIndex = (RandomNo / (RoleIndex + 1)) % NumMoves[RoleIndex];
			if (RandomIndex == 0) {
				Move[RoleIndex].ID = RelationID;
				Move[RoleIndex].Index = this->StateManager->DoesRelationIndex;
			}
		}
	}

	// Choose a move for each role
	for (unsigned int i = 0; i < NumRoles; i++) {
		// Is there a move to make
		if (NumMoves[i] == 0) {
			this->Lexicon->IO->FormatToLog(0, false, "Error: No moves for role %u\n", i);
		}
		// Make the chosen move
		this->StateManager->AddRelation(State, Move[i]);
	}

	// Clear the moves;
	Move.clear();
	NumMoves.clear();

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRulesEngine::Print() {

	this->Lexicon->IO->WriteToLog(0, true, "\n--- Rules Engine ---\n");
	this->Lexicon->IO->LogIndent = 0;

	// Print the strats
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		this->Lexicon->IO->FormatToLog(0, true, "\nStratum %u\n", i);
		this->Stratum[i]->Print();
	}
	
}




	
	
	
//-----------------------------------------------------------------------------
// ProcessRules
//-----------------------------------------------------------------------------
void hsfcRulesEngine::ProcessRules(hsfcState* State, int Step, bool LowSpeed, bool ProcessRigids) {

	// Step
	// 1 = Terminal Rules 
	// 2 = Legal Rules
	// 3 = Sees Rules
	// 4 = Next Rules
	// 5 = Goal Rules (processed on requirement)

	bool ForceLowSpeed = this->Lexicon->IO->Parameters->LowSpeedOnly || LowSpeed;
	
	// Go through all of the rules
	for (int i = this->FirstStratumIndex[Step]; i <= this->LastStratumIndex[Step]; i++) {
		if (ProcessRigids || (!this->Stratum[i]->IsRigid)) {
			this->Stratum[i]->ExecuteRules(State, ForceLowSpeed);
		}
	}
	
}

//-----------------------------------------------------------------------------
// DeleteStarata
//-----------------------------------------------------------------------------
void hsfcRulesEngine::DeleteStrata(){

	// Free the resources
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		delete this->Stratum[i];
	}
	this->Stratum.clear();

}

//-----------------------------------------------------------------------------
// SetStratumProperties
//-----------------------------------------------------------------------------
void hsfcRulesEngine::SetStratumProperties(){

	// Set the steps
	for (int i = 0; i < 6; i++) {
		this->FirstStratumIndex[i] = -1;
		this->LastStratumIndex[i] = -1;
	}

	// Find the first last Termial stratum
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		if (this->Stratum[i]->Type == hsfcStratumTerminal) this->LastStratumIndex[1] = i;
		if (this->Stratum[i]->Type == hsfcStratumGoal) this->LastStratumIndex[5] = i;
		if (this->Stratum[i]->Type == hsfcStratumLegal) this->LastStratumIndex[2] = i;
		if (this->Stratum[i]->Type == hsfcStratumSees) this->LastStratumIndex[3] = i;
		if (this->Stratum[i]->Type == hsfcStratumNext) this->LastStratumIndex[4] = i;
	}

	// Set remaining properties
	this->FirstStratumIndex[0] = 0;
	this->LastStratumIndex[0] = this->Stratum.size() - 1;
	this->FirstStratumIndex[1] = 0;
	if (this->LastStratumIndex[1] == -1) this->LastStratumIndex[1] = -1;
	this->FirstStratumIndex[5] = this->LastStratumIndex[1] + 1;
	if (this->LastStratumIndex[5] == -1) this->LastStratumIndex[5] = this->LastStratumIndex[1];
	this->FirstStratumIndex[2] = this->LastStratumIndex[5] + 1;
	if (this->LastStratumIndex[2] == -1) this->LastStratumIndex[2] = this->LastStratumIndex[5];
	this->FirstStratumIndex[3] = this->LastStratumIndex[2] + 1;
	if (this->LastStratumIndex[3] == -1) this->LastStratumIndex[3] = this->LastStratumIndex[2];
	this->FirstStratumIndex[4] = this->LastStratumIndex[3] + 1;
	if (this->LastStratumIndex[4] == -1) this->LastStratumIndex[4] = this->LastStratumIndex[3];

	// Print the steps
	if (this->Lexicon->IO->Parameters->LogDetail > 2) {
		for (unsigned int i = 0; i < 6; i++) {
			this->Lexicon->IO->FormatToLog(3, true, "    Step %d", i);
			this->Lexicon->IO->FormatToLog(3, true, "    %u to %d\n", this->FirstStratumIndex[i], this->LastStratumIndex[i]);
		}
	}

}
//-----------------------------------------------------------------------------
// CalculateRigids
//-----------------------------------------------------------------------------
bool hsfcRulesEngine::CalculateRigids(){

	hsfcTuple Term[MAX_RELATION_ARITY];
	hsfcTuple Relation;
	hsfcRelationSchema* RelationSchema;

	// Set up the internal state
	this->State = this->StateManager->CreateState();
	// Initialise the state
	this->StateManager->InitialiseState(this->State);
	this->StateManager->SetInitialState(this->State); // Will be empty

	if (this->Lexicon->IO->Parameters->LogDetail > 3) this->StateManager->PrintRelations(this->State, true);

	// Load all of the permanent instances into the schema
	for (unsigned int i = 0; i < this->Schema->Rigid.size(); i++) {

		// Convert the SCL to a relation instance
		if (!this->DomainManager->LoadTerms(this->Schema->Rigid[i], Term)) return false;
		Relation.Index = this->Lexicon->RelationIndex(Term[0].ID);
		if (Relation.Index == UNDEFINED) return false;

		// Find the relation schema
		RelationSchema = this->Schema->RelationSchema[Relation.Index];

		// Is this in the state, or only embedded
		// Include the (init ...) as they may be calculated by a rigid rule
		if (RelationSchema->IsInState) {
			// Convert to id
			if (!this->DomainManager->TermsToID(Relation.Index, Term, Relation.ID)) return false;
			// Add the relation instance to the schema
			this->StateManager->AddRelation(this->State, Relation);
		}

	}

	// Calculate all of the rigids
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		// Is this a rigid
		if (this->Stratum[i]->IsRigid) {
			this->Stratum[i]->ExecuteRules(this->State, true);
		}
	}

	if (this->Lexicon->IO->Parameters->LogDetail > 2) this->StateManager->PrintRelations(this->State, true);

	// Delete all the existing permanents and rebuild
	// This must be done in a very strict secuence
	// Rebuild Schema and Domain
	this->StateManager->CreateRigids(this->State);
	this->StateManager->CreatePermanents(this->State);
	// Rebuild StateManager
	this->StateManager->FreeState(this->State);
	this->StateManager->SetSchema(this->Schema);
	this->State = this->StateManager->CreateState();
	this->StateManager->InitialiseState(this->State);
	if (this->Lexicon->IO->Parameters->LogDetail > 2) this->StateManager->PrintRelations(this->State, true);

	this->StateManager->SetInitialState(this->State);
	if (this->Lexicon->IO->Parameters->LogDetail > 2) this->StateManager->PrintRelations(this->State, true);

	return true;

}

//-----------------------------------------------------------------------------
// OptimiseRules
//-----------------------------------------------------------------------------
void hsfcRulesEngine::OptimiseRuleInputs() {

	time_t Start;
	int Count;

	// This must be done in the Schema as well as there is a direct correlation 
	// between the Schema and the Engine in terms of rule relation ordering

	// Run the game through many playouts to get the frequency data for the relations
	this->State = this->StateManager->CreateState();
	this->StateManager->InitialiseState(this->State);

	// Reset the statistics
	for (unsigned int i = 1; i < this->Schema->RelationSchema.size(); i++) {
		this->Schema->RelationSchema[i]->Statistics.Initialise();
	}

	// Play out some number of games
	Start = clock();
	Count = 0;
	while ((Count < 1000) && (clock() < Start + 3 * TICKS_PER_SECOND)) {

		// Set the initial state
		this->StateManager->SetInitialState(this->State);
		this->AdvanceState(this->State, 1, true);
		// Play until the game is terminal
		while ((!this->IsTerminal(this->State)) && (clock() < Start + 3 * TICKS_PER_SECOND)) {
 
			// Advance to calculate all the legal moves
			this->AdvanceState(this->State, 2, true);
			this->ChooseRandomMoves(this->State);

			// Record the statistics
			this->AdvanceState(this->State, 4, true);
			for (unsigned int i = 1; i < this->Schema->RelationSchema.size(); i++) {
				this->Schema->RelationSchema[i]->Statistics.AddObservation((double)this->State->NumRelations[i]);
			}
			Count ++;

			// Advance to the next state and calculate terminal tuple
			this->AdvanceState(this->State, 1, true);
		}
	}

	// Process each stratum
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {
		// Optimise each rule
		for (unsigned int j = 0; j < this->Stratum[i]->Rule.size(); j++) {
			// Optimise
			if (!this->Stratum[i]->IsRigid)	this->Stratum[i]->Rule[j]->OptimiseInputs(this->Schema);
		}
	}

}

//-----------------------------------------------------------------------------
// CreateLookupTables
//-----------------------------------------------------------------------------
void hsfcRulesEngine::CreateLookupTables() {

	// Provess each stratum
	for (unsigned int i = 0; i < this->Stratum.size(); i++) {

		// Initialise the lookup size
		this->Stratum[i]->LookupSize = 0;

		this->Lexicon->IO->FormatToLog(3, true, "    Stratum %u\n", i);
		this->Stratum[i]->CreateLookupTables();

		// Calculate the lookup size
		this->LookupSize += this->Stratum[i]->LookupSize;

	}

	this->Lexicon->IO->FormatToLog(2, true, "  Total Lookup size = %.0f bytes\n", this->LookupSize);

}
















