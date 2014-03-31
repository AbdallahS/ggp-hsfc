//=============================================================================
// Project: High Speed Forward Chaining
// Module: GDL
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

#include "hsfcLexicon.h"

using namespace std;

#define INITIAL_NO_ATOMS 16

class hsfcGDLRelation;

//=============================================================================
// STRUCT: hsfcTupleReference
//=============================================================================
typedef struct hsfcGDLTerm {
	int PredicateIndex;
	hsfcTuple Tuple;
} hsfcGDLTerm;

//=============================================================================
// STRUCT: hsfcRelationDetail
//=============================================================================
typedef struct hsfcRelationDetail {
	int PredicateIndex;
	int Arity;
} hsfcRelationDetail;

//=============================================================================
// CLASS: hsfcGDLAtom
//=============================================================================
class hsfcGDLAtom {

public:
	hsfcGDLAtom(hsfcLexicon* Lexicon);
	~hsfcGDLAtom(void);

	void Initialise();
	void FromGDLAtom(hsfcGDLAtom* Source);
	int Read(char* Text);
	void Terms(vector<hsfcTuple>& Term);
	void Terms(int PredicateIndex, vector<hsfcGDLTerm>& Term);
	int AsText(char* Text);

	hsfcGDLRelation* Relation;
	int TermIndex;

protected:

private:
	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcGDLRelation
//=============================================================================
class hsfcGDLRelation {

public:
	hsfcGDLRelation(hsfcLexicon* Lexicon);
	~hsfcGDLRelation(void);

	void Initialise();
	void FromGDLRelation(hsfcGDLRelation* Source);
	int Read(char* Text);
	void NormaliseTerms();
	void FindZeroArity();
	void Terms(vector<hsfcTuple>& Term);
	void Terms(vector<hsfcGDLTerm>& Term);
	int Arity();
	int PredicateIndex();
	bool AddRelationDetail(vector<hsfcRelationDetail>& RelationDetail);
	int AsText(char* Text);

	vector<hsfcGDLAtom*> Atom;
	bool Not;

protected:

private:
	hsfcGDLAtom* AddAtom();

	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcGDLRule
//=============================================================================
class hsfcGDLRule {

public:
	hsfcGDLRule(hsfcLexicon* Lexicon);
	~hsfcGDLRule(void);

	void Initialise();
	void FromGDLRule(hsfcGDLRule* Source);
	int Read(char* Text);
	int Arity();
	int AsText(char* Text);

	vector<hsfcGDLRelation*> Relation;

protected:

private:
	hsfcGDLRelation* AddRelation();

	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcGDL
//=============================================================================
class hsfcGDL {

public:
	hsfcGDL(hsfcLexicon* Lexicon);
	~hsfcGDL(void);

	void Initialise();
	int Read(char* Script);
	bool GetRelationDetails();
	void Print(char* Title);

	vector<hsfcRelationDetail> RelationDetail;
	vector<hsfcGDLRelation*> Relation;
	vector<hsfcGDLRule*> Rule;

protected:

private:
	hsfcGDLRelation* AddRelation();
	hsfcGDLRule* AddRule();

	hsfcLexicon* Lexicon;

};


