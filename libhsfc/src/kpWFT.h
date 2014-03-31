//=============================================================================
// Project: High Speed Forward Chaining
// Module: WellFormedText
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
// CLASS: kpTextElement
//=============================================================================
class kpTextElement {

public:
	kpTextElement(void);
	~kpTextElement(void);

	void Initialise();
	void AddText(const char* Script, int Length);
	bool Match(char* Value);
	kpTextElement* Parent();

	char* Text;
    int Level;

	kpTextElement* NextElement;
	kpTextElement* PrevElement;

protected:

private:

};

//=============================================================================
// CLASS: kpTextStructure
//=============================================================================
class kpTextStructure {

public:
	kpTextStructure(void);
	~kpTextStructure(void);

	void Initialise(char* Name);
	kpTextElement* AddElement(const char* Script, int Length, int Level);
	kpTextElement* InsertElement(kpTextElement* Target, const char* Script, int Length, int Level);
	kpTextElement* DeleteElement(kpTextElement* Target); 
	kpTextElement* NewElement(); 
	kpTextElement* Item(int Index);
	int Count();
	void Print();

	kpTextElement* RootElement;
	kpTextElement* Recycled;

protected:

private:

};

//=============================================================================
// CLASS: kpWFT
//=============================================================================
class kpWFT {

public:
	kpWFT(void);
	~kpWFT(void);

	void Initialise(char* Name);
	void Load(const char* Script);
	void LoadFlat(const char* Script);
	void ReadFile(char* FileName);
	bool Match(char* Key, int ArgumentNo, char* Value);
	bool Exists(char* Key, int ArgumentNo);
	char* Find(char* Key, int ArgumentNo, bool Bracketed);
	void Find(char* Key, int ArgumentNo, bool Bracketed, char* Buffer);
	char* FindWhole(char* Key, int ArgumentNo, bool Bracketed, bool IncludeKey);
	char* AsText();
	void Print();

	kpTextStructure* Structure;

protected:

private:

};

