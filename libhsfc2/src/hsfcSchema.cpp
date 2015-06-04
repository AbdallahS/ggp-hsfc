//=============================================================================
// Project: High Speed Forward Chaining
// Module: Schema
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "hsfcSchema.h"

//=============================================================================
// CLASS: hsfcDomainSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcDomainSchema::hsfcDomainSchema(hsfcLexicon* Lexicon){

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcDomainSchema::~hsfcDomainSchema(void){


}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcDomainSchema::Initialise(unsigned int NameID){

	// Clear the schema
	this->NameID = NameID;
	this->Term.clear();

}

//-----------------------------------------------------------------------------
// AddTerm
//-----------------------------------------------------------------------------
bool hsfcDomainSchema::AddTerm(hsfcTuple& NewTerm){

	int Target;
	int LowerBound;
	int UpperBound;
	int Compare;

    // Binary search of a specific domain; adding any unfound values in sort order
	Target = 0;
	LowerBound = 0;
	UpperBound = 0;
	Compare = -1;
	
	// Is the domain empty
	if (this->Term.size() > 0) {

		UpperBound = this->Term.size() - 1;

		// Look for the term according to its value
		while (LowerBound <= UpperBound) {
			// Find the target value
			Target = (LowerBound + UpperBound) / 2;
			Compare = NewTerm.Index - this->Term[Target].Index;
			if (Compare == 0) {
				Compare = NewTerm.ID - this->Term[Target].ID;
			}
			// Compare the values
			if (Compare == 0) return false;
			if (Compare < 0) UpperBound = Target - 1;
			if (Compare > 0) LowerBound = Target + 1;
		}
	}

	// Not found; so add to domain

	// Last compare will have LowerBound == UpperBound == Target
	// If Compare > 0 then new value is after Target
	// If Compare < 0 then new value is before Target
	if (Compare > 0) Target++;

	// Add the new value to the domain before the Target
	if (Target == this->Term.size()) {
		this->Term.push_back(NewTerm);
	} else {
		this->Term.insert(this->Term.begin() + Target, NewTerm);
	}

	return true;

}

//-----------------------------------------------------------------------------
// AddTerms
//-----------------------------------------------------------------------------
bool hsfcDomainSchema::AddTerms(vector<hsfcTuple>& Term) {

	bool Result;

	// Add each term in the list
	Result = false;
	for (unsigned int i = 0; i < Term.size(); i++) {
		if (this->AddTerm(Term[i])) Result = true;
	}

	return Result;

}

//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------
void hsfcDomainSchema::Copy(vector<hsfcTuple>& Destination) {

	hsfcTuple NewTerm;

	// Copy all the terms from the domain
	for (unsigned int i = 0; i < this->Term.size(); i++) {
		// Is it a simple term or a relation
		if (this->Term[i].Index == 0) {
			NewTerm.Index = this->Term[i].Index;
			NewTerm.ID = this->Term[i].ID;
		} else {
			NewTerm.Index = this->Lexicon->GDLIndex(this->Term[i].Index);
			NewTerm.ID = this->Term[i].ID;
		}
		Destination.push_back(NewTerm);
	}

}

