
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <conio.h>
#include "Trie.h"
#include "HashTable.h"
#include "CircularQueue.h"
#include "FileParser.h"
#include "Heap.h"

using namespace std;

// Checks whether a frequency contains a valid positive integer.
bool parseFrequency(const string& token, int& frequency) {
    if (token.empty()) {
        return false;
    }

    int value = 0;

    for (int i = 0; i < (int)token.length(); i++) {
        if (token[i] < '0' || token[i] > '9') {
            return false;
        }

        int digit = token[i] - '0';

        if (value > (2147483647 - digit) / 10) {
            return false;
        }

        value = value * 10 + digit;
    }

    if (value <= 0) {
        return false;
    }

    frequency = value;
    return true;
}

// Loads the dictionary into the Hash Table.
bool loadHashTable(const string& filename, HashTable& table) {
    ifstream file(filename);

    if (!file.is_open()) {
        return false;
    }

    string line;

    while (getline(file, line)) {
        istringstream stream(line);

        string rawWord;
        string frequencyToken;

        if (!(stream >> rawWord)) {
            continue;
        }

        int frequency = 1;

        if (stream >> frequencyToken) {
            if (!parseFrequency(frequencyToken, frequency)) {
                continue;
            }

            string extra;

            if (stream >> extra) {
                continue;
            }
        }

        string word;

        if (normalizeWord(rawWord, word)) {
            table.insert(word, frequency);
        }
    }

    file.close();
    return true;
}

// Displays autocomplete suggestions.
void displayResults(Completion results[], int count) {
    if (count == 0) {
        cout << "No suggestions found.\n";
        return;
    }

    for (int i = 0; i < count; i++) {
        cout << i + 1 << ". "
             << results[i].word << " ("
             << results[i].frequency << ")\n";
    }
}


void displayFuzzyResults(FuzzyMatch results[], int count) {
    if (count == 0) {
        cout << "No close spelling matches found.\n";
        return;
    }

    cout << "\nDid you mean:\n";

    for (int i = 0; i < count; i++) {
        cout << i + 1 << ". "
             << results[i].word
             << " (Edit distance: "
             << results[i].editDistance
             << ", Frequency: "
             << results[i].frequency << ")\n";
    }
}


// Displays the recent search history.
void displayHistory(CircularQueue& history) {
    if (history.isEmpty()) {
        cout << "No recent searches.\n";
        return;
    }

    for (int i = 0; i < history.getSize(); i++) {
        string word;
        history.getFromNewest(i, word);

        cout << i + 1 << ". " << word << "\n";
    }
}



void testLiveInput(const Trie& trie) {
    char prefix[100] = "";
    int length = 0;

    while (true) {
        // Clear the terminal and redraw the search.
        cout << "\033[2J\033[H";

        cout << "LIVE AUTOCOMPLETE\n";
        cout << "Type lowercase letters. Backspace deletes. Enter finishes.\n\n";

        cout << "Search: " << prefix << "\n\n";

        // Get suggestions from the Trie.
        if (length > 0) {
            Completion results[5];
            int count = rankedSearch(trie, prefix, results, 5);

            cout << "Suggestions:\n";
            displayResults(results, count);
        }

        int ch = _getch();

        // Enter finishes live autocomplete.
        if (ch == 13) {
            break;
        }

        // Ignore special keys.
        if (ch == 0 || ch == 224) {
            _getch();
            continue;
        }

        // Backspace deletes the last character.
        if (ch == 8 || ch == 127) {
            if (length > 0) {
                length--;
                prefix[length] = '\0';
            }
        }
        // Accept lowercase English letters.
        else if (ch >= 'a' && ch <= 'z' && length < 99) {
            prefix[length] = (char)ch;
            length++;
            prefix[length] = '\0';
        }
    }

   
cout << "\033[2J\033[H";

cout << "Search finished. You typed: " << prefix << "\n";

if (length > 0) {

    if (trie.search(string(prefix))) {
        cout << "The word exists in the dictionary.\n";
    }
    else {
        int maxCandidates = 100;
        int maxResults = 5;

        FuzzyMatch candidates[100];
        FuzzyMatch results[5];

        int maxAllowed = getMaxEditDistance(length);

        int candidateCount = trie.getFuzzyMatches(
            string(prefix),
            candidates,
            maxCandidates,
            maxAllowed
        );

        int resultCount = rankFuzzyMatches(
            candidates,
            candidateCount,
            results,
            maxResults
        );

        displayFuzzyResults(results, resultCount);
    }
}
}




