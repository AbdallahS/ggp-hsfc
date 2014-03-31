//=============================================================================
// Project: High Speed Forward Chaining
// Module: Well Formed Text
// Authors: Michael Schofield UNSW
// 
//=============================================================================
#include "stdafx.h"
#include "kpWFT.h"

using namespace std;

//=============================================================================
// CLASS: kpTextElement
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
kpTextElement::kpTextElement(void){

	// Create the Element
	this->NextElement = NULL;
	this->PrevElement = NULL;
	this->Text = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
kpTextElement::~kpTextElement(void){

	if (Text != NULL) {
		delete(Text);
		Text = NULL;
	}

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void kpTextElement::Initialise(){

	// Remove any links
	this->NextElement = NULL;
	this->PrevElement = NULL;

	this->Text = NULL;
	this->Level = -1;

}

//-----------------------------------------------------------------------------
// AddText
//-----------------------------------------------------------------------------
void kpTextElement::AddText(const char* Script, int Length){

	// Allocate some memory for the text
	if (this->Text != NULL) delete(this->Text);
	this->Text = (char*) malloc(Length + 1);

	// Copy the text
	strncpy(this->Text, Script, Length);
	this->Text[Length] = 0;

}

//-----------------------------------------------------------------------------
// AddMatchText
//-----------------------------------------------------------------------------
bool kpTextElement::Match(char* Value){

	int Length;

	Length = strlen(Value);
	if (Length == 0) return false;

	return (strcmp(this->Text, Value) == 0);

}

//-----------------------------------------------------------------------------
// Parent
//-----------------------------------------------------------------------------
kpTextElement* kpTextElement::Parent(){

	kpTextElement* Element;
	
	// Move backward to the previous level
	Element = this;
	while ((Element->PrevElement != NULL) && (Element->Level >= this->Level)) {
		Element = Element->PrevElement;
	}

	return Element;

}

//=============================================================================
// CLASS: kpTextStructure
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
kpTextStructure::kpTextStructure(void){

	// Create a root Element
	this->RootElement = NULL;
	this->Recycled = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
kpTextStructure::~kpTextStructure(void){

	kpTextElement* Cursor;

	if (this->RootElement == NULL) return;

	// Go to the end of the List
	Cursor = this->RootElement;
	while (Cursor->NextElement != NULL) {
		Cursor = Cursor->NextElement;
	}
	// Clean up in reverse order
	while (Cursor->PrevElement != NULL) {
		Cursor = Cursor->PrevElement;
		delete(Cursor->NextElement);
		Cursor->NextElement = NULL;
	}
	delete(this->RootElement);
	this->RootElement = NULL;

	// Go to the end of the List
	Cursor = this->Recycled;
	while (Cursor->NextElement != NULL) {
		Cursor = Cursor->NextElement;
	}
	// Clean up in reverse order
	while (Cursor->PrevElement != NULL) {
		Cursor = Cursor->PrevElement;
		delete(Cursor->NextElement);
		Cursor->NextElement = NULL;
	}
	delete(this->Recycled);
	this->Recycled = NULL;

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void kpTextStructure::Initialise(char* Name){

	// Create a root Elements
	if (this->RootElement == NULL) this->RootElement = new kpTextElement();
	if (this->Recycled == NULL) this->Recycled = new kpTextElement();

	// Don't throw away the Elements, put them in the recycler
	while (this->RootElement->NextElement != NULL) {
		this->DeleteElement(this->RootElement->NextElement);
		this->RootElement->NextElement = NULL;
	}

	// Set the name of the list
	this->RootElement->Initialise();
	this->RootElement->AddText(Name, strlen(Name));
	this->RootElement->Level = 0;

}

//-----------------------------------------------------------------------------
// AddElement
//-----------------------------------------------------------------------------
kpTextElement* kpTextStructure::AddElement(const char* Script, int Length, int Level) {

	kpTextElement* Target;

	// Add the new Element to the end of the List
	Target = this->RootElement;
	while (Target->NextElement != NULL) {
		Target = Target->NextElement;
	}

	// Place it at the end;
	return this->InsertElement(Target, Script, Length, Level);

}

//-----------------------------------------------------------------------------
// InsertElement
//-----------------------------------------------------------------------------
kpTextElement* kpTextStructure::InsertElement(kpTextElement* Target, const char* Script, int Length, int Level) {


	// Create a new Element 
	kpTextElement* NewElement = this->NewElement();
	NewElement->AddText(Script, Length);
	NewElement->Level = Level;

	// Add the new Element to the list just after target
	NewElement->NextElement = Target->NextElement;
	NewElement->PrevElement = Target;
	if (Target->NextElement != NULL) 
		Target->NextElement->PrevElement = NewElement;
	Target->NextElement = NewElement;

	// Return Element for processing
	return NewElement;

}

//-----------------------------------------------------------------------------
// DeleteElement
//-----------------------------------------------------------------------------
kpTextElement* kpTextStructure::DeleteElement(kpTextElement* Target) {

	kpTextElement* Element = NULL;

	// Remove the Element from the list
	Target->PrevElement->NextElement = Target->NextElement;
	if (Target->NextElement != NULL) 
		Target->NextElement->PrevElement = Target->PrevElement;

	// Get the return Element, the Element just before the deleted Element
	Element = Target->PrevElement;

	// Add the Element to the front of the recycled list
	Target->NextElement = this->Recycled->NextElement;
	Target->PrevElement = this->Recycled;
	if (this->Recycled->NextElement != NULL) 
		this->Recycled->NextElement->PrevElement = Target;
	this->Recycled->NextElement = Target;

	return Element;

}

//-----------------------------------------------------------------------------
// NewElement
//-----------------------------------------------------------------------------
kpTextElement* kpTextStructure::NewElement() {

	kpTextElement* Element = NULL;

	// Is there anything in the recycled list
	if (this->Recycled->NextElement == NULL) {

		// Create a new Element
		Element = new kpTextElement();

	} else {

		// Take the first Element in the list
		Element = this->Recycled->NextElement;

		// Remove the Element from the list
		Element->PrevElement->NextElement = Element->NextElement;
		if (Element->NextElement != NULL) 
			Element->NextElement->PrevElement = Element->PrevElement;

	}

	// Initialise the element
	Element->Initialise();

	return Element;

}

//-----------------------------------------------------------------------------
// Item
//-----------------------------------------------------------------------------
kpTextElement* kpTextStructure::Item(int Index) {

	kpTextElement* Target;
	int ItemIndex;

	// Is the list empty
	Target = this->RootElement;
	if (Target->NextElement == NULL) return Target;
	Target = Target->NextElement;
	ItemIndex = 0;

	// Look for the Element according to the index
	while ((Target->NextElement != NULL) && ItemIndex < Index) {
		Target = Target->NextElement;
		ItemIndex++;
	}

	// Just in case we go past the end
	if (Target == NULL) {
		Target = this->RootElement;
	}

	return Target;

}

//-----------------------------------------------------------------------------
// Count
//-----------------------------------------------------------------------------
int kpTextStructure::Count() {

	int Result = 0;
	kpTextElement* Target;

	// Look for the Element according to the label
	Target = this->RootElement;
	while (Target->NextElement != NULL) {
		Target = Target->NextElement;
		Result++;
	}

	return Result;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void kpTextStructure::Print() {

	kpTextElement* Element;

	// Display the List name
	Element = this->RootElement;
	printf("%s\n", "--------------------------------");
	printf("%s\n", Element->Text);
	printf("%s\n", "--------------------------------");

	// Print all the elements
	Element = Element->NextElement;
	while (Element != NULL) {
		for (int i = 0; i < Element->Level; i++) {
			printf("%s", "  "); 
		}
		if (strchr(Element->Text, ' ') != NULL) {
			printf("\"%s\"\n", Element->Text);
		} else {
			printf("%s\n", Element->Text);
		}
		Element = Element->NextElement;
	}

}

//=============================================================================
// CLASS: kpWFT
//=============================================================================

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
kpWFT::kpWFT(void){

	// Create the Element
	this->Structure = NULL;

}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
kpWFT::~kpWFT(void){

	// Destroy any children
	if (this->Structure != NULL) {
		delete(this->Structure);
		this->Structure = NULL;
	}

}

//-----------------------------------------------------------------------------
// Initialise
//-----------------------------------------------------------------------------
void kpWFT::Initialise(char* Name){

	// Create the Structure
	if (this->Structure == NULL) this->Structure = new kpTextStructure();

	// Remove any links
	this->Structure->Initialise(Name);

}

//-----------------------------------------------------------------------------
// Load
//-----------------------------------------------------------------------------
void kpWFT::Load(const char* Script){

	int Index;
	const char* Start;
	int Length;
	int Level;

	// Rules for parsing
	// {/ # ; : '} . . . . . {\r \n} = comment
	// "literal"
	// {space spacespace} delimiter
	// ( indent
	// ) outdent

	// Process the script
	Start = Script;
	Level = 0;
	while (Start[0] != 0) {

		// Ignore leading white space or control characters, but not end of text marker
		while (((Start[0] <= ' ') || (Start[0] > '~')) && (Start[0] != 0)) {
			Start++;
		}

		// Have we reached the end of the text
		if (Start[0] == 0) {
			break;
		}

		// Parse the script 
		Index = 0;
		Length = 0;
		// Does the first character signal a comment
		if (strchr("/#;:'", Start[0]) != NULL) {
			// Its a comment so look for an end of line marker
			while ((Start[Index] != 10) && (Start[Index] != 13) && (Start[Index] != 0)) {
				Index++;
			}
			Length = Index;
		} else {
			// Its something meaningful, is it an atom or a bracket
			if ((Start[Index] == '(') || (Start[Index] == ')')) {
				Length = 1;
			} else {
				// Its an atom, so find the end
				// Is it a literal
				if (Start[0] == '"') {
					Start++;
					while ((Start[Index] != '"') && (Start[Index] != 0)) {
						Index++;
					}
					Length = Index;
				} else {
					while ((Start[Index] != '(') && (Start[Index] != ')') && (Start[Index] > ' ') && (Start[Index] != 0)) {
						Index++;
					}
					Length = Index;
				}
			}
		}

		// Was it a closing bracket
		if (Start[0] == ')') {
			Level--;
			if (Level < 0) Level = 0;
		}

		// Add the text element, even if its zero length
		this->Structure->AddElement(Start, Length, Level);

		// Was it an openning bracket
		if (Start[0] == '(') {
			Level++;
		}
		// Was it a closing "
		if (Start[Length] == '"') {
			Length++;
		}

		// Reposition the start
		Start = Start + Length;

	}

}

//-----------------------------------------------------------------------------
// LoadFlat
//-----------------------------------------------------------------------------
void kpWFT::LoadFlat(const char* Script){

	int Index;
	const char* Start;
	int Length;
	int Level;
	vector<int> Terms;
	int Arity;

	// Rules for parsing
	// {/ # ; : '} . . . . . {\r \n} = comment
	// "literal"
	// {space spacespace} delimiter
	// predicate|arity
	// ( ) are just characters

	// Process the script
	Start = Script;
	Level = 0;
	while (Start[0] != 0) {

		// Ignore leading white space or control characters, but not end of text marker
		while (((Start[0] <= ' ') || (Start[0] > '~')) && (Start[0] != 0)) {
			Start++;
		}

		// Have we reached the end of the text
		if (Start[0] == 0) {
			break;
		}

		// Parse the script 
		Index = 0;
		Length = 0;
		// Does the first character signal a comment
		if (strchr("/#;:'", Start[0]) != NULL) {
			// Its a comment so look for an end of line marker
			while ((Start[Index] != 10) && (Start[Index] != 13) && (Start[Index] != 0)) {
				Index++;
			}
			Length = Index;
		} else {
			// Its an atom, so find the end
			// Is it a literal
			if (Start[0] == '"') {
				Start++;
				while ((Start[Index] != '"') && (Start[Index] != 0)) {
					Index++;
				}
				Length = Index;
			} else {
				while ((Start[Index] != '|') && (Start[Index] != ' ') && (Start[Index] != 0)) {
					Index++;
				}
				Length = Index;
			}
		}

		// Is this an arity marker
		if (Start[Length] == '|') {
			this->Structure->AddElement("(", 1, Level);
			Level++;
			sscanf(&Start[Length+1], "%d ", &Arity);
			Terms.push_back(Arity + 1);
		}

		// Add the text element, even if its zero length
		this->Structure->AddElement(Start, Length, Level);

		// Do we go back a level
		while (Terms.size() > 0) {
			if (Terms[Terms.size() - 1] == 1) {
				Level--;
				this->Structure->AddElement(")", 1, Level);
				Terms.pop_back();
			} else {
				Terms[Terms.size() - 1]--;
				break;
			}
		}

		// Was it a closing "
		if (Start[Length] == '"') {
			Length++;
		}

		// Was it an arity marker
		if (Start[Length] == '|') {
			while ((Start[Length] != ' ') && (Start[Length] != 0)) {
				Length++;
			}
		}

		// Reposition the start
		Start = Start + Length;

	}

}

//-----------------------------------------------------------------------------
// ReadFile
//-----------------------------------------------------------------------------
void kpWFT::ReadFile(char* FileName) {

	int Length;
	char Letter;
	FILE* InputFile;
	char Script[32768];

	this->Initialise(FileName);

	// Open the input file
	InputFile = fopen(FileName, "r");
	if (InputFile == NULL) return;

	// Get the description from the file
	Length = 0;
	while ((!feof(InputFile)) && (Length < 32767)) {

		// Read a letter from the file
		fscanf(InputFile, "%c", &Letter);
		Script[Length] = Letter;
		Length++;

	}

	// Is it too long
	if (Length >= 32767) {
		printf("Error reading script in file\n%s\n", FileName);
		return;
	}

	Script[Length] = 0;
	fclose(InputFile);

	// Load the script
	this->Load(Script);

}

//-----------------------------------------------------------------------------
// Exists
//-----------------------------------------------------------------------------
bool kpWFT::Exists(char* Key, int ArgumentNo) {

	int Length;
	char* Result;
	int Count;
	int Level;
	kpTextElement* Cursor;

	// Returns an empty string if not found

	// Is it a valid key
	Length = strlen(Key);
	if (Length == 0) {
		return false;
	}

	// Locate the key as the 0th argument of an element
	Result = NULL;
	Cursor = this->Structure->RootElement;
	while (Cursor->NextElement != NULL) {
		Cursor = Cursor->NextElement;
		// Do we have a match for the key
		if ((Cursor->Text[0] != 0) && (strcmp(Cursor->Text, Key) == 0)) {

			// Note the level
			Level = Cursor->Level;
			// Find the the nth argument, the 0th element = key
			Count = 0;
			while (Cursor != NULL) {
				// Have we passed the last element
				if (Cursor->Level < Level) {
					return false;
				}
				// Was this a same level
				if ((Cursor->Level == Level) && (Cursor->Text[0] != ')')) {
					// Was this the correct element
					if (Count == ArgumentNo) {
						return true;
					}
					Count++;
				}

				Cursor = Cursor->NextElement;

			}
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// Match
//-----------------------------------------------------------------------------
bool kpWFT::Match(char* Key, int ArgumentNo, char* Value) {

	int Length;
	char* Result;
	int Count;
	int Level;
	kpTextElement* Cursor;

	// Returns an empty string if not found

	// Is it a valid key
	Length = strlen(Key);
	if (Length == 0) {
		return false;
	}

	// Locate the key as the 0th argument of an element
	Result = NULL;
	Cursor = this->Structure->RootElement;
	while (Cursor->NextElement != NULL) {
		Cursor = Cursor->NextElement;
		// Do we have a match for the key
		if ((Cursor->Text[0] != 0) && (strcmp(Cursor->Text, Key) == 0)) {

			// Note the level
			Level = Cursor->Level;
			// Find the the nth argument, the 0th element = key
			Count = 0;
			while (Cursor != NULL) {
				// Have we passed the last element
				if (Cursor->Level < Level) {
					return false;
				}
				// Was this a same level
				if ((Cursor->Level == Level) && (Cursor->Text[0] != ')')) {
					// Was this the correct element
					if (Count == ArgumentNo) {
						// Does the argument match the value provided
						return (strcmp(Cursor->Text, Value) == 0);
					}
					Count++;
				}

				Cursor = Cursor->NextElement;

			}
		}
	}

	return false;

}

//-----------------------------------------------------------------------------
// Find
//-----------------------------------------------------------------------------
void kpWFT::Find(char* Key, int ArgumentNo, bool Bracketed, char* Result) {

	int Length;
	int Count;
	int Level;
	kpTextElement* Cursor;

	// Returns an empty string if not found

	// Is it a valid key
	Length = strlen(Key);
	if (Length == 0) {
		Result[0] = 0;
	}

	// Locate the key as the 0th argument of an element
	Cursor = this->Structure->RootElement;
	while (Cursor->NextElement != NULL) {
		Cursor = Cursor->NextElement;
		// Do we have a match for the key
		if ((Cursor->Text[0] != 0) && (strcmp(Cursor->Text, Key) == 0)) {

			// Note the level
			Level = Cursor->Level;
			// Find the the nth argument, the 0th element = key
			Count = 0;
			while (Cursor != NULL) {
				// Have we passed the last element
				if (Cursor->Level == 0) {
					break;
				}
				// Was this a same level element
				if ((Cursor->Level == Level) && (Cursor->Text[0] != ')')) {
					// Was this the correct element
					if (Count == ArgumentNo) {
						// Is the argument a composite value
						if (Cursor->Text[0] == '(') {
							// Calculate the length of the text
							kpTextElement* Element = Cursor->NextElement;
							Length = 1;
							while ((Element != NULL) && (Element->Level > Level)) {
								Length = Length + strlen(Element->Text) + 1;
								Element = Element->NextElement; // Last element
							}

							// Copy the text of the composite argument
							if (Bracketed) Length = Length + 2;
							Result = (char*) malloc(Length);
							Result[0] = 0;
							if (Bracketed) strcat(Result, "(");

							// Copy in the elements
							Cursor = Cursor->NextElement;
							Level = 0;
							while (Cursor != Element) {
								if (Cursor->Level == Level)	strcat(Result, " "); // Place spaces between element of the same level
								if (strchr(Cursor->Text, ' ') != NULL) strcat(Result, "\""); // Open Quote
								strcat(Result, Cursor->Text);
								if (strchr(Cursor->Text, ' ') != NULL) strcat(Result, "\""); // Close Quote
								Level = Cursor->Level;
								Cursor = Cursor->NextElement;
							}
						} else {
							strcpy(Result, Cursor->Text); 
						}
						if (Bracketed) strcat(Result, ")");
						return;
					}
					Count++;
				}
				Cursor = Cursor->NextElement;
			}

			// The was no argument found
			Result[0] = 0;
		}
	}
			
	// The key was not found
	Result[0] = 0;

}

char* kpWFT::Find(char* Key, int ArgumentNo, bool Bracketed) {

	int Length;
	char* Result;
	int Count;
	int Level;
	kpTextElement* Cursor;

	// Returns an empty string if not found

	// Is it a valid key
	Length = strlen(Key);
	if (Length == 0) {
		Result = (char*) malloc(1);
		Result[0] = 0;
		return Result;
	}

	// Locate the key as the 0th argument of an element
	Result = NULL;
	Cursor = this->Structure->RootElement;
	while (Cursor->NextElement != NULL) {
		Cursor = Cursor->NextElement;
		// Do we have a match for the key
		if ((Cursor->Text[0] != 0) && (strcmp(Cursor->Text, Key) == 0)) {

			// Note the level
			Level = Cursor->Level;
			// Find the the nth argument, the 0th element = key
			Count = 0;
			while (Cursor != NULL) {
				// Have we passed the last element
				if (Cursor->Level == 0) {
					break;
				}
				// Was this a same level element
				if ((Cursor->Level == Level) && (Cursor->Text[0] != ')')) {
					// Was this the correct element
					if (Count == ArgumentNo) {
						// Is the argument a composite value
						if (Cursor->Text[0] == '(') {
							// Calculate the length of the text
							kpTextElement* Element = Cursor->NextElement;
							Length = 1;
							while ((Element != NULL) && (Element->Level > Level)) {
								Length = Length + strlen(Element->Text) + 1;
								Element = Element->NextElement; // Last element
							}

							// Copy the text of the composite argument
							if (Bracketed) Length = Length + 2;
							Result = (char*) malloc(Length);
							Result[0] = 0;
							if (Bracketed) strcat(Result, "(");

							// Copy in the elements
							Cursor = Cursor->NextElement;
							Level = 0;
							while (Cursor != Element) {
								if (Cursor->Level == Level)	strcat(Result, " "); // Place spaces between element of the same level
								if (strchr(Cursor->Text, ' ') != NULL) strcat(Result, "\""); // Open Quote
								strcat(Result, Cursor->Text);
								if (strchr(Cursor->Text, ' ') != NULL) strcat(Result, "\""); // Close Quote
								Level = Cursor->Level;
								Cursor = Cursor->NextElement;
							}
						} else {
							Result = (char*) malloc(strlen(Cursor->Text) + 1);
							strcpy(Result, Cursor->Text); 
						}
						if (Bracketed) strcat(Result, ")");
						return Result;
					}
					Count++;
				}
				Cursor = Cursor->NextElement;
			}

			// The was no argument found
			Result = (char*) malloc(1);
			Result[0] = 0;
			return Result;
		}
	}
			
	// The key was not found
	Result = (char*) malloc(1);
	Result[0] = 0;
	return Result;

}

//-----------------------------------------------------------------------------
// FindWhole
//-----------------------------------------------------------------------------
char* kpWFT::FindWhole(char* Key, int ArgumentNo, bool Bracketed, bool IncludeKey) {

	int Length;
	char* Result;
	int Level;
	kpTextElement* Cursor;

	// Returns an empty string if not found

	// Is it a valid key
	Length = strlen(Key);
	if (Length == 0) {
		Result = (char*) malloc(1);
		Result[0] = 0;
		return Result;
	}

	// Locate the key as the 0th argument of an element
	Result = NULL;
	Cursor = this->Structure->RootElement;
	while (Cursor->NextElement != NULL) {
		Cursor = Cursor->NextElement;
		// Do we have a match for the key
		if ((Cursor->Text[0] != 0) && (strcmp(Cursor->Text, Key) == 0)) {

			// Calculate the length of the text
			kpTextElement* Element = Cursor->NextElement;
			Level = Cursor->Level;
			Length = 1;
			while ((Element != NULL) && (Element->Level >= Level)) {
				Length = Length + strlen(Element->Text) + 1;
				Element = Element->NextElement; // Last element
			}

			// Copy the text of the composite argument
			if (Bracketed) Length = Length + 2;
			Result = (char*) malloc(Length);
			Result[0] = 0;
			if (Bracketed) strcat(Result, "(");

			// Copy in the elements
			Level = -1;
			if (!IncludeKey) Cursor = Cursor->NextElement;
			while (Cursor != Element) {
				if (Cursor->Level == Level)	strcat(Result, " "); // Place spaces between element of the same level
				if (strchr(Cursor->Text, ' ') != NULL) strcat(Result, "\""); // Open Quote
				strcat(Result, Cursor->Text);
				if (strchr(Cursor->Text, ' ') != NULL) strcat(Result, "\""); // Close Quote
				Level = Cursor->Level;
				Cursor = Cursor->NextElement;
			}

			return Result;
		}
	}
			
	// The key was not found
	Result = (char*) malloc(1);
	Result[0] = 0;
	return Result;

}

//-----------------------------------------------------------------------------
// AsText
//-----------------------------------------------------------------------------
char* kpWFT::AsText() {

	int Length;
	char* Result;
	int Level;
	kpTextElement* Cursor;

	Result = NULL;

	// Calculate the length of the text
	Cursor = this->Structure->RootElement->NextElement;
	Level = 0;
	Length = 1;
	while (Cursor != NULL) {
		Length = Length + strlen(Cursor->Text) + 1;
		if (Cursor->Level != Level) {
			Length += abs(Cursor->Level - Level);
			Level = Cursor->Level;
		}
		if (strchr(Cursor->Text, ' ') != NULL) {
			Length += 2;
		}
		Cursor = Cursor->NextElement; 
	}

	// Copy the text of the composite argument
	Result = (char*) malloc(Length);
	Result[0] = 0;

	// Copy in the elements
	Level = -1;
	Cursor = this->Structure->RootElement->NextElement;
	while (Cursor != NULL) {
		if (Cursor->Level == Level) {
			if (Level == 0)	{
				strcat(Result, "\n"); // New Line
			} else {
				strcat(Result, " "); // Place spaces between element of the same level
			}
		}
		if (strchr(Cursor->Text, ' ') != NULL) strcat(Result, "\""); // Open Quote
		strcat(Result, Cursor->Text);
		if (strchr(Cursor->Text, ' ') != NULL) strcat(Result, "\""); // Close Quote
		Level = Cursor->Level;
		Cursor = Cursor->NextElement;
	}

	return Result;

}

//-----------------------------------------------------------------------------
// Print
//-----------------------------------------------------------------------------
void kpWFT::Print(){

	// Just print it
	this->Structure->Print();

}

