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

#include "hsfcSCL.h"

using namespace std;

//=============================================================================
// CLASS: hsfcDomainSchema
//=============================================================================
class hsfcDomainSchema {

public:
	hsfcDomainSchema(hsfcLexicon* Lexicon);
	~hsfcDomainSchema(void);

	void Initialise(unsigned int NameID);
	bool AddTerm(hsfcTuple& NewTerm);
	bool AddTerms(vector<hsfcTuple>& Term);
	void Copy(vector<hsfcTuple>& Destination);
	void Intersection(vector<hsfcTuple>& Destination);
	void Print();

	unsigned int NameID;
	vector<hsfcTuple> Term;

protected:

private:
	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcRelationSchema
//=============================================================================
class hsfcRelationSchema {

public:
	hsfcRelationSchema(hsfcLexicon* Lexicon);
	~hsfcRelationSchema(void);

	void Initialise(unsigned int NameID, int Arity, unsigned int Index);
	void Copy(vector<hsfcTuple>& Destination, unsigned int DomainIndex);
	void Intersection(vector<hsfcTuple>& Destination, unsigned int DomainIndex);
	bool AddTerms(vector<hsfcTuple>& Term, unsigned int DomainIndex);
	void Print();

	unsigned int NameID;
	int Arity;
	vector<hsfcDomainSchema*> DomainSchema;	
	unsigned int Index;
	bool IsInState;
	hsfcFactType Fact;

protected:

private:
	hsfcLexicon* Lexicon;

	void DeleteDomainSchema();

};


//=============================================================================
// CLASS: hsfcRuleRelationSchema
//=============================================================================
class hsfcRuleRelationSchema {

public:
	hsfcRuleRelationSchema(hsfcLexicon* Lexicon);
	~hsfcRuleRelationSchema(void);

	void Initialise(hsfcSCLAtom* SCL);
	void Print();

	vector<hsfcRuleTermSchema> TermSchema;
	hsfcSCLAtom* SCL;
	int Function;
	hsfcRuleRelationType Type;
	unsigned int ReferenceSize;

protected:

private:
	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcRuleSchema
//=============================================================================
class hsfcRuleSchema {

public:
	hsfcRuleSchema(hsfcLexicon* Lexicon);
	~hsfcRuleSchema(void);

	void Initialise();
	void Print();

	vector<hsfcRuleRelationSchema*> RelationSchema;
	vector<hsfcDomainSchema*> VariableSchema;

protected:

private:
	void DeleteRelationSchema();
	void DeleteVariableSchema();

	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcStratumSchema
//=============================================================================
class hsfcStratumSchema {

public:
	hsfcStratumSchema(hsfcLexicon* Lexicon);
	~hsfcStratumSchema(void);

	void Initialise(hsfcSCLStratum* SCLStratum);
	void Print();

	vector<hsfcRuleSchema*> RuleSchema;
	hsfcSCLStratum* SCLStratum;
	bool IsRigid;
	hsfcStratumType Type;

protected:

private:
	void DeleteRuleSchema();

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
	bool Create(hsfcSCL* SCL);
	hsfcRelationSchema* FindRelationSchema(unsigned int NameID);
	void Print();

	vector<hsfcRelationSchema*> RelationSchema;
	vector<hsfcStratumSchema*> StratumSchema;
	vector<hsfcTuple> Rigid;
	vector<hsfcTuple> Permanent;
	vector<hsfcTuple> Initial;
	vector<hsfcReference> Next;
	hsfcSCL* SCL;

protected:

private:
	hsfcRelationSchema* AddRelationSchema(unsigned int NameID, int Arity);
	void DeleteRelationSchema();
	void DeleteStratumSchema();
	hsfcRelationSchema* FindRelationSchema(unsigned int NameID, int Arity);
	bool AddToDomain(unsigned int RelationIndex, int ArgumentIndex, hsfcTuple& Term, int* NewEntryCount);

	bool CreateRelationSchema(hsfcSCLAtom* SCLAtom);
	bool CompleteRelationSchema();
	bool CreateStratumSchema(hsfcSCLStratum* SCLStratum);
	bool CreateRuleSchema(hsfcStratumSchema* StratumSchema, hsfcSCLAtom* SCLRule);
	bool CreateRuleRelationSchema(hsfcRuleSchema* RuleSchema, hsfcSCLAtom* SCLRuleRelation);
	bool CreateRuleTermSchema(hsfcRuleRelationSchema* RuleRelationSchema, hsfcSCLAtom* Atom, unsigned int RelationNameID, int ArgumentIndex);	
	bool AddVariableRuleTerms(hsfcRuleSchema* RuleSchema);
	
	bool BuildRelationDomains(hsfcSCLAtom* SCLAtom);
	bool BuildStratumDomainsFix(hsfcStratumSchema* StratumSchema);
	bool BuildStratumDomainsVar(hsfcStratumSchema* StratumSchema, int* NewEntryCount);
	bool BuildRuleDomainsFix(hsfcRuleSchema* RuleSchema);
	bool BuildRuleDomainsVar(hsfcRuleSchema* RuleSchema, int* NewEntryCount);
	bool OptimiseRuleSchema(hsfcRuleSchema* RuleSchema);

	bool IdentifyRelationTypes();
	void SetNextReferences();

	hsfcLexicon* Lexicon;

};

