//=============================================================================
// Project: High Speed Forward Chaining
// Module: Domain
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
// CLASS: hsfcDomainManager
//=============================================================================
class hsfcDomainManager {

public:
	hsfcDomainManager(hsfcLexicon* Lexicon);
	~hsfcDomainManager(void);

	void Initialise();
	void CreateDomains(hsfcSchema* Schema);
	void FreeDomains();
	bool BuildDomains(hsfcSchema* Schema);
	bool BuildDomain(hsfcRelationSchema* RelationSchema, unsigned int Index);
	bool RebuildRigidDomain(hsfcRelationSchema* RelationSchema, unsigned int Index);
	bool TermsToID(int RelationIndex, hsfcTuple Term[], unsigned int& ID);
	bool IDToTerms(int RelationIndex, hsfcTuple Term[], unsigned int ID);
	bool LoadTerms(hsfcSCLAtom* SCLAtom, hsfcTuple Term[]);
	//unsigned int RelationAsKIF(hsfcTuple& Relation, char** KIF, bool AsGDL);
	unsigned int RelationAsKIF(hsfcTuple& Relation, char** KIF);
	void Print();

	hsfcDomain* Domain;

protected:

private:
	unsigned int KIFLength(hsfcTuple& Relation);
	void TestDomains();

	hsfcLexicon* Lexicon;
	unsigned int DomainSize;

};