//-----------------------------------------------------------------------------
// Intersection
//-----------------------------------------------------------------------------
void hsfcDomainSchema::Intersection(vector<hsfcTuple>& Destination) {

	unsigned int TermIndex;
	unsigned int DestinationIndex;

	// Work through the two lists simultaneously
	TermIndex = 0;
	DestinationIndex = 0;
	while ((TermIndex < this->Term.size()) && (DestinationIndex < Destination.size())) {

		// Are the terms the same
		if ((this->Term[TermIndex].Index == Destination[DestinationIndex].Index) && (this->Term[TermIndex].ID == Destination[DestinationIndex].ID)) {
			TermIndex++;
			DestinationIndex++;
			continue;
		}

		// Is the local domain term smaller than the outside term
		if (this->Term[TermIndex].Index < Destination[DestinationIndex].Index) {
			TermIndex++;
			continue;
		} else if ((this->Term[TermIndex].Index == Destination[DestinationIndex].Index) && (this->Term[TermIndex].ID < Destination[DestinationIndex].ID)) {
			TermIndex++;
			continue;
		}

		// The outside term must be smaller and not present in the local domain
		Destination.erase(Destination.begin() + DestinationIndex);

	}

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcDomainSchema::Print(){

	for (unsigned int i = 0; i < this->Term.size(); i++) {
		if (this->Term[i].Index == 0) {
			this->Lexicon->IO->FormatToLog(0, false, " %10s", this->Lexicon->Text(this->Term[i].ID));
		} else {
			this->Lexicon->IO->FormatToLog(0, false, " %10s", this->Lexicon->Relation(this->Term[i].Index));
		}
	}
	this->Lexicon->IO->WriteToLog(0, false, "\n");

}


//=============================================================================
// CLASS: hsfcRelationSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRelationSchema::hsfcRelationSchema(hsfcLexicon* Lexicon){

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcRelationSchema::~hsfcRelationSchema(void){

	// Clear the schema
	this->DeleteDomainSchema();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRelationSchema::Initialise(const unsigned int NameID, int Arity, unsigned int Index){

	hsfcDomainSchema* NewDomainSchema;

	// Clear the schema
	this->DeleteDomainSchema();

	// Set the properties
	this->NameID = NameID;
	this->Arity = Arity;
	this->Index = Index;
	this->IsInState = false;
	this->Fact = hsfcFactNone;

	// Create the domains
	for (int i = 0; i < Arity; i++) {
		NewDomainSchema = new hsfcDomainSchema(this->Lexicon);
		NewDomainSchema->Initialise(NameID);
		this->DomainSchema.push_back(NewDomainSchema);
	}

}

////-----------------------------------------------------------------------------
//// AddDomainTerm
////-----------------------------------------------------------------------------
//bool hsfcRelationSchema::AddDomainTerm(hsfcSCLAtom* SCLAtom, unsigned int DomainIndex) {
//
//	hsfcTuple Term;
//
//	// Check for integtrity
//	if (DomainIndex >= this->DomainSchema.size()) return false;
//
//	// Is it a single term or an embedded relation
//	if (SCLAtom->Term.size() == 0) {
//		Term.Index = 0;
//		Term.ID = SCLAtom->NameID;
//	} else {
//		Term.Index = SCLAtom->;
//		Term.ID = 0;
//	}
//
//	// Add the term
//	return this->DomainSchema[DomainIndex]->AddTerm(Term);
//
//}
//
//-----------------------------------------------------------------------------
// Copy
//-----------------------------------------------------------------------------
void hsfcRelationSchema::Copy(vector<hsfcTuple>& Destination, unsigned int DomainIndex) {

	// Copy the contents of the domain
	this->DomainSchema[DomainIndex]->Copy(Destination);

}

//-----------------------------------------------------------------------------
// Intersection
//-----------------------------------------------------------------------------
void hsfcRelationSchema::Intersection(vector<hsfcTuple>& Destination, unsigned int DomainIndex) {

	// Copy the contents of the domain
	this->DomainSchema[DomainIndex]->Intersection(Destination);

}

//-----------------------------------------------------------------------------
// AddTerms
//-----------------------------------------------------------------------------
bool hsfcRelationSchema::AddTerms(vector<hsfcTuple>& Term, unsigned int DomainIndex) {

	// Copy the contents of the domain
	return this->DomainSchema[DomainIndex]->AddTerms(Term);

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRelationSchema::Print(){

	this->Lexicon->IO->FormatToLog(0, false, "%s\n", this->Lexicon->Text(this->NameID));
	for (unsigned int i = 0; i < this->DomainSchema.size(); i++) {
		this->Lexicon->IO->FormatToLog(0, false, "     %2d:", i, NULL);
		this->DomainSchema[i]->Print();
	}

}

//-----------------------------------------------------------------------------
// DeleteDomainSchema
//-----------------------------------------------------------------------------
void hsfcRelationSchema::DeleteDomainSchema(){

	// Destroy all of the relations
	for (unsigned int i = 0; i < this->DomainSchema.size(); i++) {
		delete this->DomainSchema[i];
	}
	this->DomainSchema.clear();

}

//=============================================================================
// CLASS: hsfcRuleRelationSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRuleRelationSchema::hsfcRuleRelationSchema(hsfcLexicon* Lexicon){

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcRuleRelationSchema::~hsfcRuleRelationSchema(void){

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRuleRelationSchema::Initialise(hsfcSCLAtom* SCL){

	// Initialise
	this->TermSchema.clear();
	this->SCL = SCL;
	this->Function = hsfcFunctionNone;
	this->Type = hsfcRuleInput;
	this->ReferenceSize = 0;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRuleRelationSchema::Print(){

	for (unsigned int i = 0; i < this->TermSchema.size(); i++) {
		if (this->TermSchema[i].Type == hsfcTypeRelation) {
			this->Lexicon->IO->FormatToLog(0, false, "%s  ", this->Lexicon->Text(this->TermSchema[i].NameID));
		}
		if (this->TermSchema[i].Type == hsfcTypeFixed) {
			this->Lexicon->IO->FormatToLog(0, false, "'%s'  ", this->Lexicon->Text(this->TermSchema[i].NameID));
		}
		if (this->TermSchema[i].Type == hsfcTypeVariable) {
			this->Lexicon->IO->FormatToLog(0, false, "?%02d  ", this->TermSchema[i].VariableIndex, NULL);
		}
	}
	this->Lexicon->IO->WriteToLog(0, false, "\n");

}

//=============================================================================
// CLASS: hsfcRuleSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcRuleSchema::hsfcRuleSchema(hsfcLexicon* Lexicon){

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcRuleSchema::~hsfcRuleSchema(void){

	// Destroy all of the schemas
	this->DeleteRelationSchema();
	this->DeleteVariableSchema();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcRuleSchema::Initialise(){

	// Destroy all of the schemas
	this->DeleteRelationSchema();
	this->DeleteVariableSchema();

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcRuleSchema::Print(){

	for (unsigned int i = 0; i < this->RelationSchema.size(); i++) {
		if (this->RelationSchema[i]->Type == hsfcRuleOutput) this->Lexicon->IO->WriteToLog(0, false, "<= ");
		if (this->RelationSchema[i]->Type == hsfcRuleInput) this->Lexicon->IO->WriteToLog(0, false, "=> ");
		if (this->RelationSchema[i]->Type == hsfcRuleCondition) this->Lexicon->IO->WriteToLog(0, false, "?? ");
		if (this->RelationSchema[i]->Type == hsfcRulePreCondition) this->Lexicon->IO->WriteToLog(0, false, "!! ");
		this->RelationSchema[i]->Print();
	}

	this->Lexicon->IO->FormatToLog(0, false, "%s\n", "Variables");
	for (unsigned int i = 0; i < this->VariableSchema.size(); i++) {
		this->Lexicon->IO->FormatToLog(0, false, "   %02d:", i, NULL);
		this->VariableSchema[i]->Print();
	}

}

//-----------------------------------------------------------------------------
// DeleteRelationSchema
//-----------------------------------------------------------------------------
void hsfcRuleSchema::DeleteRelationSchema(){

	// Destroy all of the relations
	for (unsigned int i = 0; i < this->RelationSchema.size(); i++) {
		delete this->RelationSchema[i];
	}
	this->RelationSchema.clear();

}

//-----------------------------------------------------------------------------
// DeleteVariableSchema
//-----------------------------------------------------------------------------
void hsfcRuleSchema::DeleteVariableSchema(){

	// Destroy all of the relations
	for (unsigned int i = 0; i < this->VariableSchema.size(); i++) {
		delete this->VariableSchema[i];
	}
	this->VariableSchema.clear();

}

//=============================================================================
// CLASS: hsfcStratumSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcStratumSchema::hsfcStratumSchema(hsfcLexicon* Lexicon) {

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcStratumSchema::~hsfcStratumSchema(void){

	this->DeleteRuleSchema();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcStratumSchema::Initialise(hsfcSCLStratum* SCLStratum){

	// Clear the schema
	this->DeleteRuleSchema();

	// Set properties
	this->SCLStratum = SCLStratum;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcStratumSchema::Print(){

	for (unsigned int i = 0; i < this->RuleSchema.size(); i++) {
		this->Lexicon->IO->FormatToLog(0, false, "\nRule %d\n", i, NULL);
		this->RuleSchema[i]->Print();
	}
	this->Lexicon->IO->WriteToLog(0, false, "\n-------------------------------\n");

}

//-----------------------------------------------------------------------------
// DeleteRuleSchema
//-----------------------------------------------------------------------------
void hsfcStratumSchema::DeleteRuleSchema(){

	// Destroy all of the Rules
	for (unsigned int i = 0; i < this->RuleSchema.size(); i++) {
		delete this->RuleSchema[i];
	}
	this->RuleSchema.clear();

}

//=============================================================================
// CLASS: hsfcSchema
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
hsfcSchema::hsfcSchema(hsfcLexicon* Lexicon) {

	// Set up the Lexicon
	this->Lexicon = Lexicon;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
hsfcSchema::~hsfcSchema(void){

	this->DeleteRelationSchema();
	this->DeleteStratumSchema();

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void hsfcSchema::Initialise(){

	// Clear the schema
	this->DeleteRelationSchema();
	this->DeleteStratumSchema();

	// Add the 0th relation for the Lexicon
	this->RelationSchema.push_back(NULL);
	this->Rigid.clear();
	this->Permanent.clear();
	this->Initial.clear();
	this->Next.clear();

}

//-----------------------------------------------------------------------------
// Create
//-----------------------------------------------------------------------------
bool hsfcSchema::Create(hsfcSCL* SCL){

	int NewEntryCount;

	// Record the SCL
	this->SCL = SCL;

	// Initialise the schema
	this->Initialise();

	// Create a schema for each of the relations
	for (unsigned int i = 0; i < SCL->Statement.size(); i++) {
		if (!this->CreateRelationSchema(SCL->Statement[i])) {
			return false;
		}
	}

	// Create a schema for each of the strata
	for (unsigned int i = 0; i < SCL->Stratum.size(); i++) {
		if (!this->CreateStratumSchema(SCL->Stratum[i])) {
			return false;
		}
	}

	// Complete the schema for the relations
	if (!this->CompleteRelationSchema()) {
		return false;
	}

	// Build the domains from each of the relations
	for (unsigned int i = 0; i < SCL->Statement.size(); i++) {
		if (!this->BuildRelationDomains(SCL->Statement[i])) {
			return false;
		}
	}

	// Build the domains from the rules
	for (unsigned int i = 0; i < this->StratumSchema.size(); i++) {
		if (!this->BuildStratumDomainsFix(this->StratumSchema[i])) {
			return false;
		}
	}

	// Build the domains from the rules
	NewEntryCount = 1;
	while (NewEntryCount > 0) {

		NewEntryCount = 0;
		for (unsigned int i = 0; i < this->StratumSchema.size(); i++) {
			if (!this->BuildStratumDomainsVar(this->StratumSchema[i], &NewEntryCount)) {
				return false;
			}
		}

	}

	// Identify type of relations
	this->Lexicon->IO->WriteToLog(2, true, "Identify Relation Types ...\n");
	if (!this->IdentifyRelationTypes()) {
		return false;
	}
	this->Lexicon->IO->WriteToLog(2, true, "succeeded\n");

	// Create Next references
	this->Lexicon->IO->WriteToLog(2, true, "Setting (next ...) references\n");
	this->SetNextReferences();

	if (this->Lexicon->IO->Parameters->LogDetail > 3) this->Print();
	if (this->Lexicon->IO->Parameters->LogDetail > 2) this->Lexicon->Print();

	return true;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void hsfcSchema::Print(){

	this->Lexicon->IO->WriteToLog(0, false, "\n--- Relation Schema ---\n");

	for (unsigned int i = 1; i < this->RelationSchema.size(); i++) {
		this->Lexicon->IO->FormatToLog(0, false, "\n%3d ", i, NULL);
		this->RelationSchema[i]->Print();
	}

	this->Lexicon->IO->WriteToLog(0, false, "\n--- Rule Schema ---\n");

	for (unsigned int i = 0; i < this->StratumSchema.size(); i++) {
		this->StratumSchema[i]->Print();
	}

}

//-----------------------------------------------------------------------------
// FindRelationSchema
//-----------------------------------------------------------------------------
hsfcRelationSchema* hsfcSchema::AddRelationSchema(unsigned int NameID, int Arity){

	hsfcRelationSchema* NewRelationSchema;

	// Is the arity too big
	if (Arity > MAX_RELATION_ARITY) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: arity too big hsfcSchema::AddRelationSchema\n");
		return NULL; 
	}

	// Are there too many relations
	if (this->RelationSchema.size() > MAX_RELATION_SCHEMAS) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: too many relations hsfcSchema::AddRelationSchema\n");
		return NULL; 
	}

	// Add the a new relation schrema
	NewRelationSchema = new hsfcRelationSchema(this->Lexicon);
	NewRelationSchema->Initialise(NameID, Arity, this->RelationSchema.size());
	this->RelationSchema.push_back(NewRelationSchema);

	// Add the name to the lexicon
	if (this->Lexicon->RelationIndex(this->Lexicon->Text(NameID), true) != this->RelationSchema.size() - 1) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: name index mismatch in hsfcSchema::AddRelationSchema\n");
		return NULL; 
	}

	return NewRelationSchema;

}

//-----------------------------------------------------------------------------
// DeleteRelations
//-----------------------------------------------------------------------------
void hsfcSchema::DeleteRelationSchema(){

	// Destroy all of the relations
	for (unsigned int i = 0; i < this->RelationSchema.size(); i++) {
		delete this->RelationSchema[i];
	}
	this->RelationSchema.clear();

}

//-----------------------------------------------------------------------------
// DeleteStratumSchema
//-----------------------------------------------------------------------------
void hsfcSchema::DeleteStratumSchema(){

	// Destroy all of the Rules
	for (unsigned int i = 0; i < this->StratumSchema.size(); i++) {
		delete this->StratumSchema[i];
	}
	this->StratumSchema.clear();

}

//-----------------------------------------------------------------------------
// FindRelationSchema
//-----------------------------------------------------------------------------
hsfcRelationSchema* hsfcSchema::FindRelationSchema(unsigned int NameID, int Arity){

	// Find the relation schema; 0th schema is for the Lexicon
	for (unsigned int i = 1; i < this->RelationSchema.size(); i++) {
		if (this->RelationSchema[i]->NameID == NameID) return this->RelationSchema[i];
	}

	return AddRelationSchema(NameID, Arity);

}

//--- Overload ----------------------------------------------------------------
hsfcRelationSchema* hsfcSchema::FindRelationSchema(unsigned int NameID){

	// Find the relation schema; 0th schema is for the Lexicon
	for (unsigned int i = 1; i < this->RelationSchema.size(); i++) {
		if (this->RelationSchema[i]->NameID == NameID) return this->RelationSchema[i];
	}

	return NULL;

}

//-----------------------------------------------------------------------------
// AddToDomain
//-----------------------------------------------------------------------------
bool hsfcSchema::AddToDomain(unsigned int RelationIndex, int ArgumentIndex, hsfcTuple& Term, int* NewEntryCount) {

	// Check the values
	if (RelationIndex >= this->RelationSchema.size()) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: bad relation index in hsfcSchema::AddToDomain\n");
		return false;
	}
	if (ArgumentIndex >= this->RelationSchema[RelationIndex]->Arity) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: bad argument index in hsfcSchema::AddToDomain\n");
		return false;
	}
	
	// Perform an integrity test
	if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->Term.size() >= MAX_DOMAIN_ENTRIES) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: too many domain entries in hsfcSchema::AddToDomain\n");
		return false; 
	}

	// Add the term to the relation domain
	// Return of false just means it was already there
	if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->AddTerm(Term)) (*NewEntryCount)++;

	// Make sure that the domains for (init ()) (true ()) (next ()) are all the same
	if (this->Lexicon->PartialMatch(this->RelationSchema[RelationIndex]->NameID, "init:")) {
		// Process the (true ())
		RelationIndex = this->Lexicon->TrueFrom(RelationIndex);
		// Add the term to the alternate relation
		if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->AddTerm(Term)) (*NewEntryCount)++;
		// Process the (next ())
		RelationIndex = this->Lexicon->NextFrom(RelationIndex);
		// Add the term to the alternate relation
		if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->AddTerm(Term)) (*NewEntryCount)++;
		return true;
	}
	if (this->Lexicon->PartialMatch(this->RelationSchema[RelationIndex]->NameID, "true:")) {
		// Process the (true ())
		RelationIndex = this->Lexicon->InitFrom(RelationIndex);
		// Add the term to the alternate relation
		if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->AddTerm(Term)) (*NewEntryCount)++;
		// Process the (next ())
		RelationIndex = this->Lexicon->NextFrom(RelationIndex);
		// Add the term to the alternate relation
		if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->AddTerm(Term)) (*NewEntryCount)++;
		return true;
	}
	if (this->Lexicon->PartialMatch(this->RelationSchema[RelationIndex]->NameID, "next:")) {
		// Process the (true ())
		RelationIndex = this->Lexicon->InitFrom(RelationIndex);
		// Add the term to the alternate relation
		if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->AddTerm(Term)) (*NewEntryCount)++;
		// Process the (next ())
		RelationIndex = this->Lexicon->TrueFrom(RelationIndex);
		// Add the term to the alternate relation
		if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->AddTerm(Term)) (*NewEntryCount)++;
		return true;
	}

	// Make sure that the domains for (legal ()) (does ()) are all the same
	if (this->Lexicon->Match(this->RelationSchema[RelationIndex]->NameID, "legal/2")) {
		// Process the (does ())
		RelationIndex = this->Lexicon->RelationIndex("does/2", false);
		// Add the term to the alternate relation
		if (this->RelationSchema[RelationIndex]->DomainSchema[ArgumentIndex]->AddTerm(Term)) (*NewEntryCount)++;
		return true;
	}

	return true;

}

