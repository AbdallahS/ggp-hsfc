//=============================================================================
// Project: High Speed Forward Chaining
// Module: Well Formed Text
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

#include "hsfcLexicon.h"

//=============================================================================
// CLASS: hsfcWFTElement
//=============================================================================
class hsfcWFTElement {

public:
	hsfcWFTElement(hsfcLexicon* Lexicon);
	~hsfcWFTElement(void);

	void Initialise(const char* Script, int Length);
	hsfcWFTElement* AddChild(const char* Script, int Length);
	bool Match(const char* Value);
	void RemoveComments(const char* Prefix);
	int TextLength();
	char* AsText(char* Text, bool Space);
	void Print();

    int Level;
	hsfcWFTElement* Parent;
	vector<hsfcWFTElement*> Child;
	unsigned int LexiconIndex;

protected:

private:
	void DeleteChildren();

	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcTextStructure
//=============================================================================
class hsfcTextStructure {

public:
	hsfcTextStructure(hsfcLexicon* Lexicon);
	~hsfcTextStructure(void);

	void Initialise();
	void RemoveComments(const char* Prefix);
	hsfcWFTElement* AddElement(hsfcWFTElement* Parent, const char* Script, int Length);


	hsfcWFTElement* InsertElement(hsfcWFTElement* Target, const char* Script, int Length, int Level);
	hsfcWFTElement* NewElement(); 
	hsfcWFTElement* Item(int Index);
	char* AsText();
	void Print();

	hsfcWFTElement* RootElement;

protected:

private:
	hsfcLexicon* Lexicon;

};

//=============================================================================
// CLASS: hsfcWFT
//=============================================================================
class hsfcWFT {

public:
	hsfcWFT(hsfcLexicon* Lexicon);
	~hsfcWFT(void);

	void Initialise();
	void Load(const char* Script, const char* CommentPrefix);
	void ReadFile(const char* FileName, const char* CommentPrefix);
	//void RemoveComments(const char* Prefix);
	char* AsText();
	void Print();

	hsfcTextStructure* Structure;

protected:

private:
	hsfcLexicon* Lexicon;

	char* CleanUpKeywords(const char* Script);
	void Replace(char* Text, char* Pattern);

};

