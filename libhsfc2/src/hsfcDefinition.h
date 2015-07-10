//=============================================================================
// Project: High Speed Forward Chaining
// Module: Lexicon
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <climits>

#define TICKS_PER_SECOND 1000
#define UNDEFINED UINT_MAX

#define MAX_RELATION_SCHEMAS 16384
#define MAX_RELATION_ARITY 128
#define MAX_ID_COUNT INT_MAX
#define MAX_DOMAIN_SIZE INT_MAX / 2
#define MAX_DOMAIN_ENTRIES 16384
#define MAX_NO_OF_INPUTS 32

#define XMAX_GAME_ROUNDS 1000

using namespace std;

// hsfcIO
class hsfcIO;

// hsfcLexicon
class hsfcLexicon;

// hsfcWFT
class hsfcWFTElement;
class hsfcWFTStructure;
class hsfcWFT;

// hsfcGDL
class hsfcGDLAtom;
class hsfcGDL;

// hsfcSCL
class hsfcSCLAtom;
class hsfcSCLStratum;
class hsfcSCL;

// hsfcSchema
class hsfcDomainSchema;
class hsfcRelationSchema;
class hsfcRuleRelationSchema;
class hsfcRuleSchema;
class hsfcStratumSchema;
class hsfcSchema;

// hsfcDomain
class hsfcDomainManager;

// hsfcRule
class hsfcRule;
class hsfcStratum;
class hsfcRulesEngine;

// hsfcEngine
class hsfcEngine;

//=============================================================================
// STRUCT: hsfcParameters
//=============================================================================
typedef struct hsfcParameters {
	char* LogFileName;
	int LogDetail;
	unsigned int MaxRelationSize;
	unsigned int MaxLookupSize;
	unsigned int MaxStateSize;
	unsigned int MaxPlayoutRound;
	bool LowSpeedOnly;
	bool SCLOnly;
	bool SchemaOnly;
	unsigned int StateSize;
	unsigned int TotalLookupSize;
	double TimeBuildSchema;
	double TimeOptimise;
	double TimeBuildLookup;
	double Playouts;
	double GamesPerSec;
	double AveRounds;
	double StDevRounds;
	double AveScore0;
	double StDevScore0;
	double TreeNodes1;
	double TreeNodes2;
	double TreeNodes3;
	double TreeAveRounds;
	double TreeStDevRounds;
	double TreeAveScore0;
	double TreeStDevScore0;
} hsfcParameters;

//=============================================================================
// STRUCT: hsfcTuple
//=============================================================================
typedef struct hsfcTuple {
	unsigned int Index;
	unsigned int ID;
} hsfcTuple;

//=============================================================================
// STRUCT: hsfcBufferTerm
//=============================================================================
typedef struct hsfcBufferTerm {
	unsigned int Index;
	unsigned int ID;
	int InputIndex;
} hsfcBufferTerm;

//=============================================================================
// STRUCT: hsfcInputPath
//=============================================================================
typedef struct hsfcInputPath {
	unsigned int StratumIndex;
	unsigned int InputIndex;
	int InputNameID;
} hsfcInputPath;

//=============================================================================
// STRUCT: hsfcReference
//=============================================================================
typedef struct hsfcReference {
	unsigned int SourceIndex;
	unsigned int DestinationIndex;
} hsfcReference;

//=============================================================================
// STRUCT: hsfcRuleTerm
//=============================================================================
typedef struct hsfcRuleTerm {
	int RelationIndex;
	hsfcTuple Tuple;
} hsfcRuleTerm;

//=============================================================================
// STRUCT: hsfcCalculator
//=============================================================================
typedef struct hsfcCalculator {
	hsfcTuple Value;
	hsfcTuple* Term;
	unsigned int TermSize;
	hsfcReference* Relation;
	unsigned int RelationSize;
	hsfcReference* Link;
	unsigned int LinkSize;
	hsfcReference* Variable;
	unsigned int VariableSize;
	hsfcReference* Fixed;
	unsigned int FixedSize;
} hsfcCalculator;

//=============================================================================
// STRUCT: hsfcLegalMove
//=============================================================================
typedef struct hsfcLegalMove {
	int RoleIndex;
	char* Text;
	hsfcTuple Tuple;	
} hsfcLegalMove;

//=============================================================================
// ENUM: hsfcStratumType
//=============================================================================
enum hsfcStratumType {
	hsfcStratumNone = 0x00000000, 
	hsfcStratumTerminal = 0x00000001, 
	hsfcStratumGoal = 0x00000002, 
	hsfcStratumLegal = 0x00000003, 
	hsfcStratumSees = 0x00000004, 
	hsfcStratumNext = 0x00000005 
};

//=============================================================================
// ENUM: hsfcRuleRelationFn
//=============================================================================
enum hsfcRuleRelationFn {
	hsfcFunctionNone = 0x00000000, 
	hsfcFunctionNot = 0x00000001, 
	hsfcFunctionDistinct = 0x00000002 
};

//=============================================================================
// ENUM: hsfcRuleRelationType
//=============================================================================
enum hsfcRuleRelationType {
	hsfcRuleInput = 0x00000000, 
	hsfcRuleCondition = 0x00000001, 
	hsfcRulePreCondition = 0x00000003,
	hsfcRuleOutput = 0x00000004 
};

//=============================================================================
// ENUM: hsfcRuleTermType
//=============================================================================
enum hsfcRuleTermType {
	hsfcTypeRelation = 0x00000000, 
	hsfcTypeVariable = 0x00000001, 
	hsfcTypeFixed = 0x00000002 
};

//=============================================================================
// ENUM: hsfcRigidity
//=============================================================================
enum hsfcRigidity {
	hsfcRigidityNone = 0x00000000, 
	hsfcRigiditySome = 0x00000001, 
	hsfcRigidityFull = 0x00000002 
};

//=============================================================================
// ENUM: hsfcFactType
//=============================================================================
enum hsfcFactType {
	hsfcFactNone = 0x00000000, 
	hsfcFactEmbedded = 0x00000001, 
	hsfcFactAux = 0x00000002, 
	hsfcFactInit = 0x00000003, 
	hsfcFactTrue = 0x00000004, 
	hsfcFactNext = 0x00000005 
};

//=============================================================================
// STRUCT: hsfcRuleTermSchema
//=============================================================================
typedef struct hsfcRuleTermSchema {
	unsigned int NameID;
	unsigned int RelationIndex;
	unsigned int EmbeddedIndex;
	unsigned int Arity;
	hsfcRuleTermType Type;
	int VariableIndex;
	int ArgumentIndex;
	hsfcTuple Term;
} hsfcRuleTemplate;

//=============================================================================
// STRUCT: hsfcDomainRecord
//=============================================================================
typedef struct hsfcDomainRecord {
	hsfcTuple Relation;
	unsigned int IndexBase;
} hsfcDomainRecord;

//=============================================================================
// STRUCT: hsfcDomain
//=============================================================================
typedef struct hsfcDomain {
	unsigned int  NameID;
	bool Rigid;
	unsigned int Arity;
	unsigned int* Size;
	unsigned int IDCount;
	bool Complete;
	unsigned int* RecordSize;
	hsfcDomainRecord** Record;
} hsfcDomain;

//=============================================================================
// STRUCT: hsfcState
//=============================================================================
typedef struct hsfcState {
	int CurrentStep;
	int Round;
	unsigned int* NumRelations;
	unsigned int* MaxNumRelations;
	unsigned int** RelationID;
	bool** RelationExists;
	unsigned int** RelationIDSorted;
} hsfcState;

