//=============================================================================
// Project: High Speed Forward Chaining
// Module: Grinder
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include "hsfcEngine.h"

using namespace std;

//=============================================================================
// CLASS: hsfcEngine
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcEngine::hsfcEngine(void){

	// Allocate the memory
	this->Lexicon = NULL;
	this->StateManager = NULL;
	this->DomainManager = NULL;
	this->Schema = NULL;
	this->RulesEngine = NULL;
	this->SCL = NULL;
	this->GDL = NULL;
	this->WFT = NULL;

	this->TextWFT = NULL;
	this->TextGDL = NULL;
	this->TextSCL = NULL;

	this->Parameters = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcEngine::~hsfcEngine(void){

	// Free the resources
	if (this->Lexicon != NULL) {
		delete this->Lexicon;
	}
	if (this->RulesEngine != NULL) {
		delete this->RulesEngine;
	}
	if (this->DomainManager != NULL) {
		delete this->DomainManager;
	}
	if (this->StateManager != NULL) {
		delete this->StateManager;
	}
	if (this->Schema != NULL) {
		delete this->Schema;
	}
	if (this->SCL != NULL) {
		delete this->SCL;
	}
	if (this->GDL != NULL) {
		delete this->GDL;
	}
	if (this->WFT != NULL) {
		delete this->WFT;
	}

	if (this->TextWFT != NULL) {
		delete this->TextWFT;
	}

	if (this->TextGDL != NULL) {
		delete this->TextGDL;
	}

	if (this->TextSCL != NULL) {
		delete this->TextSCL;
	}

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
bool hsfcEngine::Initialise(string* Script, hsfcParameters* Parameters) {

	try {

		// Free the resources
		if (this->Lexicon != NULL) {
			delete this->Lexicon;
		}
		if (this->RulesEngine != NULL) {
			delete this->RulesEngine;
		}
		if (this->DomainManager != NULL) {
			delete this->DomainManager;
		}
		if (this->StateManager != NULL) {
			delete this->StateManager;
		}
		if (this->Schema != NULL) {
			delete this->Schema;
		}
		if (this->SCL != NULL) {
			delete this->SCL;
		}
		if (this->GDL != NULL) {
			delete this->GDL;
		}
		if (this->WFT != NULL) {
			delete this->WFT;
		}

		if (this->TextWFT != NULL) {
			delete this->TextWFT;
		}

		if (this->TextGDL != NULL) {
			delete this->TextGDL;
		}

		if (this->TextSCL != NULL) {
			delete this->TextSCL;
		}

		// Create the lexicon
		this->Lexicon = new hsfcLexicon();
		this->Lexicon->Initialise(Parameters);
		this->Parameters = this->Lexicon->IO->Parameters;

		// Set properties
		this->NumRoles = 0;

		// Create the schema
		this->Schema = new hsfcSchema(this->Lexicon);
		this->Schema->Initialise();

		// Create the domain manager
		this->DomainManager = new hsfcDomainManager(this->Lexicon);
		this->DomainManager->Initialise();

		// Create the state manager
		this->StateManager = new hsfcStateManager(this->Lexicon, this->DomainManager);
		this->StateManager->Initialise();

		// Create the rules engine
		this->RulesEngine = new hsfcRulesEngine(this->Lexicon, this->StateManager, this->DomainManager);
		this->RulesEngine->Initialise();

		// Create the SCL
		this->SCL = new hsfcSCL(Lexicon);
		this->SCL->Initialise();

		// Create resources for converting test to relations
		this->TextWFT = new hsfcWFT(Lexicon);
		this->TextGDL = new hsfcGDL(Lexicon);
		this->TextSCL = new hsfcSCL(Lexicon);

		if (Script != NULL) {
			return this->Create(Script->c_str());
		}

		return false;

	}
	catch (int e) {

		cout << "Initialise::Exception " << e << endl;
		return false;

	}

}

//-----------------------------------------------------------------------------
// InitialiseFromFile
//-----------------------------------------------------------------------------
bool hsfcEngine::InitialiseFromFile(string* FileName, hsfcParameters* Paramaters) {

	try {

		// Initialise the engine
		this->Initialise(NULL, Paramaters);

		// Create the engine
		if (FileName != NULL) {
			return this->CreateFromFile(FileName->c_str());
		}

		return false;

	}
	catch (int e) {

		cout << "InitialiseFromFile::Exception " << e << endl;
		return false;

	}

}

//-----------------------------------------------------------------------------
// CreateGameState
//-----------------------------------------------------------------------------
bool hsfcEngine::CreateGameState(hsfcState** GameState) {

	try {

		// Create the base pointer
		*GameState = this->StateManager->CreateState();

		// Set up the data storage and rigids
		this->StateManager->InitialiseState(*GameState);

		// Set the number of roles
		this->NumRoles = (*GameState)->NumRelations[this->StateManager->RoleRelationIndex];

		return true;

	}
	catch (int e) {

		cout << "CreateGameState::Exception " << e << endl;
		return false;

	}

}

//-----------------------------------------------------------------------------
// FreeGameState
//-----------------------------------------------------------------------------
void hsfcEngine::FreeGameState(hsfcState* GameState) {

	try {

		// Free the memory
		this->StateManager->FreeState(GameState);

	}
	catch (int e) {

		cout << "FreeGameState::Exception " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// SetInitialGameState
//-----------------------------------------------------------------------------
void hsfcEngine::SetInitialGameState(hsfcState* GameState) {

	try {

		// Create the base pointer
		this->StateManager->SetInitialState(GameState);

	}
	catch (int e) {

		cout << "SetInitialGameState::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// CopyGameState
//-----------------------------------------------------------------------------
void hsfcEngine::CopyGameState(hsfcState* Destination, hsfcState* Source) {

	try {

		// Create the base pointer
		this->StateManager->FromState(Destination, Source);

	}
	catch (int e) {

		cout << "CopyGameState::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// GetLegalMoves
//-----------------------------------------------------------------------------
void hsfcEngine::GetLegalMoves(hsfcState* GameState, vector< vector<hsfcLegalMove> >& LegalMove) {

	try {

		// Clear the legal moves
		for (unsigned int i = 0; i < LegalMove.size(); i++) {
			LegalMove[i].clear();
		}

		// Advance the state to create the terminal relation tuple
		if (GameState->CurrentStep < 1) this->RulesEngine->AdvanceState(GameState, 1, false);
		if (this->RulesEngine->IsTerminal(GameState)) return;

		// Advance the state to create the legal relation tuples
		if (GameState->CurrentStep < 2) this->RulesEngine->AdvanceState(GameState, 2, false);

		// Get the moves
		this->RulesEngine->GetLegalMoves(GameState, LegalMove);

	}
	catch (int e) {

		cout << "GetLegalMoves::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// DoMove
//-----------------------------------------------------------------------------
void hsfcEngine::DoMove(hsfcState* GameState, vector<hsfcLegalMove>& DoesMove) {

	try {

		// The game step must be exactly after legal move tuples are calculated
		if (GameState->CurrentStep != 2) return;

		// Place the legal move tuples in the database
		for (unsigned int i = 0; i < DoesMove.size(); i++) {
			this->StateManager->AddRelation(GameState, DoesMove[i].Tuple);
		}

		// Advance the state to calculate the next tuples
		this->RulesEngine->AdvanceState(GameState, 4, false);

		// Advance the state to the next state
		this->RulesEngine->AdvanceState(GameState, 0, false);

	}
	catch (int e) {

		cout << "DoMove::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// IsTerminal
//-----------------------------------------------------------------------------
bool hsfcEngine::IsTerminal(hsfcState* GameState) {

	try {

		// Advance the state to create the terminal relation tuple
		if (GameState->CurrentStep < 1) this->RulesEngine->AdvanceState(GameState, 1, false);
		return this->RulesEngine->IsTerminal(GameState);

	}
	catch (int e) {

		cout << "IsTerminal::Exception: " << e << endl;
		return true;

	}

}

//-----------------------------------------------------------------------------
// GetGoalValues
//-----------------------------------------------------------------------------
void hsfcEngine::GetGoalValues(hsfcState* GameState, vector<int>& GoalValue) {

	int Value;

	try {

		// Clear the goal values
		GoalValue.clear();

		// Advance the state to create the terminal relation tuple
		if (GameState->CurrentStep < 1) this->RulesEngine->AdvanceState(GameState, 1, false);
		this->RulesEngine->AdvanceState(GameState, 5, false);

		// Return if the game is not terminal
		if (!this->RulesEngine->IsTerminal(GameState)) return;

		// Go through all of the roles
		for (unsigned int i = 0; i < GameState->NumRelations[this->StateManager->RoleRelationIndex]; i++) {
			Value = this->RulesEngine->GoalValue(GameState, i);
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
void hsfcEngine::PlayOut(hsfcState* GameState, vector<int>& GoalValue) {

	int Value;

	try {

		// Clear the goal values
		GoalValue.clear();
		for (unsigned int i = 0; i < GameState->NumRelations[this->StateManager->RoleRelationIndex]; i++) {
			GoalValue.push_back(0);
		}

		// Do an integrity check on the state
		if (GameState->CurrentStep > 2) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: State at wrong step in hsfcEngine::PlayOut\n");
			return;
		}

		// Advance the state to create the terminal relation tuple
		if (GameState->CurrentStep < 1) this->RulesEngine->AdvanceState(GameState, 1, false);

		// Play until the game is terminal
		while ((!this->RulesEngine->IsTerminal(GameState)) && (GameState->Round <= this->Parameters->MaxPlayoutRound)) {

			// Advance to calculate all the legal moves
			if (GameState->CurrentStep < 2) this->RulesEngine->AdvanceState(GameState, 2, false);

			// Get the legal move tuples
			this->RulesEngine->ChooseRandomMoves(GameState);

			// Advance to the next state
			this->RulesEngine->AdvanceState(GameState, 0, false);
			if (this->Parameters->LogDetail > 3) this->StateManager->PrintRelations(GameState, false);

			// Advance to calculate terminal tuple
			this->RulesEngine->AdvanceState(GameState, 1, false);

		}

		// Go through all of the roles
		GoalValue.clear();
		for (unsigned int i = 0; i < GameState->NumRelations[this->StateManager->RoleRelationIndex]; i++) {
			Value = this->RulesEngine->GoalValue(GameState, i);
			GoalValue.push_back(Value);
		}

	}
	catch (int e) {

		cout << "PlayOut::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// Validate
//-----------------------------------------------------------------------------
void hsfcEngine::Validate(string* GDLFileName, hsfcParameters& Parameters) {

	hsfcState* GameState;
	hsfcState* Node1;
	hsfcState* Node2;
	hsfcState* Node3;
	hsfcState* Playout;
	vector< vector<hsfcLegalMove> > LegalMove;
	vector<hsfcLegalMove> DoesMove;
	vector< vector<hsfcLegalMove> > Node1RoleMove;
	vector< vector<hsfcLegalMove> > Node2RoleMove;
	vector< vector<hsfcLegalMove> > Node3RoleMove;
	vector<int> Node1RoleMoveIndex;
	vector<int> Node2RoleMoveIndex;
	vector<int> Node3RoleMoveIndex;
	int Node1MoveCount;
	int Node2MoveCount;
	int Node3MoveCount;
	vector<int> GoalValue;
	int Count;
	double SumGoalValue;
	double SumGoalValue2;
	double SumRounds;
	double SumRounds2;
	double Average;
	double StdDev;
	int Node1Count;
	int Node2Count;
	int Node3Count;
	int NumGames;
	clock_t Start;
	clock_t Finish;
	double GamesPerSecond;

	// Clear the paramaters
	Parameters.TimeBuildSchema = 0;
	Parameters.TimeOptimise = 0;
	Parameters.TimeBuildLookup = 0;
	Parameters.Playouts = 0;
	Parameters.GamesPerSec = 0;
	Parameters.AveRounds = 0;
	Parameters.StDevRounds = 0;
	Parameters.AveScore0 = 0;
	Parameters.StDevScore0 = 0;
	Parameters.TreeNodes1 = 0;
	Parameters.TreeNodes2 = 0;
	Parameters.TreeNodes3 = 0;
	Parameters.TreeAveRounds = 0;
	Parameters.TreeStDevRounds = 0;
	Parameters.TreeAveScore0 = 0;
	Parameters.TreeStDevScore0 = 0;

	// Initialise the this with the game rules and game Parameters
	if (!this->InitialiseFromFile(GDLFileName, &Parameters)) {
		this->Lexicon->IO->WriteToLog(0, false, "Fatal Error: failed to create Engine\n");
		return;
	}

	// Create the game state so we can play
	// This just allocates the memory and creates an empty database
	if (!this->CreateGameState(&GameState)) {
		this->Lexicon->IO->WriteToLog(0, false, "Fatal Error: failed to create game state\n");
		return;
	}

	// Initialise the state in the game
	this->SetInitialGameState(GameState);

	this->Lexicon->IO->WriteToLog(1, false, "\n=== Validating ===\n\n");
	this->Lexicon->IO->LogIndent = 2;

	// Show times
	this->Lexicon->IO->WriteToLog(1, true, "Build Time\n");
	this->Lexicon->IO->FormatToLog(1, true, "  TimeBuildSchema %.3f sec\n", Parameters.TimeBuildSchema);
	this->Lexicon->IO->FormatToLog(1, true, "     TimeOptimise %.3f sec\n", Parameters.TimeOptimise);
	this->Lexicon->IO->FormatToLog(1, true, "  TimeBuildLookup %.3f sec\n", Parameters.TimeBuildLookup);

	// Get the legal moves from the initial game state and print them out
	this->GetLegalMoves(GameState, LegalMove);
	if (this->Lexicon->IO->Parameters->LogDetail > 2) this->StateManager->PrintRelations(GameState, true);
	if (Parameters.LogDetail > 1) {
		this->Lexicon->IO->WriteToLog(1, true, "Legal Moves\n");
		for (unsigned int i = 0; i < LegalMove.size(); i++) {
			for (unsigned int j = 0; j < LegalMove[i].size(); j++) {
				this->GetMoveText(LegalMove[i][j]);
				this->Lexicon->IO->FormatToLog(2, true, "  %s\n", LegalMove[i][j].Text);
			}
		}
	}


	// Choose some moves
	this->RulesEngine->ChooseRandomMoves(GameState);
	this->RulesEngine->AdvanceState(GameState, 4, true);

	vector<hsfcTuple> Fluent;
	this->GetStateFluents(GameState, Fluent);
	this->PrintState(GameState, false);

	// Perform tests to gain the following statistics
	// Average & StdDeviation for Random Playout Depth
	// Average & StdDeviation for Role 0 score  ather Random Playouts
	// State Count for full game tree at Depth = 1, 2, 3

	// Initialise the statistics
	Count = 0;
	SumGoalValue = 0;
	SumGoalValue2 = 0;
	SumRounds = 0;
	SumRounds2 = 0;

	// Play one game and time it
	Start = clock();
	this->SetInitialGameState(GameState);
	this->PlayOut(GameState, GoalValue);
	Finish = clock();

	// Include these games in the stats
	Count++;
	SumGoalValue += (double)GoalValue[0];
	SumGoalValue2 += (double)GoalValue[0] * (double)GoalValue[0];
	SumRounds += (double)GameState->Round;
	SumRounds2 += (double)GameState->Round * (double)GameState->Round;

	// Set the number of games for the test
	NumGames = 1000;
	if ((Finish - Start) > 10) NumGames = 300;
	if ((Finish - Start) > 40) NumGames = 100;
	if ((Finish - Start) > 160) NumGames = 30;
	if ((Finish - Start) > 640) NumGames = 10;
	if ((Finish - Start) > 2560) NumGames = 3;
	if ((Finish - Start) > 10240) NumGames = 1;

	// Play the game
	Start = clock();
	for (int i = 1; i < NumGames; i ++) {

		// Reset the game state
		this->SetInitialGameState(GameState);

		// Do a random playout
		this->PlayOut(GameState, GoalValue);

		// Record the stats
		Count++;
		SumGoalValue += (double)GoalValue[0];
		SumGoalValue2 += (double)GoalValue[0] * (double)GoalValue[0];
		SumRounds += (double)GameState->Round;
		SumRounds2 += (double)GameState->Round * (double)GameState->Round;

	}
	Finish = clock();
	GamesPerSecond = (double)TICKS_PER_SECOND * (double)Count / ((double)Finish - (double)Start + 0.5);

	// Calculate statistics
	this->Lexicon->IO->WriteToLog(1, true, "\nGame Statistics\n");
	this->Lexicon->IO->LogIndent = 4;
	this->Lexicon->IO->FormatToLog(1, true, "From %d Playouts\n", Count);
	this->Lexicon->IO->FormatToLog(1, true, "  Games Per Second %.3f\n", GamesPerSecond);
	Parameters.Playouts = Count;
	Parameters.GamesPerSec = GamesPerSecond;

	Average = SumRounds / (double)Count;
	StdDev = sqrt((SumRounds2 / (double)Count) - (Average * Average));
	this->Lexicon->IO->WriteToLog(1, true, "Rounds Per Game\n");
	this->Lexicon->IO->FormatToLog(1, true, "  Average = %f\n", Average);
	this->Lexicon->IO->FormatToLog(1, true, "   StdDev = %f\n", StdDev);
	Parameters.AveRounds = Average;
	Parameters.StDevRounds = StdDev;

	Average = SumGoalValue / (double)Count;
	StdDev = sqrt((SumGoalValue2 / (double)Count) - (Average * Average));
	this->Lexicon->IO->WriteToLog(1, true, "Role[0] Score\n");
	this->Lexicon->IO->FormatToLog(1, true, "  Average = %f\n", Average);
	this->Lexicon->IO->FormatToLog(1, true, "   StdDev = %f\n", StdDev);
	Parameters.AveScore0 = Average;
	Parameters.StDevScore0 = StdDev;

	// Construct a tree to depth 3 and count the nodes

	// Initialise the statistics
	Node1Count = 0;
	Node2Count = 0;
	Node3Count = 0;
	Count = 0;
	SumGoalValue = 0;
	SumGoalValue2 = 0;
	SumRounds = 0;
	SumRounds2 = 0;

	// Initialise the state in the game
	this->SetInitialGameState(GameState);
	this->CreateGameState(&Node1);
	this->CreateGameState(&Node2);
	this->CreateGameState(&Playout);
	this->SetInitialGameState(Playout);

	// Get the legal moves from the game state for depth = 1
	this->TreeGetMoves(this, GameState, Node1RoleMove, Node1RoleMoveIndex, Node1MoveCount);
	Node1Count += Node1MoveCount;

	// Count the depth = 2 nodes if reasonable to do so
	if ((NumGames > 10) && (Node1MoveCount < 100) && (Node1MoveCount > 0)) {

		// Record the state so we can enumerate the moves
		this->CopyGameState(Node1, GameState);

		// Go through all of the moves
		do {
		
			// Select the next set of moves
			this->TreeSelectMoves(this, DoesMove, Node1RoleMove, Node1RoleMoveIndex);
			this->DoMove(GameState, DoesMove);

			// Copy the state and do a playout
			this->CopyGameState(Playout, GameState);
			this->PlayOut(Playout, GoalValue);
			// Record the stats
			Count++;
			SumGoalValue += (double)GoalValue[0];
			SumGoalValue2 += (double)GoalValue[0] * (double)GoalValue[0];
			SumRounds += (double)Playout->Round;
			SumRounds2 += (double)Playout->Round * (double)Playout->Round;

			// Get the legal moves from the game state 
			this->TreeGetMoves(this, GameState, Node2RoleMove, Node2RoleMoveIndex, Node2MoveCount);
			Node2Count += Node2MoveCount;

			// Third layer
			// Count the depth = 3 nodes if reasonable to do so
			if ((NumGames > 30) && (Node1MoveCount < 30) && (Node2MoveCount > 0)) {

				// Record the state so we can enumerate the moves
				this->CopyGameState(Node2, GameState);

				// Go through all of the moves
				do {
				
					// Select the next set of moves
					this->TreeSelectMoves(this, DoesMove, Node2RoleMove, Node2RoleMoveIndex);
					this->DoMove(GameState, DoesMove);

					// Copy the state and do a playout
					this->CopyGameState(Playout, GameState);
					this->PlayOut(Playout, GoalValue);
					// Record the stats
					Count++;
					SumGoalValue += (double)GoalValue[0];
					SumGoalValue2 += (double)GoalValue[0] * (double)GoalValue[0];
					SumRounds += (double)Playout->Round;
					SumRounds2 += (double)Playout->Round * (double)Playout->Round;

					// Get the legal moves from the game state 
					this->TreeGetMoves(this, GameState, Node3RoleMove, Node3RoleMoveIndex, Node3MoveCount);
					Node3Count += Node3MoveCount;

					// Third layer

					// Return to the original game state
					this->CopyGameState(GameState, Node2);

				} while (this->TreeAdvanceIndex(this, Node2RoleMove, Node2RoleMoveIndex));

			}

			// Return to the original game state
			this->CopyGameState(GameState, Node1);

		} while (this->TreeAdvanceIndex(this, Node1RoleMove, Node1RoleMoveIndex));

	}

	// Print the node statistics
	this->Lexicon->IO->LogIndent = 2;
	this->Lexicon->IO->WriteToLog(1, true, "\nGame Tree\n");
	this->Lexicon->IO->WriteToLog(1, true, "  Tree Nodes\n");
	this->Lexicon->IO->FormatToLog(1, true, "    Depth 1 = %d\n", Node1Count);
	this->Lexicon->IO->FormatToLog(1, true, "    Depth 2 = %d\n", Node2Count);
	this->Lexicon->IO->FormatToLog(1, true, "    Depth 3 = %d\n", Node3Count);
	Parameters.TreeNodes1 = Node1Count;
	Parameters.TreeNodes2 = Node2Count;
	Parameters.TreeNodes3 = Node3Count;

	// Stats from depth 1 playouts
	this->Lexicon->IO->WriteToLog(1, true, "\nGame Statistics From Tree\n");
	this->Lexicon->IO->LogIndent = 4;
	this->Lexicon->IO->FormatToLog(1, true, "From %d Playouts\n", Count);
	if (Count > 0) {
		Average = SumRounds / (double)Count;
		StdDev = sqrt((SumRounds2 / (double)Count) - (Average * Average));
		this->Lexicon->IO->WriteToLog(1, true, "Rounds Per Game\n");
		this->Lexicon->IO->FormatToLog(1, true, "  Average = %f\n", Average);
		this->Lexicon->IO->FormatToLog(1, true, "   StdDev = %f\n", StdDev);
		Parameters.TreeAveRounds = Average;
		Parameters.TreeStDevRounds = StdDev;
		Average = SumGoalValue / (double)Count;
		StdDev = sqrt((SumGoalValue2 / (double)Count) - (Average * Average));
		this->Lexicon->IO->WriteToLog(1, true, "Role[0] Score\n");
		this->Lexicon->IO->FormatToLog(1, true, "  Average = %f\n", Average);
		this->Lexicon->IO->FormatToLog(1, true, "   StdDev = %f\n", StdDev);
		Parameters.TreeAveScore0 = Average;
		Parameters.TreeStDevScore0 = StdDev;
	}


	// Clean up the objects
	this->FreeGameState(GameState);
	this->FreeGameState(Node1);
	this->FreeGameState(Node2);

	return;

}

//-----------------------------------------------------------------------------
// GetMoveText
//-----------------------------------------------------------------------------
void hsfcEngine::GetMoveText(hsfcLegalMove& Move) {

	try {

		// Convert the relation to text
		this->DomainManager->RelationAsKIF(Move.Tuple, &(Move.Text));

	}
	catch (int e) {

		cout << "GetMoveText::GetMoveText: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// GetMoveText
//-----------------------------------------------------------------------------
void hsfcEngine::GetMoveText(hsfcTuple& Move, string& Text) {

    char* TempText;

	try {

		// Convert the move to text
        TempText = NULL;
		this->DomainManager->RelationAsKIF(Move, &TempText);
        Text.assign(TempText);
        delete[] TempText;
	}
	catch (int e) {

		cout << "GetMoveText::GetMoveText: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// PrintState
//-----------------------------------------------------------------------------
void hsfcEngine::PrintState(hsfcState* GameState, bool ShowRigids) {

	try {

		// Print the state
		this->StateManager->PrintRelations(GameState, ShowRigids);

	}
	catch (int e) {

		cout << "PrintState::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// GetStateFluents
//-----------------------------------------------------------------------------
void hsfcEngine::GetStateFluents(hsfcState* GameState, vector<hsfcTuple>& Fluent) {

	try {

		// Clear any fluents
		Fluent.clear();

		// This call is independent of the Current Step
		// It will return an empty list if the state hos not been initialised

		// Get the moves
		this->StateManager->GetFluents(GameState, Fluent);

	}
	catch (int e) {

		cout << "GetStateFluents::Exception: " << e << endl;

	}

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcEngine::CreateFromFile(const char* FileName) {

	char* Script;
	bool Result;

	this->Lexicon->IO->FormatToLog(1, true, "\n%s\n", FileName);

	// Read the file
	Script = NULL;
	if (!this->ReadFile(FileName, &Script)) {
		if (Script != NULL) delete[] Script;
		return false;
	}

	// Load the script and create the engine
	Result = this->Create(Script);

	// Clean up
	delete[] Script;

	return Result;

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcEngine::Create(const char* Script) {

	clock_t Start;
	clock_t Finish;
	int Length;

	// Create the High Speed Forward Chaining engine

	// Initialise the timing
	Start = clock();

	// Read Well Formed Text from the file
	this->Lexicon->IO->WriteToLog(1, false, "\n=== Read Script ===\n\n");
	this->Lexicon->IO->LogIndent = 2;
	this->WFT = new hsfcWFT(Lexicon);
	this->WFT->Initialise();
	this->WFT->Load(Script, "/#;:'");

	// Create the GDL from the WFT
	this->Lexicon->IO->WriteToLog(1, false, "\n=== Read GDL ===\n\n");
	this->Lexicon->IO->LogIndent = 2;
	this->GDL = new hsfcGDL(Lexicon);
	if (!this->GDL->Read(WFT)) return false;

	// Convert the GDL to SCL
	this->Lexicon->IO->WriteToLog(1, false, "\n=== Read SCL ===\n\n");
	this->Lexicon->IO->LogIndent = 2;
	if (!this->SCL->Read(this->GDL)) return false;
	if (this->Parameters->SCLOnly) return false;

	// Create the schema
	this->Lexicon->IO->WriteToLog(1, false, "\n=== Create Schema ===\n\n");
	this->Lexicon->IO->LogIndent = 2;
	if (!this->Schema->Create(this->SCL)) return false;

	// Create the domains
	this->Lexicon->IO->WriteToLog(1, false, "\n=== Build Domains ===\n\n");
	this->Lexicon->IO->LogIndent = 2;
	this->DomainManager->CreateDomains(this->Schema);
	if (!this->DomainManager->BuildDomains(this->Schema)) return false;

	// Set the schema in the state manager
	this->Lexicon->IO->WriteToLog(1, false, "\n=== Build State Manager ===\n\n");
	this->Lexicon->IO->LogIndent = 2;
	if (!this->StateManager->SetSchema(this->Schema)) return false;
	if (this->Parameters->SchemaOnly) return false;

	// Record the time
	Finish = clock();
	this->Parameters->TimeBuildSchema = (double)(Finish - Start) / (double)TICKS_PER_SECOND;
	Start = clock();

	// Create the rules engine
	this->Lexicon->IO->WriteToLog(1, false, "\n=== Create Rules Engine ===\n\n");
	this->Lexicon->IO->LogIndent = 2;
	if (!this->RulesEngine->Create(this->Schema, this->Parameters->LowSpeedOnly)) return false;

	// Record the time
	Finish = clock();
	this->Parameters->TimeBuildLookup = (double)(Finish - Start) / (double)TICKS_PER_SECOND;

	return true;

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcEngine::ReadFile(const char* FileName, char** Script) {

	int Length;
	char Letter;
	FILE* InputFile;
	int FileSize;

	// Read Well Formed Text from the file
	this->Lexicon->IO->LogIndent = 2;
	this->Lexicon->IO->WriteToLog(1, false, "\n=== Read File ===\n\n");
	this->Lexicon->IO->FormatToLog(2, true, "Reading File '%s'\n", FileName);

	// Open the input file
	InputFile = fopen(FileName, "r");
	if (InputFile == NULL) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: GDL file does not exist '%s'\n");
		return false;
	}

    // Find the filesize
    fseek(InputFile, 0, SEEK_END);
    FileSize = ftell(InputFile);
	this->Lexicon->IO->FormatToLog(2, true, "File size = %d\n", FileSize);
    rewind(InputFile);

    // Load the file into memory
    *Script = new char[FileSize + 1];
	// Get the description from the file
	Length = 0;
	while ((!feof(InputFile)) && (Length < FileSize)) {

		// Read a letter from the file
		fscanf(InputFile, "%c", &Letter);
		(*Script)[Length] = Letter;
		Length++;

	}

	// Is it too long
	if (Length > FileSize) {
		this->Lexicon->IO->FormatToLog(0, false, "Error: GDL file size mismatch '%s'", FileName);
		return false;
	}

	(*Script)[Length] = 0;
	fclose(InputFile);
	this->Lexicon->IO->FormatToLog(2, true, "%d bytes read\n", Length);

	return true;

}

//-----------------------------------------------------------------------------
// TreeGetMoves
//-----------------------------------------------------------------------------
void hsfcEngine::TreeGetMoves(hsfcEngine* Engine, hsfcState* GameState, vector< vector<hsfcLegalMove> >& RoleMove, vector<int>& RoleMoveIndex, int& MoveCount) {

	hsfcLegalMove NewMove;

	// Get the legal moves from the game state 
	this->GetLegalMoves(GameState, RoleMove);

	// Set properties
	RoleMoveIndex.clear();
	MoveCount = 1;

	// Check there are moves for all roles
	if (RoleMove.size() == Engine->NumRoles) {
		for (unsigned int i = 0; i < Engine->NumRoles; i++) {
			MoveCount *= RoleMove[i].size();
			RoleMoveIndex.push_back(0);
		}
	} else {
		this->Lexicon->IO->WriteToLog(0, false, "Warning: No moves in hsfcEngine::TreeGetMoves\n");
		MoveCount = 0;
		for (unsigned int i = 0; i < Engine->NumRoles; i++) {
			RoleMoveIndex.push_back(0);
		}
	}

}

//-----------------------------------------------------------------------------
// TreeSelectMoves
//-----------------------------------------------------------------------------
void hsfcEngine::TreeSelectMoves(hsfcEngine* Engine, vector<hsfcLegalMove>& DoesMove, vector< vector<hsfcLegalMove> >& RoleMove, vector<int>& RoleMoveIndex) {

	// Load the current indexed moves
	DoesMove.resize(Engine->NumRoles);
	for (unsigned int i = 0; i < Engine->NumRoles; i++) {
		DoesMove[i].RoleIndex = i;
		DoesMove[i].Text = NULL;
		DoesMove[i].Tuple.Index = RoleMove[i][RoleMoveIndex[i]].Tuple.Index;
		DoesMove[i].Tuple.ID = RoleMove[i][RoleMoveIndex[i]].Tuple.ID;
	}

}

//-----------------------------------------------------------------------------
// TreeAdvanceIndex
//-----------------------------------------------------------------------------
bool hsfcEngine::TreeAdvanceIndex(hsfcEngine* Engine, vector< vector<hsfcLegalMove> >& RoleMove, vector<int>& RoleMoveIndex) {

	// Advance move index
	for (unsigned int i = 0; i < Engine->NumRoles; i++) {
		(RoleMoveIndex[i])++;
		if (RoleMoveIndex[i] < RoleMove[i].size()) return true;
		RoleMoveIndex[i] = 0;
	}

	return false;

}

