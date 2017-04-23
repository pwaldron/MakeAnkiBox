// This program tests the validity of a Blitzkrieg DAWG file, and demonstrates the new Dawg-Node configuration.
// Updated on Monday, December 29, 2011.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

#define MAX 15
#define MAX_INPUT 120
#define BIG_IT_UP -32
#define LOWER_IT 32
#define SIZE_OF_CHARACTER_SET 26
#define WILD_CARD '?'
#define DAWG_DATA "OWL2016-DAWG.dat"
#define WORD_LIST_OUTPUT "New_Dawg_Word_List.txt"

// These values define the format of the "Dawg" node encoding.
#define CHILD_BIT_SHIFT 10
#define END_OF_WORD_BIT_MASK 0X00000200
#define END_OF_LIST_BIT_MASK 0X00000100
#define LETTER_BIT_MASK 0X000000FF

// Define the boolean type as an enumeration.
typedef enum {FALSE = 0, TRUE = 1} Bool;
typedef Bool* BoolPtr;

// When reading strings from a file, the new-line character is appended, and this macro will remove it before processing.
#define CUT_OFF_NEW_LINE(string) (string[strlen(string) - 1] = '\0')

// For speed, define these two simple functions as macros.  They modify the "LettersToWorkWith" string in the recursive anagrammer.
#define REMOVE_CHAR_FROM_STRING(thestring, theposition, shiftsize) ( memmove(thestring + theposition, thestring + theposition + 1, shiftsize) )
#define INSERT_CHAR_IN_STRING(ts, tp, thechar, shiftsize) ( (memmove(ts + tp + 1, ts + tp, shiftsize)), (ts[tp] = thechar) )

// These are the predefined characters that exist in the DAWG data file.
const unsigned char CharacterSet[SIZE_OF_CHARACTER_SET] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

// Enter the graph at the correct position without scrolling through a predefined list of level "0" nodes.
const unsigned char EntryNodeIndex[SIZE_OF_CHARACTER_SET] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
 20, 21, 22, 23, 24, 25, 26 };

// This function converts any lower case letters in the string "RawWord", into capitals, so that the whole string is made of capital letters.
void MakeMeAllCapital(char *RawWord){
	int X;
	for ( X = 0; X < strlen(RawWord); X++ ){
		if ( RawWord[X] >= 'a' && RawWord[X] <= 'z' ) RawWord[X] = RawWord[X] + BIG_IT_UP;
	}
}

#define DAWG_LETTER(thearray, theindex) (thearray[theindex]&LETTER_BIT_MASK)
#define DAWG_END_OF_WORD(thearray, theindex) (thearray[theindex]&END_OF_WORD_BIT_MASK)
#define DAWG_NEXT(thearray, theindex) ((thearray[theindex]&END_OF_LIST_BIT_MASK)? 0: theindex + 1)
#define DAWG_CHILD(thearray, theindex) (thearray[theindex]>>CHILD_BIT_SHIFT)

bool IsValidWord(int *masterDawg, int dawgPosition, string charBank, int position=0) 
{
	/*
	cout << "Started IsValidWord" << endl;
	cout << "\tcharBank = " << charBank << endl;
	cout << "\tposition = " << position << endl;
	cout << endl;	
	*/
	
	do {
		if (DAWG_LETTER(masterDawg, dawgPosition) == charBank[position]) {
			if (position == charBank.size()-1) {
				return DAWG_END_OF_WORD(masterDawg, dawgPosition);
			}
			if (DAWG_CHILD(masterDawg, dawgPosition)) {
				return IsValidWord(masterDawg, DAWG_CHILD(masterDawg, dawgPosition), charBank, position+1);
			}
		}
		
		dawgPosition = DAWG_NEXT(masterDawg, dawgPosition);
	}
	while (dawgPosition);

	return false;
}

