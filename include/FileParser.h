#ifndef FILEPARSER_H
#define FILEPARSER_H
#include <string>
#include "Trie.h"
using namespace std;

bool normalizeWord(const string& raw, string& out); //converts the raw words to the form that the Trie accepts which is lowercase alphabet only

int loadDictionary(const string& filename, Trie& trie, int& skippedLines); //reads words and frequencies from the file and inserts them into the Trie

int rankedSearch(const Trie& trie, const string& prefix, Completion results[], int maxResults); // Finds up to maxResults ranked autocomplete suggestions.

#endif