int main() {
    Trie trie;
    HashTable table;
    CircularQueue history(5);

    string filename = "../data/dictionary.txt";
    int skipped = 0;

    int loaded = loadDictionary(filename, trie, skipped);

    if (loaded == -1) {
        cout << "Could not open dictionary file.\n";
        return 1;
    }

    if (!loadHashTable(filename, table)) {
        cout << "Could not load Hash Table.\n";
        return 1;
    }

    cout << "Dictionary loaded successfully.\n";
    cout << "Loaded entries: " << loaded << "\n";
    cout << "Skipped lines: " << skipped << "\n";
    testLiveInput(trie);

    cout << "\nCommands:\n";
    cout << "Enter a prefix to search.\n";
    cout << ":lookup word\n";
    cout << ":add word frequency\n";
    cout << ":remove word\n";
    cout << ":history\n";
    cout << ":quit\n";

    string input;

    while (true) {
        cout << "\n> ";
        getline(cin, input);

        if (input == ":quit") {
            break;
        }

        if (input.empty()) {
            continue;
        }

        if (input == ":history") {
            displayHistory(history);
            continue;
        }

        if (input.compare(0, 8, ":lookup ") == 0) {
            string rawWord = input.substr(8);
            string word;
            int frequency;

            if (normalizeWord(rawWord, word) &&
                table.search(word, frequency)) {
                cout << word << " has frequency "
                     << frequency << "\n";
            }
            else {
                cout << "Word not found.\n";
            }

            continue;
        }

        if (input.compare(0, 5, ":add ") == 0) {
            istringstream stream(input.substr(5));

            string rawWord;
            string frequencyToken;
            string extra;
            string word;
            int frequency;

            if (stream >> rawWord >> frequencyToken &&
                !(stream >> extra) &&
                normalizeWord(rawWord, word) &&
                parseFrequency(frequencyToken, frequency)) {

                trie.insert(word, frequency);
                table.insert(word, frequency);

                cout << "Word added successfully.\n";
            }
            else {
                cout << "Invalid input. Use: :add word frequency\n";
            }

            continue;
        }

        if (input.compare(0, 8, ":remove ") == 0) {
            string rawWord = input.substr(8);
            string word;

            if (normalizeWord(rawWord, word) &&
                trie.search(word)) {

                trie.remove(word);
                table.remove(word);

                cout << "Word removed successfully.\n";
            }
            else {
                cout << "Word not found.\n";
            }

            continue;
        }

        string prefix;

        if (!normalizeWord(input, prefix)) {
            cout << "Invalid prefix.\n";
            continue;
        }

        
history.enqueue(prefix);

Completion results[5];
int count = rankedSearch(trie, prefix, results, 5);

if (count > 0) {
    displayResults(results, count);
}
else {
    cout << "No autocomplete suggestions found.\n";

    FuzzyMatch candidates[100];
    FuzzyMatch fuzzyResults[5];

    int maxAllowed = getMaxEditDistance((int)prefix.length());

    int candidateCount = trie.getFuzzyMatches(
        prefix,
        candidates,
        100,
        maxAllowed
    );

    int resultCount = rankFuzzyMatches(
        candidates,
        candidateCount,
        fuzzyResults,
        5
    );

    displayFuzzyResults(fuzzyResults, resultCount);
}

    }

    cout << "Program terminated.\n";

    return 0;
}
