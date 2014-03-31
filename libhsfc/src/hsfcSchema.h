//=============================================================================
// Project: High Speed Forward Chaining
// Module: Schema
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#pragma once

#include <stdio.h>
#include <iostream>
#include <math.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <limits.h>

#include "hsfcGDL.h"

#define DEBUG false

using namespace std;

class hsfcRelationSchema;

//=============================================================================
// ENUM: hsfcFact
//=============================================================================
enum hsfcFact {
	hsfcFactNone, 
	hsfcFactPersistent, 
	hsfcFactPermanent,
	hsfcFactInitial,
	hsfcFactNext
};

//=============================================================================
// ENUM: hsfcFunction
//=============================================================================
enum hsfcFunction {
	hsfcFunctionNone = 0x00000000, 
	hsfcFunctionDistinct = 0x00000001, 
	hsfcFunctionTrue = 0x00000002, 
	hsfcFunctionNot = 0x00000004 
};

//=============================================================================
// ENUM: hsfcRuleRelationType
//=============================================================================
enum hsfcRuleRelationType {
	hsfcRuleNone,
	hsfcRuleResult,
	hsfcRulePreCondition,
	hsfcRuleInput,
	hsfcRuleCondition
};

//=============================================================================
// STRUCT: hsfcDomainEntry
//=============================================================================
typedef struct hsfcDomainEntry {
	hsfcTuple Tuple;
	int Index;
	int Count;
} hsfcDomainEntry;

//=============================================================================
// STRUCT: hsfcDomain
//=============================================================================
typedef struct hsfcDomain {
	hsfcDomainEntry* Entry;
	int Count;
	int Size;
} hsfcDomain;

//=============================================================================
// STRUCT: hsfcRelationLink
//=============================================================================
typedef struct hsfcRelationLink {
	int SourceListIndex;
	int DestinationListIndex;
} hsfcRelationLink;

//=============================================================================
// STRUCT: hsfcRuleTemplate
//=============================================================================
typedef struct hsfcRuleTemplate {
	int PredicateIndex;
	hsfcRelationSchema* RelationSchema;
	bool Fixed;
	hsfcTuple Tuple;
	int BufferIndex;
	int ArgumentIndex;
	int DomainSize;
} hsfcRuleTemplate;

//=============================================================================
// STRUCT: hsfcRuleCompactTerm
//=============================================================================
typedef struct hsfcRuleCompactTerm {
	hsfcRelationSchema* RelationSchema;
	int ArgumentIndex;
	hsfcTuple Tuple;
} hsfcRuleCompactTerm;

//=============================================================================
// STRUCT: hsfcRuleTerm
//=============================================================================
typedef struct hsfcRuleTerm {
	int RelationIndex;
	hsfcTuple Tuple;
} hsfcRuleTerm;

//=============================================================================
// STRUCT: hsfcBufferTerm
//=============================================================================
typedef struct hsfcBufferTerm {
	int RelationIndex;
	int ID;
	vector<hsfcTuple> Domain;
} hsfcBufferTerm;

//=============================================================================
// CLASS: hsfcRelationSchema
//=============================================================================
class hsfcRelationSchema {

public:
	hsfcRelationSchema(hsfcLexicon* Lexicon);
	~hsfcRelationSchema(void);

	void Initialise(int PredicateIndex, int Arity);
	void FromRelationSchema(hsfcRelationSchema* Source);
	void AddToDomain(int DomainIndex, hsfcDomainEntry* Entry);
	void AddToFactDomain(vector<hsfcDomainEntry>& Term);
	void IndexDomains();
	double RelationSize();
	void IntersectBufferDomain(vector<hsfcTuple>& BufferDomain, unsigned int DomainIndex);
	void AddToBufferDomain(vector<hsfcTuple>& BufferDomain, unsigned int DomainIndex);
	int ID(vector<hsfcTuple>& Term);
	int ID(hsfcRuleCompactTerm Term[], int Offset, int NumTerms);
	void Terms(int ID, vector<hsfcTuple>& Term);
	void Terms(int ID, vector<hsfcRuleTerm>& Term);
	int GetDomainCount(int Index);
	void ListDomain(vector<string>& List);
	void PrintDomains();
	void Print();

