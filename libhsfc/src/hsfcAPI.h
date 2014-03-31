//=============================================================================
// Project: High Speed Forward Chaining
// Module: API
// Authors: Michael Schofield UNSW
// 
//=============================================================================

#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>
#include <vector>

using namespace std;

#include "hsfcEngine.h"
#include "kpWFT.h"

//=============================================================================
// CLASS: hsfcGDLManager
//=============================================================================
class hsfcGDLManager {

public:
	hsfcGDLManager(void);
	~hsfcGDLManager(void);

	int Initialise(string* GDLFileName, hsfcGDLParamaters& Paramaters);
	int CreateGameState(hsfcState** GameState);
	void FreeGameState(hsfcState* GameState);
	void CopyGameState(hsfcState* Destination, hsfcState* Source);
	void SetInitialGameState(hsfcState* GameState);
	void GetLegalMoves(hsfcState* GameState, vector<hsfcLegalMove>& LegalMove);
	void DoMove(hsfcState* GameState, vector<hsfcLegalMove>& DoesMove);
	bool IsTerminal(hsfcState* GameState);
	void GetGoalValues(hsfcState* GameState, vector<int>& GoalValue);
	void PlayOut(hsfcState* GameState, vector<int>& GoalValue);
	void GetRoles(vector<string>& Role);
	char* RelationAsKIF(hsfcTuple& Tuple);
	char* GameStateAsText(hsfcState* GameState);
	bool GameStateFromText(hsfcState* GameState, char* Text);
	void PrintState(hsfcState* GameState);

	int NumRoles;

protected:

private:

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;
	hsfcEngine* Engine;

};