//-----------------------------------------------------------------------------
// CreateRelationSchema
//-----------------------------------------------------------------------------
bool hsfcSchema::CreateRelationSchema(hsfcSCLAtom* SCLAtom){

	hsfcRelationSchema* RelationSchema;

	// Read the atom and build the relation schema
	RelationSchema = this->FindRelationSchema(SCLAtom->NameID, SCLAtom->Term.size());
	if (RelationSchema == NULL) {
		return false;
	}

	// Process each of the embedded relations
	for (unsigned int i = 0; i < SCLAtom->Term.size(); i++) {
		if (SCLAtom->Term[i]->Term.size() > 0) {
			if (!this->CreateRelationSchema(SCLAtom->Term[i])) {
				return false;
			}
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// CompleteRelationSchema
//-----------------------------------------------------------------------------
bool hsfcSchema::CompleteRelationSchema(){

	char* Name;

	// Check all of the fluents for (init ()) (true ()) (next())
	for (unsigned int i = 1; i < this->RelationSchema.size(); i++) {
		// Is it an (init())
		if (this->Lexicon->PartialMatch(this->RelationSchema[i]->NameID, "init:")) {
			// Does it have a matching (true())
			if (this->Lexicon->TrueFrom(i) == 0) {
				Name = this->Lexicon->Copy(this->RelationSchema[i]->NameID, true);
				Name[0] = 't';Name[1] = 'r';Name[2] = 'u';Name[3] = 'e';
				this->AddRelationSchema(this->Lexicon->Index(Name), this->RelationSchema[i]->Arity);
				delete[] Name;
			}
		}
		// Is it an (true())
		if (this->Lexicon->PartialMatch(this->RelationSchema[i]->NameID, "true:")) {
			// Does it have a matching (next())
			if (this->Lexicon->NextFrom(i) == 0) {
				Name = this->Lexicon->Copy(this->RelationSchema[i]->NameID, true);
				Name[0] = 'n';Name[1] = 'e';Name[2] = 'x';Name[3] = 't';
				this->AddRelationSchema(this->Lexicon->Index(Name), this->RelationSchema[i]->Arity);
				delete[] Name;
			}
		}
		// Is it an (next())
		if (this->Lexicon->PartialMatch(this->RelationSchema[i]->NameID, "next:")) {
			// Does it have a matching (init())
			if (this->Lexicon->InitFrom(i) == 0) {
				Name = this->Lexicon->Copy(this->RelationSchema[i]->NameID, true);
				Name[0] = 'i';Name[1] = 'n';Name[2] = 'i';Name[3] = 't';
				this->AddRelationSchema(this->Lexicon->Index(Name), this->RelationSchema[i]->Arity);
				delete[] Name;
			}
		}
	}

	// Make sure (doies/2 ...) exist
	this->FindRelationSchema(this->Lexicon->Index("does/2"), 2);

	return true;

}

//-----------------------------------------------------------------------------
// CreateStratumSchema
//-----------------------------------------------------------------------------
bool hsfcSchema::CreateStratumSchema(hsfcSCLStratum* SCLStratum){

	hsfcStratumSchema* NewStratumSchema;

	// Create a new schema
	NewStratumSchema = new hsfcStratumSchema(this->Lexicon);
	NewStratumSchema->Initialise(SCLStratum);
		
	// Create schema for each of the rules
	for (unsigned int i = 0; i < SCLStratum->Rule.size(); i++) {
		if (!this->CreateRuleSchema(NewStratumSchema, SCLStratum->Rule[i])) return false;
	}

	// Add the new schema
	this->StratumSchema.push_back(NewStratumSchema);

	return true;

}

//-----------------------------------------------------------------------------
// CreateRuleSchema
//-----------------------------------------------------------------------------
bool hsfcSchema::CreateRuleSchema(hsfcStratumSchema* StratumSchema, hsfcSCLAtom* SCLRule){

	hsfcRuleSchema* NewRuleSchema;

	// Create a new rule schema
	NewRuleSchema = new hsfcRuleSchema(this->Lexicon);
	NewRuleSchema->Initialise();
		
	// Create schema for each of the rule relations
	for (unsigned int i = 0; i < SCLRule->Term.size(); i++) {
		// First create the schema for each of the relations
		if (!this->CreateRelationSchema(SCLRule->Term[i])) return false;
		// Now create the schema for the rule relations
		if (!this->CreateRuleRelationSchema(NewRuleSchema, SCLRule->Term[i])) return false;
	}

	// Set the new rule schema properties for all variables
	if (!this->AddVariableRuleTerms(NewRuleSchema)) return false;
	if (!this->OptimiseRuleSchema(NewRuleSchema)) return false;

	// Does the new rule impact the stratum type
	if (this->Lexicon->PartialMatch(SCLRule->Term[0]->NameID, "terminal")) StratumSchema->Type = hsfcStratumTerminal; 
	if (this->Lexicon->PartialMatch(SCLRule->Term[0]->NameID, "goal")) StratumSchema->Type = hsfcStratumGoal; 
	if (this->Lexicon->PartialMatch(SCLRule->Term[0]->NameID, "legal")) StratumSchema->Type = hsfcStratumLegal; 
	if (this->Lexicon->PartialMatch(SCLRule->Term[0]->NameID, "sees")) StratumSchema->Type = hsfcStratumSees; 
	if (this->Lexicon->PartialMatch(SCLRule->Term[0]->NameID, "next:")) StratumSchema->Type = hsfcStratumNext; 

	// Add the new schema
	StratumSchema->RuleSchema.push_back(NewRuleSchema);

	return true;

}

//-----------------------------------------------------------------------------
// CreateRuleRelationSchema
//-----------------------------------------------------------------------------
bool hsfcSchema::CreateRuleRelationSchema(hsfcRuleSchema* RuleSchema, hsfcSCLAtom* SCLRuleRelation) {

	hsfcRuleRelationSchema* NewRuleRelationSchema;

	// Create a new rule relation schema
	NewRuleRelationSchema = new hsfcRuleRelationSchema(this->Lexicon);

	// Set new reule relation properties
	NewRuleRelationSchema->Initialise(SCLRuleRelation);
	if (!this->CreateRuleTermSchema(NewRuleRelationSchema, SCLRuleRelation, SCLRuleRelation->NameID, UNDEFINED)) return false;

	if (SCLRuleRelation->Not) NewRuleRelationSchema->Function = NewRuleRelationSchema->Function | hsfcFunctionNot; 
	if (SCLRuleRelation->Distinct) NewRuleRelationSchema->Function = NewRuleRelationSchema->Function | hsfcFunctionDistinct; 

	// Add any fixed terms from the rule into the correct relation schema
	//if (!this->AddFixedRuleTerms(RelationSchema, SCLRuleRelation)) return false;

	// Add the new schema
	RuleSchema->RelationSchema.push_back(NewRuleRelationSchema);

	return true;

}

//-----------------------------------------------------------------------------
// CreateRuleTermSchema
//-----------------------------------------------------------------------------
bool hsfcSchema::CreateRuleTermSchema(hsfcRuleRelationSchema* RuleRelationSchema, hsfcSCLAtom* Atom, unsigned int RelationNameID, int ArgumentIndex) {

	hsfcRuleTermSchema NewTermSchema;

	/* Rules
					(cell/2 (at/2	?x		?y)		b)

	SCLAtom			cell/2
		term				at/2					b
			term					?x		?y

	NameID			cell/2	at/2	?x		?y		b
	RelationIndex	cell/2	cell/2	at/2	at/2	cell/2
	EmbeddedIndex	cell/2	at/2	~		~		~
	Arity			2		2		~		~		~
	ArgumentIndex	~		0		0		1		1
	VariableIndex	~		~		0		1		~
	Type			rel		rel		var		var		none
	*/

	// Set the properties of the relation schema from the scl
	NewTermSchema.NameID = Atom->NameID;
	NewTermSchema.RelationIndex = UNDEFINED;
	NewTermSchema.EmbeddedIndex = UNDEFINED;
	NewTermSchema.VariableIndex = UNDEFINED;
	NewTermSchema.Type = hsfcTypeFixed;
	NewTermSchema.ArgumentIndex = ArgumentIndex;
	NewTermSchema.Term.Index = 0;
	NewTermSchema.Term.ID = Atom->NameID;
	NewTermSchema.Arity = 0;

	// Is this the main rule relation
	if (ArgumentIndex == UNDEFINED) {

		NewTermSchema.Type = hsfcTypeRelation;
		NewTermSchema.RelationIndex = this->Lexicon->RelationIndex(Atom->NameID);
		NewTermSchema.EmbeddedIndex = this->Lexicon->RelationIndex(Atom->NameID);
		NewTermSchema.Arity = this->RelationSchema[this->Lexicon->RelationIndex(Atom->NameID)]->Arity;

	// This is a variable, a term or an embedded relation
	} else {

		// Is this a term
		if (Atom->Term.size() == 0) {

			// Is it a fixed term or a variable
			if (this->Lexicon->IsVariable(Atom->NameID)) {
				NewTermSchema.Type = hsfcTypeVariable;
				NewTermSchema.VariableIndex = 0;
				NewTermSchema.RelationIndex = this->Lexicon->RelationIndex(RelationNameID);
			} else {
				NewTermSchema.RelationIndex = this->Lexicon->RelationIndex(RelationNameID);
			}

		// This is an embedded relation
		} else {

			// This is a relation
			NewTermSchema.Type = hsfcTypeRelation;
			NewTermSchema.RelationIndex = this->Lexicon->RelationIndex(RelationNameID);
			NewTermSchema.EmbeddedIndex = this->Lexicon->RelationIndex(Atom->NameID);
			if (NewTermSchema.EmbeddedIndex == UNDEFINED) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: bad relation index hsfcSchema::CreateRuleTermSchema\n");
				return false;
			}
			NewTermSchema.Arity = this->RelationSchema[this->Lexicon->RelationIndex(Atom->NameID)]->Arity;

		}

	}

	// Was there an error in finding the relation index
	if (NewTermSchema.RelationIndex == UNDEFINED) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: bad relation index hsfcSchema::CreateRuleTermSchema\n");
		return false;
	}

	// Add the new term schema
	RuleRelationSchema->TermSchema.push_back(NewTermSchema);

	// Process the children
	for (unsigned int i = 0; i < Atom->Term.size(); i++) {
		if (!this->CreateRuleTermSchema(RuleRelationSchema, Atom->Term[i], Atom->NameID, i)) return false;
	}

	return true;

}

//-----------------------------------------------------------------------------
// AddVariableRuleTerms
//-----------------------------------------------------------------------------
bool hsfcSchema::AddVariableRuleTerms(hsfcRuleSchema* RuleSchema) {

	hsfcDomainSchema* NewDomainSchema;
	vector<int> InputCount;

	// Inspect each rule relation
	for (unsigned int i = 0; i < RuleSchema->RelationSchema.size(); i++) {
		// Inspect each term in the rule relation
		for (unsigned int j = 0; j < RuleSchema->RelationSchema[i]->TermSchema.size(); j++) {
			// Is it a variable term
			if (RuleSchema->RelationSchema[i]->TermSchema[j].Type == hsfcTypeVariable) {

				// Does the variable already exist
				for (unsigned int k = 0; k < RuleSchema->VariableSchema.size(); k++) {
					if (RuleSchema->VariableSchema[k]->NameID == RuleSchema->RelationSchema[i]->TermSchema[j].NameID) {
						RuleSchema->RelationSchema[i]->TermSchema[j].VariableIndex = k;
						// Count the input for this variable
						if ((i > 0) && (RuleSchema->RelationSchema[i]->Function == hsfcFunctionNone)) {
							InputCount[k] += 1;
						}
						goto NextTerm;
					}
				}

				// Create a new variable
				NewDomainSchema = new hsfcDomainSchema(this->Lexicon);
				NewDomainSchema->Initialise(RuleSchema->RelationSchema[i]->TermSchema[j].NameID);
				// Add the new variable to the schema
				RuleSchema->RelationSchema[i]->TermSchema[j].VariableIndex = RuleSchema->VariableSchema.size();
				RuleSchema->VariableSchema.push_back(NewDomainSchema);
				// Count the inputs for this variable
				if ((i > 0) && (RuleSchema->RelationSchema[i]->Function == hsfcFunctionNone)) {
					InputCount.push_back(1);
				} else {
					InputCount.push_back(0);
				}
			}
			// Process the next term
			NextTerm:;
		}
	}

	// Test for a variable with not input
	for (unsigned int i = 0; i < InputCount.size(); i++) {
		if (InputCount[i] < 1) {
			this->Lexicon->IO->FormatToLog(0, false, "Error: variable '%s' with no input in rule for '%s' in hsfcSchema::AddVariableRuleTerms\n", this->Lexicon->Text(RuleSchema->VariableSchema[i]->NameID), this->Lexicon->Text(RuleSchema->RelationSchema[0]->TermSchema[0].NameID));
			return false; 
		}
	}


	return true;

}

//-----------------------------------------------------------------------------
// BuildRelationDomains
//-----------------------------------------------------------------------------
bool hsfcSchema::BuildRelationDomains(hsfcSCLAtom* SCLAtom) {

	unsigned int RelationIndex;
	unsigned int EmbeddedRelationIndex;
	hsfcTuple DomainTerm;
	int EntryCount = 0;

	// Read the atom and build the relation schema
	RelationIndex = this->Lexicon->RelationIndex(SCLAtom->NameID);
	if (RelationIndex == UNDEFINED) {
		return false;
	}

	// Add domain entries for each child term
	for (unsigned int i = 0; i < SCLAtom->Term.size(); i++) {
		// Is it an embedded relation or a term
		if (SCLAtom->Term[i]->Term.size() == 0) {
			DomainTerm.Index = 0;
			DomainTerm.ID = SCLAtom->Term[i]->NameID;
		} else {
			EmbeddedRelationIndex = this->Lexicon->RelationIndex(SCLAtom->Term[i]->NameID);
			if (EmbeddedRelationIndex == UNDEFINED) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: EmbeddedRelationIndex is undefined hsfcSchema::BuildRelationDomains\n");
				return false;
			}
			DomainTerm.Index = EmbeddedRelationIndex;
			DomainTerm.ID = 0;
		}
		if (!this->AddToDomain(RelationIndex, i, DomainTerm, &EntryCount)) {
			return false;
		}
	}
	
	// Process each of the children
	for (unsigned int i = 0; i < SCLAtom->Term.size(); i++) {
		if (SCLAtom->Term[i]->Term.size() > 0) {
			if (!this->BuildRelationDomains(SCLAtom->Term[i])) {
				return false;
			}
		}
	}
	
	return true;

}

