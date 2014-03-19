//=============================================================================
// Project: High Speed Forward Chaining
// Module: Term
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

using namespace std;

//=============================================================================
// Referencing
//=============================================================================
/*
Index
	0 based offset for array
	TermIndex identifies offset in Term[]; as in Term[Index]
	RelationIndex identifies offset in Term[]; as in Relation[Index]

ID
	Unique identified for item
	Encoded from DomainIndex of all of the attributes

Lexicon Term
	RelationIndex = -1
	TupleID = TermIndex

GDL Term
	RelationIndex = -1
	TupleID = Lexicon.TupleIndex

RelationSchema Term
	RelationIndex 
	TupleID
	
RuleRelationSchema Template
	RelationSchema
	Fixed
	Term.RelationIndex 
	Term.ID
	BufferIndex

RuleRelationSchema Term
	RelationIndex
	Term.RelationIndex 
	Term.ID
	
Buffer Term
	Fixed
	RelationIndex 
	TupleID

*/
//=============================================================================
// STRUCT: hsfcTuple
//=============================================================================
typedef struct hsfcTuple {
	int RelationIndex;
	int ID;
} hsfcTuple;

//=============================================================================
// CLASS: hsfcLexicon
//=============================================================================
class hsfcLexicon {

public:
	hsfcLexicon(void);
	~hsfcLexicon(void);

	void Initialise();
	int Index(const char* Value);
	void Parse(char* Text, vector<hsfcTuple>& Reference);
	bool Match(int Index, char* Text);
	bool PartialMatch(int Index, char* Text);
	bool MatchText(int Index, char* Text);
	bool IsVariable(int Index);
	const char* Text(int Index);
	unsigned int Size();
	void Print();

protected:

private:
	unsigned int AddTerm(const char* Value);

	vector<string> Term;
	vector<int> TermIndex;

};

