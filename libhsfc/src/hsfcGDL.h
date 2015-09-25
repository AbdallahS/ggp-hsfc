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

#include "hsfcWFT.h"

//=============================================================================
// CLASS: hsfcGDLAtom
//=============================================================================
class hsfcGDLAtom {

public:
	hsfcGDLAtom(hsfcLexicon* Lexicon);
	~hsfcGDLAtom(void);

	void Initialise();
	bool Read(hsfcWFTElement* WFT);
	void DeleteTerms();
	void Print();

	vector<hsfcGDLAtom*> Term;
	int PredicateIndex;

protected:

private:
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
	bool Read(hsfcWFT* WFT);
	void Print();

	vector<hsfcGDLAtom*> Statement;
	vector<hsfcGDLAtom*> Rule;

protected:

private:
	void DeleteRules();
	void DeleteStatements();

	hsfcLexicon* Lexicon;

};

