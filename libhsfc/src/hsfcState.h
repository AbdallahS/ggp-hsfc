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

#include "hsfcSchema.h"

using namespace std;

//=============================================================================
// STRUCT: hsfcState
//=============================================================================
typedef struct hsfcState {
	int CurrentStep;
	int Round;
	int* NumRelations;
	int* MaxNumRelations;
	int** RelationID;
	//long long** TupleID64;
	bool** RelationExists;
	int** RelationIDSorted;
} hsfcState;

//=============================================================================
// CLASS: hsfcStateManager
//=============================================================================
class hsfcStateManager {

public:
	hsfcStateManager(hsfcLexicon* Lexicon);
	~hsfcStateManager(void);

	void Initialise();
	void SetSchema(hsfcSchema* Schema, int MaxRelationSize);

	// State Methods
	hsfcState* CreateState(void);
	void FreeState(hsfcState* State);
	void InitialiseState(hsfcState* State);
	void FromState(hsfcState* State, hsfcState* Source);
	void ResetState(hsfcState* State);
	void SetInitialState(hsfcState* State);
	void AdvanceState(hsfcState* State, int Step);
	void NextState(hsfcState* State);

	void AddRelation(hsfcState* State, hsfcTuple* Tuple);
	bool RelationExists(hsfcState* State, hsfcTuple* Tuple);
	bool CalculateStateSize();

	void CompareStates(hsfcState* State1, hsfcState* State2);
	void PrintRelations(hsfcState* State, bool PermanentFacts);

	hsfcSchema* Schema;
	int RoleRelationIndex;
	int TerminalRelationIndex;
	int GoalRelationIndex;
	int LegalRelationIndex;
	int DoesRelationIndex;
	int SeesRelationIndex;
	int* NextRelationIndex;
	int NoNextRelation;

	int StateSize;
	double MaxStateSize;
	int MaxRelationSize;

	//int DoesListIndex[MAX_NO_OF_RELATION_LISTS];
	//int LegalNoRoles[MAX_ARITY];
	//int LegalRoleNo[MAX_ARITY][MAX_NO_OF_ROLES];
	//int SeesNoRoles[MAX_ARITY];
	//int SeesRoleNo[MAX_ARITY][MAX_NO_OF_ROLES];
	//hsfcTuple* NextReference[MAX_ARITY];
	//int NoNextReferences[MAX_ARITY];
	//hsfcTuple* PossibleFact;
	//int NoPossibleFacts;
	//hsfcTuple* Initial;
	//int NoInitials;

	//int ReferenceSize;
	//int KnowledgeBaseSize;

protected:

private:

	hsfcLexicon* Lexicon;
	int NumRelationLists;

};


