//=============================================================================
// Project: High Speed Forward Chaining
// Module: State
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#pragma once

#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>

#include "hsfcDomain.h"

using namespace std;

namespace HSFC { class HSFCManager; };

//=============================================================================
// CLASS: hsfcStateManager
//=============================================================================
class hsfcStateManager {

    friend class HSFC::HSFCManager;

public:
	hsfcStateManager(hsfcLexicon* Lexicon, hsfcDomainManager* DomainManager);
	~hsfcStateManager(void);

	void Initialise();
	bool SetSchema(hsfcSchema* Schema);

	// State Methods
	hsfcState* CreateState(void);
	void FreeState(hsfcState* State);
	void InitialiseState(hsfcState* State);
	void ResetState(hsfcState* State);
	void FromState(hsfcState* State, hsfcState* Source);
	void SetInitialState(hsfcState* State);
	void NextState(hsfcState* State);

	bool AddRelation(hsfcState* State, hsfcTuple& Tuple);
	bool RelationExists(hsfcState* State, hsfcTuple& Tuple);
	void PrintRelations(hsfcState* State, bool ShowRigids);

	void CreatePermanents(hsfcState* State);

	//bool CalculateStateSize();
	//void CompareStates(hsfcState* State1, hsfcState* State2);
	//char* StateAsText(hsfcState* State);
	//bool StateFromText(hsfcState* State, char* Text);
	//void AdvanceState(hsfcState* State, int Step);

	unsigned int RoleRelationIndex;
	unsigned int TerminalRelationIndex;
	unsigned int GoalRelationIndex;
	unsigned int LegalRelationIndex;
	unsigned int DoesRelationIndex;
	unsigned int SeesRelationIndex;
	unsigned int* NextRelationIndex;
	unsigned int NoNextRelation;

	unsigned int* GoalToRole;
	unsigned int* LegalToRole;
	unsigned int* DoesToRole;
	unsigned int* SeesToRole;

	unsigned int MaxRelationSize;

protected:

private:

	hsfcLexicon* Lexicon;
	hsfcDomainManager* DomainManager;
	hsfcSchema* Schema;

	unsigned int NumRelationLists;

	unsigned int StateSize;

};


