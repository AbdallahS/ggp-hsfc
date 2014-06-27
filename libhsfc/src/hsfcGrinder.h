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

#include "hsfcState.h"

using namespace std;

#define MAX_NUM_ROLES 8
#define MAX_NO_OF_INPUTS 32

//=============================================================================
// STRUCT: hsfcBufferEntry
//=============================================================================
typedef struct hsfcBufferEntry {
	int RelationIndex;
	int ID;
	int InputIndex;
} hsfcBufferEntry;

//=============================================================================
// CLASS: hsfcRule
//=============================================================================
class hsfcRule {

public:
	hsfcRule(hsfcLexicon* Lexicon, hsfcStateManager* StateManager);
	~hsfcRule(void);

	void Initialise();
	void FromSchema(hsfcRuleSchema* RuleSchema, bool LowSpeed);
	void Execute(hsfcState* State, bool Audit);
	void SetSpeed(double MaxRefernceSize, double MaxReltaionSize);
	void Grind();
	void HighSpeedExecute(hsfcState* State);
	void HighSpeedAudit(hsfcState* State);
	void Print();
	void PrintBuffer();

	int Transactions;
	int ReferenceSize;

	hsfcRuleRelationSchema* ResultRelation;
	hsfcRuleRelationSchema** InputRelation;
	hsfcRuleRelationSchema** ConditionRelation;
	hsfcRuleRelationSchema** PreConditionRelation;
	hsfcRuleSchema* RuleSchema;
	bool LowSpeed;

protected:

private:
	void ClearBuffer();
	void ClearBuffer(int LowInputIndex);
	bool LoadBuffer(int InputIndex, hsfcState* State, bool Audit);
	bool LoadBuffer(int InputIndex);
	bool CheckPreConditions(hsfcState* State, bool Audit);
	bool CheckConditions(hsfcState* State, bool Audit);
	int CheckCondition(int Index);
	int CheckResult();
	bool InitialiseInput(hsfcState* State);
	bool AdvanceInput(int InputIndex, hsfcState* State);
	bool AdvanceInput(int InputIndex);
	bool AdvanceInput(int* LowInputIndex, int InputIndex, hsfcState* State);
	bool AdvanceInput(int* LowInputIndex, int InputIndex);

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;

	int* Cursor;
	hsfcBufferEntry* Buffer;
	int BufferSize;

	int Result;
	int* ResultLookup;

	int NumInputs;
	int* InputCount;
	int* Input;
	int* MaxInputLookup;
	int** InputLookup;

	int NumConditions;
	int* Condition;
	int* ConditionFunction;
	int* MaxConditionLookup;
	int** ConditionLookup;

	int NumPreConditions;
	int* PreCondition;
	int* PreConditionFunction;
	int* MaxPreConditionLookup;
	int** PreConditionLookup;

};

//=============================================================================
// CLASS: hsfcGrinderEngine
//=============================================================================
class hsfcGrinderEngine {

public:
	hsfcGrinderEngine(hsfcLexicon* Lexicon, hsfcStateManager* StateManager);
	~hsfcGrinderEngine(void);

	void Initialise();
	bool Create(char* Script, int MaxRelationSize, double MaxReferenceSize);
	void OptimiseRules(bool OrderInputs);
	void GrindRules();

	void SetInitialState(hsfcState* State);
	void AdvanceState(hsfcState* State, int Step);
	void ProcessRules(hsfcState* State, int Step);
	bool IsTerminal(hsfcState* State);
	void GetLegalMoves(hsfcState* State, vector<hsfcTuple>& Move);
	void ChooseRandomMoves(hsfcState* State);
	//int GoalValue(hsfcState* State, int RoleIndex);

	void ResetStatistics();
	void CollectStatistics(hsfcState* State);
	void PrintStatistics();

	void Print();

	hsfcSchema* Schema;
	vector<hsfcRule*> Rule;
	int Playouts;
	int States;
	int FirstRuleIndex[6];
	int LastRuleIndex[6];
	double EstReferenceSize;
	int RulePermutations;
	double MaxRefernceSize;

protected:

private:

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;

};

//=============================================================================
// CLASS: hsfcGrinder
//=============================================================================
class hsfcGrinder {

public:
	hsfcGrinder(hsfcLexicon* Lexicon, hsfcStateManager* StateManager);
	~hsfcGrinder(void);

	void Initialise();
	bool Create(char* Script, int MaxRelationSize, double MaxReferenceSize);
	void Optimise();
	void Print();

	hsfcGrinderEngine* Engine;

protected:

private:
	void ResetStatistics();
	void CollectStatistics();
	void PrintStatistics();

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;

};

