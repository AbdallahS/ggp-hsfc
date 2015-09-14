//=============================================================================
// Project: High Speed Forward Chaining
// Module: Grinder
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#pragma once

#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>

#include "hsfcRule.h"

using namespace std;

namespace HSFC { class HSFCManager; }

//=============================================================================
// CLASS: hsfcEngine
//=============================================================================
class hsfcEngine {

    friend class HSFC::HSFCManager;

public:
	hsfcEngine(void);
	~hsfcEngine(void);

	bool Initialise(string* Script, hsfcParameters* Parameters);
	bool InitialiseFromFile(string* GDLFileName, hsfcParameters* Parameters);
	bool CreateGameState(hsfcState** GameState);
	void FreeGameState(hsfcState* GameState);
	void SetInitialGameState(hsfcState* GameState);
	void CopyGameState(hsfcState* Destination, hsfcState* Source);
	void GetLegalMoves(hsfcState* GameState, vector< vector<hsfcLegalMove> >& LegalMove);
	void DoMove(hsfcState* GameState, vector<hsfcLegalMove>& DoesMove);
	bool IsTerminal(hsfcState* GameState);
	void GetGoalValues(hsfcState* GameState, vector<int>& GoalValue);
	void PlayOut(hsfcState* GameState, vector<int>& GoalValue);
	void Validate(string* GDLFileName, hsfcParameters& Parameters);
	void GetMoveText(hsfcLegalMove& Move);
	void GetMoveText(hsfcTuple& Move, string& Text);
	void PrintState(hsfcState* GameState, bool ShowRigids);
	void GetStateFluents(hsfcState* GameState, vector<hsfcTuple>& Fluent);

	unsigned int NumRoles;
	hsfcParameters* Parameters;
	hsfcLexicon* Lexicon;

protected:

private:
	bool Create(const char* Script);
	bool CreateFromFile(const char* FileName);
	bool ReadFile(const char* FileName, char** Script);
	void TreeGetMoves(hsfcEngine* Engine, hsfcState* GameState, vector< vector<hsfcLegalMove> >& RoleMove, vector<int>& RoleMoveIndex, int& MoveCount);
	void TreeSelectMoves(hsfcEngine* Engine, vector<hsfcLegalMove>& DoesMove, vector< vector<hsfcLegalMove> >& RoleMove, vector<int>& RoleMoveIndex);
	bool TreeAdvanceIndex(hsfcEngine* Engine, vector< vector<hsfcLegalMove> >& RoleMove, vector<int>& RoleMoveIndex);

	hsfcStateManager* StateManager;
	hsfcDomainManager* DomainManager;
	hsfcSchema* Schema;
	hsfcRulesEngine* RulesEngine;
	hsfcSCL* SCL;
	hsfcGDL* GDL;
	hsfcWFT* WFT;

	hsfcWFT* TextWFT;
	hsfcGDL* TextGDL;
	hsfcSCL* TextSCL;

};

