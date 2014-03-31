//=============================================================================
// Project: High Speed Forward Chaining
// Module: GDL
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcGDL.h"

using namespace std;

//=============================================================================
// CLASS: hsfcGDLAtom
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGDLAtom::hsfcGDLAtom(hsfcLexicon* Lexicon){

	// Allocate the memory
	this->Relation = NULL;
	this->TermIndex = 0;

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGDLAtom::~hsfcGDLAtom(void){

	// Destroy the memory
	if (this->Relation != NULL) delete this->Relation;

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcGDLAtom::Initialise(){

	// Initialise the properties
	if (this->Relation != NULL) delete this->Relation;
	this->Relation = NULL;
	this->TermIndex = 0;

}

//-----------------------------------------------------------------------------
// FromGDLAtom
//-----------------------------------------------------------------------------
void hsfcGDLAtom::FromGDLAtom(hsfcGDLAtom* Source) {

	this->Initialise();

	// Copy the source
	if (Source->TermIndex != 0) {
		this->TermIndex = Source->TermIndex;
	}
	if (Source->Relation != NULL) {
		this->Relation = new hsfcGDLRelation(this->Lexicon);
		this->Relation->FromGDLRelation(Source->Relation);
	}

}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------
int hsfcGDLAtom::Read(char* Text) {

	char TermText[128];
	int Index = 0;

	this->Initialise();

	// Is it a single term or a relation
	if (Text[0] == '(') {
		// Create a new relation and read it
		this->Relation = new hsfcGDLRelation(this->Lexicon);
		this->Relation->Initialise();
		Index = this->Relation->Read(Text);
	} else {
		// Read the term text 
		TermText[0] = 0;
		for (Index = 0; Index < 127; Index++) {
			if ((Text[Index] <= ' ') || (Text[Index] == '(') || (Text[Index] == ')')) {
				TermText[Index] = 0;
				goto AddTerm;
			}
			TermText[Index] = Text[Index];
		}
		// Term text is too long
		printf("Error: Term text too long\n %s\n", Text);
		abort();

AddTerm:
		// Was there any text
		if (TermText[0] == 0) {
			printf("Error: No term text found\n %s\n", Text);
			abort();
		}
		
		// Load the term text
		this->TermIndex = this->Lexicon->Index(TermText);
	}

	return Index;

}

//-----------------------------------------------------------------------------
// Terms
//-----------------------------------------------------------------------------
void hsfcGDLAtom::Terms(vector<hsfcTuple>& Term) {

	hsfcTuple NewTerm;

	// Important: The list must be cleared before the original call
	// as calls may be recursive

	// Is it a single term
	if (this->TermIndex!= 0) {
		NewTerm.RelationIndex = -1;
		NewTerm.ID = this->TermIndex;
		Term.push_back(NewTerm);
		return;
	}

	// Is it a relation
	if (this->Relation != NULL) {
		this->Relation->Terms(Term);
		return;
	}

}

// --- Overload ---------------------------------------------------------------
void hsfcGDLAtom::Terms(int PredicateIndex, vector<hsfcGDLTerm>& Term) {

	hsfcGDLTerm NewTerm;

	// Important: The list must be cleared before the original call
	// as calls may be recursive

	// Is it a single term
	if (this->TermIndex!= 0) {
		NewTerm.PredicateIndex = PredicateIndex;
		NewTerm.Tuple.RelationIndex = -1;
		NewTerm.Tuple.ID = this->TermIndex;
		Term.push_back(NewTerm);
		return;
	}

	// Is it a relation
	if (this->Relation != NULL) {
		this->Relation->Terms(Term);
		return;
	}


}

//-----------------------------------------------------------------------------
// AsText
//-----------------------------------------------------------------------------
int hsfcGDLAtom::AsText(char* Text){

	int Index;

	// Construct the text
	Index = 0;
	if (this->TermIndex != 0) {
		Index += sprintf(&Text[Index], "%s", this->Lexicon->Text(this->TermIndex));
	}
	if (this->Relation != NULL) {
		Index += this->Relation->AsText(Text); 
	}

	return Index;

}

//=============================================================================
// CLASS: hsfcGDLRelation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGDLRelation::hsfcGDLRelation(hsfcLexicon* Lexicon){

	// Allocate the memory
	this->Atom.resize(16);

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGDLRelation::~hsfcGDLRelation(void){

	// Free the atoms
	for (unsigned int i = 0; i < this->Atom.size(); i++) {
		delete this->Atom[i];
	}
	this->Atom.clear();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcGDLRelation::Initialise(){

	// Free the atoms
	for (unsigned int i = 0; i < this->Atom.size(); i++) {
		delete this->Atom[i];
	}
	this->Atom.clear();

	// Set the properties
	this->Not = false;

}

//-----------------------------------------------------------------------------
// FromGDLRelation
//-----------------------------------------------------------------------------
void hsfcGDLRelation::FromGDLRelation(hsfcGDLRelation* Source){

	hsfcGDLAtom* NewAtom;

	this->Initialise();

	// Copy the atoms
	for (unsigned int i = 0; i < Source->Atom.size(); i++) {
		// Copy the atom
		NewAtom = this->AddAtom();
		NewAtom->FromGDLAtom(Source->Atom[i]);
	}

}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------
int hsfcGDLRelation::Read(char* Text) {

	hsfcGDLAtom* NewAtom;
	int Level;
	int Index;

	/* Example of relation structures
	terminal
	(cell 1 1)
	(legal xplayer (move 1 1))
	(succ ?x ?y)
	*/

	// Look for empty string 
	if (Text[0] == 0) return 0;

	// Traverse the text reading atoms / relations as we go
	Index = 0;
	Level = 0;
	do {

		// Ignore any leading white space
		if (Text[Index] <= ' ') {
			Index++;
			continue;
		}

		// Is this the end of a relation
		if (Text[Index] == ')') {
			Level--;
			if (Level > 0) Index++;  // Must return pointing to the last ')'
			continue;
		}

		// Is this the start of a relation
		if (Text[Index] == '(') {
			Level++;
			// Is this the first '(' or an embedded relation
			if (Level == 1) {
				Index++;
				continue;
			}
		}

		// This must be an atom
		NewAtom = this->AddAtom();
		Index += NewAtom->Read(&Text[Index]);

	} while ((Level > 0) && (Text[Index] != 0));

	return Index;

}

//-----------------------------------------------------------------------------
// NormaliseTerms
//-----------------------------------------------------------------------------
void hsfcGDLRelation::NormaliseTerms() {

	char Predicate[256];

	// This must only be run once
	// Calls are recursive

	if (this->Lexicon->Match(this->PredicateIndex(), "distinct")) return;

	// Is this an (next (...)) relation
	if (this->Lexicon->Match(this->PredicateIndex(), "next")) {
		sprintf(Predicate, "next>%s", this->Lexicon->Text(this->Atom[1]->Relation->PredicateIndex()));
		this->Atom[0]->TermIndex = this->Lexicon->Index(Predicate);
	}

	// Converts (nested (mark ?x ?y) (piece ?p))
	// into (nested|2 (mark|2 ?x ?y) (piece|1 ?p))
	sprintf(Predicate, "%s|%d", this->Lexicon->Text(this->PredicateIndex()), this->Arity());
	this->Atom[0]->TermIndex = this->Lexicon->Index(Predicate);

	// Look at each atom for a nested relation
	for (unsigned int i = 0; i < this->Atom.size(); i++) {
		if (this->Atom[i]->Relation != NULL) {
			this->Atom[i]->Relation->NormaliseTerms();
		}
	}

}

//-----------------------------------------------------------------------------
// FindZeroArity
//-----------------------------------------------------------------------------
void hsfcGDLRelation::FindZeroArity() {

	hsfcGDLRelation* NewRelation;
	hsfcGDLAtom* NewAtom;
	bool FoundZeroArity;
	int AtomIndex;

	FoundZeroArity = false;
	if (this->Lexicon->Match(this->PredicateIndex(), "init")) {
		if (this->Atom[1]->Relation == NULL) {
			FoundZeroArity = true;
			AtomIndex = 1;
		}
	}
	if (this->Lexicon->Match(this->PredicateIndex(), "next")) {
		if ((this->Atom[1]->Relation == NULL) && (!this->Lexicon->PartialMatch(this->Atom[1]->TermIndex, "?"))) {
			FoundZeroArity = true;
			AtomIndex = 1;
		}
	}
	if (this->Lexicon->Match(this->PredicateIndex(), "not")) {
		if ((this->Atom[1]->Relation == NULL) && (!this->Lexicon->PartialMatch(this->Atom[1]->TermIndex, "?"))) {
			FoundZeroArity = true;
			AtomIndex = 1;
		}
	}
	if (this->Lexicon->Match(this->PredicateIndex(), "true")) {
		if ((this->Atom[1]->Relation == NULL) && (!this->Lexicon->PartialMatch(this->Atom[1]->TermIndex, "?"))) {
			FoundZeroArity = true;
			AtomIndex = 1;
		}
	}

	if (FoundZeroArity) {
		// Copy the atom
		NewAtom = new hsfcGDLAtom(this->Lexicon);
		NewAtom->FromGDLAtom(this->Atom[AtomIndex]);
		// Create a new relation
		NewRelation = new hsfcGDLRelation(this->Lexicon);
		NewRelation->Initialise();
		// Add the old atom to the new zero arity relation
		NewRelation->Atom.push_back(NewAtom);
		// Add the new relation to the atoms for the primary relation
		this->Atom[AtomIndex]->Relation = NewRelation;
		this->Atom[AtomIndex]->TermIndex = 0;
	}

}

//-----------------------------------------------------------------------------
// Terms
//-----------------------------------------------------------------------------
void hsfcGDLRelation::Terms(vector<hsfcTuple>& Term) {

	// Important: The list must be cleared before the original call
	// as calls may be recursive

	// Add in the term for each atom
	for (unsigned int i = 0; i < this->Atom.size(); i++) {
		this->Atom[i]->Terms(Term);
	}

}

// --- Overload ---------------------------------------------------------------
void hsfcGDLRelation::Terms(vector<hsfcGDLTerm>& Term) {

	// Important: The list must be cleared before the original call
	// as calls may be recursive

	// Add in the term for each atom
	for (unsigned int i = 0; i < this->Atom.size(); i++) {
		this->Atom[i]->Terms(this->PredicateIndex(), Term);
	}

}

//-----------------------------------------------------------------------------
// Arity
//-----------------------------------------------------------------------------
int hsfcGDLRelation::Arity(){

	return this->Atom.size() - 1;

}

//-----------------------------------------------------------------------------
// PredicateIndex
//-----------------------------------------------------------------------------
int hsfcGDLRelation::PredicateIndex(){

	if (this->Atom.size() == 0) return 0;
	return this->Atom[0]->TermIndex;

}

//-----------------------------------------------------------------------------
// AddRelation
//-----------------------------------------------------------------------------
bool hsfcGDLRelation::AddRelationDetail(vector<hsfcRelationDetail>& RelationDetail){

	hsfcRelationDetail Detail;

	// Get the detail of this relation
	Detail.PredicateIndex = this->PredicateIndex();
	Detail.Arity = this->Arity();

	// Check if its already there
	for (unsigned int i = 0; i < RelationDetail.size(); i++) {
		if (Detail.PredicateIndex == RelationDetail[i].PredicateIndex) {
			if (Detail.Arity == RelationDetail[i].Arity) {
				return true;
			} else {
				printf("Error: Conflicting arity %s\n", this->Lexicon->Text(Detail.PredicateIndex));
				return false;
			}
		}
	}

	// Allocate the memory
	RelationDetail.push_back(Detail);

	// Add the relation details for any embedded relation
	for (unsigned int i = 0; i < this->Atom.size(); i++) {
		if (this->Atom[i]->Relation != NULL) {
			if (!this->AddRelationDetail(RelationDetail)) return false;
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// AsText
//-----------------------------------------------------------------------------
int hsfcGDLRelation::AsText(char* Text){

	int Index;

	Index = 0;
	if (this->Not) Index += sprintf(&Text[Index], "not "); 
	Index += sprintf(&Text[Index], "(");
	for (unsigned int i = 0; i < this->Atom.size(); i++) {
		if (i > 0) Index += sprintf(&Text[Index], " ");
		Index += this->Atom[i]->AsText(&Text[Index]);
	}
	Index += sprintf(&Text[Index], ")");

	return Index;

}

//-----------------------------------------------------------------------------
// AddAtom
//-----------------------------------------------------------------------------
hsfcGDLAtom* hsfcGDLRelation::AddAtom(){

	hsfcGDLAtom* NewAtom;

	// Allocate the memory
	NewAtom = new hsfcGDLAtom(this->Lexicon);
	NewAtom->Initialise();
	this->Atom.push_back(NewAtom);

	return NewAtom;

}

//=============================================================================
// CLASS: hsfcGDLRule
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGDLRule::hsfcGDLRule(hsfcLexicon* Lexicon){

	// Allocate the memory
	this->Relation.resize(16);

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGDLRule::~hsfcGDLRule(void){

	// Free the Relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		delete this->Relation[i];
	}
	this->Relation.clear();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcGDLRule::Initialise(){

	// Free the Relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		delete this->Relation[i];
	}
	this->Relation.clear();

}

//-----------------------------------------------------------------------------
// FromGDLRule
//-----------------------------------------------------------------------------
void hsfcGDLRule::FromGDLRule(hsfcGDLRule* Source){

	hsfcGDLRelation* NewRelation;

	this->Initialise();

	// Copy the Relations
	for (unsigned int i = 0; i < Source->Relation.size(); i++) {
		// Copy the Relation
		NewRelation = this->AddRelation();
		NewRelation->FromGDLRelation(Source->Relation[i]);
	}

}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------
int hsfcGDLRule::Read(char* Text) {

	hsfcGDLRelation* NewRelation;
	int Level;
	int Index;

	/* Example of Rule structures
	terminal
	(cell 1 1)
	(legal xplayer (move 1 1))
	(succ ?x ?y)
	*/

	// Look for empty string 
	if (Text[0] == 0) return 0;

	// Traverse the text reading Relations / Rules as we go
	// All rules start with '(<= '
	Index = 4;
	Level = 1;
	do {

		// Ignore any leading white space
		if (Text[Index] <= ' ') {
			Index++;
			continue;
		}

		// Is this the end of a relation
		if (Text[Index] == ')') {
			Level--;
			if (Level > 0) Index++;  // Must return pointing to the last ')'
			continue;
		}

		// Is this the start of a relation
		if (Text[Index] == '(') {
			Level++;
		}

		// This must be a relation
		NewRelation = this->AddRelation();
		Index += NewRelation->Read(&Text[Index]);

	} while ((Level > 0) && (Text[Index] != 0));

	return Index;

}

//-----------------------------------------------------------------------------
// Arity
//-----------------------------------------------------------------------------
int hsfcGDLRule::Arity(){

	return this->Relation.size() - 1;

}

//-----------------------------------------------------------------------------
// AsText
//-----------------------------------------------------------------------------
int hsfcGDLRule::AsText(char* Text){

	int Index;

	Index = 0;
	Index += sprintf(&Text[Index], "(<= ");
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		if (i > 0) Index += sprintf(&Text[Index], "    ");
		Index += this->Relation[i]->AsText(&Text[Index]);
		Index += sprintf(&Text[Index], "\n");
	}
	Index += sprintf(&Text[Index], ")");

	return Index;

}

//-----------------------------------------------------------------------------
// AddRelation
//-----------------------------------------------------------------------------
hsfcGDLRelation* hsfcGDLRule::AddRelation(){

	hsfcGDLRelation* NewRelation;

	// Allocate the memory
	NewRelation = new hsfcGDLRelation(this->Lexicon);
	NewRelation->Initialise();
	this->Relation.push_back(NewRelation);

	return NewRelation;

}

//=============================================================================
// CLASS: hsfcGDL
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcGDL::hsfcGDL(hsfcLexicon* Lexicon){

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcGDL::~hsfcGDL(void){

	// Free the relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		delete this->Relation[i];
	}
	this->Relation.clear();

	// Free the rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete this->Rule[i];
	}
	this->Rule.clear();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcGDL::Initialise(){

	// Free the relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		delete this->Relation[i];
	}
	this->Relation.clear();

	// Free the rules
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		delete this->Rule[i];
	}
	this->Rule.clear();

}

//-----------------------------------------------------------------------------
// Read
//-----------------------------------------------------------------------------
int hsfcGDL::Read(char* Script) {

	hsfcGDLRelation* NewRelation;
	hsfcGDLRule* NewRule;
	int Index;

	/* Example of GDL structures
	(role black)
	(init (cell a 1 wr))
	(<= terminal
		(true (step 201)))
	(<= (next (cell ?u ?v b))
		(does ?player (move ?p ?u ?v ?x ?y)))
	(<= (legal ?player (move ?piece ?u ?v ?x ?y))
		(true (control ?player))
		(or (true (check ?player rook ?tx ?ty)) (true (check ?player queen ?tx ?ty)))
		(not (occupied_by_player ?x ?y ?player))
		(piece_owner_type ?piece ?player ?ptype)
		(distinct ?ptype king)
		(piece_owner_type ?king ?player king)
		(true (cell ?kx ?ky ?king))
		(legal2 ?player (move ?piece ?u ?v ?x ?y))
		(blocks_rook_threat ?x ?y ?tx ?ty ?kx ?ky)
		(not (threatened_with_capture ?player ?tx ?ty ?kx ?ky ?u ?v)))
	)
	*/

	// Traverse the script reading rules / relations as we go
	Index = 0;
	while (true) {

		// Ignore any leading white space or the last ')' of the rule or relation
		while ((Script[Index] <= ' ') || (Script[Index] == ')')) {
			if (Script[Index] == 0) return Index;
			Index++;
		}

		// Is this a rule or relation
		if (strncmp(&Script[Index], "(<= ", 4) == 0) {
			// Read the rule
			NewRule = this->AddRule();
			Index += NewRule->Read(&Script[Index]);
		} else {
			// Read the relation
			NewRelation = this->AddRelation();
			Index += NewRelation->Read(&Script[Index]);
		}
	}

}

//-----------------------------------------------------------------------------
// GetRelationDetails
//-----------------------------------------------------------------------------
bool hsfcGDL::GetRelationDetails() {

	// Construct a list of relations and their arity
	this->RelationDetail.clear();

	// Check all relations
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		if (!this->Relation[i]->AddRelationDetail(this->RelationDetail)) return false;
	}

	// Check all rule relations
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		for (unsigned int j = 0; j < this->Rule[i]->Relation.size(); j++) {
			if (!this->Rule[i]->Relation[j]->AddRelationDetail(this->RelationDetail)) return false;
		}
	}

	for (unsigned int i = 0; i < this->RelationDetail.size(); i++) {
		printf("%s / %d\n", this->Lexicon->Text(this->RelationDetail[i].PredicateIndex), this->RelationDetail[i].Arity);
	}

	// Find any literals that are really zero arity relations
	// eg. (legal agent (dirt)) (at 1 1 dirt) ==> (at 1 1 (dirt))


	return true;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcGDL::Print(char* Title) {

	char Text[2048];
	int Index;

	printf("\n--- %s ----------------------------------------------------\n", Title);

	// Print relations
	printf("Relations\n");
	for (unsigned int i = 0; i < this->Relation.size(); i++) {
		Index = this->Relation[i]->AsText(Text);
		Text[Index] = 0;
		printf("%s\n", Text);
	}

	// Print rules
	printf("Rules\n");
	for (unsigned int i = 0; i < this->Rule.size(); i++) {
		Index = this->Rule[i]->AsText(Text);
		Text[Index] = 0;
		printf("%s\n", Text);
	}

}

//-----------------------------------------------------------------------------
// AddRelation
//-----------------------------------------------------------------------------
hsfcGDLRelation* hsfcGDL::AddRelation(){

	hsfcGDLRelation* NewRelation;

	// Allocate the memory
	NewRelation = new hsfcGDLRelation(this->Lexicon);
	NewRelation->Initialise();
	this->Relation.push_back(NewRelation);

	return NewRelation;

}

//-----------------------------------------------------------------------------
// AddRule
//-----------------------------------------------------------------------------
hsfcGDLRule* hsfcGDL::AddRule(){

	hsfcGDLRule* NewRule;

	// Allocate the memory
	NewRule = new hsfcGDLRule(this->Lexicon);
	NewRule->Initialise();
	this->Rule.push_back(NewRule);

	return NewRule;

}


