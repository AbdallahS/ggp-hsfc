//=============================================================================
// Project: High Speed Forward Chaining
// Module: Engine
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include "hsfcEngine.h"
#include "hsfc_config.h"

using namespace std;

//=============================================================================
// CLASS: hsfcEngine
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcEngine::hsfcEngine(hsfcLexicon* Lexicon, hsfcStateManager* StateManager){

	// Allocate the memory
	this->Lexicon = Lexicon;
	this->StateManager = StateManager;
	this->Grinder = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcEngine::~hsfcEngine(void){

	// Free the resources
	if (this->Grinder != NULL) {
		if (this->Grinder != NULL) delete(this->Grinder);
	}
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		//if (this->Rule[i] != NULL) delete(this->Rule[i]);
		//this->Rule[i] = NULL;
	}
	this->Rule.clear();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcEngine::Initialise(){

	// Create the Schema
	if (this->Grinder == NULL) {
		this->Grinder = new hsfcGrinder(this->Lexicon, this->StateManager);
	}
	this->Grinder->Initialise();

	// Clear the rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		if (this->Rule[i] != NULL) delete(this->Rule[i]);
		this->Rule[i] = NULL;
	}
	this->Rule.clear();

	// Set properties
	this->GoalRelation = NULL;
	this->TimeGrind = 0;
	this->TimeLowSpeedRun = 0;
	this->TimeReadGDL = 0;
	this->TimeOptimise = 0;
	this->ReferenceSize = 0;

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcEngine::Create(char* Script, hsfcGDLParamaters& Paramaters) {

	bool CreatedGDL;

	// Initialise everything
	this->Initialise();

	// Create the Schema
	this->StartClock = clock();
	CreatedGDL = this->Grinder->Create(Script, Paramaters.MaxRelationSize, Paramaters.MaxReferenceSize);
	this->StopClock = clock();
	this->TimeReadGDL = (float)(this->StopClock - this->StartClock) / (float)CLOCKS_PER_SEC; 
	if (!CreatedGDL) return false;

	if (DEBUG) this->Grinder->Print();

	// Calculate the state size
	if (!this->StateManager->CalculateStateSize()) Paramaters.ReadGDLOnly = true;

	// Was this read the GDL only
	if (Paramaters.ReadGDLOnly) return true;

	this->TimeOptimise = 0;
	this->TimeLowSpeedRun = 0;

	// Conduct low speed playouts to get statistics
	this->StartClock = clock();
	this->Grinder->Optimise();
	this->StopClock = clock();
	this->TimeLowSpeedRun = (float)(this->StopClock - this->StartClock) / (float)CLOCKS_PER_SEC; 

	// Optimise the Schema
	this->StartClock = clock();
	this->Grinder->Engine->OptimiseRules(Paramaters.OrderRules);
	this->StopClock = clock();
	this->TimeOptimise = (float)(this->StopClock - this->StartClock) / (float)CLOCKS_PER_SEC; 

	// Was this to calculate the Schema only
	if (Paramaters.SchemaOnly) return true;

	// Optimise the rule based on the statistics
	this->StartClock = clock();
	this->Grinder->Engine->GrindRules();
	this->StopClock = clock();
	this->TimeGrind = (float)(this->StopClock - this->StartClock) / (float)CLOCKS_PER_SEC; 

	// Calculate the reference size
	this->ReferenceSize = 0;
	for (unsigned int i = 0; i < this->Grinder->Engine->Rule.size(); i++) {
		this->ReferenceSize += this->Grinder->Engine->Rule[i]->ReferenceSize;
	}

	// Copy the rule pointers
	for (unsigned int i = 0; i < this->Grinder->Engine->Rule.size(); i++) {
		this->Rule.push_back(this->Grinder->Engine->Rule[i]);
	}

	// Set up the starting point for the rule execution
	for (int i = 0; i < 6; i++) {
		this->FirstRuleIndex[i] = this->Grinder->Engine->FirstRuleIndex[i];
		this->LastRuleIndex[i] = this->Grinder->Engine->LastRuleIndex[i];
	}

	// Set up the goal relation
	this->GoalRelation = this->Grinder->Engine->Schema->Relation[this->StateManager->GoalRelationIndex];

	return true;

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcEngine::CreateFromFile(const char* FileName, hsfcGDLParamaters& Paramaters) {

	int Length;
	char Letter;
	FILE* InputFile;
	int FileSize;
	char* Buffer;
	bool Result;

	// Open the input file
	InputFile = fopen(FileName, "r");
	if (InputFile == NULL) {
		printf("Error: GDL file does not exist\n%s\n", FileName);
		abort();
	}

    // Find the filesize
    fseek(InputFile, 0, SEEK_END);
    FileSize = ftell(InputFile);
    rewind(InputFile);

    // Load the file into memory
    Buffer = (char*) malloc (sizeof(char)*FileSize);
	// Read one character at a time
	Length = 0;
	while (!feof(InputFile)) {
		// Read a letter from the file
		fscanf(InputFile, "%c", &Letter);
		// Ignore control characters
		if ((Letter < ' ') || (Letter > '~')) Letter = ' ';
		// Ignore multiple spaces
		if ((Letter != ' ') || ((Length != 0) && (Buffer[Length - 1] != ' '))) {
			Buffer[Length] = Letter;
			Length++;
		}
	}
	Buffer[Length] = 0;
    fclose(InputFile);

	// Create the hsfc engine
	Result = this->Create(Buffer, Paramaters);

	// Clean up
	delete[] Buffer;

	return Result;

}

//-----------------------------------------------------------------------------
// PlayOut
//-----------------------------------------------------------------------------
void hsfcEngine::PlayOut(hsfcState* State) {

	// Play out the current state and get the goal vlaues

	// Advance to step 1 and test for terminal
	this->AdvanceState(State, 1);

	// Playout until terminal
	while (!this->IsTerminal(State)) {

		// Choose some random moves
		this->AdvanceState(State, 2);
		this->ChooseRandomMoves(State);

		// Action the moves and calcuilate the next state
		this->AdvanceState(State, 4);
		// Advance to the next state and test for terminal
		this->AdvanceState(State, 1);

	}

	// Get the goal values
	this->AdvanceState(State, 5);

}

//-----------------------------------------------------------------------------
// Audit
//-----------------------------------------------------------------------------
void hsfcEngine::Audit() {

	hsfcState* LowSpeedState;
	hsfcState* HighSpeedState;
	int ID;

	// Create states
	LowSpeedState = this->StateManager->CreateState();
	HighSpeedState = this->StateManager->CreateState();

	// Initialise both states
	this->StateManager->InitialiseState(LowSpeedState);
	this->StateManager->SetInitialState(LowSpeedState);
	this->StateManager->InitialiseState(HighSpeedState);
	this->StateManager->SetInitialState(HighSpeedState);

	// Print the initial states
	this->StateManager->PrintRelations(LowSpeedState, false);
	this->StateManager->PrintRelations(HighSpeedState, false);

	// Play the game
	while (true) {

		// Process all of the rules up to terminal
		printf("--- terminal ---\n");
		for (int i = this->FirstRuleIndex[1]; i <= this->LastRuleIndex[1]; i++) {
			printf("%3d =>", i);
			this->Rule[i]->Execute(LowSpeedState, true);
			printf("\n");
			printf("%3d =>", i);
			this->Rule[i]->HighSpeedAudit(HighSpeedState);
			printf("\n\n");
		}

		if (this->IsTerminal(LowSpeedState)) {
			this->StateManager->PrintRelations(LowSpeedState, false);
			this->StateManager->PrintRelations(HighSpeedState, false);
			break;
		}

		// Process all of the rules up to goal
		printf("--- goal ---\n");
		for (int i = this->FirstRuleIndex[5]; i <= this->LastRuleIndex[5]; i++) {
			printf("%3d =>", i);
			this->Rule[i]->Execute(LowSpeedState, true);
			printf("\n");
			printf("%3d =>", i);
			this->Rule[i]->HighSpeedAudit(HighSpeedState);
			printf("\n\n");
		}

		// Process all of the rules up to legal
		printf("--- legal ---\n");
		for (int i = this->FirstRuleIndex[2]; i <= this->LastRuleIndex[2]; i++) {
			printf("%3d =>", i);
			this->Rule[i]->Execute(LowSpeedState, true);
			printf("\n");
			printf("%3d =>", i);
			this->Rule[i]->HighSpeedAudit(HighSpeedState);
			printf("\n\n");
		}

		// Choose a random move; the same moe in both states
		this->ChooseRandomMoves(LowSpeedState);
		for (int i = 0; i < LowSpeedState->NumRelations[this->StateManager->DoesRelationIndex]; i++) {
			ID = LowSpeedState->RelationID[this->StateManager->DoesRelationIndex][i];
			HighSpeedState->RelationID[this->StateManager->DoesRelationIndex][i] = ID;
			HighSpeedState->RelationExists[this->StateManager->DoesRelationIndex][ID] = true;
		}
		HighSpeedState->NumRelations[this->StateManager->DoesRelationIndex] = LowSpeedState->NumRelations[this->StateManager->DoesRelationIndex];

		// Process all of the rules up to sees
		printf("--- sees ---\n");
		for (int i = this->FirstRuleIndex[3]; i <= this->LastRuleIndex[3]; i++) {
			printf("%3d =>", i);
			this->Rule[i]->Execute(LowSpeedState, true);
			printf("\n");
			printf("%3d =>", i);
			this->Rule[i]->HighSpeedAudit(HighSpeedState);
			printf("\n\n");
		}

		// Process all of the rules up to next
		printf("--- next ---\n");
		for (int i = this->FirstRuleIndex[4]; i <= this->LastRuleIndex[4]; i++) {
			printf("%3d =>", i);
			this->Rule[i]->Execute(LowSpeedState, true);
			printf("\n");
			printf("%3d =>", i);
			this->Rule[i]->HighSpeedAudit(HighSpeedState);
			printf("\n\n");
		}

		// Print the initial states
		this->StateManager->CompareStates(LowSpeedState, HighSpeedState);

		// Go to the next state
		this->StateManager->NextState(LowSpeedState);
		this->StateManager->NextState(HighSpeedState);

		// Print the initial states
		this->StateManager->CompareStates(LowSpeedState, HighSpeedState);

	}

	// Free the states
	this->StateManager->FreeState(LowSpeedState);
	this->StateManager->FreeState(HighSpeedState);


}

//-----------------------------------------------------------------------------
// SetInitialState
//-----------------------------------------------------------------------------
void hsfcEngine::SetInitialState(hsfcState* State) {

	// Set the initial state
	this->StateManager->SetInitialState(State);

}

//-----------------------------------------------------------------------------
// AdvanceState
//-----------------------------------------------------------------------------
void hsfcEngine::AdvanceState(hsfcState* State, int Step) {

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
void hsfcEngine::ProcessRules(hsfcState* State, int Step) {

	// Step
	// 1 = Terminal Rules 
	// 2 = Legal Rules
	// 3 = Sees Rules
	// 4 = Next Rules
	// 5 = Goal Rules (processed on requirement)
	
	// Go through all of the rules
	for (int i = this->FirstRuleIndex[Step]; i <= this->LastRuleIndex[Step]; i++) {
		if (this->Rule[i]->LowSpeed) {
			this->Rule[i]->Execute(State, false);
		} else {
			this->Rule[i]->HighSpeedExecute(State);
		}
	}
	
}

//-----------------------------------------------------------------------------
// IsTerminal
//-----------------------------------------------------------------------------
bool hsfcEngine::IsTerminal(hsfcState* State) {

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
void hsfcEngine::GetLegalMoves(hsfcState* State, vector<hsfcTuple>& Move) {

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
void hsfcEngine::ChooseRandomMoves(hsfcState* State) {

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

//-----------------------------------------------------------------------------
// GoalValue
//-----------------------------------------------------------------------------
int hsfcEngine::GoalValue(hsfcState* State, int RoleIndex){

	int Result;
	vector<hsfcRuleTerm> Term;
	int Value;
	int RelationID;
	int RelationRoleIndex;

	Result = 0;

	// Assumes the states is at Step 1 or greater
	// Execute the goal rules
	if (State->CurrentStep != 5) this->ProcessRules(State, 5);

	// Go through the Goal relations
	for (int i = 0; i < State->NumRelations[this->StateManager->GoalRelationIndex]; i++) {
		RelationID = State->RelationID[this->StateManager->GoalRelationIndex][i];
		RelationRoleIndex = RelationID % State->NumRelations[this->StateManager->RoleRelationIndex];
		// Is this the right role
		if (RelationRoleIndex == RoleIndex) {
			this->GoalRelation->Terms(RelationID, Term);
			Value = atoi(this->Lexicon->Text(Term[2].Tuple.ID));
			if (Value > Result) Result = Value;
		}
	}

	return Result;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcEngine::Print(){

	this->Grinder->Print();

}