//-----------------------------------------------------------------------------
// BuildStratumDomainsFix
//-----------------------------------------------------------------------------
bool hsfcSchema::BuildStratumDomainsFix(hsfcStratumSchema* StratumSchema) {

	// Calculate domain values for every rule
	for (unsigned int i = 0; i < StratumSchema->RuleSchema.size(); i++) {
		// Add in the fixed terms
		if (!this->BuildRuleDomainsFix(StratumSchema->RuleSchema[i])) return false;
	}

	return true;

}

//-----------------------------------------------------------------------------
// BuildStratumDomainsVar
//-----------------------------------------------------------------------------
bool hsfcSchema::BuildStratumDomainsVar(hsfcStratumSchema* StratumSchema, int* NewEntryCount) {

	int PrevNewEntryCount;
	
	// Continue this process until there are no new entries
	PrevNewEntryCount = -1;
	while (PrevNewEntryCount != *NewEntryCount) {
		PrevNewEntryCount = *NewEntryCount;
	
		// Calculate domain values for every rule
		for (unsigned int i = 0; i < StratumSchema->RuleSchema.size(); i++) {
			// Add in the variable terms
			if (!this->BuildRuleDomainsVar(StratumSchema->RuleSchema[i], NewEntryCount)) {
				return false;
			}
		}

	}

	return true;

}

