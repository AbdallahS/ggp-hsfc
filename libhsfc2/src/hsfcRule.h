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

//=============================================================================
// CLASS: hsfcRule
//=============================================================================
class hsfcRule {

public:
	hsfcRule(hsfcLexicon* Lexicon, hsfcStateManager* StateManager, hsfcDomainManager* DomainManager);
	~hsfcRule(void);

	void Initialise();
	void FromSchema(hsfcRuleSchema* RuleSchema, bool LowSpeed);
	void OptimiseInputs(hsfcSchema* Schema);
	void CreateLookupTable();
	int Execute(hsfcState* State);
	int HighSpeedExecute(hsfcState* State);
	int Test(hsfcState* State);
	void Print(bool ResetVariables);

	int Transactions;
	double LookupSize;
	bool LowSpeed;
	int SelfReferenceCount;

protected:

private:
	void ClearRule();
	void BuildCalculator(hsfcRuleRelationSchema* RuleRelationSchema, hsfcCalculator* Calculator);
	bool CalculateValue(hsfcCalculator& Calculator);
	bool CalculateTerms(hsfcCalculator& Calculator);
	void TestCalculator(hsfcCalculator& Calculator);
	void PrintCalculator(hsfcCalculator& Calculator, bool ResetVariables);

	void ClearVariables(int LowInputIndex);
	bool InitialiseInput(hsfcState* State);
	bool AdvanceInput(int* LowInputIndex, int InputIndex, hsfcState* State);
	bool AdvanceInput(int* LowInputIndex, int InputIndex);
	bool LoadInput(int InputIndex, hsfcState* State);
	bool LoadInput(int InputIndex);
	unsigned int PreConditionID(int Index);
	bool CheckPreConditions(hsfcState* State);
	unsigned int ConditionID(int Index);
	bool CheckConditions(hsfcState* State);
	unsigned int ResultID();
	bool PostResult(hsfcState* State);

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;
	hsfcDomainManager* DomainManager;
	hsfcRuleSchema* RuleSchema;

	int* Cursor;
	hsfcBufferTerm* Variable;
	int VariableSize;

	int Result;
	hsfcCalculator ResultCalculator;
	unsigned int ResultLookupSize;
	unsigned int* ResultLookup;

	int NumInputs;
	int* InputCount;
	int* Input;
	hsfcCalculator* InputCalculator;
	unsigned int* MaxInputLookup;
	unsigned int* InputLookupSize;
	unsigned int** InputLookup;

	int NumConditions;
	int* Condition;
	hsfcCalculator* ConditionCalculator;
	int* ConditionFunction;
	unsigned int* MaxConditionLookup;
	unsigned int* ConditionLookupSize;
	unsigned int** ConditionLookup;

	int NumPreConditions;
	int* PreCondition;
	hsfcCalculator* PreConditionCalculator;
	int* PreConditionFunction;
	unsigned int* MaxPreConditionLookup;
	unsigned int* PreConditionLookupSize;
	unsigned int** PreConditionLookup;

};

//=============================================================================
// CLASS: hsfcStratum
//=============================================================================
class hsfcStratum {

public:
	hsfcStratum(hsfcLexicon* Lexicon, hsfcStateManager* StateManager, hsfcDomainManager* DomainManager);
	~hsfcStratum(void);

	void Initialise();
	bool Create(hsfcStratumSchema* StratumSchema, bool LowSpeedOnly);
	void CreateLookupTables();
	void ExecuteRules(hsfcState* State, bool LowSpeed);
	void TestRules(hsfcState* State);
	void Print();

	vector<hsfcRule*> Rule;
	int SelfReferenceCount;
	bool MultiPass;
	bool IsRigid;
	hsfcStratumType Type;
	double LookupSize;

protected:

private:
	void DeleteRules();

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;
	hsfcDomainManager* DomainManager;

};

//=============================================================================
// CLASS: hsfcRulesEngine
//=============================================================================
class hsfcRulesEngine {

public:
	hsfcRulesEngine(hsfcLexicon* Lexicon, hsfcStateManager* StateManager, hsfcDomainManager* DomainManager);
	~hsfcRulesEngine(void);

	void Initialise();
	bool Create(hsfcSchema* Schema, bool LowSpeedOnly);

	void SetInitialState(hsfcState* State);
	void AdvanceState(hsfcState* State, int Step, bool LowSpeed);
	bool IsTerminal(hsfcState* State);
	int GoalValue(hsfcState* State, int RoleIndex);
	void GetLegalMoves(hsfcState* State, vector< vector<hsfcLegalMove> >& LegalMove);
	void ChooseRandomMoves(hsfcState* State);
	void Print();

	vector<hsfcStratum*> Stratum;
	int FirstStratumIndex[6];
	int LastStratumIndex[6];
	double LookupSize;

protected:

private:
	void ProcessRules(hsfcState* State, int Step, bool LowSpeed, bool ProcessRigids);
	void DeleteStrata();
	void SetStratumProperties();
	bool CalculateRigids();
	void OptimiseRuleInputs();
	void CreateLookupTables();

	hsfcLexicon* Lexicon;
	hsfcStateManager* StateManager;
	hsfcDomainManager* DomainManager;
	hsfcState* State;
	hsfcSchema* Schema;
	vector<vector<int> > Step;

};
