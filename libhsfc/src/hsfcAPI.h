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

// DPR20140401 Adding HSFCManager as a friend class because I need to
// access the StateManager and want to minimise the amount of
// Michael's code that I change.
namespace HSFC { class HSFCManager; };

//=============================================================================
// CLASS: hsfcGDLManager
//=============================================================================
class hsfcGDLManager {

    friend class HSFC::HSFCManager;

public:
	hsfcGDLManager(void);
	~hsfcGDLManager(void);

	int Initialise(string* Script, hsfcGDLParamaters& Paramaters);
	int InitialiseFromFile(string* GDLFileName, hsfcGDLParamaters& Paramaters);
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
	void GetIDCount(vector<int>& Count);
	void GetFluents(hsfcState* GameState, vector<hsfcTuple>& Fluent);
	void GetExists(hsfcState* GameState, void*& Pattern, int& Size);
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