	int IDCount;
	double IDCountDbl;
	int PredicateIndex;
	int Arity;
	hsfcFact Fact;
	int Index;
	double AveListLength;
	double Samples;
	bool DomainIsComplete;
	bool HasComplexEntries;
	bool IsInState;

protected:

private:
	hsfcLexicon* Lexicon;
	vector<vector<hsfcDomainEntry> > vDomain;	
	hsfcDomain** Domain;

};


//=============================================================================
// CLASS: hsfcRuleRelationSchema
//=============================================================================
class hsfcRuleRelationSchema {

public:
	hsfcRuleRelationSchema(hsfcLexicon* Lexicon);
	~hsfcRuleRelationSchema(void);

	void Initialise();
	void FromRuleRelationSchema(hsfcRuleRelationSchema* Source);
	void Create(vector<hsfcGDLTerm>& Term, vector<hsfcBufferTerm>& Buffer);
	bool Terms(int TupleID, vector<hsfcRuleTerm>& Term); 
	int ID(vector<hsfcTuple>& Term, bool Validate);
	void Print();

	int PredicateIndex;
	hsfcRuleRelationType Type;
	int Function;
	vector<hsfcRuleTemplate> Template;
	int ReferenceSize;
	double SortOrder;

protected:

private:
	hsfcLexicon* Lexicon;
	int Arity;
	hsfcFact Fact;

};

//=============================================================================
// CLASS: hsfcRuleSchema
//=============================================================================
class hsfcRuleSchema {

public:
	hsfcRuleSchema(hsfcLexicon* Lexicon);
	~hsfcRuleSchema(void);

	void Initialise();
	void Create(hsfcGDLRule* Rule);
	int Optimise(bool OrderInputs);
	void Stratify();
	void Print();

	vector<hsfcRuleRelationSchema*> Relation;
	vector<hsfcBufferTerm> Buffer;
	bool SelfReferencing;
	int Stratum;
	double EstReferenceSize;

protected:

private:
	double Cost(vector<int>& InputIndex);
	void Sort();

	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcSchema
//=============================================================================
class hsfcSchema {

public:
	hsfcSchema(hsfcLexicon* Lexicon);
	~hsfcSchema(void);

	void Initialise();
	bool Create(const char* FileName);
	void GetRoles(vector<string>& Role);
	void PrintRelation(hsfcTuple* Tuple, bool CRLF);
	void RelationAsText(hsfcTuple* Tuple, char* Text);
	void Print();

	vector<hsfcRelationSchema*> Relation;
	vector<hsfcRuleSchema*> Rule;
	vector<hsfcTuple> Fact;
	vector<hsfcTuple> Initial;
	vector<hsfcRelationLink> Next;

protected:

private:
	bool ReadGDL(const char* FileName);
	bool ReadRules(char* RuleScript);
	bool ReadDomains(char* DomainScript);
	bool CreateSCL();
	void Stratify();
	void SetPermanentFacts();
	void SetInitialRelations();
	void SetNextRelations();
	void AddRelationSchema(hsfcGDLRelation* GDLRelation);
	void AddRuleSchema(hsfcGDLRule* GDLRule);
	hsfcRelationSchema* RelationSchema(int PredicateIndex, int Arity);
	hsfcRelationSchema* RelationSchema(int PredicateIndex);
	hsfcRelationSchema* RelationSchemaByName(int PredicateIndex);
	bool RelationSchemaExists(char* Predicate);
	bool RuleIsRequired(hsfcRuleSchema* Rule, unsigned int FirstIndex, unsigned int LastIndex);
	bool Required(int OutputPredicateIndex, int InputPredicateIndex);
	void NestTerms(vector<hsfcGDLTerm>& Term, int RelationOffset, int PredicateIndex);
	void PrintRelations();
	void PrintRules();
	void PrintReferences();

	hsfcLexicon* Lexicon;
	hsfcGDL* GDL;
	hsfcGDL* SCL;
	vector<hsfcTuple> Stratum;
	vector<hsfcRelationDetail> RelationDetail;

};

//=============================================================================
// Functions
//=============================================================================
int Factorial(int Number);