// Find a pattern based on CharBank starting with the character at position
void Pattern(int *masterDawg, int dawgPosition, string charBank, vector<string> &words, int position=0, string currentWord=string("")) 
{
/*	
	cout << "Started Pattern" << endl;
	cout << "\tcharBank = " << charBank << endl;
	cout << "\tposition = " << position << endl;
	cout << "\tcurrentWord = " << currentWord << endl;
	cout << "\tcharBank.size() = " << charBank.size() << endl;
	cout << "\tdawgPosition = " << dawgPosition << endl;
	cout << endl;	
*/	
	
	do {
		if ( (DAWG_LETTER(masterDawg, dawgPosition) == charBank[position]) || (charBank[position] == '?')) {
			if (DAWG_END_OF_WORD(masterDawg, dawgPosition) && (charBank.size() == position+1)) {
				currentWord += string(1, char(DAWG_LETTER(masterDawg, dawgPosition)));
				words.push_back(currentWord);
//				cout << "Found pattern match: " << currentWord << endl;
			}
			if (DAWG_CHILD(masterDawg, dawgPosition)) {
				Pattern(masterDawg, DAWG_CHILD(masterDawg, dawgPosition), charBank, words, position+1, currentWord + string(1, char(DAWG_LETTER(masterDawg, dawgPosition))));
			}
		}

		
		dawgPosition = DAWG_NEXT(masterDawg, dawgPosition);
	}
	while (dawgPosition);
}

bool TakesInternalBackHook(int *masterDawg, int dawgPosition, string charBank, int position=0)
{
	return (IsValidWord(masterDawg, dawgPosition, charBank.substr(0, charBank.size()-1)));
}

bool TakesInternalFrontHook(int *masterDawg, int dawgPosition, string charBank, int position=0)
{
	return (IsValidWord(masterDawg, dawgPosition, charBank.substr(1, charBank.size()-1)));
}

bool BackHooks(int *masterDawg, int dawgPosition, string charBank, string &hooks, int position=0) {
	/*
	cout << "BackHooks" << endl;
	cout << "\tcharBank = " << charBank << endl;
	cout << "\tposition = " << position << endl;
	cout << endl;
	*/
	
	if (position == charBank.size()) {
		do {
			if (DAWG_END_OF_WORD(masterDawg, dawgPosition)) {
				hooks += char(DAWG_LETTER(masterDawg, dawgPosition));	
//				cout << char(DAWG_LETTER(masterDawg, dawgPosition));			
			}
			
			dawgPosition = DAWG_NEXT(masterDawg, dawgPosition);
		} while (dawgPosition);	
	}
	else {
		do {
			if (DAWG_LETTER(masterDawg, dawgPosition) == charBank[position]) {
				BackHooks(masterDawg, DAWG_CHILD(masterDawg, dawgPosition), charBank, hooks, position+1);
			}
		
			dawgPosition = DAWG_NEXT(masterDawg, dawgPosition);
		} while (dawgPosition);	
	
	}
}

void FrontHooks(int *masterDawg, int dawgPosition, string charBank, string &hooks, int position=0) {
	do {
		if (IsValidWord(masterDawg, DAWG_CHILD(masterDawg, dawgPosition), charBank, 0)) {
			hooks += char(DAWG_LETTER(masterDawg, dawgPosition));
//			cout << char(DAWG_LETTER(masterDawg, dawgPosition));		
		}	
	
		dawgPosition = DAWG_NEXT(masterDawg, dawgPosition);
	} while (dawgPosition);
}


