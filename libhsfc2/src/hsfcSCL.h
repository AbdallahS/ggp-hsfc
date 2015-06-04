//=============================================================================
// Project: High Speed Forward Chaining
// Module: SCL
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

#include "hsfcGDL.h"

//=============================================================================
// CLASS: hsfcSCLAtom
//=============================================================================
class hsfcSCLAtom {

public:
	hsfcSCLAtom(hsfcLexicon* Lexicon);
	~hsfcSCLAtom(void);

	void Initialise();
	void FromSCLAtom(hsfcSCLAtom* Source);
	bool Read(hsfcGDLAtom* GDLAtom);
	void SetQualifiedName(unsigned int ParentNameID, unsigned int ArgumentIndex);
	bool RequiredFor(vector<hsfcSCLAtom*>& Rule);
	void DeleteTerms();
	void Print();

	vector<hsfcSCLAtom*> Term;
	int PredicateIndex;
	int NameID;
	bool Not;
	bool Distinct;

protected:

private:
	hsfcLexicon* Lexicon;

};


//=============================================================================
// CLASS: hsfcSCLStratum
//=============================================================================
class hsfcSCLStratum {

public:
	hsfcSCLStratum(hsfcLexicon* Lexicon);
	~hsfcSCLStratum(void);

	void Initialise();
	void AddRule(hsfcSCLAtom* Rule);
	bool HasOutput(int NameID);
	void TracePath(vector<hsfcInputPath>& Path, vector<hsfcSCLStratum*>& Stratum); 
	bool PathIsCircular(vector<hsfcInputPath>& Path, vector<unsigned int>& CircularPath); 
	void Combine(hsfcSCLStratum* Source);
	bool PartialMatch(const char* Text);
	bool Match(const char* Text);
	bool RequiredFor(vector<hsfcSCLStratum*>& Stratum, unsigned int First, unsigned int Last);

	vector<hsfcSCLAtom*> Rule;
	vector<int> Input;
	vector<int> Output;
	bool LinearPaths;
	int SelfReferenceCount;

protected:

private:
	void DeleteRules();

	hsfcLexicon* Lexicon;

};


//=============================================================================
// CLASS: hsfcSCL
//=============================================================================
class hsfcSCL {

public:
	hsfcSCL(hsfcLexicon* Lexicon);
	~hsfcSCL(void);

	void Initialise();
	bool Read(hsfcGDL* GDL);
	bool ReadStatement(hsfcGDL* GDL);
	void Print();

	vector<hsfcSCLAtom*> Statement;
	vector<hsfcSCLAtom*> Rule;
	vector<hsfcSCLStratum*> Stratum;

protected:

private:
	void DeleteRules();
	void DeleteStatements();
	void DeleteStrata();
	void DeleteTerm(vector<hsfcSCLAtom*>& Terms, int Index, bool DeleteChildren);
	bool Normalise();
	bool Stratify();

	hsfcLexicon* Lexicon;

};



