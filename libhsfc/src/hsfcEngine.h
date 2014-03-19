//=============================================================================
// Project: High Speed Forward Chaining
// Module: Engine
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#pragma once

#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>

#include "hsfcGrinder.h"

using namespace std;

//=============================================================================
// Modules
//=============================================================================
/*
Lexicon 
	STRUCT: hsfcTuple
	CLASS: hsfcLexicon
GDL
	STRUCT: hsfcTupleReference
	CLASS: hsfcGDLAtom
	CLASS: hsfcGDLRelation
	CLASS: hsfcGDLRule
	CLASS: hsfcGDL
Schema
	ENUM: hsfcFact
	ENUM: hsfcFunction
	ENUM: hsfcRuleRelationType
	STRUCT: hsfcRelationLink
	STRUCT: hsfcRuleTemplate
	STRUCT: hsfcRuleTerm
	CLASS: hsfcRelationSchema
	CLASS: hsfcRuleRelationSchema
	CLASS: hsfcRuleSchema
	CLASS: hsfcSchema
State
	STRUCT: hsfcState
	CLASS: hsfcStateManager
Grinder
	CLASS: hsfcRule
	CLASS: hsfcGrinderEngine
	CLASS: hsfcGrinder
Engine
	CLASS: hsfcEngine

*/

//=============================================================================
// STRUCT: hsfcGDLParamaters
//=============================================================================
typedef struct hsfcGDLParamaters {
	int MaxRelationSize;
	int MaxReferenceSize;
	bool OrderRules;
	bool ReadGDLOnly;
	bool SchemaOnly;
} hsfcGDLParamaters;

//=============================================================================
// STRUCT: hsfcLegalMove
//=============================================================================
typedef struct hsfcLegalMove {
	int RoleIndex;
	char Text[256];
	hsfcTuple Tuple;	
} hsfcLegalMove;

//=============================================================================
// CLASS: hsfcEngine
//=============================================================================
class hsfcEngine {

public:
	hsfcEngine(hsfcLexicon* Lexicon, hsfcStateManager* StateManager);
	~hsfcEngine(void);

	void Initialise();
	bool Create(const char* FileName, hsfcGDLParamaters& Paramaters);
	void PlayOut(hsfcState* State);
	void Audit();
	void SetInitialState(hsfcState* State);
	void AdvanceState(hsfcState* State, int Step);
	void ProcessRules(hsfcState* State, int Step);
	bool IsTerminal(hsfcState* State);
	void GetLegalMoves(hsfcState* State, vector<hsfcTuple>& Move);
	void ChooseRandomMoves(hsfcState* State);
	int GoalValue(hsfcState* State, int RoleIndex);
	void Print();

	hsfcGrinder* Grinder;
	vector<hsfcRule*> Rule;

	float TimeReadGDL;
	float TimeLowSpeedRun;
	float TimeOptimise;
	float TimeGrind;
	unsigned int ReferenceSize;

protected:

private:
	void CreateRules();

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;
	int FirstRuleIndex[6];
	int LastRuleIndex[6];
	hsfcRelationSchema* GoalRelation;

	time_t StartClock;
	time_t StopClock;

};

