//=============================================================================
// Project: High Speed Forward Chaining
// Module: Example
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>

#include "hsfcAPI.h"

using namespace std;

//=============================================================================
// Main
//=============================================================================
int main(int argc, char* argv[]) {

	hsfcGDLManager* GDLManager;
	hsfcGDLParamaters GDLParamaters;
	string* GDLFileName;
	hsfcState* PlayOutGame;
	hsfcState* SearchNode;
	hsfcState* GameFromText;
	vector<hsfcLegalMove> LegalMove;
	vector<hsfcLegalMove> DoesMove;
	vector<int> GoalValue;
	vector<int> SumGoalValue;
	int ReturnCode;
	vector<string> Role;

	// Create the GDLManager
	GDLManager = new hsfcGDLManager();

	// Identify the GDL file; must be a fully qualified path and file name
	GDLFileName = new string("J:\\HSFC\\Experiments\\DNF\\Published\\GDL\\tictactoe.gdl");

	// Set up the GDLManager paramaters
	GDLParamaters.ReadGDLOnly = false;			// Validate the GDL without creating the schema
	GDLParamaters.SchemaOnly = false;			// Validate the GDL & Schema without grounding the rules
	GDLParamaters.MaxRelationSize = 1000000;	// Max bytes per relation for high speed storage
	GDLParamaters.MaxReferenceSize = 1000000;	// Max bytes per lookup table for grounding
	GDLParamaters.OrderRules = true;			// Optimise the rule execution cost

	// Initialise the GDL Manager with the game rules and game paramaters
	ReturnCode = GDLManager->InitialiseFromFile(GDLFileName, GDLParamaters);

	/* Return Codes
		0 - Success
		1 - GDL File not found
		2 - A rigid relation size > GDLParamaters.MaxRelationSize
		.
		.
		99 - Unknown
	*/

	// Was the initialisation successful
	if (ReturnCode > 0) {
		cout << "GDLManager initialisation failed: Code = " << ReturnCode << endl;
		delete GDLManager;
		return 0;
	}

	// Print a list of roles and their endecies
	GDLManager->GetRoles(Role);
	for (unsigned int i = 0; i < Role.size(); i++) {
		cout << i << "\t" << Role[i].c_str() << endl;
	}

	// In this example we have two game states
	// 1: Records the game state as we navigate through a search tree
	// 2: Used to perform playouts as an evaluation function

	// Create the search tree game state so we can conduct a search
	// This just allocates the memory and creates an empty database
	ReturnCode = GDLManager->CreateGameState(&SearchNode);

	/* Return Codes
		0 - Success
		1 - See stdout for error information
	*/

	if (ReturnCode > 0) {
		cout << "GDLManager failed to create an empty game state: Code = " << ReturnCode << endl;
		delete GDLManager;
		return 0;
	}

	// Create the playout game state to be used as an evaluation function
	ReturnCode = GDLManager->CreateGameState(&PlayOutGame);
	if (ReturnCode > 0) {
		cout << "GDLManager failed to create an empty game state: Code = " << ReturnCode << endl;
		delete GDLManager;
		return 0;
	}

	// Initialise the tree search node to the initial state in the game
	GDLManager->SetInitialGameState(SearchNode);

	// Print the state; remember the permanent facts won't show
	GDLManager->PrintState(SearchNode);

	// Get some legal moves from the game state and print them out
	GDLManager->GetLegalMoves(SearchNode, LegalMove);
	cout << endl << "Legal Moves" << endl;
	for (unsigned int i = 0; i < LegalMove.size(); i++) cout << LegalMove[i].Text << endl;

	// Prepare the list of moves
	DoesMove.clear();

	// Choose the first legal move for each role
	cout << endl << "Selected Moves" << endl;
	for (int RoleIndex = 0; RoleIndex < GDLManager->NumRoles; RoleIndex++) {
		for (unsigned int i = 0; i < LegalMove.size(); i++) {
			if (LegalMove[i].RoleIndex == RoleIndex) {
				// Load the move
				DoesMove.push_back(LegalMove[i]);
				// Print the move in KIF
				printf("%s\n", GDLManager->RelationAsKIF(LegalMove[i].Tuple));
				break;
			}
		}
	}

	// Do the moves and advance the search node to the next round
	GDLManager->DoMove(SearchNode, DoesMove);

	// Convert the game state to text in the form of relations "#.#"
	char* Text = GDLManager->GameStateAsText(SearchNode);
	cout << endl << "State as Text" << endl;
	cout << Text << endl;

	// Create an empty database to receive the text
	ReturnCode = GDLManager->CreateGameState(&GameFromText);
	if (ReturnCode > 0) {
		cout << "GDLManager failed to create an empty game state: Code = " << ReturnCode << endl;
		delete GDLManager;
		return 0;
	}

	// Copy the text into the empty game
	cout << endl << "State from Text" << endl;
	if (GDLManager->GameStateFromText(GameFromText, Text)) {
		cout << "Success" << endl;
	} else {
		cout << "Failure" << endl;
	}
	GDLManager->PrintState(GameFromText);

	// Check to see if the game is terminal at this point
	if (GDLManager->IsTerminal(SearchNode)) {
		delete GDLManager;
		return 0;
	}

	// Prepare the evaluation function
	SumGoalValue.clear();
	for (int i = 0; i < GDLManager->NumRoles; i++) SumGoalValue.push_back(0);

	// Perform 10 playouts from the search node and sum the goal values
	for (int i = 0; i < 10; i++) {
		GDLManager->CopyGameState(PlayOutGame, SearchNode);
		GDLManager->PlayOut(PlayOutGame, GoalValue);
		for (int i = 0; i < GDLManager->NumRoles; i++) SumGoalValue[i] += GoalValue[i];
	}

	// Report the average goal values
	cout << endl << "Average Goal Values" << endl;
	for (int i = 0; i < GDLManager->NumRoles; i++) cout << "Role" << i << ": " << SumGoalValue[i] / 10 << endl;

	// Clean up the objects
	GDLManager->FreeGameState(GameFromText);
	GDLManager->FreeGameState(PlayOutGame);
	GDLManager->FreeGameState(SearchNode);
	delete GDLFileName;
	delete GDLManager;

	// Pause to read the output
	printf("\nPause...\n");
	getchar();

	return 0;

}