//-----------------------------------------------------------------------------
// BuildRuleDomainsFixed
//-----------------------------------------------------------------------------
bool hsfcSchema::BuildRuleDomainsFix(hsfcRuleSchema* RuleSchema) {

	unsigned int RelationIndex;
	unsigned int DomainIndex;
	hsfcTuple NewTerm;
	int EntryCount = 0;

	// This only considers the output of each rule and (distinct ...)
	// Any unique fixed terms in a rule input or condition will generate a fail
	//		as they will be absent from the domain for that term
	
	// Check every rule
	for (unsigned int j = 0; j < RuleSchema->RelationSchema.size(); j++) {
		// Only the output and any (distinct ...)
		if ((j == 0) || ((RuleSchema->RelationSchema[j]->Function & hsfcFunctionDistinct) == hsfcFunctionDistinct)) {
			// Check each term except for the first one
			for (unsigned int k = 1; k < RuleSchema->RelationSchema[j]->TermSchema.size(); k++) {
				// Is this a fixed term
				if (RuleSchema->RelationSchema[j]->TermSchema[k].Type == hsfcTypeFixed) {
					NewTerm.Index = 0;
					NewTerm.ID = RuleSchema->RelationSchema[j]->TermSchema[k].NameID;
					RelationIndex = RuleSchema->RelationSchema[j]->TermSchema[k].RelationIndex;
					DomainIndex = RuleSchema->RelationSchema[j]->TermSchema[k].ArgumentIndex;
					this->AddToDomain(RelationIndex, DomainIndex, NewTerm, &EntryCount);
				}
				// Is this an embedded relation
				if (RuleSchema->RelationSchema[j]->TermSchema[k].Type == hsfcTypeRelation) {
					NewTerm.Index = RuleSchema->RelationSchema[j]->TermSchema[k].EmbeddedIndex;
					NewTerm.ID = 0;
					RelationIndex = RuleSchema->RelationSchema[j]->TermSchema[k].RelationIndex;
					DomainIndex = RuleSchema->RelationSchema[j]->TermSchema[k].ArgumentIndex;
					this->AddToDomain(RelationIndex, DomainIndex, NewTerm, &EntryCount);
				}
			}
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// BuildRuleDomainsVariable
//-----------------------------------------------------------------------------
bool hsfcSchema::BuildRuleDomainsVar(hsfcRuleSchema* RuleSchema, int* NewEntryCount) {

	vector<hsfcTuple> DomainTerm;
	bool FirstOccurance;
	hsfcRelationSchema* RelationSchema;
	unsigned int DomainIndex;

	// !!! Important vector<hsfcTuple> DomainTerm must be sorted

	// Calculate the domain of a each variable
	for (unsigned int i = 0; i < RuleSchema->VariableSchema.size(); i++) {
		// Clear the domain values
		DomainTerm.clear();
		FirstOccurance = true;
		// Find all positive instance of the variable
		for (unsigned int j = 1; j < RuleSchema->RelationSchema.size(); j++) {
			// Is this an input
			if (RuleSchema->RelationSchema[j]->Function == hsfcFunctionNone) {
				// Inspect each term in the relation
				for (unsigned int k = 1; k < RuleSchema->RelationSchema[j]->TermSchema.size(); k++) {
					// Is this the vaiable in question
					if (RuleSchema->RelationSchema[j]->TermSchema[k].VariableIndex == i) {
						// Find the relation schema for this variable
						RelationSchema = this->RelationSchema[RuleSchema->RelationSchema[j]->TermSchema[k].RelationIndex];
						DomainIndex = RuleSchema->RelationSchema[j]->TermSchema[k].ArgumentIndex;
						if (FirstOccurance) {
							RelationSchema->Copy(DomainTerm, DomainIndex);
							FirstOccurance = false;
						} else {
							RelationSchema->Intersection(DomainTerm, DomainIndex);
						}
					}
				}
			}
		}

		// Add the values to the variable domain
		if (RuleSchema->VariableSchema[i]->AddTerms(DomainTerm)) {
			// Check all of the rule relations
			for (unsigned int j = 0; j < RuleSchema->RelationSchema.size(); j++) {
				// Is this variable in the output of the rule or a (distinct ...)
				if ((j == 0) || ((RuleSchema->RelationSchema[j]->Function & hsfcFunctionDistinct) == hsfcFunctionDistinct)) {
					// Find the correct variable term
					for (unsigned int k = 0; k < RuleSchema->RelationSchema[j]->TermSchema.size(); k++) {
						if (RuleSchema->RelationSchema[j]->TermSchema[k].VariableIndex == i) {
							// Add the values to the relation domain
							for (unsigned int l = 0; l < DomainTerm.size(); l++) {
								this->AddToDomain(RuleSchema->RelationSchema[j]->TermSchema[k].RelationIndex, RuleSchema->RelationSchema[j]->TermSchema[k].ArgumentIndex, DomainTerm[l], NewEntryCount);
							}
						}
					}
				}
			}
		}

	}

	return true;

}

//-----------------------------------------------------------------------------
// OptimiseRuleSchema
//-----------------------------------------------------------------------------
bool hsfcSchema::OptimiseRuleSchema(hsfcRuleSchema* RuleSchema) {

	int Count;
	vector<bool> Found;
	hsfcRuleRelationSchema* RelationSchema;

	// Identify Preconditions
	// Reorder the inputs to improve speed
	// Identify Conditions

	// Is there anything in the rule
	if (RuleSchema->RelationSchema.size() == 0) {
		this->Lexicon->IO->WriteToLog(0, false, "Error: empty rule in hsfcSchema::OptimiseRuleSchema\n");
		return false;
	}

	// Mark the output
	RuleSchema->RelationSchema[0]->Type = hsfcRuleOutput;

	// Set all the inputs
	for (unsigned int i = 1; i < RuleSchema->RelationSchema.size(); i++) {

		// Count the variables
		Count = 0;
		for (unsigned int j = 0; j < RuleSchema->RelationSchema[i]->TermSchema.size(); j++) {
			if (RuleSchema->RelationSchema[i]->TermSchema[j].Type == hsfcTypeVariable) {
				Found.push_back(false);
				Count++;
			}
		}

		// By default its an input
		RuleSchema->RelationSchema[i]->Type = hsfcRuleInput;
		// Is it a precondition
		if (Count == 0) {
			RuleSchema->RelationSchema[i]->Type = hsfcRulePreCondition;
			continue;
		}
		// Is it a function
		if (RuleSchema->RelationSchema[i]->Function != hsfcFunctionNone) {
			RuleSchema->RelationSchema[i]->Type = hsfcRuleCondition;
		}

	}

	// Resort any self referencing inputs to the beginning
	// i starts at 2 as 0 = output, if 1 = selfreference then its already first
	for (unsigned int i = 2; i < RuleSchema->RelationSchema.size(); i++) {
		// Is the input the same as the output
		if (RuleSchema->RelationSchema[i]->TermSchema[0].NameID == RuleSchema->RelationSchema[0]->TermSchema[0].NameID) {
			// Shuffle it to the front
			RelationSchema = RuleSchema->RelationSchema[i];
			RuleSchema->RelationSchema.erase(RuleSchema->RelationSchema.begin() + i);
			RuleSchema->RelationSchema.insert(RuleSchema->RelationSchema.begin() + 1, RelationSchema);
		}
	}	
		
	// Look for redundant inputs
	for (unsigned int i = 1; i < RuleSchema->RelationSchema.size(); i++) {

		// Is it an input
		if (RuleSchema->RelationSchema[i]->Type == hsfcRuleInput) {

			// Count the variables
			Count = 0;
			for (unsigned int j = 0; j < RuleSchema->RelationSchema[i]->TermSchema.size(); j++) {
				if (RuleSchema->RelationSchema[i]->TermSchema[j].Type == hsfcTypeVariable) {
					if (!Found[RuleSchema->RelationSchema[i]->TermSchema[j].VariableIndex]) {
						Found[RuleSchema->RelationSchema[i]->TermSchema[j].VariableIndex] = true;
						Count++;
					}
				}
			}
			// Is it redundant
			if (Count == 0) {
				RuleSchema->RelationSchema[i]->Type = hsfcRuleCondition;
			}
		}

	}

	// Clean up
	Found.clear();

	return true;

}

//-----------------------------------------------------------------------------
// IdentifyRelationTypes
//-----------------------------------------------------------------------------
bool hsfcSchema::IdentifyRelationTypes() {

	hsfcRelationSchema* RelationSchema;

	/* Fact Types
	None - Defualt, and embedded
	Aux - Transient, can include entries in Permanents 
	Rigid - Whole relations that never change
	Init - All initial values of True 
	True - All in the state 
	Next - All next values of True 
	*/

	// Check every relation
	for (unsigned int i = 1; i < this->RelationSchema.size(); i++) {

		// Set the default
		this->RelationSchema[i]->Fact = hsfcFactNone; // Used later to identify the cyclic input to a rule
		this->RelationSchema[i]->IsInState = false;

		// Is it a (init:pred ...)
		if (this->Lexicon->PartialMatch(this->RelationSchema[i]->NameID, "init:")) {
			this->RelationSchema[i]->Fact = hsfcFactInit;
			this->RelationSchema[i]->IsInState = true;
			continue;
		}
		// Is it a (true:pred ...)
		if (this->Lexicon->PartialMatch(this->RelationSchema[i]->NameID, "true:")) {
			this->RelationSchema[i]->Fact = hsfcFactTrue;
			this->RelationSchema[i]->IsInState = true;
			continue;
		}
		// Is it a (next:pred ...)
		if (this->Lexicon->PartialMatch(this->RelationSchema[i]->NameID, "next:")) {
			this->RelationSchema[i]->Fact = hsfcFactNext;
			this->RelationSchema[i]->IsInState = true;
			continue;
		}
		// Is it a (role)
		if (this->Lexicon->Match(this->RelationSchema[i]->NameID, "role/1")) {
			this->RelationSchema[i]->Fact = hsfcFactRigid;
			this->RelationSchema[i]->IsInState = true;
		}
		// Is it a (terminal)
		if (this->Lexicon->Match(this->RelationSchema[i]->NameID, "terminal/0")) {
			this->RelationSchema[i]->Fact = hsfcFactAux;
			this->RelationSchema[i]->IsInState = true;
			continue;
		}
		// Is it a (goal ...)
		if (this->Lexicon->Match(this->RelationSchema[i]->NameID, "goal/2")) {
			this->RelationSchema[i]->Fact = hsfcFactAux;
			this->RelationSchema[i]->IsInState = true;
			continue;
		}
		// Is it a (legal ...)
		if (this->Lexicon->Match(this->RelationSchema[i]->NameID, "legal/2")) {
			this->RelationSchema[i]->Fact = hsfcFactAux;
			this->RelationSchema[i]->IsInState = true;
			continue;
		}
		// Is it a (does ...)
		if (this->Lexicon->Match(this->RelationSchema[i]->NameID, "does/2")) {
			this->RelationSchema[i]->Fact = hsfcFactAux;
			this->RelationSchema[i]->IsInState = true;
			continue;
		}
		// Is it a (sees ...)
		if (this->Lexicon->Match(this->RelationSchema[i]->NameID, "sees/2")) {
			this->RelationSchema[i]->Fact = hsfcFactAux;
			this->RelationSchema[i]->IsInState = true;
			continue;
		}
		//// Is it a (distinct ...)
		//if (this->Lexicon->Match(this->RelationSchema[i]->NameID, "distinct/2")) {
		//	this->RelationSchema[i]->Fact = hsfcFactAux;
		//	this->RelationSchema[i]->IsInState = false;
		//	continue;
		//}

	}

	// Inspect the stratum to find the rigids
	// They are already ordered according to dependency
	// All outputs and inputs are required in the state
	for (unsigned int i = 0; i < this->StratumSchema.size(); i++) {

		// Are all of its inputs rigids
		this->StratumSchema[i]->IsRigid = true;
		for (unsigned int j = 0; j < this->StratumSchema[i]->SCLStratum->Input.size(); j++) {
			// Find the relation schema
			RelationSchema = this->FindRelationSchema(this->StratumSchema[i]->SCLStratum->Input[j]);
			if (RelationSchema == NULL) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: null relationschema in hsfcSchema::IdentifyRelationTypes\n");
				return false;
			}
			// Ignore Distinct
			if (this->Lexicon->PartialMatch(RelationSchema->NameID, "distinct")) continue;
			// Test if it might be rigid; if the input is already classified non rigid, then it fails
			if ((RelationSchema->Fact != hsfcFactNone) && (RelationSchema->Fact != hsfcFactRigid)) {
				this->StratumSchema[i]->IsRigid = false;
				break;
			}
		}

		// Classify the outputs
		for (unsigned int j = 0; j < this->StratumSchema[i]->SCLStratum->Output.size(); j++) {
			// Find the relation schema
			RelationSchema = this->FindRelationSchema(this->StratumSchema[i]->SCLStratum->Output[j]);
			if (RelationSchema == NULL) {
				this->Lexicon->IO->WriteToLog(0, false, "Error: null relationschema in hsfcSchema::IdentifyRelationTypes\n");
				return false;
			}
			// Its an output so its in the state
			RelationSchema->IsInState = true;
			// Is it yet to be classified
			if (RelationSchema->Fact == hsfcFactNone) {
				if (this->StratumSchema[i]->IsRigid) {
					RelationSchema->Fact = hsfcFactRigid;
				} else {
					RelationSchema->Fact = hsfcFactAux;
				}
			}
		}

		// Now classify all the inputs
		// Remember the strata are ordered
		for (unsigned int j = 0; j < this->StratumSchema[i]->SCLStratum->Input.size(); j++) {
			// Find the relation schema
			RelationSchema = this->FindRelationSchema(this->StratumSchema[i]->SCLStratum->Input[j]);
			// Ignore Distinct
			if (this->Lexicon->PartialMatch(RelationSchema->NameID, "distinct")) continue;
			// If the statum is rigid then all the inputs are rigid
			// If the input is not yet classified then its cyclic rigid or declared rigid
			if (RelationSchema->Fact == hsfcFactNone) {
				RelationSchema->Fact = hsfcFactRigid;
			}
			// All rule inputs are in the state
			RelationSchema->IsInState = true;

		}

	}

	// Look for any fact that is declared rigid
	for (unsigned int j = 0; j < this->SCL->Statement.size(); j++) {
		// Find the relation
		RelationSchema = this->FindRelationSchema(this->SCL->Statement[j]->NameID);
		if (RelationSchema == NULL) {
			this->Lexicon->IO->WriteToLog(0, false, "Error: null relationschema in hsfcSchema::IdentifyRelationTypes\n");
			return false;
		}
		// Is it yet to be classified
		if (RelationSchema->Fact == hsfcFactNone) {
			// All declared rigids are in the state
			RelationSchema->Fact = hsfcFactRigid;
			RelationSchema->IsInState = true;
		}
	}

	// Lastly everything else is embedded
	for (unsigned int i = 1; i < this->RelationSchema.size(); i++) {
		if (RelationSchema->Fact == hsfcFactNone) {
			this->RelationSchema[i]->Fact = hsfcFactEmbedded; 
			this->RelationSchema[i]->IsInState = false;
		}
	}

	return true;

}

//-----------------------------------------------------------------------------
// SetNextReferences
//-----------------------------------------------------------------------------
void hsfcSchema::SetNextReferences(){

	int TrueIndex;
	hsfcReference NewReference;

	// Reset the list
	this->Next.clear();

	// Link all Next --> True
	for (unsigned int i = 1; i < this->RelationSchema.size(); i++) {
		// Is this a (next ...)
		if (this->RelationSchema[i]->Fact == hsfcFactNext) {
			// Create the reference
			TrueIndex = this->Lexicon->TrueFrom(this->RelationSchema[i]->Index);
			NewReference.SourceIndex = this->RelationSchema[i]->Index;
			NewReference.DestinationIndex = TrueIndex;
			this->Next.push_back(NewReference);
		}
	}

}



