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

#include "hsfcIO.h"

//=============================================================================
// CLASS: hsfcStatistic
//=============================================================================
class hsfcStatistic {

public:
	hsfcStatistic(void);
	~hsfcStatistic(void);

	void Initialise();
	void AddObservation(double Value);
	double Average();
	double StdDev();

	double Count;
	double Sum;
	double Sum2;

protected:

private:

};

//=============================================================================
// CLASS: hsfcLexicon
//=============================================================================
class hsfcLexicon {

public:
	hsfcLexicon(void);
	~hsfcLexicon(void);

	void Initialise(hsfcParameters* Parameters);
	unsigned int Index(const char* Value);
	unsigned int RelationIndex(const char* Value, bool Add);
	unsigned int RelationIndex(unsigned int NameID);
	unsigned int GDLIndex(unsigned int ID);
	unsigned int NewRigidNameID(int Arity);
	void Parse(const char* Text, vector<hsfcTuple>& Reference);
	bool Match(unsigned int ID, const char* Text);
	bool PartialMatch(unsigned int ID, const char* Text);
	bool MatchText(unsigned int ID, const char* Text);
	bool IsVariable(unsigned int ID);
	bool IsUsed(const char* Letter, bool IgnoreComments);
	const char* Text(unsigned int ID);
	char* Copy(unsigned int ID, bool WithArity);
	const char* Relation(unsigned int ID);
	unsigned int Size();
	unsigned int TrueFrom(unsigned int RelationIndex);
	unsigned int NextFrom(unsigned int RelationIndex);
	unsigned int InitFrom(unsigned int RelationIndex);
	void Print();

	hsfcIO* IO;

protected:

private:
	unsigned int AddTerm(const char* Value);
	unsigned int AddName(const char* Value);

	vector<string> Term;
	vector<unsigned int> TermIndex;
	vector<string> RelationName;
	vector<unsigned int> RelationNameID;
	vector<bool> RelationIsRigid;
	int UniqueNum;

};

