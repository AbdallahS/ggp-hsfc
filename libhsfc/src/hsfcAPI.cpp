//=============================================================================
// Project: High Speed Forward Chaining
// Module: API
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include "hsfcAPI.h"

//=============================================================================
// CLASS: hsfcGDLManager
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGDLManager::hsfcGDLManager(void) {

	// Create the lexicon
	this->Lexicon = new hsfcLexicon();

	// Create the state manager
	this->StateManager = new hsfcStateManager(this->Lexicon);

	// Create the engine
	this->Engine = new hsfcEngine(this->Lexicon, this->StateManager);

	// Set default values
	this->NumRoles = 0;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGDLManager::~hsfcGDLManager(void) {

	// Free the resources
	delete this->Engine;
	delete this->StateManager;
	//delete this->Lexicon;

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
int hsfcGDLManager::Initialise(string* Script, hsfcGDLParamaters& Paramaters) {

	int Result = 0;

	// Initialise the engine
	this->Lexicon->Initialise();
	this->StateManager->Initialise();
	this->Engine->Initialise();

	// Create the schema from the gdl file
	this->Engine->Create(Script->c_str(), Paramaters);

	return Result;

}

//-----------------------------------------------------------------------------
// InitialiseFromFile
//-----------------------------------------------------------------------------
int hsfcGDLManager::InitialiseFromFile(string* GDLFileName, hsfcGDLParamaters& Paramaters) {

	int Result = 0;

	// Initialise the engine
	this->Lexicon->Initialise();
	this->StateManager->Initialise();
	this->Engine->Initialise();

	// Create the schema from the gdl file
	this->Engine->CreateFromFile(GDLFileName->c_str(), Paramaters);

	return Result;

}

//-----------------------------------------------------------------------------
// CreateGameState
//-----------------------------------------------------------------------------
int hsfcGDLManager::CreateGameState(hsfcState** GameState) {

	try {

		// Create the base pointer
		*GameState = this->StateManager->CreateState();

		// Set up the data storage and rigids
		this->StateManager->InitialiseState(*GameState);

		// Set to the initial game state
		this->StateManager->SetInitialState(*GameState);

		// Set the number of roles
		this->NumRoles = (*GameState)->NumRelations[this->StateManager->RoleRelationIndex];

		// TODO: Create a unique identified in GDLManager, and GameState so they can be matched

		return 0;

	}
	catch (int e) {

		cout << "CreateGameState::Exception " << e << endl;
		return 1;

	}

}

//-----------------------------------------------------------------------------
// FreeGameState
//-----------------------------------------------------------------------------
void hsfcGDLManager::FreeGameState(hsfcState* GameState) {

	try {

		// Free the memory
		this->StateManager->FreeState(GameState);

	}
	catch (int e) {

		cout << "FreeGameState::Exception " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// CopyGameState
//-----------------------------------------------------------------------------
void hsfcGDLManager::CopyGameState(hsfcState* Destination, hsfcState* Source) {

	try {

		// Create the base pointer
		this->StateManager->FromState(Destination, Source);

	}
	catch (int e) {

		cout << "CopyGameState::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// SetInitialGameState
//-----------------------------------------------------------------------------
void hsfcGDLManager::SetInitialGameState(hsfcState* GameState) {

	try {

		// Create the base pointer
		this->StateManager->SetInitialState(GameState);

	}
	catch (int e) {

		cout << "SetInitialGameState::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// GetLegalMoves
//-----------------------------------------------------------------------------
void hsfcGDLManager::GetLegalMoves(hsfcState* GameState, vector<hsfcLegalMove>& LegalMove) {

	hsfcLegalMove NewLegalMove;
	int RoleIndex;

	try {

		// Clear the legal moves
		LegalMove.clear();

		// Advance the state to create the terminal relation tuple
		if (GameState->CurrentStep < 1) this->Engine->AdvanceState(GameState, 1);
		if (this->Engine->IsTerminal(GameState)) return;

		// Advance the state to create the legal relation tuples
		if (GameState->CurrentStep < 2) this->Engine->AdvanceState(GameState, 2);

		// Go through all of the legal maoves
		for (int i = 0; i < GameState->NumRelations[this->StateManager->LegalRelationIndex]; i++) {

			// Who is the move for
			RoleIndex = GameState->RelationID[this->StateManager->LegalRelationIndex][i] % GameState->NumRelations[this->StateManager->RoleRelationIndex];
			// Create the new legal move
			NewLegalMove.RoleIndex = RoleIndex;
			NewLegalMove.Tuple.RelationIndex = this->StateManager->DoesRelationIndex;
			NewLegalMove.Tuple.ID = GameState->RelationID[this->StateManager->LegalRelationIndex][i];
			this->StateManager->Schema->RelationAsText(&(NewLegalMove.Tuple), NewLegalMove.Text);
			LegalMove.push_back(NewLegalMove);

		}

	}
	catch (int e) {

		cout << "GetLegalMoves::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// DoMove
//-----------------------------------------------------------------------------
void hsfcGDLManager::DoMove(hsfcState* GameState, vector<hsfcLegalMove>& DoesMove) {

	try {

		// The game step must be exactly after legal move tuples are calculated
		if (GameState->CurrentStep != 2) return;

		// Place the legal move tuples in the database
		for (unsigned int i = 0; i < DoesMove.size(); i++) {
			this->StateManager->AddRelation(GameState, &(DoesMove[i].Tuple));
		}

		// Advance the state to calculate the next tuples
		this->Engine->AdvanceState(GameState, 4);

		// Advance the state to the next state
		this->Engine->AdvanceState(GameState, 0);

	}
	catch (int e) {

		cout << "DoMove::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// IsTerminal
//-----------------------------------------------------------------------------
bool hsfcGDLManager::IsTerminal(hsfcState* GameState) {

	try {

		// Advance the state to create the terminal relation tuple
		if (GameState->CurrentStep < 1) this->Engine->AdvanceState(GameState, 1);
		return this->Engine->IsTerminal(GameState);

	}
	catch (int e) {

		cout << "IsTerminal::Exception: " << e << endl;
		return true;

	}

}

//-----------------------------------------------------------------------------
// GetGoalValues
//-----------------------------------------------------------------------------
void hsfcGDLManager::GetGoalValues(hsfcState* GameState, vector<int>& GoalValue) {

	int Value;

	try {

		// Clear the goal values
		GoalValue.clear();

		// Advance the state to create the terminal relation tuple
		if (GameState->CurrentStep < 1) this->Engine->AdvanceState(GameState, 1);

		// Return if the game is not terminal
		if (!this->Engine->IsTerminal(GameState)) return;

		// Go through all of the roles
		for (int i = 0; i < GameState->NumRelations[this->StateManager->RoleRelationIndex]; i++) {
			Value = this->Engine->GoalValue(GameState, i);
			GoalValue.push_back(Value);
		}

	}
	catch (int e) {

		cout << "GetGoalValues::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// PlayOut
//-----------------------------------------------------------------------------
void hsfcGDLManager::PlayOut(hsfcState* GameState, vector<int>& GoalValue) {

	int Value;

	try {

		// Clear the goal values
		GoalValue.clear();

		// Advance the state to create the terminal relation tuple
		if (GameState->CurrentStep < 1) this->Engine->AdvanceState(GameState, 1);

		// Play until the game is terminal
		while (!this->Engine->IsTerminal(GameState)) {

			// Advanc eto calculate all the legal moves
			this->Engine->AdvanceState(GameState, 2);

			// Get the legal move tuples
			this->Engine->ChooseRandomMoves(GameState);

			// Advance to the next state
			this->Engine->AdvanceState(GameState, 0);

			// Advance to calculate terminal tuple
			this->Engine->AdvanceState(GameState, 1);

		}

		// Go through all of the roles
		for (int i = 0; i < GameState->NumRelations[this->StateManager->RoleRelationIndex]; i++) {
			Value = this->Engine->GoalValue(GameState, i);
			GoalValue.push_back(Value);
		}

	}
	catch (int e) {

		cout << "PlayOut::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// GetRoles
//-----------------------------------------------------------------------------
void hsfcGDLManager::GetRoles(vector<string>& Role) {

	try {

		// Add in each of the roles 
		this->StateManager->Schema->GetRoles(Role);

	}
	catch (int e) {

		cout << "GetRoles::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// RelationAsKIF
//-----------------------------------------------------------------------------
char* hsfcGDLManager::RelationAsKIF(hsfcTuple& Tuple) {

	kpWFT KIF;
	char Text[256];
	char* Empty;

	try {

		// Create an empty string
		Empty = new char[1];
		Empty[0] = 0;

		// Convert relation to text
		// This call has a max string length of 250
		this->StateManager->Schema->RelationAsText(&Tuple, Text);

		// Initialise the KIF 
		KIF.Initialise("");
		KIF.LoadFlat(Text);

		// Return the KIF
		return KIF.AsText();

	}
	catch (int e) {

		cout << "RelationAsKIF::Exception: " << e << endl;
		return Empty;

	}

}

//-----------------------------------------------------------------------------
// GameStateAsText
//-----------------------------------------------------------------------------
char* hsfcGDLManager::GameStateAsText(hsfcState* GameState) {

	char* Empty;

	try {

		// Create an empty string
		Empty = new char[1];
		Empty[0] = 0;

		// Convert the state to text
		return this->StateManager->StateAsText(GameState);

	}
	catch (int e) {

		cout << "GameStateAsText::Exception: " << e << endl;
		return Empty;

	}

}

//-----------------------------------------------------------------------------
// GameStateFromText
//-----------------------------------------------------------------------------
bool hsfcGDLManager::GameStateFromText(hsfcState* GameState, char* Text) {

	try {

		// Convert the text to a state
		return this->StateManager->StateFromText(GameState, Text);

	}
	catch (int e) {

		cout << "GameStateFromText::Exception: " << e << endl;
		return false;

	}

}

//-----------------------------------------------------------------------------
// PrintState
//-----------------------------------------------------------------------------
void hsfcGDLManager::PrintState(hsfcState* GameState) {

	try {

		// Print the state
		this->StateManager->PrintRelations(GameState, false);

	}
	catch (int e) {

		cout << "PrintState::Exception: " << e << endl;

	}

}


