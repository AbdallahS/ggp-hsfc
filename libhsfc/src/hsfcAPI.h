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
	void DoMove(hsfcState* GameState, vector<hsfcLegalMove>& LegalMove);
	bool IsTerminal(hsfcState* GameState);
	void GetGoalValues(hsfcState* GameState, vector<int>& GoalValue);
	void PlayOut(hsfcState* GameState, vector<int>& GoalValue);
	void PrintState(hsfcState* GameState);

	int NumRoles;

protected:

private:

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;
	hsfcEngine* Engine;

};


