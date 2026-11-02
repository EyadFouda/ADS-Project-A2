
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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

        displayResults(results, count);
    }

    cout << "Program terminated.\n";

    return 0;
}