void Anagram(int *masterDawg, int dawgPosition, string charBank, vector<string> &stringList, string currentWord=string(""))
{
	/*
	cout << "Entered Anagram()" << endl;
	cout << "\tcharBank = " << charBank << endl;
	cout << "\tcurrentWord = " << currentWord << endl;
	cout << endl;	
	*/
		
	do {
		// Does the letter of interest exist in the string?
		int pos = charBank.find_first_of (char(DAWG_LETTER(masterDawg, dawgPosition)));
		
		


		// If so, use it and continue anagramming		
		if (pos  <= charBank.size()) {
			currentWord += charBank[pos];
			charBank.erase(pos, 1);
			
			if (charBank.empty()) {
				if (DAWG_END_OF_WORD(masterDawg, dawgPosition)) {
					stringList.push_back(currentWord);
					//cout << currentWord << endl;
				}
			}
			if (DAWG_CHILD(masterDawg, dawgPosition)) {
				Anagram(masterDawg, DAWG_CHILD(masterDawg, dawgPosition), charBank, stringList, currentWord);
			}
			
			charBank += currentWord.back();
			currentWord.pop_back();
			
			sort(charBank.begin(), charBank.end());		
		}
		
		// Is there a wildcard?		
		else {
			int pos = charBank.find_first_of ('?');
			
			if (pos <= charBank.size()) {			
				currentWord += char(DAWG_LETTER(masterDawg, dawgPosition));
				charBank.erase(pos, 1);
			
				if (charBank.empty()) {
					if (DAWG_END_OF_WORD(masterDawg, dawgPosition)) {
						stringList.push_back(currentWord);
					}
				}
				if (DAWG_CHILD(masterDawg, dawgPosition)) {
					Anagram(masterDawg, DAWG_CHILD(masterDawg, dawgPosition), charBank, stringList, currentWord);
				}
			
				charBank += '?';
				currentWord.pop_back();
				
				sort(charBank.begin(), charBank.end());		
			}	
		}
		
		dawgPosition = DAWG_NEXT(masterDawg, dawgPosition);
					
	} while(dawgPosition);
}

int main() {
	int NumberOfNodes;
	int *TheDawgArray;
	FILE *Lexicon;
	FILE *WordList;
	unsigned char *DecisionInput;
	Bool FetchData = TRUE;
	unsigned char FirstChar;
	int InputSize;
	
	DecisionInput = (unsigned char *)malloc(MAX_INPUT*sizeof(char));
	Lexicon = fopen(DAWG_DATA, "rb");
	fread(&NumberOfNodes, sizeof(int), 1, Lexicon);
	TheDawgArray = (int *)malloc(NumberOfNodes*sizeof(int));
	fread(TheDawgArray, sizeof(int), NumberOfNodes, Lexicon);
	fclose(Lexicon);

	
	vector<string> stringList;
	string input, str;
	
	while (getline(cin, input)) {
		transform(input.begin(), input.end(), input.begin(), ::toupper);	
		
		stringList.clear();
		Anagram(TheDawgArray, 1, input, stringList);
	
		for (vector<string>::iterator it=stringList.begin(); it!=stringList.end(); ++it) {
			if (distance(stringList.begin(), it)==0) {
				cout  << input << " <table>\t" ;			
			}
	
			str.clear();
			FrontHooks(TheDawgArray, 1, *it, str);
			
			transform(str.begin(), str.end(), str.begin(), ::tolower);
			
			cout << "<tr> <td class=\"fronthook\">" << str << "</td> ";
			cout << "<td class=\"mainword\">" ;
			if (TakesInternalFrontHook(TheDawgArray, 1, *it))
				cout << "&centerdot;";
			cout << (*it) ;			
			if (TakesInternalBackHook(TheDawgArray, 1, *it))
				cout << "&centerdot;";
				
			cout << "</td> ";
			cout << "";
			
			str.clear();
			BackHooks(TheDawgArray, 1, *it, str);
			transform(str.begin(), str.end(), str.begin(), ::tolower);
			cout << "<td class=\"backhook\">" << str << "</td> ";
			cout << "</tr> ";
			
		}
		cout << "</table>" << endl;
	}
	
	
/*
	stringList.clear();
	Pattern(TheDawgArray, 1, string("Z?P"), stringList);
	copy(stringList.begin(), stringList.end(), ostream_iterator<string>(cout, "\n"));
	cout << endl;
	
	cout << "CAP: is " << (IsValidWord(TheDawgArray, 1, string("CAP")) ? "" : "NOT") << " a valid word" << endl;
	cout << "RETINAS: is " << (IsValidWord(TheDawgArray, 1, string("RETINAS")) ? "" : "NOT") << " a valid word" << endl;
	cout << "ZZZ: is " << (IsValidWord(TheDawgArray, 1, string("ZZZ")) ? "" : "NOT") << " a valid word" << endl;
	cout << endl;

	cout << "ABC: is " << (IsValidWord(TheDawgArray, 1, string("ABC")) ? "" : "NOT") << " a valid word" << endl;
	cout << endl;

	cout << "InternalBackHook(RETINAS): " << TakesInternalBackHook(TheDawgArray, 1, string("RETINAS")) << endl;
	cout << "InternalBackHook(ANESTRI): " << TakesInternalBackHook(TheDawgArray, 1, string("ANESTRI")) << endl;
	cout << "InternalBackHook(CAPE): " <<  TakesInternalBackHook(TheDawgArray, 1, string("CAPE")) << endl;
	cout << "InternalBackHook(CAP):"  <<  TakesInternalBackHook(TheDawgArray, 1, string("CAP")) << endl;
	cout << "InternalBackHook(FUNDI): "<<  TakesInternalBackHook(TheDawgArray, 1, string("FUNDI")) << endl;
	cout << endl;
				
	cout << "InternalFrontHook(RETINAS): " <<  TakesInternalFrontHook(TheDawgArray, 1, string("RETINAS")) << endl;
	cout << "InternalFrontHook(ANESTRI): " << TakesInternalFrontHook(TheDawgArray, 1, string("ANESTRI")) << endl;
	cout << "InternalFrontHook(CAPE): " << TakesInternalFrontHook(TheDawgArray, 1, string("CAPE")) << endl;
	cout << "InternalFrontHook(CAP): " << TakesInternalFrontHook(TheDawgArray, 1, string("CAP")) << endl;
	cout << "InternalFrontHook(ESTOVERS): " << TakesInternalFrontHook(TheDawgArray, 1, string("ESTOVERS")) << endl;
	cout << endl;
	
	
	cout << "APE ";
	str.clear(); 
	BackHooks(TheDawgArray, 1, string("APE"), str);
	cout << str;
	cout << endl;
	cout << endl;

	str.clear();
	FrontHooks(TheDawgArray, 1, string("APE"), str);
	cout << str;
	cout << " APE"; 
	cout << endl;
	cout << endl;

	str.clear();
	FrontHooks(TheDawgArray, 1, string("APE"), str);
	cout << str;
	cout << (TakesInternalFrontHook(TheDawgArray, 1, string("APE")) ? "-" : "");
	cout << " APE ";
	cout << (TakesInternalBackHook(TheDawgArray, 1, string("APE")) ? "-" : "");
	str.clear();
	BackHooks(TheDawgArray, 1, string("APE"), str);
	cout << str;
	cout << endl;
	cout << endl;
	

	stringList.clear();
	cout << "Anagrams of RETINAS:" << endl;
	Anagram(TheDawgArray, 1, string("RETINAS"), stringList);
	copy(stringList.begin(), stringList.end(), ostream_iterator<string>(cout, "\n"));
	cout << endl;

	stringList.clear();				
	cout << "Anagrams of RETINA??:" << endl;
	Anagram(TheDawgArray, 1, string("RETINA??"), stringList);
	copy(stringList.begin(), stringList.end(), ostream_iterator<string>(cout, "\n"));
	cout << endl;


	stringList.clear();
	Pattern(TheDawgArray, 1, string("????F"), stringList);
	copy(stringList.begin(), stringList.end(), ostream_iterator<string>(cout, "\n"));
*/
				
	return 0;
}
